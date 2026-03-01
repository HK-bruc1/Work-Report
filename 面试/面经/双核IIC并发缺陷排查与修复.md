# 双核 SDK 下软件 IIC 总线并发冲突排查与修复

## 一、背景

### 产品需求

本项目为一款 TWS 智能耳机，基于杰理芯片开发，主要功能包括：

- **带头部追踪的空间音效**：通过耳机内置 IMU 传感器（ICM42670P）实时采集头部姿态数据，驱动空间音效算法动态调整声场方位，实现沉浸式音频体验
- **心率血氧监测**：通过 PPG 传感器芯片（HX3011）采集光学数据，实时计算心率与血氧饱和度

### 硬件连接

SDK 仅提供一套软件模拟 I2C 接口（Soft IIC 0），两颗传感器均挂载在同一组 GPIO 上：

```
SCL → IO_PORTC_06
SDA → IO_PORTC_07

├── ICM42670P（IMU，地址 0x68/0x69）
└── HX3011（PPG心率血氧，地址 0x44）
```

### 开发历程

项目最初在**单核 SDK**上完成功能开发与验证，两个功能同时运行未发现任何异常。

后期因产品升级，切换至**双核 SDK**，沿用原有业务代码，未作架构改动。

---

## 二、问题现象

双核 SDK 集成完成后，进行功能联调时出现以下问题：

- 单独开启空间音效（头部追踪）：**正常**
- 单独开启心率血氧监测：**正常**
- **两个功能同时开启：心率血氧初始化概率性失败，或初始化成功后数据异常（持续输出 0 或错误值），重启后偶发恢复**

问题复现率约 30%～60%，具有明显随机性，且在压力测试（连续开关功能）时几乎必现。

### 根本原因：IMU 持续活跃，锁对其毫无约束

理解下面两个失败场景的前提只有一条：**只要系统开启了头部追踪空间音效，IMU 驱动就在 Core 0 上持续运行，不断轮询 IIC 总线，没有任何停歇。** 而 IMU 驱动使用裸 IIC，从不获取任何锁。

HR 驱动虽然每次 IIC 事务都用 `spin_lock(&sensor_iic)` 自我保护，但这把锁对 IMU 没有任何约束力——IMU 根本不知道这把锁的存在，不会在操作 IIC 前去检查它。结果是：**无论 HR 在什么时候操作 IIC，只要 IMU 此刻也在操作 IIC（这随时都可能发生），总线就会冲突。**

### 失败场景一：初始化阶段——chip ID 读取失败，传感器无法识别

`hr_sensor_init()` 调用 `hx3011_chip_check()` 来校验芯片身份。函数内部的每次尝试分三步走：

```c
// hx3011.c:274
bool hx3011_chip_check(void)
{
    for (i = 0; i < 10; i++) {
        hx3011_write_reg(0x01, 0x00);   // ① IIC 写软复位（有锁，写完即释放锁）
        hx3011_delay(5);                // ② 等待芯片复位：5ms 空窗，锁已释放
        hx3011_chip_id = hx3011_read_reg(0x00);  // ③ IIC 读 chip ID（有锁）
        if (hx3011_chip_id == 0x27) {
            return true;
        }
    }
    return false;
}
```

步骤①结束后锁立刻释放，步骤②的 5ms 是完全开放的窗口。此时 Core 0 上的 IMU 轮询任务随时在执行裸 IIC，与步骤③抢占总线，导致 `hx3011_read_reg(0x00)` 读回错误值（非 0x27）。即使重试 10 次，只要 IMU 持续活跃，每次读取都有被干扰的概率，最终 `hx3011_chip_check()` 全部失败，上层打印：

```
hx3011_chip_check fail
>>>>hrSensor_Init ERROR
```

HR 传感器初始化失败，后续心率血氧功能完全不可用。

### 失败场景二：运行阶段——开启心率检测时概率性失败

HR 初始化成功后，用户触发心率检测时，调用链如下：

```c
// hrSensor_manage.c:172
hr_sensor_measure_hr_start()
  └── hr_sensor_io_ctl(HR_SENSOR_ENABLE, ...)
        └── hx3011_init(HRS_MODE)          // hx3011.c:1060
              └── hx3011_hrs_enable()      // 依次写入多个配置寄存器
```

`hx3011_hrs_enable()` 需要向 HX3011 连续写入多个寄存器（LED 电流、采样率、AGC 参数等）。每次写操作都通过 `hrsensor_write_nbyte()` 单独加锁，但**锁在每次单条 IIC 事务结束后立即释放**——HR 驱动的锁只保护"一条 IIC 事务"，不保护"整个配置序列"。

两条写命令之间锁已释放，Core 0 的 IMU（从不持锁）可以随时注入：

```
时间轴（两核同时运行）：

Core 1（HR 配置）:  [spinlock→写寄存器A→unlock]  空隙  [spinlock→写寄存器B→unlock] ...
                                                    ↑
Core 0（IMU 轮询）:                          裸 IIC 注入（无锁，毫无阻拦）
                                            → 总线冲突，HX3011 状态机复位
                                            → 寄存器 B 的写操作遭遇 NACK 或数据错位
```

`hx3011_hrs_enable()` 中途某条寄存器写失败，HX3011 进入错误配置状态。函数本身不做整体事务回滚，`hr_sensor_measure_hr_start()` 调用链对外"返回成功"，但传感器寄存器配置不完整，后续数据采集输出全为 0 或乱值。由于 IMU 轮询是持续的，冲突随时可能发生，表现为心率检测**有时正常、有时开启后立即输出异常**，带有明显的概率性。

---

## 三、排查过程

### 3.1 初步怀疑：硬件或驱动配置问题

首先确认硬件连接和驱动配置未变动，排查上拉电阻、供电时序等硬件因素，逐一确认无异常。

将两个功能的初始化顺序互换，问题依然随机出现，排除初始化时序依赖问题。

### 3.2 加打印日志定位失败位置

在心率驱动的 IIC 读写函数中加入错误日志：

```c
// hrSensor_manage.c
if (0 == iic_tx_byte(hrSensor_info->iic_hdl, w_chip_id)) {
    r_printf("\n hrSensor iic w err 0\n");  // 从机地址阶段无 ACK
    goto __wend;
}
```

复现问题后，串口输出：

```
hrSensor iic w err 0
hrSensor iic r err 0
```

**从机地址阶段即无 ACK 应答**，说明不是数据内容错误，而是 IIC 通信在最早的寻址阶段就已失败——从机根本没有响应。

正常情况下，从机对正确地址必然回 ACK，这种现象只有两种可能：
1. 从机芯片断电或复位
2. 总线波形已经损坏，从机无法识别有效的起始信号或地址帧

供电确认正常，怀疑方向转向**总线波形损坏**。

### 3.3 逻辑分析仪抓取波形

使用逻辑分析仪同时抓取 SCL 和 SDA 波形，在问题复现时截获异常帧：

```
正常帧：
SCL  ‾‾|_|‾|_|‾|_|‾|_|‾|_|‾|_|‾|_|‾|_|‾|__
SDA  ‾‾‾[0 1 0 0 0 1 0 0]‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
        ← 地址 0x44, W=0 ←    ACK↓

异常帧（复现时）：
SCL  ‾‾|_|‾|_|‾______|‾|_|‾|_|‾|_|‾|_|‾
SDA  ‾‾‾[0 1 0  ↑此处SDA被意外拉高 0 1 0 0]
                ← 波形在第3位被打断，出现异常电平跳变
```

SCL 保持高电平期间 SDA 出现了不该有的跳变——这在 I2C 协议中会被从机误判为 **START 或 STOP 信号**，导致从机状态机复位，后续字节全部错位，最终无法识别地址给出 ACK。

### 3.4 定位波形损坏的根因

波形在传输过程中被"插入"了额外的电平跳变，最可能的原因是：**另一段代码在同一时刻也在操作相同的 GPIO**。

重新审视系统架构：

- BR52 是**真正的双核处理器**（`CPU_CORE_NUM = 2`），两个核心可以同时执行不同任务
- 空间音效的 IMU 数据采集运行在 **Core 0**（音频处理核）
- 心率血氧监测运行在 **Core 1**（应用逻辑核）
- 两者各自独立轮询，互不知晓对方的存在

查看 IMU 驱动（`icm_42670p.c`）的 IIC 读写实现：

```c
// SDK/apps/common/device/sensor/imu_sensor/icm_42670p/icm_42670p.c
u32 icm42670p_I2C_Read_NBytes(unsigned char devAddr, ...)
{
    iic_start(icm42670p_info->iic_hdl);      // 直接操作 GPIO
    iic_tx_byte(icm42670p_info->iic_hdl, devAddr);
    // ...
    iic_stop(icm42670p_info->iic_hdl);
    // ← 无任何锁保护
}
```

再看心率驱动（`hrSensor_manage.c`）：

```c
// SDK/apps/common/device/sensor/hr_sensor/hrSensor_manage.c
u8 hrsensor_read_nbyte(...)
{
    spin_lock(&sensor_iic);                  // 有 spinlock 保护
    iic_start(hrSensor_info->iic_hdl);
    // ...
    iic_stop(hrSensor_info->iic_hdl);
    spin_unlock(&sensor_iic);
}
```

**问题根源清晰**：心率驱动用了 `spin_lock` 保护自己的 IIC 事务，但 IMU 驱动完全没有锁——IMU 在访问 IIC 时根本不检查 `sensor_iic` 这把锁，心率驱动的锁对 IMU 形同虚设。

两个核心在同一时刻各自执行 GPIO bit-bang，对同一对引脚（PORTC_06/07）交替写入，最终产生的波形是两段操作的混叠，从机完全无法解析。

### 3.5 确认单核下为何没有问题

单核 SDK 中同样存在这个代码结构缺陷（IMU 驱动无锁），但未出现问题，有两层原因叠加。

**原因一：软件 IIC 底层对每个字节有关中断保护。** 查看 `iic_soft.c`：

```c
// iic_soft.c
u8 soft_iic_tx_byte(soft_iic_dev iic, u8 byte)
{
    local_irq_disable();          // 关中断：屏蔽 OS tick，防止字节传输被打断
    IIC_SCL_L(...);
    for (u32 i = 0; i < 8; i++) {
        // ... 8 bit 移位输出 + ACK 检测 ...
    }
    local_irq_enable();           // 开中断：字节传输完毕
    return ack;
}

u8 soft_iic_rx_byte(soft_iic_dev iic, u8 ack)
{
    local_irq_disable();
    // ... 8 bit 接收 ...
    local_irq_enable();
    return byte;
}
```

每个字节的 8 bit 移位过程被 `local_irq_disable` 保护，单核下 OS tick 无法在单个字节传输期间触发任务切换，**单个字节的时序是完整的**。

**原因二：IIC 事务执行时间极短，字节间隙被命中的概率趋近于零。** 查看延时函数：

```c
static inline u32 iic_get_delay(soft_iic_dev iic)
{
    u32 delay_num = 0;
    return delay_num;  // 永远返回 0，配置的延时参数被忽略
}
```

所有 `soft_iic_delay(dly_t)` 实际上是 `soft_iic_delay(0)` = 零循环，无任何等待。`soft_iic_start`、`soft_iic_stop` 虽然没有 `local_irq_disable`，但整个过程只有 4 次 GPIO 寄存器写操作，在 CPU 高速执行下仅需纳秒级时间。整个 IIC 事务（START + 若干字节 + STOP）总耗时只有**几微秒**，而 RTOS 的 OS tick 周期通常为 1 ms。

值得一提的是，对比两套 SDK 发现：单核 SDK 的 `mpu6887p.c` 针对这一问题做了额外加固，将 `local_irq_disable` 的范围扩展到整个 IIC 事务（包括 START/STOP），彻底封住了字节间隙的任务切换窗口：

```c
// 单核 SDK mpu6887p.c
u16 mpu6887p_I2C_Read_NBytes(...)
{
    local_irq_disable();          // 关中断覆盖整个事务，含 START/STOP
    iic_start(...);
    iic_tx_byte(...);
    // ...
    iic_stop(...);
    local_irq_enable();
    return i;
}
```

而双核 SDK 的 `mpu6887p.c` 则去掉了这一保护——`local_irq_disable` 在多核环境下只能屏蔽**当前核心**的中断，另一个核心完全不受影响，单核的保护手段在双核场景下失效，因此被移除，理应由上层的跨核互斥机制来替代，但实际上并未补上。

综合来看，单核下任务切换依赖 OS tick 触发，在几微秒的事务窗口内命中的概率极低，实测长时间运行从未复现。

**双核下则完全不同**：两个核心天然同时执行，不需要等待任何调度时机，只要两个传感器的轮询任务同时处于活跃状态，冲突就是**必然发生**而非概率事件。

---

## 四、问题根因总结

```
根因：SDK 中 IMU 驱动与 HR 驱动共用同一套软件 IIC 总线，
      但 IMU 驱动（icm_42670p.c 等）未参与 IIC 总线级互斥，
      在双核并发执行时两个核心同时操作相同 GPIO，导致波形损坏。

单核不发作：① soft_iic_tx/rx_byte 内部有 local_irq_disable，单字节不会被打断；
            ② iic_get_delay() 返回 0，事务执行时间仅几μs，
               OS tick（~1ms）命中字节间隙的概率极低，实测从未复现。

双核必发作：local_irq_disable 只屏蔽当前核的中断，对另一核无效；
            两核天然同时执行，冲突为确定性事件，
            HR 驱动的 spinlock 对不持锁的 IMU 驱动无约束力。
```

---

## 五、修复方案

### 5.1 设计思路

需要在总线层面建立互斥，保证任意时刻只有一个核心在操作 SCL/SDA 引脚。

选择 **spinlock** 而非 mutex 的原因：

| | Spinlock | Mutex |
|---|---|---|
| 等待方式 | 忙等（空转轮询） | 阻塞（OS 挂起任务） |
| 适用场景 | 双核 + 临界区极短 | 临界区较长，或单核 RTOS |
| IIC 事务耗时 | 几十～几百 μs，空转代价可忽略 | 触发 OS 调度切换开销反而更大 |

IIC 单次事务时间极短，Core 1 空转等待几十微秒的代价远小于一次任务上下文切换，spinlock 是最轻量的正确选择。

spinlock 的原子性由硬件保证：

```c
// SDK/interface/driver/cpu/br52/asm/cpu.h
static inline void q32DSP_testset(u8 volatile *ptr)
{
    asm volatile(
        " 1:            \n\t "
        " testset b[%0] \n\t "   // 单时钟周期原子完成：读取并置位
        " ifeq goto 1b  \n\t "   // 已被占用则继续轮询
        : : "p"(ptr) : "memory"
    );
}
```

### 5.2 具体修改

**第一步：在 `hrSensor_manage.c` 中将 `sensor_iic` 改为全局可见**

```c
// hrSensor_manage.c
// 修改前：
spinlock_t sensor_iic;

// 修改后：
spinlock_t g_iic0_bus_lock;   // 重命名为总线级锁，语义更清晰
```

**第二步：IMU 驱动引入同一把锁**

在 `icm_42670p.c` 中引用并使用总线锁：

```c
// icm_42670p.c
extern spinlock_t g_iic0_bus_lock;

u32 icm42670p_I2C_Read_NBytes(unsigned char devAddr, ...)
{
    spin_lock(&g_iic0_bus_lock);      // 新增：获取总线锁
    u32 i = 0;
    iic_start(icm42670p_info->iic_hdl);
    if (0 == iic_tx_byte(icm42670p_info->iic_hdl, devAddr)) {
        goto __iic_exit_r;
    }
    // ... 其余不变 ...
__iic_exit_r:
    iic_stop(icm42670p_info->iic_hdl);
    spin_unlock(&g_iic0_bus_lock);    // 新增：释放总线锁
    return i;
}

u32 icm42670p_I2C_Write_NBytes(unsigned char devAddr, ...)
{
    spin_lock(&g_iic0_bus_lock);      // 新增
    u32 i = 0;
    iic_start(icm42670p_info->iic_hdl);
    // ...
    iic_stop(icm42670p_info->iic_hdl);
    spin_unlock(&g_iic0_bus_lock);    // 新增
    return i;
}
```

对 SDK 中其他所有无保护的 IMU 驱动做同样处理：`qmi8658c.c`、`lsm6dsl.c`、`sh3001.c`、`mpu9250.c`。

注意：`gSensor_manage.c` 中通过 `extern` 引用了同名的旧锁，重命名后需同步修改：

```c
// gSensor_manage.c（需同步修改）
#if TCFG_HRSENSOR_ENABLE
extern spinlock_t g_iic0_bus_lock;   // 原为 sensor_iic，随同重命名
extern u8 sensor_iic_init_status;
```

**第三步：锁的初始化统一在板级初始化中完成**

```c
// board_config.c 或 app_main.c 的初始化阶段
extern spinlock_t g_iic0_bus_lock;
spin_lock_init(&g_iic0_bus_lock);
```

### 5.3 修复效果

| 场景 | 修复前 | 修复后 |
|---|---|---|
| 单独运行空间音效 | 正常 | 正常 |
| 单独运行心率血氧 | 正常 | 正常 |
| 两者同时运行（单核） | 正常（概率侥幸） | 正常（有保护） |
| 两者同时运行（双核） | 概率性失败 30%～60% | **稳定正常，压力测试 0 失败** |

---

## 六、举一反三：SDK 中同类问题的完整清单

排查过程中发现，这一缺陷在 SDK 的 IMU 驱动中具有系统性：

| 驱动文件 | 对应芯片 | 修复前状态 |
|---|---|---|
| `icm_42670p.c` | ICM42670P | ❌ 无保护 |
| `qmi8658c.c` | QMI8658 | ❌ 无保护 |
| `lsm6dsl.c` | LSM6DSL | ❌ 无保护 |
| `sh3001.c` | SH3001 | ❌ 无保护 |
| `mpu9250.c` | MPU9250 | ❌ 无保护 |
| `mpu6887p.c` | MPU6887P | ⚠️ 仅 `local_irq_disable`（单核有效，双核无效） |
| `gSensor_manage.c` | MPU6050 等旧型号 | ✅ 已有 `spin_lock`，且与 HR 共享同一把锁 |
| `hrSensor_manage.c` | HX3011/HX3918 | ✅ 已有 `spin_lock` |

旧的 `gSensor` 路径（MPU6050）在设计时已考虑了与 HR 驱动的共存关系（代码中有 `extern spinlock_t sensor_iic` 显式共享锁），但后续新增的 IMU 驱动各自独立开发，遗漏了总线级互斥这一环节。

---

## 七、经验总结

1. **单核验证通过不等于双核安全**：单核下由于任务分时执行，极短的临界区（几μs 的 IIC 事务）在概率上几乎不会被 OS tick 命中，掩盖了并发问题。切换到真正双核并行时，所有侥幸都会变成必然。

2. **共享总线的锁必须对所有访问者生效**：只给其中一个驱动加锁是无效的，持锁方保护了自己，但不持锁的另一方照样会破坏总线。锁的保护范围必须覆盖同一物理资源的**所有使用路径**。

3. **Spinlock 适合双核 + 短临界区的场景**：IIC 事务时间极短（几十～几百 μs），空转等待的代价可以忽略，使用 spinlock 比触发 OS 调度的 mutex 开销更小，也是驱动层最常见的正确选型。
