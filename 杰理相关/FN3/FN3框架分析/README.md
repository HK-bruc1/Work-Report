基于V138公版软件的鼎合丰内部版本V1

# 替换提示音

为了方便知道耳机的状态，把所有的音效改成中文。

## 替换

打开配置工具入口，点击配置工具：

![image-20250425091528271](./README.assets/image-20250425091528271.png)

1. 收集对应的中文MP3格式的音效素材
2. 把原有音效删除，新音效改为同名MP3
3. 然后点击打开选中即可
4. 格式跟原来一样，点击保存提示音文件
5. 再点击保存到bin即可编译生效。

# 打印

打开打印总开关：`apps\earphone\include\app_config.h`

```c
/*
 * 系统打印总开关
 */

#define LIB_DEBUG    0  //打开打印
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

 #define CONFIG_DEBUG_ENABLE

#ifndef CONFIG_DEBUG_ENABLE
#define CONFIG_DEBUG_LITE_ENABLE  //轻量级打印开关, 默认关闭
#endif
```

## 使用

```c
printf("=== Log Test Start ===\n");
    
    // 测试普通打印
    printf("Normal printf working\n");
    
    // 测试日志级别打印
    log_info("Info log test\n");
    log_debug("Debug log test\n");
    log_error("Error log test\n");
    
    printf("=== Log Test End ===\n");
```

**普通printf输出**：

```c
[00:00:00.321]=== Log Test Start ===
[00:00:00.322]Normal printf working
```

- 格式：`[时间戳]内容`
- 特点：简单直接，只显示时间和内容

**log_info输出**：

```c
[00:00:00.322][Info]: [APP]Info log test
```

- 格式：`[时间戳][Info]: [APP]内容`
- 特点：包含了日志级别(Info)和模块标签(APP)

**log_error输出**：

```c
[00:00:00.323](error): <Error>: [APP]Error log test
```

- 格式：`[时间戳](error): <Error>: [APP]内容`
- 特点：错误日志有特殊标记，更容易识别

**时间戳格式**：

- 格式：`[HH:MM:SS.mmm]`
- 例如：`[00:00:00.321]`
- 精确到毫秒级别

## 日志输出

对于日志输出，需要以下配置：

**必需的宏定义**（这些要在包含头文件之前定义）：

```c
// 定义模块名称
#define LOG_TAG_CONST       APP    // 模块名称（大写）
#define LOG_TAG             "[APP]" // 模块标签（方括号格式）

// 启用需要的日志级别
#define LOG_ERROR_ENABLE    // 启用错误日志
#define LOG_DEBUG_ENABLE    // 启用调试日志
#define LOG_INFO_ENABLE     // 启用信息日志
//#define LOG_DUMP_ENABLE   // 启用数据打印（可选）
#define LOG_CLI_ENABLE      // 启用命令行（可选）
```

**必需的头文件**：

```c
#include "debug.h"          // 包含调试相关定义
```

**完整示例**：

```c
#include "app_config.h"     // 首先包含应用配置

// 日志配置（必须在debug.h之前）
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE

// 包含日志头文件
#include "debug.h"

// 使用示例
void test_function(void)
{
    log_info("Info message\n");
    log_debug("Debug message\n");
    log_error("Error message\n");
}
```

记住：

- 宏定义必须在包含 debug.h 之前

- app_config.h 中确保 LIB_DEBUG = 0

- 只有定义了对应的 ENABLE 宏，对应级别的日志才会输出

这样配置后就可以使用各种日志函数了：

- log_info() - 信息日志

- log_debug() - 调试日志

- log_error() - 错误日志

## 彩色打印(不会)

### 各种应用模式的选择

**也就是说耳机一启动进入什么模式应用，已经在代码中写死了。如果SDK应用于耳机就进入耳机模式应用，状态机一直执行耳机模式应用中的任务？**

**启动模式的选择确实在代码中预设了优先级顺序:**

```c
if (get_charge_online_flag()) {
    // 充电模式 -> idle应用
} else {
    #if  TCFG_ENTER_PC_MODE
        // PC模式
    #elif TCFG_ENTER_HEARING_AID_MODE
        // 助听器模式
    #elif TCFG_ADC_IIS_ENABLE
        // Linein模式
    #else
        // 默认耳机模式
    #endif
}
```

**这种设计是基于产品形态的:**

- 如果SDK用于耳机产品,就默认进入耳机模式应用
- 如果用于助听器产品,就定义`TCFG_ENTER_HEARING_AID_MODE`进入助听器模式
- 如果用于音频盒子,就定义`TCFG_ADC_IIS_ENABLE`进入Linein模式

一旦进入某个模式应用(比如耳机模式),状态机就会:

- 初始化该模式需要的模块(比如蓝牙、按键等)
- 持续处理该模式下的各种事件(按键事件、蓝牙事件等)
- 维护该模式下的各种状态(连接状态、播放状态等)

在` apps\earphone\board\br36\board_config.h`中可以看到板级配置的选择:

```c
#define CONFIG_BOARD_AC700N_DEMO
// #define CONFIG_BOARD_AC7006F_EARPHONE
// #define CONFIG_BOARD_AC700N_SD_PC_DEMO
// #define CONFIG_BOARD_AC700N_HEARING_AID
// #define CONFIG_BOARD_AC700N_IIS_LINEIN
```

- 板级配置文件定义的是与硬件设定相关的配置（**比如串口的具体IO口等**），后续会根据板级配置文件做对应模块的初始化与应用。

所以这个SDK的设计思路是:

- 通过宏定义来确定产品形态
- 根据产品形态决定启动时进入哪个应用模式
- 进入模式后就专注于处理该模式下的所有功能

这种设计方式的优点是:

- 代码结构清晰,每种产品形态的功能相对独立
- 通过宏定义方便切换不同产品形态
- 状态机模式便于管理复杂的应用逻辑

缺点是:

- 启动模式相对固定,运行时切换模式需要额外开发
- 不同模式之间的代码复用需要特别注意

### SDK模式应用的选择与切换机制

#### 模式应用的选择

在`app_main.c`中，模式应用的选择是通过条件编译宏和运行时状态判断来确定的：

```c
if (get_charge_online_flag()) {
    // 充电状态 -> 进入idle应用
    init_intent(&it);
    it.name = "idle";
    it.action = ACTION_IDLE_MAIN;
    start_app(&it);
} else {
    // 非充电状态下，根据宏定义选择应用模式
    #if TCFG_ENTER_PC_MODE
        // PC模式
        init_intent(&it);
        it.name = "pc";
        it.action = ACTION_PC_MAIN;
        start_app(&it);
    #elif TCFG_ENTER_HEARING_AID_MODE
        // 助听器模式
        init_intent(&it);
        it.name = "hearing_aid";
        it.action = ACTION_HEARING_AID_MAIN;
        start_app(&it);
    #elif TCFG_ADC_IIS_ENABLE
        // Linein模式
        init_intent(&it);
        it.name = "linein";
        it.action = ACTION_LINEIN_MAIN;
        start_app(&it);
    #else
        // 默认耳机模式
        init_intent(&it);
        it.name = "earphone";
        it.action = ACTION_EARPHONE_MAIN;
        start_app(&it);
    #endif
}
```

#### 模式应用的固定性

硬编码决定初始模式：

- 模式应用是通过宏定义选择的，这些宏在编译时就已确定
- 程序运行后，这些宏定义的值是固定的，无法更改

运行时不会切换模式：

- 一旦选定了一个模式应用（如耳机模式），程序就会一直在这个模式下运行
- 不会自动切换到另一种模式应用（如PC模式或助听器模式）

**唯一例外是充电状态：**

- **只有在检测到充电状态变化时，才可能切换模式应用**
- **例如，从非充电状态进入充电状态时，会切换到idle应用**

#### 模式应用的设计意图

这种设计有其合理性：

1. 专注性：
   - 每种模式应用专注于特定的功能场景
   - 耳机模式专注于蓝牙音频和通话
   - 助听器模式专注于音频放大和处理
   - PC模式专注于USB连接和数据传输
2. 资源优化：
   - 通过宏定义选择模式，可以在编译时优化代码大小
   - 不需要的模式代码不会被编译进固件
3. 简化逻辑：
   - 避免了复杂的模式切换逻辑
   - 减少了潜在的状态管理问题

####  实际应用场景

在实际产品中，通常会根据产品定位选择一种主要模式：

- 蓝牙耳机产品 → 选择耳机模式
- 助听器产品 → 选择助听器模式
- 多功能产品 → 可能需要自定义模式切换逻辑

如果确实需要在运行时切换模式，开发者需要修改框架代码，实现自定义的模式切换机制，而不是依赖默认的应用选择逻辑。

总结来说，您的理解非常准确：SDK的设计是基于编译时选择一个固定的模式应用，运行后不会自动切换到其他模式应用（除非充电状态变化）。这种设计简化了代码逻辑，但也限制了运行时的灵活性。

### 总结

应用模式的这一些宏定义在代码编译时就会被预处理器处理,生成确定的代码路径。

烧录到耳机后:

- 代码已经是编译后的固件
- 宏定义的条件判断已经不存在
- 只保留了被选中的那个模式的代码
- 其他模式的代码在编译时就被剔除了

如果要改变模式,必须:

- 修改源代码中的宏定义
- 重新编译代码
- 重新烧录固件

如果确实需要运行时切换模式,需要:

- 在编译时保留多个模式的代码
- 设计模式切换的机制(比如通过按键组合)
- 实现模式切换时的资源释放和重新初始化
- 这需要额外的开发工作,并且会增加固件大小

在` board_config.h`中的配置:

```c
#define CONFIG_BOARD_AC700N_DEMO
// #define CONFIG_BOARD_AC7006F_EARPHONE
// #define CONFIG_BOARD_AC700N_SD_PC_DEMO
```

这些配置一旦确定并编译烧录,就不能在运行时改变了。

所以:

- 这种基于宏定义的设计主要是面向不同产品形态的开发
- 不是为了实现运行时的模式切换
- 如果产品需要运行时切换模式,需要在开发时就考虑进去并专门设计实现

# 耳机模式应用介绍

耳机模式应用是SDK中的一个核心应用，它负责处理蓝牙耳机的主要功能。以下是耳机模式应用的主要处理逻辑：

## 应用注册与初始化

耳机模式应用通过`REGISTER_APPLICATION`宏在系统中注册：

```c
REGISTER_APPLICATION(app_earphone) = {
    .name   = "earphone",
    .action = ACTION_EARPHONE_MAIN,
    .ops    = &app_earphone_ops,
    .state  = APP_STA_DESTROY,
};
```

这个注册过程定义了：

- 应用名称："earphone"
- 应用的主要动作：ACTION_EARPHONE_MAIN
- 应用的操作函数集：app_earphone_ops
- 初始状态：APP_STA_DESTROY（未创建状态）

## 应用状态机

耳机模式应用的核心是一个状态机，通过`state_machine`函数实现:

```c
static int state_machine(struct application *app, enum app_state state, struct intent *it)
```

这个状态机处理以下几个状态：

**APP_STA_CREATE（创建状态）**

- 播放开机提示音
- 初始化基本设置

**APP_STA_START（启动状态）**

- 设置系统时钟为蓝牙正常工作频率
- 初始化蓝牙功能选择
- 注册蓝牙处理函数
- 初始化耳机状态
- 初始化蓝牙协议栈
- 启用按键消息
- 启用自动关机功能
- 初始化蓝牙低功耗特性
- 根据配置启用自动进入sniff模式

**APP_STA_PAUSE（暂停状态）**

- 将蓝牙切换到后台运行
- 保存当前音量设置

**APP_STA_RESUME（恢复状态）**

- 恢复前台运行
- 禁用自动关机

**APP_STA_STOP（停止状态）**

- 停止当前操作

 **APP_STA_DESTROY（销毁状态）**

- 退出蓝牙应用
- 关闭扫描和连接功能
- 取消页面和连接操作
- 关闭蓝牙电源

## 事件处理

耳机模式应用通过`event_handler`函数处理各种系统事件：

```c
static int event_handler(struct application *app, struct sys_event *event)
```

处理的主要事件类型包括：

**按键事件（SYS_KEY_EVENT）**

- 处理用户按键输入
- 调用`app_earphone_key_event_handler`处理具体按键操作

 **蓝牙事件（SYS_BT_EVENT）**

- 处理蓝牙连接状态变化
- 处理HCI事件
- 处理TWS（真无线立体声）事件

 **设备事件（SYS_DEVICE_EVENT）**

- 处理充电事件
- 处理配置工具事件
- 处理电源事件
- 处理充电盒事件
- 处理入耳检测事件
- 处理ANC（主动降噪）盒事件

## 蓝牙连接状态处理

通过`bt_connction_status_event_handler`函数处理蓝牙连接状态变化：

```c
static int bt_connction_status_event_handler(struct bt_event *bt)
```

处理的主要状态包括：

**蓝牙初始化完成（BT_STATUS_INIT_OK）**

- 初始化耳机自定义按键
- 设置SBC编解码器参数
- 初始化BLE（低功耗蓝牙）
- 更新UI状态
- 启动设备连接

**蓝牙连接成功（BT_STATUS_CONNECTED）**

- 禁用自动关机
- 更新UI状态
- 播放连接成功提示音
- 处理TWS连接状态

**蓝牙断开连接（BT_STATUS_DISCONNECT）**

- 播放断开连接提示音
- 更新UI状态

**电话状态处理**

- 来电处理（BT_STATUS_PHONE_INCOME）
- 拨出电话处理（BT_STATUS_PHONE_OUT）
- 通话中处理（BT_STATUS_PHONE_ACTIVE）
- 挂断电话处理（BT_STATUS_PHONE_HANGUP）
- 电话号码处理（BT_STATUS_PHONE_NUMBER）

**音乐播放状态处理**

- 开始播放（BT_STATUS_A2DP_MEDIA_START）
- 停止播放（BT_STATUS_A2DP_MEDIA_STOP）

## 低功耗管理

耳机模式应用实现了多种低功耗管理功能：

**Sniff模式管理**

- 通过`bt_check_enter_sniff`和`bt_check_exit_sniff`函数管理进入和退出sniff模式
- 设置sniff参数，如间隔时间、尝试次数等

**自动关机管理**

- 通过`sys_auto_shut_down_enable`和`sys_auto_shut_down_disable`函数管理自动关机功能
- 在特定条件下（如连接状态、通话状态）禁用自动关机

## 音频处理

耳机模式应用处理多种音频相关功能：

**A2DP音频解码**

- 通过`a2dp_dec_open`和`a2dp_dec_close`函数管理A2DP音频解码
- 处理音乐播放和停止

**通话音频处理**

- 处理SCO音频连接和断开
- 管理通话音量同步

**提示音播放**

- 播放开机、连接、断开等提示音
- 播放电话号码提示音

## 总结

耳机模式应用是一个**基于状态机的复杂应用**，它通过处理各种系统事件和蓝牙事件来管理耳机的工作状态。主要功能包括：

1. 蓝牙连接管理
2. 音乐播放控制
3. 通话处理
4. 低功耗管理
5. 提示音播放
6. TWS（真无线立体声）功能
7. 用户按键处理

当SDK启动后，如果没有定义其他模式应用宏，且不处于充电状态，系统会默认进入耳机模式应用。**耳机模式应用会初始化蓝牙协议栈，注册各种事件处理函数，然后进入正常工作状态，等待用户操作和蓝牙事件。**

整个应用的流程是：

1. 系统启动 → app_main.c中的app_main()函数
2. 检测是否充电，如果不充电则启动耳机模式应用
3. 通过`start_app()`函数启动耳机模式应用
4. 执行`earphone.c`中的`state_machine()`函数，进入`APP_STA_CREATE`和`APP_STA_START`状态
5. 初始化蓝牙和各种功能
6. 进入事件循环，处理各种系统事件和蓝牙事件

这种基于状态机的设计使得应用逻辑清晰，易于维护和扩展。

# SDK的状态机运行机制

**当SDK中的app_main中启动耳机模式应用后，程序是不是就一直在耳机模式应用的状态机中轮询各状态以及处理各事件？所有的事件和状态处理都围绕状态机处理函数进行？**

**STM32的智能锁项目，状态机也不是一直轮询各状态的，他是进入某个状态后，在状态任务中处理后，退出时会把状态重新设置。状态机会再扫描一次各状态，从而又进入某种状态任务中执行。反复重复此步骤，程序一直在其中一个状态中执行，状态会切换。**

您的理解基本正确，但需要稍微澄清一些细节。让我详细解释SDK中耳机模式应用的运行机制：

## 状态机与事件处理的关系

**状态机不是轮询的**

- 状态机函数`state_machine()`不是一直在轮询的，**而是在特定时刻被调用**
- 它主要负责**处理应用的生命周期状态转换**（创建、启动、暂停、恢复、停止、销毁）
- **每次状态变化时，框架会调用一次状态机函数**

事件驱动机制

- SDK采用的是事件驱动模型，而不是轮询模型
  - 之前STM32智能锁的状态机就是轮询模型。
  - 只不过状态没有改变时，外部的while(1)总是会进入同一个状态中去处理任务而已。
  - 事件驱动是进入一个状态后，会一直在这里执行，直到状态改变，才会切到另一个状态去执行。
- 系统有一个主事件循环，在`start_app()`之后，程序会进入这个事件循环
  - `start_app()`之后相当于选择一个模式应用了，程序会进入这个事件循环
- 当有事件发生时（如按键按下、蓝牙连接等），事件会被放入队列
- 主循环从队列取出事件，然后调用当前应用的`event_handler()`函数处理

## 实际运行流程

1. 启动流程
   - `app_main.c`中调用`start_app(&it)`启动耳机模式应用
   - 框架找到注册的"earphone"应用
   - 调用`state_machine()`函数，传入`APP_STA_CREATE`状态
     - 状态被改变，状态机会被再次调用。
   - 然后再次调用`state_machine()`函数，传入`APP_STA_START`状态
   - 此时耳机模式应用完成初始化，进入正常运行状态
2. 正常运行阶段
   - 系统进入主事件循环，等待事件发生
   - 当有事件发生时，调用对应模式应用的`event_handler()`函数处理
   - `event_handler()`根据事件类型分发到不同的处理函数：
     - 按键事件 → `app_earphone_key_event_handler()`
     - 蓝牙事件 → `bt_connction_status_event_handler()`或`bt_hci_event_handler()`
     - 设备事件 → 各种设备特定的处理函数
3. 状态切换
   - 当需要切换应用状态时（如切换到其他应用、进入后台等）
   - 框架会再次调用`state_machine()`函数，传入相应的状态
   - 例如，切换到其他应用时，会依次调用`APP_STA_PAUSE`和`APP_STA_DESTROY`

## 关键点说明

1. 不是轮询而是事件驱动
   - 状态机不是一直在运行的，只在状态转换时被调用
   - 日常运行中主要是事件处理函数在工作
2. 状态机与事件处理器的分工
   - 状态机：处理应用生命周期的状态转换
   - 事件处理器：处理运行过程中的各种事件
3. 系统架构
   - 底层有一个RTOS（实时操作系统）
   - RTOS提供任务调度、事件队列等机制
   - 应用框架基于RTOS构建，提供状态机和事件处理机制
4. 多任务协作
   - 除了主事件循环外，系统还有其他任务在运行
   - 例如蓝牙协议栈任务、音频处理任务等
   - 这些任务通过事件机制与主应用任务协作

所以，总结来说：程序不是一直在状态机中轮询，而是在事件驱动模式下运行。状态机只负责处理应用状态转换，而日常的事件处理是由事件处理器完成的。整个系统是多任务协作的，通过事件机制进行通信。

# SDK中的主事件循环实现

在SDK中，主事件循环的实现分布在多个组件中，主要涉及以下几个关键部分：

## 任务队列和消息处理机制

SDK使用RTOS（实时操作系统）提供的任务队列机制来实现事件循环。主要的API包括：

### 消息获取函数

```c
// 在 include_lib\system\app_msg.h 中定义
void app_task_get_msg(int *msg, int msg_size, int block);
```

这个函数用于从任务队列中获取消息，是事件循环的核心部分。各个应用模块在其主循环中调用此函数等待事件。

### 消息发送函数

```c
// 在 include_lib\system\app_msg.h 中定义
int app_task_put_usr_msg(int msg, int arg_num, ...);
int app_task_put_key_msg(int msg, int value);
```

这些函数用于向任务队列发送消息，触发事件处理。

### 底层RTOS API

```c
// 在 apps\middleware\basic\dhf_os.c 中使用
INT32 DHFReceiveMsg(const HCSHANDLE hMsgQueue, void* pvMsg, int nMaxMsgBytes, unsigned int dwTimeout)
{
    INT32 nRet = 0;
    nRet = os_taskq_pend((const char*)hMsgQueue, (int*)pvMsg, nMaxMsgBytes);
    if (nRet!=OS_TASKQ)
    {
        return -1;
    }
    return 0;
}

INT32 DHFSendMsg(const HCSHANDLE hMsgQueue, void* pvMsg, int nMsgBytes, unsigned int dwTimeout)
{
    return os_taskq_post_msg((const char*)hMsgQueue, 2, nMsgBytes, pvMsg);
}
```

这些函数是对底层RTOS消息队列API的封装

## 各应用模块的主循环实现

各个应用模块都有自己的主循环，它们都遵循类似的模式：

### 无线麦克风应用的主循环

```c
// 在 apps\earphone\wireless_mic\process\adapter_process.c 中
int adapter_process_run(struct adapter_pro *pro)
{
    if (pro == NULL) {
        return false;
    }

    int msg[32];
    int ret = 0;
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        if (adapter_process_event_parse(pro, (struct sys_event *)&msg[1]) == 0) {
            if (pro->event_handle) {
                ret = pro->event_handle((struct sys_event *)(&msg[1]));
            }
        }
        if (ret) {
            break;
        }
    }
    return ret;
}
```

### 关键词识别(KWS)任务的主循环

```c
// 在 apps\common\jl_kws\jl_kws_main.c 中
static void kws_task(void *p)
{
    int msg[8];
    int ret;
    
    __this->kws_state = KWS_STATE_INIT;
    __this->kws_task_state = KWS_TASK_STATE_INIT;

    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret == OS_TASKQ) {
            switch (msg[1]) {
            case KWS_SPEECH_RECOGNITION_RUN:
                kws_speech_recognition_run();
                break;
            case KWS_SPEECH_RECOGNITION_STOP:
                kws_speech_recognition_stop();
                break;
            case KWS_SPEECH_RECOGNITION_CLOSE:
                kws_speech_recognition_close();
                os_sem_post((OS_SEM *)msg[2]);
                break;
            default:
                break;
            }
        }
    }
}
```

### 设备管理器的主循环

```c
// 在 apps\common\dev_manager\dev_manager.c 中
static void dev_manager_task(void *p)
{
    int msg[32];
    int res;
    
    // 初始化代码...
    
    while (1) {
        res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
            case OS_TASKQ:
                switch (msg[0]) {
                    case Q_EVENT:
                        break;
                    case Q_MSG:
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}
```

## 事件通知机制

SDK使用`sys_event_notify`函数将各种系统事件（如按键、蓝牙、设备等）通知给应用程序：

```c
// 在 include_lib\system\event.h 中声明
void sys_event_notify(struct sys_event *e);
```

例如，在无线麦克风应用中：

```c
// 在 apps\earphone\wireless_mic\process\adapter_process.c 中
void adapter_process_event_notify(u8 event, int value)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg = (void *)DEVICE_EVENT_FROM_ADAPTER;
    e.u.dev.event = event;
    e.u.dev.value = value;

    sys_event_notify(&e);
}
```

## 应用框架与事件处理

应用框架通过 `start_app`函数启动应用，并设置事件处理函数：

```c
// 在 include_lib\system\app_core.h 中声明
int start_app(struct intent *it);
```

每个应用都有一个事件处理函数，用于处理系统事件：

```c
// 在 apps\earphone\earphone.c 中
static int event_handler(struct application *app, struct sys_event *event)
{
    // 事件处理代码...
    switch (event->type) {
    case SYS_KEY_EVENT:
        // 处理按键事件
        break;
    case SYS_BT_EVENT:
        // 处理蓝牙事件
        break;
    case SYS_DEVICE_EVENT:
        // 处理设备事件
        break;
    default:
        break;
    }
    // ...
}
```

## 主事件循环的工作流程

综合以上信息，SDK中的主事件循环工作流程如下：

1. 系统初始化后，通过`app_main()`函数选择并启动一个应用模式（如耳机模式）
2. `start_app()`函数启动应用，设置应用的状态机和事件处理函数
3. 应用的主任务进入一个无限循环，通过`app_task_get_msg()`或`os_taskq_pend()`等待事件
4. 当系统中发生事件（如按键按下、蓝牙连接等）时，相关模块调用`sys_event_notify()`通知系统
5. 系统将事件放入任务队列
6. 应用的主循环从队列中获取事件，并调用应用的事件处理函数处理
7. 事件处理函数根据事件类型和内容执行相应的操作

这种基于事件驱动的架构使得系统能够高效地响应各种事件，同时保持低功耗特性。

## 总结

SDK中的主事件循环是基于RTOS的任务队列和事件通知机制实现的。每个应用模块都有自己的主循环，通过统一的API从任务队列获取事件并处理。系统通过事件通知机制将各种事件（如按键、蓝牙、设备等）发送到任务队列，由应用的事件处理函数进行处理。

这种设计使得系统能够灵活地响应各种事件，同时保持模块化和可扩展性。不同的应用模式（如耳机模式、PC模式等）可以共享相同的事件处理框架，但实现不同的处理逻辑。

# 耳机模式应用的启动流程

## 启动流程

### 系统初始化与应用选择 (apps\earphone\app_main.c)

```c
void app_main()
{
    // 系统初始化...
    
    if (get_charge_online_flag()) {
        // 充电状态 -> 启动idle应用
        init_intent(&it);
        it.name = "idle";
        it.action = ACTION_IDLE_MAIN;
        start_app(&it);
    } else {
        // 非充电状态，根据宏定义选择应用模式
        #if TCFG_ENTER_PC_MODE
            // PC模式...
        #elif TCFG_ENTER_HEARING_AID_MODE
            // 助听器模式...
        #elif TCFG_ADC_IIS_ENABLE
            // Linein模式...
        #else
            // 默认耳机模式
            printf("启动耳机模式应用\n");
            init_intent(&it);
            it.name = "earphone";
            it.action = ACTION_EARPHONE_MAIN;
            start_app(&it);
        #endif
    }
}
```

### 应用启动 (系统库中的app_core.c，未在代码中直接看到)

`start_app()`函数会：

1. 查找注册的"earphone"应用

2. 调用应用的状态机函数（**共用的状态机函数**），传入APP_STA_CREATE状态

   1. ```c
          case APP_STA_CREATE:
              /* set_adjust_conn_dac_check(0); */
      #if TCFG_APP_LINEIN_EN
              extern u8 poweron_tone_play_flag;
              if (poweron_tone_play_flag == 0) {
                  poweron_tone_play_flag = 1;
                  if (app_var.play_poweron_tone) {
                      extern void poweron_tone_play_callback(void *priv);
                      tone_play_index_with_callback(IDEX_TONE_POWER_ON, 1, poweron_tone_play_callback, NULL);
                  }
              } else {
                  STATUS *p_tone = get_tone_config();
                  tone_play_index(p_tone->bt_init_ok, 1);
              }
      #endif
      #if TCFG_SD0_ENABLE || TCFG_PC_ENABLE
              STATUS *p_tone = get_tone_config();
              tone_play_index(p_tone->bt_init_ok, 1);
      #elif !TCFG_APP_LINEIN_EN
              //if (app_var.play_poweron_tone)
      		if(1)
      		 {
                  STATUS *p_tone = get_tone_config();
                  tone_play_index(p_tone->power_on, 1);
              } else {
      #ifdef CONFIG_CURRENT_SOUND_DEAL_ENABLE
                  dac_analog_power_control(0);
      #endif
              }
      #endif
              break;
      ```

3. 再次调用状态机函数，传入APP_STA_START状态

   1. 根据不同应用调用对应的主事件循环逻辑

   2. ```c
      case APP_STA_START:
              //进入蓝牙模式,UI退出充电状态
              ui_update_status(STATUS_EXIT_LOWPOWER);
              if (!it) {
                  break;
              }
              switch (it->action) {
              case ACTION_EARPHONE_MAIN:
                  /*
                   * earphone 模式初始化
                   */
      
                  /*set_mode_normal 0:user normal parm  1:user bqb parm*/
                  /* set_mode_dut 0:user normal parm  1:user bqb parm */
                  /* 库里模式设置 set set_mode_normal=0,set_mode_dut=0 */
      
                  extern void set_tx_mode(u8 set_mode_normal, u8 set_mode_dut);
                  /* set_tx_mode(0,1);//量产程序set */
      
                  clk_set("sys", BT_NORMAL_HZ);
                  u32 sys_clk =  clk_get("sys");
                  bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);
      
                  bt_function_select_init();
                  bredr_handle_register();
                  EARPHONE_STATE_INIT();
                  DHFAppCommand_init();//samson added by 20250329
                  btstack_init();
      
                  set_bt_version(BLUETOOTH_CORE_SPEC_54);
      #if TCFG_USER_TWS_ENABLE
                  tws_profile_init();
                  sys_key_event_filter_disable();
      #endif
                  if (BT_MODE_IS(BT_FCC)) { //FCC进行频偏校准
                      bt_ble_fre_offset_write_cbk(fre_offset_write_handle, fre);
      
                      bt_ble_fre_offset_read_cbk(fre_offset_read_handle);
      
                  }
                  /* 按键消息使能 */
                  sys_key_event_enable();
                  sys_auto_shut_down_enable();
                  bt_sniff_feature_init();
                  sys_auto_sniff_controle(MY_SNIFF_EN, NULL);
                  app_var.dev_volume = -1;
                  break;
              case ACTION_A2DP_START:
      #if (BT_SUPPORT_MUSIC_VOL_SYNC && CONFIG_BT_BACKGROUND_ENABLE)
                  app_audio_set_volume(APP_AUDIO_STATE_MUSIC, app_var.bt_volume, 1);
                  r_printf("BT_BACKGROUND RESET ACTION_A2DP_START STATE_MUSIC bt_volume=%d\n", app_var.bt_volume);
      #endif
                  a2dp_dec_open(0);
                  break;
              case ACTION_BY_KEY_MODE:
                  user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                  break;
              case ACTION_TONE_PLAY:
                  STATUS *p_tone = get_tone_config();
                  tone_play_index(p_tone->bt_init_ok, 1);
                  break;
              case ACTION_DO_NOTHING:
                  break;
              }
              break;
      ```

### 耳机模式应用状态机 (apps\earphone\earphone.c)

```c
static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int error = 0;

    r_printf("bt_state_machine=%d\n", state);
    switch (state) {
    case APP_STA_CREATE:
        // 创建应用，播放开机提示音等
        break;
        
    case APP_STA_START:
        // 启动应用
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_EARPHONE_MAIN:
            // 耳机模式初始化
            clk_set("sys", BT_NORMAL_HZ);
            u32 sys_clk = clk_get("sys");
            bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

            bt_function_select_init();
            bredr_handle_register();
            EARPHONE_STATE_INIT();
            DHFAppCommand_init();
            btstack_init();

            set_bt_version(BLUETOOTH_CORE_SPEC_54);
            #if TCFG_USER_TWS_ENABLE
            tws_profile_init();
            sys_key_event_filter_disable();
            #endif
            
            // 按键消息使能
            sys_key_event_enable();
            sys_auto_shut_down_enable();
            bt_sniff_feature_init();
            sys_auto_sniff_controle(MY_SNIFF_EN, NULL);
            app_var.dev_volume = -1;
            break;
        // 其他action处理...
        }
        break;
    // 其他状态处理...
    }
    return error;
}
```

## 主事件循环机制

**耳机模式应用本身并没有显式的主事件循环。实际上，主事件循环是由底层RTOS提供的，位于系统库中。应用通过注册事件处理函数来响应事件。**

### 事件处理函数注册 (apps\earphone\earphone.c)

```c
static const struct application_operation app_earphone_ops = {
    .state_machine = state_machine,
    .event_handler = event_handler,
};

REGISTER_APPLICATION(app_earphone) = {
    .name = "earphone",
    .action = ACTION_EARPHONE_MAIN,
    .ops = &app_earphone_ops,
    .state = APP_STA_DESTROY,
};
```

### 事件处理函数实现 (apps\earphone\earphone.c)

```c
static int event_handler(struct application *app, struct sys_event *event)
{
    if (SYS_EVENT_REMAP(event)) {
        g_printf("****SYS_EVENT_REMAP**** \n");
        return 0;
    }

    switch (event->type) {
    case SYS_KEY_EVENT:
        // 按键事件处理
        if (bt_user_priv_var.fast_test_mode) {
            audio_adc_mic_demo_close();
            tone_play_index(IDEX_TONE_NORMAL, 1);
        }
        #if CONFIG_BT_BACKGROUND_ENABLE
        if (bt_in_background()) {
            break;
        }
        #endif
        app_earphone_key_event_handler(event);
        break;
        
    case SYS_BT_EVENT:
        // 蓝牙事件处理
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            bt_hci_event_handler(&event->u.bt);
        }
        // 其他蓝牙事件处理...
        break;
        
    case SYS_DEVICE_EVENT:
        // 设备事件处理
        // 充电、电源、耳机入耳检测等事件处理...
        break;
        
    // 其他事件类型处理...
    }
    
    return false;
}
```

## 按键事件响应流程

当按下按键时，事件响应流程如下：

### 按键扫描与事件生成 (系统库中)

1. 系统定时器定期调用按键扫描函数
2. 按键扫描函数检测到按键状态变化
3. 生成按键事件，并通过`sys_event_notify()`函数通知系统

### 事件分发 (系统库中)

1. 系统将按键事件放入任务队列
2. RTOS的主事件循环从队列中获取事件
3. 系统调用当前应用的事件处理函数

### 耳机模式应用的按键处理 (apps\earphone\earphone.c)

```c
static int event_handler(struct application *app, struct sys_event *event)
{
    // ...
    switch (event->type) {
    case SYS_KEY_EVENT:
        // 按键事件处理
        app_earphone_key_event_handler(event);
        break;
    // ...
    }
    // ...
}
```

### 具体按键功能处理 (apps\earphone\key_event_deal.c)

```c
int app_earphone_key_event_handler(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    u8 key_event = key_table[key->value][key->event];
    
    // 根据按键值和事件类型执行相应操作
    switch (key_event) {
    case KEY_MUSIC_PP:
        // 播放/暂停
        break;
    case KEY_VOLUME_UP:
        // 音量增加
        break;
    // 其他按键处理...
    }
    
    return false;
}
```

## 总结：耳机模式应用的运行机制

1. 启动流程：
   - `app_main()`选择启动耳机模式应用
   - `start_app()`找到注册的"earphone"应用并启动
   - 调用状态机函数，执行APP_STA_CREATE和APP_STA_START状态的处理
   - 在APP_STA_START状态中初始化蓝牙、按键等功能
2. 主事件循环：
   - 主事件循环由底层RTOS提供，不在应用代码中显式实现
   - 应用通过注册事件处理函数来响应事件
   - 系统事件（如按键、蓝牙、设备等）通过`sys_event_notify()`通知系统
   - 系统将事件放入任务队列，并调用当前应用的事件处理函数
3. 按键事件响应：
   - 按键扫描函数检测到按键状态变化，生成按键事件
   - 系统将按键事件放入任务队列
   - 系统调用耳机模式应用的事件处理函数
   - 事件处理函数调用`app_earphone_key_event_handler()`处理具体按键功能

这种基于事件驱动的架构使得系统能够高效地响应各种事件，同时保持低功耗特性。耳机模式应用不需要自己实现主事件循环，只需要实现事件处理函数，由系统负责事件的分发和调用。
