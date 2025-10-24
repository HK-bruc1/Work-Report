# TWS耳机LED UI灯效流程深度分析

## 概述

本文档深入分析TWS耳机SDK中LED UI灯效系统的完整流程机制。LED灯效系统采用**独立的消息处理架构，与主消息系统并行运行，通过专门的消息处理器实现灯效状态的响应式更新。**

## 1. LED UI系统架构概览

### 1.1 LED UI独立消息处理机制

**与主消息系统的并行运行**：

LED UI系统通过**全局消息处理器**的方式工作，这与之前分析的**按模式消息处理不同**：

- 所有模式都一定用的灯效。就直接注册进os，根据系统状态去响应对应灯效。

```c
// LED UI消息处理器注册 (led_ui_msg_handler.c:637)
APP_MSG_HANDLER(led_msg_entry) = {
    .owner      = 0xff,    // 全局处理器，不限定特定模式
    .from       = 0xff,    // 监听所有消息源
    .handler    = ui_led_msg_handler,
};
```

**关键差异**：

- **主消息系统**：基于模式的消息处理(bt_mode, music_mode等)
- **LED UI系统**：全局监听，跨模式响应

### 1.2 LED UI消息分发架构

**位置**: `SDK/apps/earphone/ui/led/led_ui_msg_handler.c:606`

- 在这里进行消息分发。
- 不同的消息类型对应有不同的UI灯效状态。

```c
static int ui_led_msg_handler(int *msg)
{
    switch (msg[0]) {
#if TCFG_USER_TWS_ENABLE
    case MSG_FROM_TWS:              // TWS同步消息
        ui_tws_msg_handler(msg + 1);
        break;
#endif
    case MSG_FROM_BT_STACK:         // 蓝牙协议栈消息
        ui_bt_stack_msg_handler(msg + 1);
        break;
    case MSG_FROM_APP:              // 应用层消息
        ui_app_msg_handler(msg + 1);
        break;
    case MSG_FROM_BATTERY:          // 电池消息
        ui_battery_msg_handler(msg + 1);
        break;
    case MSG_FROM_PWM_LED:          // LED硬件消息
        ui_pwm_led_msg_handler(msg + 1);
        break;
    }
    return 0;
}
```

## 2. LED UI消息处理分支详析

### 2.1 ui_app_msg_handler - 应用状态灯效

**位置**: `led_ui_msg_handler.c:169`

处理系统状态变化对应的灯效更新：

```c
static int ui_app_msg_handler(int *msg)
{
    u8 channel = tws_api_get_local_channel();  // 获取TWS通道(左右耳区分)

    switch (msg[0]) {
    case APP_MSG_ENTER_MODE:
        led_enter_mode(msg[1] & 0xff);         // 模式切换灯效
        break;
        
    case APP_MSG_POWER_ON:
        // 开机灯效 - 红蓝交替慢闪
        led_ui_set_state(LED_STA_RED_BLUE_SLOW_FLASH_ALTERNATELY, DISP_NON_INTR);
        break;
        
    case APP_MSG_POWER_OFF:
        // 关机灯效处理
        if (msg[1] == POWEROFF_NORMAL || msg[1] == POWEROFF_NORMAL_TWS) {
            dhf_poweroff_flag = 1;
            // 可以设置关机灯效，如红灯闪三次
        }
        break;
        
    case APP_MSG_TWS_UNPAIRED:
        // TWS未配对状态灯效
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            // 主耳：红蓝快速交替闪
            led_ui_set_state(LED_STA_RED_BLUE_FAST_FLASH_ALTERNATELY, DISP_CLEAR_OTHERS);
        } else {
            // 副耳：红灯每5秒闪一次
            led_ui_set_state(LED_STA_RED_FLASH_1TIMES_PER_5S, DISP_CLEAR_OTHERS);
        }
        break;
        
    case APP_MSG_BT_IN_PAGE_MODE:
        // 蓝牙配对模式灯效
        led_ui_set_state(LED_STA_RED_BLUE_SLOW_FLASH_ALTERNATELY, DISP_CLEAR_OTHERS);
        break;
        
    // 更多应用状态...
    }
    return 0;
}
```

### 2.2 ui_battery_msg_handler - 电池状态灯效

**位置**: `led_ui_msg_handler.c:99`

处理电池相关的灯效显示：

```c
static int ui_battery_msg_handler(int *msg)
{
    switch (msg[0]) {
    case MSG_LOW_POWER:
        // 低电量警告灯效
        if (msg[1] < POWEROFF_LOWBAT_VAL) {
            led_ui_set_state(LED_STA_RED_FLASH_3TIMES, DISP_CLEAR_OTHERS);
        }
        break;
        
    case MSG_CHARGE_START:
        // 开始充电灯效
        led_ui_set_state(LED_STA_RED_ON, DISP_CLEAR_OTHERS);
        break;
        
    case MSG_CHARGE_FULL:
        // 充电完成灯效  
        led_ui_set_state(LED_STA_BLUE_ON, DISP_CLEAR_OTHERS);
        break;
        
    case MSG_CHARGE_CLOSE:
        // 停止充电灯效
        led_ui_set_state(LED_STA_ALL_OFF, DISP_CLEAR_OTHERS);
        break;
    }
    return 0;
}
```

### 2.3 ui_bt_stack_msg_handler - 蓝牙状态灯效

处理蓝牙连接状态的灯效反馈：

```c
static int ui_bt_stack_msg_handler(int *msg)
{
    struct bt_event *bt = (struct bt_event *)msg;
    
    switch (bt->event) {
    case BT_STATUS_FIRST_CONNECTED:
        // 第一台设备连接成功
        led_ui_set_state(LED_STA_BLUE_FLASH_2TIMES, DISP_CLEAR_OTHERS);
        break;
        
    case BT_STATUS_FIRST_DISCONNECT:
        // 设备断开连接
        led_ui_set_state(LED_STA_RED_BLUE_FAST_FLASH_ALTERNATELY, DISP_CLEAR_OTHERS);
        break;
        
    case BT_STATUS_A2DP_MEDIA_START:
        // A2DP音频开始播放
        led_ui_set_state(LED_STA_BLUE_SLOW_FLASH, DISP_CLEAR_OTHERS);
        break;
        
    case BT_STATUS_PHONE_INCOME:
        // 来电提示灯效
        led_ui_set_state(LED_STA_BLUE_FAST_FLASH, DISP_CLEAR_OTHERS);
        break;
    }
    return 0;
}
```

## 3. LED灯效核心实现机制

### 3.1 led_ui_set_state() - 核心灯效设置函数

**位置**: `SDK/apps/common/ui/pwm_led/led_ui_api.c:315`

- 所有的灯效设置都是用这个接口。

```c
void led_ui_set_state(enum led_state_name name, enum led_disp_mode disp_mode)
{
    struct led_state_obj *obj;
    const struct led_state_map *map;

    // 步骤1: 从灯效配置表中查找指定灯效
    for (int i = 0; ; i++) {
        map = &g_led_state_table[i];
        if (map->name == 0) break;          // 查找结束
        if (name != map->name) continue;     // 继续查找匹配项
        
        // 步骤2: 找到匹配项，创建灯效状态对象
        obj = zalloc(sizeof(*obj));
        obj->name       = name;
        obj->disp_mode  = disp_mode;
        
        // 步骤3: 根据灯效类型设置参数
        if (map->time_flag == TIME_EFFECT_MODE) {
            // 硬件控制灯效
            obj->time = (const led_pdata_t *)map->arg1;
        } else {
            // 软件控制灯效
            obj->table      = (const struct led_state_item *)map->arg1;
            obj->table_size = map->time_flag;
        }
        
        // 步骤4: 添加到灯效管理链表并启动
        led_ui_add_state(obj);
        break;
    }
}
```

### 3.2 灯效显示模式 (disp_mode参数详解)

```c
enum led_disp_mode : u8 {
    DISP_NON_INTR      = 0x01,  // 不可中断：当前灯效不允许被其他灯效打断
    DISP_CLEAR_OTHERS  = 0x02,  // 清除其他：清除其他灯效，独占显示
    DISP_RECOVERABLE   = 0x04,  // 可恢复：被打断后可以恢复的周期灯效
    DISP_TWS_SYNC      = 0x08,  // TWS同步：需要双耳同步的灯效
    DISP_TWS_SYNC_RX   = 0x10,  // TWS接收：从机收到的同步灯效
    DISP_TWS_SYNC_TX   = 0x20,  // TWS发送：主机发起的同步灯效
};
```

**使用示例**：

```c
// 开机灯效 - 不可中断，独占显示
led_ui_set_state(LED_STA_POWERON, DISP_NON_INTR | DISP_CLEAR_OTHERS);

// TWS同步灯效 - 需要双耳同步
led_ui_set_state(LED_STA_BLUE_FLASH_3TIMES, DISP_TWS_SYNC);

// 可恢复灯效 - 被打断后可恢复
led_ui_set_state(LED_STA_BLUE_SLOW_FLASH, DISP_RECOVERABLE);
```

### 3.3 灯效状态机制

**灯效动作类型** (`led_ui_api.h:46`):

```c
enum {
    LED_ACTION_WAIT,        // 等待：延时指定时间后执行下一动作
    LED_ACTION_CONTINUE,    // 继续：立即执行下一动作
    LED_ACTION_LOOP,        // 循环：回到第一个动作，形成循环效果
    LED_ACTION_END,         // 结束：灯效播放完毕
};
```

**灯效状态项结构**：

```c
struct led_state_item {
    u8 led_name;        // LED名称 (LED_RED, LED_BLUE)
    u8 time_msec;       // 亮的时间，单位50ms
    u8 brightiness;     // 亮度值 (0-255)
    u8 breath_time;     // 呼吸效果时间，0为非呼吸灯效
    u8 action;          // 动作类型 (WAIT/CONTINUE/LOOP/END)
};
```

## 4. LED灯效定义与自定义方法深度解析

### 4.1 预定义灯效状态

**位置**: `SDK/apps/earphone/include/led_config.h:13`

```c
enum led_state_name : u8 {
    // 基础控制
    LED_STA_NONE = 0,           // 无状态
    LED_STA_ALL_ON,             // 所有灯全亮
    LED_STA_ALL_OFF,            // 所有灯全灭
    
    // 红灯灯效
    LED_STA_RED_ON,             // 红灯常亮
    LED_STA_RED_ON_1S,          // 红灯亮1秒
    LED_STA_RED_FLASH_1TIMES,   // 红灯闪1次
    LED_STA_RED_FLASH_3TIMES,   // 红灯闪3次
    LED_STA_RED_SLOW_FLASH,     // 红灯慢闪
    LED_STA_RED_FAST_FLASH,     // 红灯快闪
    LED_STA_RED_BREATHE,        // 红灯呼吸
    
    // 蓝灯灯效
    LED_STA_BLUE_ON,            // 蓝灯常亮
    LED_STA_BLUE_FLASH_3TIMES,  // 蓝灯闪3次
    LED_STA_BLUE_SLOW_FLASH,    // 蓝灯慢闪
    LED_STA_BLUE_BREATHE,       // 蓝灯呼吸
    
    // 组合灯效
    LED_STA_RED_BLUE_SLOW_FLASH_ALTERNATELY,  // 红蓝交替慢闪
    LED_STA_RED_BLUE_FAST_FLASH_ALTERNATELY,  // 红蓝交替快闪
    LED_STA_RED_BLUE_BREATHE_ALTERNATELY,     // 红蓝交替呼吸
    
    // 特殊灯效
    LED_STA_POWERON,            // 开机灯效
    LED_STA_LOWPOWER_POWEROFF,  // 低电关机灯效
};
```

### 4.2 自定义灯效的两种实现形式

LED UI系统支持**两种不同的灯效实现方式**，分别适用于不同的应用场景：

#### 4.2.1 第一种形式：软件控制灯效 (LED_STATE_TABLE)

**特点**：
- 基于软件状态机控制
- **灵活性极高，可精确控制每个动作**
- **适合复杂的灯效序列和组合效果**
- **CPU占用相对较高，但可实现任意复杂度灯效**

**实现步骤**：

**步骤1：定义灯效动作序列**

```c
// 示例：红蓝灯交替闪烁3次的复杂灯效
const struct led_state_item red_blue_led_5s_flashs_3_times[] = {
    // 初始状态：关闭所有灯
    { LED_RED,   0, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_CONTINUE },
    { LED_BLUE,  0, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_CONTINUE },

    // 第一次闪烁：红灯先闪
    { LED_RED,   2, LED_RED_BRIGHTNESS,  NON_BREATH_MODE, LED_ACTION_WAIT },     // 红灯亮100ms
    { LED_RED,   2, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_WAIT },     // 红灯灭100ms
    { LED_BLUE,  2, LED_BLUE_BRIGHTNESS, NON_BREATH_MODE, LED_ACTION_WAIT },     // 蓝灯亮100ms
    { LED_BLUE,  2, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_WAIT },     // 蓝灯灭100ms

    // 第二次闪烁：间隔时间更长
    { LED_RED,   6, LED_RED_BRIGHTNESS,  NON_BREATH_MODE, LED_ACTION_WAIT },     // 红灯亮300ms
    { LED_RED,   6, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_WAIT },     // 红灯灭300ms
    { LED_BLUE,  6, LED_BLUE_BRIGHTNESS, NON_BREATH_MODE, LED_ACTION_WAIT },     // 蓝灯亮300ms
    { LED_BLUE,  6, LED_BRIGHT_OFF,      NON_BREATH_MODE, LED_ACTION_WAIT },     // 蓝灯灭300ms

    // 结束动作：循环或结束
    { LED_BLUE,  68, LED_BRIGHT_OFF,     NON_BREATH_MODE, LED_ACTION_LOOP },     // 等待3.4秒后循环
};
```

**步骤2：在配置表中注册**

```c
const struct led_state_map g_led_state_table[] = {
    // 使用LED_STATE_TABLE宏注册软件控制灯效
    { LED_STA_RED_BLUE_5S_FLASHS_3_TIMES, LED_STATE_TABLE(red_blue_led_5s_flashs_3_times) },
    
    // LED_STATE_TABLE宏展开为：ARRAY_SIZE(red_blue_led_5s_flashs_3_times), red_blue_led_5s_flashs_3_times
};
```

**动作类型详解**：

```c
enum {
    LED_ACTION_WAIT,        // 等待：延时time_msec*50ms后执行下一动作
    LED_ACTION_CONTINUE,    // 继续：立即执行下一动作(用于同时控制多个LED)
    LED_ACTION_LOOP,        // 循环：回到第一个动作，形成循环灯效
    LED_ACTION_END,         // 结束：灯效播放完毕，触发结束回调
};
```

其他参数说明：[3.3 灯效状态机制](###3.3 灯效状态机制)

#### 4.2.2 第二种形式：硬件控制灯效 (TIME_EFFECT_MODE)

**特点**：
- 基于硬件PWM直接控制
- 性能高，CPU占用低
- 适合简单的周期性灯效
- 硬件直接执行，时序精确

**实现步骤**：

**步骤1：定义硬件配置结构**

- 到时候配的时候，把结构体丢给AI吧。

```c
// 示例：红灯每5秒闪1次的硬件灯效
const struct led_platform_data led_red_flash_1times_per_5s_config = {
    .ctl_option = CTL_LED0_ONLY,        // 控制LED0(红灯)
    .ctl_mode = CYCLE_ONCE_BRIGHT,      // 周期性单次亮灭模式
    .ctl_cycle = 100,                   // 周期时间 100*50ms = 5秒
    .ctl_cycle_num = 0,                 // 0表示无限循环
    .once_bright.bright_time = 2,       // 亮的时间 2*50ms = 100ms
};

// 示例：蓝灯呼吸灯效
const struct led_platform_data led_blue_breathe_config = {
    .ctl_option = CTL_LED1_ONLY,        // 控制LED1(蓝灯)
    .ctl_mode = CYCLE_BREATHE_BRIGHT,   // 呼吸模式
    .ctl_cycle = 40,                    // 呼吸周期 40*50ms = 2秒
    .ctl_cycle_num = 0,                 // 无限循环
    .breathe_bright.bright_time = 30,   // 渐亮时间 30*50ms = 1.5秒
    .breathe_bright.brightest_keep_time = 5, // 最亮保持时间 5*50ms = 250ms
};
```

**步骤2：在配置表中注册**

```c
const struct led_state_map g_led_state_table[] = {
    // 使用TIME_EFFECT_MODE标记注册硬件控制灯效
    { LED_STA_RED_FLASH_1TIMES_PER_5S,  TIME_EFFECT_MODE, &led_red_flash_1times_per_5s_config },
    { LED_STA_BLUE_BREATHE,             TIME_EFFECT_MODE, &led_blue_breathe_config },
};
```

**硬件控制模式详解**：

```c
enum {
    ALWAYS_BRIGHT,          // 常亮模式
    ALWAYS_EXTINGUISH,      // 常灭模式
    CYCLE_ONCE_BRIGHT,      // 周期性单次亮灭
    CYCLE_TWICE_BRIGHT,     // 周期性双次亮灭
    CYCLE_BREATHE_BRIGHT,   // 呼吸灯模式
};

typedef struct led_platform_data {
    const led_board_cfg_t *board_cfg;
    u8 ctl_option;                  //参考枚举led_ctl_option
    u8 ctl_mode;                    //参考枚举led_ctl_mode
    u8 ctl_cycle;                   //控制周期, 单位50ms, 比如每5s闪一次灯，那么5s就是控制周期
    u8 ctl_cycle_num;               //控制周期的个数，值为0时，则控制周期无限循环，值为n时，则第n次控制周期之后,灯自动关闭
    union {
        struct {                    //周期单闪, 如灯每5s闪一次，每次亮100ms
            u8 bright_time;         //灯亮的时间，单位50ms
        } once_bright;
        struct {                    //周期双闪, 如灯每5s闪两次，第一次亮100ms，间隔50ms，再第二次亮100ms
            u8 first_bright_time;   //第一次灯亮的时间，单位50ms
            u8 bright_gap_time;     //间隔时间，单位50ms
            u8 second_bright_time;  //第二次灯亮的时间，单位50ms
        } twice_bright;
        struct {                    //周期呼吸，如灯5s呼吸一次，每次呼吸亮2s
            u8 bright_time;         //灯亮的时间，单位50ms，等于 占空比自增自减的时间 + 最大占空比保持的时间
            u8 brightest_keep_time; //亮度增到最大的时候，如果需要保持的时间，单位50ms，该时间要小于 bright_time
        } breathe_bright;
    };
    void (*cbfunc)(u32 cnt);        //灯效结束回调函数
} led_pdata_t;
```

### 4.3 两种形式的对比分析

| 特性 | 软件控制 (LED_STATE_TABLE) | 硬件控制 (TIME_EFFECT_MODE) |
|------|---------------------------|---------------------------|
| **控制方式** | 软件状态机 | 硬件PWM直接控制 |
| **性能消耗** | CPU占用较高 | CPU占用极低 |
| **时序精度** | 受系统调度影响 | 硬件精确控制 |
| **灵活性** | 极高，任意复杂序列 | 有限，标准模式组合 |
| **适用场景** | 复杂灯效序列、多LED协调 | 简单周期灯效、基础闪烁 |
| **开发复杂度** | 较复杂，需定义动作序列 | 简单，配置参数即可 |
| **内存占用** | 较高(状态机+动作表) | 较低(配置结构体) |

### 4.4 选择建议与最佳实践

**选择硬件控制的场景**：

- 简单的单色闪烁、呼吸灯效
- 对性能要求高的场景
- 周期性重复的基础灯效
- TWS同步要求高的灯效

**选择软件控制的场景**：
- 需要多LED精确协调的复杂灯效
- 不规律的灯效序列
- 需要中间状态判断的灯效
- 自定义动画效果

**示例对比**：

```c
// 简单闪烁 - 推荐硬件控制
{ LED_STA_BLUE_SLOW_FLASH, TIME_EFFECT_MODE, &led_blue_slow_flash_config },

// 复杂序列 - 推荐软件控制  
{ LED_STA_CUSTOM_SEQUENCE, LED_STATE_TABLE(custom_complex_sequence) },
```

## 5. LED硬件控制层分析

### 5.1 PWM LED硬件接口

**位置**: `led_ui_api.c:64`

```c
void pwm_led_status_display(const struct led_state_item *action)
{
    led_pdata_t hw_data = {0};

    // LED选择
    switch (action->led_name) {
    case LED_RED:
        hw_data.ctl_option = CTL_LED0_ONLY;
        break;
    case LED_BLUE:
        hw_data.ctl_option = CTL_LED1_ONLY;
        break;
    }

    // 灯效模式设置
    if (action->breath_time) {
        // 呼吸灯效模式
        hw_data.ctl_mode = CYCLE_BREATHE_BRIGHT;
        hw_data.breathe_bright.bright_time = action->breath_time;
        hw_data.ctl_cycle = action->breath_time + 1;
        hw_data.breathe_bright.brightest_keep_time = action->breath_time / 4;
    } else {
        // 普通开关模式
        if (action->brightiness) {
            hw_data.ctl_mode = ALWAYS_BRIGHT;
        } else {
            hw_data.ctl_mode = ALWAYS_EXTINGUISH;
        }
    }
    
    // 硬件控制执行
    led_effect_output(&hw_data, 0);
}
```

### 5.2 TWS灯效同步机制

LED UI支持TWS双耳灯效同步：

```c
// TWS同步灯效示例
led_ui_set_state(LED_STA_BLUE_FLASH_3TIMES, DISP_TWS_SYNC);
```

**同步流程**：
1. 主机设置灯效时添加`DISP_TWS_SYNC`标记
2. 系统自动向从机发送同步数据
3. 从机接收后自动播放相同灯效
4. 硬件层保持双耳灯效时序同步

## 6. LED UI消息处理流程总结

### 6.1 完整消息流程

```
系统状态变化 (如蓝牙连接) 
    ↓
产生对应消息 (MSG_FROM_BT_STACK)   //灯效和模式都有这个消息类型的处理，保证了灯效和功能的并行更新。
    ↓
全局LED消息处理器拦截 (ui_led_msg_handler)
    ↓
分发到具体处理分支 (ui_bt_stack_msg_handler)
    ↓
调用灯效设置函数 (led_ui_set_state)
    ↓
查找灯效配置表 (g_led_state_table)
    ↓
创建灯效状态对象 (led_state_obj)
    ↓
启动灯效状态机 (led_ui_state_machine_run)
    ↓
硬件PWM控制输出 (pwm_led_status_display)
    ↓
TWS同步 (可选，基于disp_mode)
```

### 6.2 与主消息系统的协作

**并行处理机制**：
- **主消息系统**：处理功能逻辑、模式切换、用户交互
- **LED UI系统**：监听状态变化，提供视觉反馈

**消息共享**：
- L**ED UI系统监听主消息系统的所有消息类型**
- **根据消息内容自动更新对应的灯效显示**
- **不干扰主消息系统的正常功能流程**

## 7. 自定义灯效最佳实践

### 7.1 设计原则

1. **状态明确**：每种系统状态对应明确的灯效
2. **优先级管理**：重要状态使用`DISP_NON_INTR`
3. **TWS一致性**：需要同步的使用`DISP_TWS_SYNC`
4. **资源优化**：避免过于复杂的灯效序列

### 7.2 常见应用场景

**开机/关机**：
```c
led_ui_set_state(LED_STA_POWERON, DISP_NON_INTR);
```

**蓝牙配对**：
```c
led_ui_set_state(LED_STA_RED_BLUE_FAST_FLASH_ALTERNATELY, DISP_CLEAR_OTHERS);
```

**电池提醒**：
```c
led_ui_set_state(LED_STA_RED_FLASH_3TIMES, DISP_CLEAR_OTHERS);
```

**音频播放**：
```c
led_ui_set_state(LED_STA_BLUE_SLOW_FLASH, DISP_RECOVERABLE);
```

这种LED UI系统设计实现了状态驱动的视觉反馈机制，为用户提供了直观的TWS耳机状态指示，同时保持了良好的系统架构和扩展性。

# 灯效流程

## 单耳

```c
开机未连接
[00:00:02.169][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.170][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:04.235][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:04.236][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:04.240][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:10.251][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:00:23.962][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:00:36.692][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:00:48.558][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE

连接手机蓝牙后
[00:04:51.325][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:04:57.334][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:04:59.887][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE

手动断开-1
[00:07:00.212][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:07:00.467][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:07:00.698][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE

超距离断开-2
[00:01:26.835][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:12:10.076][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:01:27.064][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:01:41.092][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:01:55.117][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE

连接后放歌
[00:06:18.690][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:06:22.731][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE

停止放歌
[00:06:28.999][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
[00:06:33.821][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE

来电
[00:08:33.328][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:08:33.349][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_INCOME-------------------------------------------------------

[00:08:33.427][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SCO_CONNECTION_REQ----------BT_CALL_INCOMING---------

拒接/挂断
[00:08:40.326][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE

通话中
[00:11:05.634][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE

亮屏幕/熄屏幕会返回播歌状态
开屏幕都会默认返回播歌状态，如果确实没播放，就会
[00:14:14.021][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
```

### 有连接记录，开机不连接

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON

```

### 有连接记录，开机连接

```c

```

### 有连接记录，开机连接后不放歌

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.617][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.619][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.303][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:05.599][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:00:08.666][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:00:08.669][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_CONNECTED  
[00:00:14.639][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:17.478][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:22.516][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE    
```

### 有连接记录，开机连接后放歌后暂停

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.617][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.619][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.303][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:05.599][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:00:08.666][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:00:08.669][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_CONNECTED  
[00:00:14.639][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:17.478][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:22.516][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE  
[00:06:38.356][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE
[00:07:08.259][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
[00:07:12.082][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:08:25.233][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE    
```

### 有连接记录，开机连接后，超距断开

```c

```

### TWS配对后连接手机

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE 
[00:00:03.298][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED
[00:00:03.299][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:03.466][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED
[00:00:03.468][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag    
```

### TWS配对后连接手机开启游戏模式后退出游戏模式

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE 
[00:00:03.298][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED
[00:00:03.299][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:03.466][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED
[00:00:03.468][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag  
    
[00:05:13.002][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:05:20.972][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY

[00:07:37.751][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:08:29.518][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY    
```



## 双耳

```c
开机(有配对记录之后)
[00:00:01.269][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:01.270][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.288][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.289][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.292][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:08.306][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:00:16.321][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE。。。TWS回连超时后最终
[00:01:12.417][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE

开机（没有配对记录情况下）
[00:00:01.359][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:01.360][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.432][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.433][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.437][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:08.445][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE



TWS配对成功
[00:15:28.908][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:15:49.388][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED--------
[00:00:19.116][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODEIP

TWS断开
[00:23:47.398][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH--------
[00:23:47.441][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
回连成功
[00:24:57.898][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED------------------------------


连接手机成功
[00:18:07.685][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:18:13.728][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:18:21.728][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE

断开手机
[00:19:15.220][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:19:15.366][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT

超距离断开
[00:20:47.541][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:20:47.592][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
```

## 各灯效限定

- 开机
  - `[00:00:02.169][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON`
- 关机
  - `[00:30:48.142][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF`
- 未连接手机之前的灯效
  - `[00:07:00.467][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT`
- 连接手机后的灯效
  - `[00:04:51.325][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED`
- 超距离断开
  - `[00:01:27.064][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE`
  - 会覆盖断开的`[00:07:00.467][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT`
- 播放歌曲
  - `[00:06:22.731][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE`
- 停止放歌
  - `[00:06:28.999][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP`
- 来电
  - `[00:08:33.349][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_INCOME------------------------------`
  - `[00:08:33.427][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SCO_CONNECTION_REQ------BT_CALL_INCOMING`
  - 避免覆盖，一个就行
- 拒接/挂断/通话中
  - `[00:08:40.326][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE`
- TWS配对成功
  - `[00:15:49.388][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED--------`
- TWS断开
  - `[00:23:47.398][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH--------`

## 充电状态相关灯效

### 关机状态插入充电到充满

```c
[00:00:00.273][APP_CHARGE]charge_ldo5v_in_deal
[00:00:00.274]TWS_EVENT_SYNC_FUN_CMD: 2 //另一边关机命令
[00:00:00.274][CHARGE]charge_start
[00:00:00.275][APP_CHARGE]set wdt to 32s!
[00:00:00.275][APP_CHARGE]charge_start_deal
[00:00:00.276][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_START, 0); //发送消息
[00:00:00.277][PWM_LED]led_name = 3, disp_mode = 0x2 //灯效流程获取到消息更新灯效
[00:00:00.277][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_STARTS<> //开始充电灯效日志
[00:00:00.375][CHARGE]constant_current_progi_volt_config, 233, cur_vbat: 4264 mV
S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>*****************S<>*S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>
[00:00:58.276][APP_CHARGE]charge_full_deal
[00:00:58.277][CHARGE]charge_close
[00:00:58.277][APP_CHARGE]charge_close_deal
[00:00:58.278][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_CLOSE, 0);//发送消息
[00:00:58.279][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_CLOSE//结束充电灯效日志，最终是这里
[00:00:58.280][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_ERR
[00:00:58.281][PWM_LED]led_name = 17, disp_mode = 0x2 //灯效流程获取到消息更新灯效
[00:00:58.282][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_LDO5V_OFF //结束充电灯效日志
```

- 开启灯口保护后可以使充满灯效常亮
- 不开启灯口保护会使充满灯效亮一会后熄灭

### 充电拔出

```c
[00:00:00.236][APP_CHARGE]charge_ldo5v_in_deal
[00:00:00.237]TWS_EVENT_SYNC_FUN_CMD: 2
[00:00:00.237][CHARGE]charge_start
[00:00:00.238][APP_CHARGE]set wdt to 32s!
[00:00:00.238][APP_CHARGE]charge_start_deal
[00:00:00.239][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_START, 0);
[00:00:00.240][PWM_LED]led_name = 3, disp_mode = 0x2
[00:00:00.240][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_STARTS<>
[00:00:00.338][CHARGE]constant_current_progi_volt_config, 233, cur_vbat: 3804 mV
[00:00:00.339][CHARGE]constant_current_progi_volt_config, 245, max_progi: 904 mV
S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>**************************S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S<>S
[00:02:07.127][APP_CHARGE]charge_ldo5v_off_deal
[00:02:07.127][CHARGE]charge_close
[00:02:07.128][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_LDO5V_OFF, 0);
[00:02:07.129][APP_CHARGE]set wdt to 4s!
[00:02:07.129][APP_CHARGE]ldo5v off,enter softpoweroff
[00:02:07.130][PMU]=============power_set_soft_poweroff============
[00:02:07.131][PMU]sf_keep_lpctmu: 0
[00:02:07.131][PMU]sf_keep_pvdd: 0
[00:02:07.132][PMU]sf_keep_nvdd: 0
[00:02:07.132][PMU]sf_vddio_keep: 1
[00:02:07.132][PMU]keep_lrc: 0
[00:02:07.133][PMU]sfc_bit_mode: 2, port: 0
```

- 开启灯口保护后，拔出，充电中的红灯不会熄灭

```c
void charge_ldo5v_off_deal(void)
{
    int abandon = 0;
    int off_type = LDO5V_OFF_TYPE_NORMAL_ON;
    bool lowpower_flag = FALSE, is_bt_mode, is_idle_mode;
    const struct app_charge_handler *handler;

    log_info("%s\n", __FUNCTION__);

    //拨出交换
    batmgr_send_msg(POWER_EVENT_POWER_CHANGE, 0);

    charge_full_flag = 0;

    charge_close();

    batmgr_send_msg(BAT_MSG_CHARGE_LDO5V_OFF, 0);
    log_info("batmgr_send_msg(BAT_MSG_CHARGE_LDO5V_OFF, 0);\n");
#if _TCFG_PWMLED_PORT_PROTECT_ENABLE
    //灯口保护中，发了消息但是灯效没有变化，这里手动更新一次
    led_ui_set_state(LED_STA_ALL_OFF, DISP_CLEAR_OTHERS);
#endif
```
