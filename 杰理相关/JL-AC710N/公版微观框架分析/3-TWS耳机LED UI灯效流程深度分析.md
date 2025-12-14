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

## 有蓝牙和TWS连接记录，开机不连接直到自动关机

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.305][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED 
[00:00:03.306][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:09.315][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:17.324][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:31.349][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:45.373][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:59.398][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:01:13.423][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:03:02.637][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:03:05.503][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:03:05.504][LED_UI]LED_STATE_END: name = 9    
```

- `APP_MSG_BT_IN_PAGE_MODE`有一点像回连手机或者TWS。

## 有蓝牙连接记录，开机连接TWS不连接手机直到自动关机

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.305][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED 
[00:00:03.306][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:07.478][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED
[00:00:07.481][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:09.315][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:17.324][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:31.349][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:45.373][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:59.398][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:01:13.423][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:01:19.501][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE
[00:03:15.808][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:03:17.479][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH------goto_poweroff_flag--------break-
[00:03:18.973][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:03:18.974][LED_UI]LED_STATE_END: name = 9    
```

## 有蓝牙连接记录，开机连接手机中途出仓对耳

- 考虑的是最快的回连手机速度

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.305][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED 
[00:00:03.306][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:07.481][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE----app_msg_power_on_flag
[00:00:10.624][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:00:10.628][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_CONNECTED
[00:00:15.642][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:15.642][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
    
回连手机成功后出仓对耳
[00:04:21.290][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED-----
[00:04:21.674][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:04:26.746][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE

中途碰手机
[00:07:24.748][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:07:24.995][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE 
[00:07:30.756][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
[00:07:35.789][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
    
中途触摸按键
[00:09:10.659][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:09:15.678][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
    
中途进入游戏模式再退出
[00:10:26.488][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY 
[00:10:50.252][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY

中途断开对耳
[00:11:59.295][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH--
[00:11:59.405][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:12:04.442][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
    
中途放歌再停止
[00:15:13.149][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE
[00:15:16.464][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
[00:16:57.242][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE 
    
中途单耳超距断开直到自动关机
    耳机回连手机超时后，不会主动连接手机，除了手机手动回连
[00:08:03.629][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:08:03.652][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:08:17.686][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:08:31.712][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:10:09.884][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE 
[00:13:09.906][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:13:12.740][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:13:12.741][LED_UI]LED_STATE_END: name = 9    
```

- 如果不加限制，单耳回连手机成功后，再出仓对耳，最后的灯效可定是TWS连接灯效
- 如果不加限制，已经连接手机了，中途触发任何灯效都会被覆盖
- 有几个case一定不能设置灯效，因为频繁或者周期触发

## 单耳断开手机

```c
[00:00:35.731][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:35.995][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:00:36.298][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE    
```

## 双耳断开手机

```c
[00:03:20.872][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:03:20.873][PWM_LED]led_name = 10, disp_mode = 0xa
[00:03:20.875][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_DISCONNECT
```

## 其他流程

### 单耳已连接，对耳开机后连接蓝牙以及TWS，灯效异常

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.298][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED
[00:00:03.299][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
[00:00:04.080][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED-----
```

- 灯效执行到这里已经连接上了蓝牙了。
  - `TWS_EVENT_CONNECTED`这里要进行判断，是否为蓝牙连接状态来决定闪烁什么灯效。
  - 这里不能使用灯效标志位了，使用实际的API返回的连接状态
- 直接断掉，双耳直接断开。可能只有入仓才会有时间主从切换吧。

### 双耳已连接单耳入仓

```c
不会有任何灯效更新
```

- 所以这里不需要区分蓝牙状态。
- TWS断开的灯效只在蓝牙未连接状态下显示即可。

## 充电状态相关灯效

```c
单耳
[00:00:35.008][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:00:35.011][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:35.024][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT  
[00:00:37.828][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:00:37.829][LED_UI]LED_STATE_END: name = 9

双耳
[00:00:51.537][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:00:51.547][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:51.557][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:00:53.414][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH------goto_poweroff_flag--------break-------
[00:00:54.712][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:00:54.713][LED_UI]LED_STATE_END: name = 9    
```

- 从这里就可以看出，为啥上次关机后会有闪烁其他灯效直到彻底关机，关机灯效刚开就被覆盖了
- 所以在蓝牙断开，TWS断开的地方要加限制，如果是关机导致的，那就只显示关机灯效。

## 出仓灯效流程

```bash
>S<>S<>S<>R**************************************************************************************************************************************************************************************************************************************************
[00:00:07.769][APP_CHARGE]charge_ldo5v_off_deal
[00:00:07.769][CHARGE]charge_close
[00:00:07.770][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_LDO5V_OFF, 0);
[00:00:07.771][APP_CHARGE]set wdt to 4s!
[00:00:07.771][APP_CHARGE]ldo5v off,task switch to BT
[00:00:07.772]anc_resume
[00:00:07.772][LP_KEY]lp_touch_key_charge_mode_exit
[00:00:07.773][LP_KEY]lp_touch_key_identify_algo_reset
[00:00:07.774][LP_KEY]lpctmu enable
[00:00:07.774][APP_CHARGE]charge_close_deal
[00:00:07.775][APP_CHARGE]batmgr_send_msg(BAT_MSG_CHARGE_CLOSE, 0);
[00:00:07.776][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_LDO5V_OFF
[00:00:07.777][PWM_LED]led_name = 2, disp_mode = 0xa
[00:00:07.777][led_tws]led_tws_sync_start: 2
[00:00:07.778][led_tws]led_name=2, time=0, result = -1
[00:00:07.779][EARPHONE]bt mode
[00:00:07.780]tone_player_faild: tone_en/bt.*
[00:00:07.780][EARPHONE]<<<<<<<<<<<<<<bt_dual_conn_config=0>>>>>>>>>>
[00:00:07.782]le_support:1 1
[00:00:07.782]le_config:2 2 1 1
[00:00:07.782][LMP]>>>btctrler_task_init<<<
[00:00:07.794][HCI_LMP]init
[00:00:07.794][HCI_LMP]open
[00:00:07.795]bredr_bd_init
[00:00:07.795]bredr_bd_init_success
[00:00:07.796][LP_KEY]ch:4 res:3777 cur:1 net:2
[00:00:08.113][HCI_LMP]local_name DB31
[00:00:08.113][HCI_LMP]250 4
[00:00:08.114][LMP]HCI_WRITE_SCAN_ENABLE : 0
[00:00:08.115]tws_afh_init
[00:00:08.115][PWM_LED]led_name = 2, disp_mode = 0x2
[00:00:08.116][LED_UI]MSG_FROM_BATTERY----ui_battery_msg_handler----BAT_MSG_CHARGE_CLOSE
[00:00:08.117][EARPHONE]BT_STATUS_INIT_OK
[00:00:08.118]read anc_info succ,state:anc_init,mode:ANC_OFF
[00:00:08.119]################# multi_protocol init
[00:00:08.119]multi_protocol_profile_init 0x4000 1 0
[00:00:08.120]spp_file: apps/common/third_party_profile/jieli/online_db/spp_online_db.c
[00:00:08.121]app_spp_recieve_callback_register 41b678
[00:00:08.122][POWEROFF]sys_auto_shut_down_enable

[00:00:08.123]---------have tws info----------
EE A2 09 BB 08 78 
[00:00:08.124]tws_local_channel: R
[00:00:08.124]dual_conn_btstack_event_handler:3
[00:00:08.125][EARPHONE] BT STATUS DEFAULT
[00:00:08.125]ui_bt_stack_msg_handler:3
[00:00:08.126][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:08.127][PWM_LED]led_name = 11, disp_mode = 0x2
[00:00:08.128][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:08.129]tws_create_connection_internal

02 A5 D5 F1 34 21 
[00:00:08.130][BDMGR]sort_0_edr
qpg 34
c
02 A5 D5 F1 34 21 
[00:00:08.131][BDMGR]sort_0_edr
qpg 34
qps 34
[00:00:08.132][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED
[00:00:08.133][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIREDp
[00:00:08.162]check: 77
cbbe
[00:00:08.185]tws_link------1
[00:00:08.186][BDMGR]sort_0_edr
ide 1000
[00:00:08.186]tws_conn_set_max_slot=5
[00:00:08.187][BDMGR]sort_tws_0_edr
tws 80 60 0
ide 1000
[00:00:08.188]tws_conn_start_probe 1 
[00:00:08.200]tlcap_rx_role_switch_req: 4102

[00:00:08.201]instant: 0

[00:00:08.214]tlcap_channel_open: 4, 2

[00:00:08.215][BT-TWS]tws-user: role= 1, phone_link_connection 0, reason=0,event= 2
[00:00:08.216][BT-TWS]tws_event_pair_suss: 82
[00:00:08.217]tws_local_channel: R

[00:00:08.217][BATTERY]tws_sync_bat_level: 9,100
[00:00:08.218]tws_sniff_check_enable
[00:00:08.219]write_scan_conn_enable rets=0xfe318ee
[00:00:08.219]write_scan_conn_enable=0,0

[00:00:08.220][LMP]HCI_WRITE_SCAN_ENABLE : 0
[00:00:08.221][LMP]HCI_CNACLE_CREATE_CONNECTION
[00:00:08.222][POWEROFF]sys_auto_shut_down_disable
[00:00:08.222][POWEROFF]sys_auto_shut_down_enable
[00:00:08.223][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED------------------------------
[00:00:08.224][PWM_LED]led_name = 2, disp_mode = 0x2
[00:00:08.225]online debug tws status callback 0x2 role:1
[00:01:44.971][BATTERY]set_sibling_bat_level: 0, 10
[00:01:44.973]set_vol[idle]:music=16
[00:01:44.973]set_vol[idle]:call=15
[00:01:44.974]vol_sync: 16, 15

[00:01:44.974][EARPHONE]tws_sync_dual_conn_info_func: 0
[00:01:45.171]tws_tone_play: 91a57d, tone_en/tws_conn.*
[00:01:45.172]tone_player: tone_en/tws_conn.*
[00:01:45.173]pipeline_uuid: 7674

[00:01:45.173][CLOCK]---sys clk set : 192000000
[00:01:45.174][CLOCK]---SYSPLL EN : 1
[00:01:45.175][CLOCK]---D_PLL EN  : 0
[00:01:45.175][CLOCK]---HSB CLK : 192000000
[00:01:45.176][CLOCK]---LSB CLK : 24000000
[00:01:45.176][CLOCK]---SFC CLK : 96000000
[00:01:45.177][CLOCK]---HSB_PLL_DIV : 1 * 1
[00:01:45.177][CLOCK]---LSB_PLL_DIV : 1 * 1
[00:01:45.178][CLOCK]---SFC_DIV : 0
[00:01:45.178][CLOCK]--SYS DVDD  adaptive:13 SFR:13 -> DVDD_VOL_123V  @ 1206mv
[00:01:45.179][CLOCK]--SYS RVDD  adaptive:13 SFR:13 -> RVDD_VOL_123V  @ 1236mv
[00:01:45.180][CLOCK]--SYS DCVDD fix_mode:5 SFR:5 -> DCVDD_VOL_125V @ 1252mv
[00:01:45.181][CLOCK]---RANGE    : 6 / 0
[00:01:45.182][JLSTREAM]create_stream: 41be78, 0
[00:01:45.183]demuxer_type_match: tws_conn.wts
[00:01:45.184][JLSTREAM]jlstream_multi_thread create:jlstream_0
[00:01:45.185][P_MEM_C]Defrage disbale : 0 / 1
[00:01:45.186][DAC]sample 44100
[00:01:45.186][STREAM]format negotiation: suss
  { source,             8000, 1 ch, 16bit }
  { bt_audio_sync,      44100, 1 ch, 16bit }
  { dac,                44100, 1 ch, 16bit }
[00:01:45.189][JLSTREAM]frame_time: 8, 80,frame_len:128
[00:01:45.190]vol_dB :10, -18
[00:01:45.190]vol_2_gain: 10, 16, 2063
[00:01:45.191]audio_state:idle->tone,max_vol:16
[00:01:45.191]dvol_max:16384
[00:01:45.192]set_vol[tone]:tone=10
[00:01:45.192]vol_dB :10, -18
[00:01:45.192]vol_2_gain: 10, 16, 2063
[00:01:45.193][fade]state:tone,max_volume:16,cur:10,10
[00:01:45.193]set_vol[tone]:=10
[00:01:45.194][SW_DVOL]Gain:10,AVOL:3,DVOL:16384
[00:01:45.194]convert_data_ioc_start bit_wide, 0 0 15
[00:01:45.196][DAC]>>>>>>>>>>>>add dac syncts 41cfac
[00:01:45.197][JLSTREAM]send_callback: app_core, event 8, err 0
[00:01:45.198][DAC]__audio_dac_try_power_on
[00:01:45.198]>>>>>>>>>>>>>>>>>> channel: 1
[00:01:45.209]>> audio_common_power_open cur_status:0
[00:01:45.272]sync_call: 2a1e3095
[00:01:45.274]sync_call: 2a1e3095
[00:01:45.329][DAC]delay set : 2204, 108, 880
U
[00:01:45.330][DAC]audio_dac_fifo_start : 0x0, 10, DAC_CON = 0xf802f185
[00:01:45.331][fade]state:tone,max_volume:16,cur:10,10
[00:01:45.331]set_vol[tone]:=10
[00:01:45.332][SW_DVOL]Gain:10,AVOL:3,DVOL:16384
[00:01:45.332][DAC_NODE]dac_node:<1923138976 1922678976 14375>
[00:01:45.333]tone_callback: 8, 0
[00:01:45.335][JLSTREAM]dst_delay: 450, 299
[00:01:45.336]clr_device_in_page_list
[00:01:45.336][led_tws]led_name=26, time=1, result = 2
[00:01:45.337][PWM_LED]led_name = 26, disp_mode = 0x12
[00:01:45.338][led_tws]led_name=26, time=1, result = 2
[00:01:45.338][PWM_LED]led_name = 26, disp_mode = 0x12
u
[00:01:45.465][JLSTREAM]dec_err: 40
u
[00:01:45.486][JLSTREAM]dec_end: 0
U
[00:01:45.497][DAC]>>>>>>>>>>>>del dac syncts 41cfac
[00:01:45.497][DAC]DAV VOL0 : 0x40004000
[00:01:45.549][DAC]Audio DAC Stop
[00:01:45.549][JLSTREAM]send_callback: app_core, event 10, err 0
[00:01:45.550]tone_callback: a, 0
[00:01:45.550][JLSTREAM]free_stream: 41be78
[00:01:45.551][JLSTREAM]decoder_release: 41bf5c
[00:01:45.552][JLSTREAM]release: jlstream 20
[00:01:45.565]sync_call: 2a1e3095
[00:01:45.566][led_tws]led_name=26, time=1, result = 2
[00:01:45.566][PWM_LED]led_name = 26, disp_mode = 0x12
[00:01:49.182][clock-manager]cpu0: 6% cpu1: 0% jlstream: 0% curr_clk:192000000  min_clk:24000000 dest_clk:24000000, 1
[00:01:49.184][CLOCK]---sys clk set : 24000000
[00:01:49.195][CLOCK]---SYSPLL EN : 1
[00:01:49.195][CLOCK]---D_PLL EN  : 0
[00:01:49.196][CLOCK]---HSB CLK : 24000000
[00:01:49.187][CLOCK]---LSB CLK : 24000000
[00:01:49.187][CLOCK]---SFC CLK : 48000000
[00:01:49.188][CLOCK]---HSB_PLL_DIV : 1 * 1
[00:01:49.189][CLOCK]---LSB_PLL_DIV : 1 * 1
[00:01:49.190][CLOCK]---SFC_DIV : 0
[00:01:49.190][CLOCK]--SYS DVDD  adaptive:2 SFR:2 -> DVDD_VOL_090V  @ 897mv
[00:01:49.192][CLOCK]--SYS RVDD  adaptive:6 SFR:6 -> RVDD_VOL_102V  @ 1027mv
[00:01:49.193][CLOCK]--SYS DCVDD fix_mode:5 SFR:5 -> DCVDD_VOL_125V @ 1252mv
[00:01:49.194][CLOCK]---RANGE    : 0 / 0
[00:01:49.974]first_ap_timer: 312
[00:01:49.975][APP_AUDIO]VOL_SAVE 16
[00:01:50.169][LINK]link_sniff_init_lp_ws 0
[00:01:50.170][BDMGR]sort_tws_0_edr
tws 400 12 0
ide 1000
[00:01:50.171]tws_conn_enter_sniff
[00:01:50.172][BT-TWS]tws-user: role= 1, phone_link_connection 229, reason=15,event= 20
[00:01:50.173][BT-TWS]TWS_EVENT_MODE_CHANGE : 1
[00:01:50.174]online debug tws status callback 0x14 role:1
[00:01:50.459]RESTS : 0xfe1e446
[00:01:50.460]>> audio_common_power_close cur_status:1
S<>wS<>w*
[00:01:50.859]page_state: 0, 1006, 0

[00:01:50.860]write_scan_conn_enable rets=0xfe3146a
[00:01:50.861]write_scan_conn_enable=0,0

[00:01:50.862][LMP]HCI_WRITE_SCAN_ENABLE : 0S<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>w
[00:01:53.182][clock-manager]cpu0: 5% cpu1: 0% jlstream: 0% curr_clk:24000000  min_clk:24000000 dest_clk:24000000, 1
[00:01:53.184][CLOCK]---sys clk set : 24000000S<>wS<>wS<>wS<>wS<>wS<>w*******************S<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>w**S<>w
[00:01:56.619]PWMLED_TWS_SYNC_CMD_SET
[00:01:56.620]ret = 1, how_long_ago：66250us , 0s,sync_time = 0
[00:01:56.621]pwm_led_set_sync fail,request repeat cnt = 0S<>w
[00:01:56.871]PWMLED_TWS_SYNC_CMD_READS<>w
[00:01:57.120]PWMLED_TWS_SYNC_CMD_SET
[00:01:57.120]ret = 1, how_long_ago：247500us , 0s,sync_time = 0
[00:01:57.121]pwm_led_set_sync fail,request repeat cnt = 1
[00:01:57.124]PWMLED_TWS_SYNC_CMD_READS<>w
[00:01:57.182][clock-manager]cpu0: 1% cpu1: 0% jlstream: 0% curr_clk:24000000  min_clk:24000000 dest_clk:24000000, 1
[00:01:57.184][CLOCK]---sys clk set : 24000000S<>wS<>w
[00:01:57.610]PWMLED_TWS_SYNC_CMD_SET
[00:01:57.611]ret = 1, how_long_ago：251250us , 0s,sync_time = 0
[00:01:57.612]pwm_led_set_sync fail,request repeat cnt = 2
[00:01:57.614]link_conn_exit_sniff
[00:01:57.615][BDMGR]sort_tws_0_edr
[00:01:57.616]	
tws 80 12 0
ide 1000
[00:01:57.616]event_mode_change
[00:01:57.617]tws_conn_exit_sniff
[00:01:57.618][BT-TWS]tws-user: role= 0, phone_link_connection 0, reason=0,event= 20
[00:01:57.619][BT-TWS]TWS_EVENT_MODE_CHANGE : 0
[00:01:57.619]tws_sniff_check_enable
[00:01:57.620]online debug tws status callback 0x14 role:0
[00:01:57.961]sync_call: 2a1e3095
[00:01:57.962][led_tws]led_name=26, time=1, result = 2
[00:01:57.963][PWM_LED]led_name = 26, disp_mode = 0x12
[00:01:58.167]sync_call: 2a1e3095
[00:01:58.168][led_tws]led_name=26, time=1, result = 2
[00:01:58.169][PWM_LED]led_name = 26, disp_mode = 0x12
[00:01:58.264]sync_call: 2a1e3095
[00:01:58.265][led_tws]led_name=26, time=1, result = 2
[00:01:58.266][PWM_LED]led_name = 26, disp_mode = 0x12
[00:01:58.566]sync_call: 2a1e3095
[00:01:58.567][led_tws]led_name=26, time=1, result = 2
[00:01:58.568][PWM_LED]led_name = 26, disp_mode = 0x12
[00:02:01.183][clock-manager]cpu0: 2% cpu1: 0% jlstream: 0% curr_clk:24000000  min_clk:24000000 dest_clk:24000000, 1
[00:02:01.184][CLOCK]---sys clk set : 24000000
[00:02:02.662]first_ap_timer: 312
[00:02:02.857][LINK]link_sniff_init_lp_ws 0
[00:02:02.858][BDMGR]sort_tws_0_edr
tws 400 12 0
ide 1000
[00:02:02.859]tws_conn_enter_sniff
[00:02:02.860][BT-TWS]tws-user: role= 1, phone_link_connection 229, reason=15,event= 20
[00:02:02.862][BT-TWS]TWS_EVENT_MODE_CHANGE : 1
[00:02:02.862]online debug tws status callback 0x14 role:1
S<>wS<>wS<>wS<>wS<>wS<>wS<>w*****************S<>wS<>wS<>wS<>wS<>w
[00:02:05.183][clock-manager]cpu0: 1% cpu1: 0% jlstream: 0% curr_clk:24000000  min_clk:24000000 dest_clk:24000000, 1
[00:02:05.185][CLOCK]---sys clk set : 24000000S<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>wS<>w
[00:02:09.182][clock-manager]cpu0: 1% cpu1: 0% jlstream: 0% curr_clk:24000000  min_clk:24000000 dest_clk:24000000, 1
[00:02:09.184][CLOCK]---sys clk set : 24000000S<>wS<>w**S<>w
[00:02:09.826]PWMLED_TWS_SYNC_CMD_SET
[00:02:09.827]ret = 1, how_long_ago：251250us , 0s,sync_time = 0
[00:02:09.828]pwm_led_set_sync fail,request repeat cnt = 0
[00:02:09.830]PWMLED_TWS_SYNC_CMD_READS<>w*S<>wS<>w
```

## 重点

- 蓝牙连接后，不能被任何灯效打断除了是蓝牙连接后才有的功能灯效
  - 游戏模式，放音乐，来电等
  - 游戏模式进入可以有灯效
    - 但是出去时要跟蓝牙连接灯效保持一致。
      - 因为游戏模式在蓝牙连接后才能开启。

## 各灯效确认

最好跟提示音近一点的case，避免割裂感。

### 开机灯效以及后续灯效

```c
[00:00:02.263][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_ON
[00:00:02.607][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:02.619][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_INIT_OK
[00:00:02.621][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_ENTER_MODE
[00:00:03.305][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_PAIRED 
[00:00:03.306][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_TWS_UNPAIRED
```

- `APP_MSG_POWER_ON`设置为开机灯效。
  - 复杂的步骤由函数实现
- 其他case一概不设置灯效，避免打断。

### 关机灯效

```c
[00:13:09.906][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:13:12.740][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:13:12.741][LED_UI]LED_STATE_END: name = 9   
```

- `APP_MSG_POWER_OFF`设置为关机灯效。
- `LED_MSG_STATE_END`这个不设置灯效
- 可以通过这里的打印得知关机前的最后一个灯效效果名称

### TWS连接成功

```c
[00:04:21.290][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTED-----
[00:04:21.674][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:04:26.746][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
```

- `TWS_EVENT_CONNECTED`设置为TWS连接成功的灯效
- `BT_STATUS_SNIFF_STATE_UPDATE`这个为周期调用函数，不能设置等效。
- TWS连接成功了，中途可能被回连case `APP_MSG_BT_IN_PAGE_MODE`打断，所以要加限制。
  - 避免被`APP_MSG_BT_IN_PAIRING_MODE`打断，这个不能设置灯效。
    - 因为会覆盖蓝牙断开灯效。
- 在连接手机成功的状态下，TWS连接与断开都不允许覆盖蓝牙连接灯效
  - 这个要加限制。

### TWS连接断开

```c
[00:11:59.295][LED_UI]MSG_FROM_TWS----ui_tws_msg_handler----TWS_EVENT_CONNECTION_DETACH--
[00:11:59.405][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:12:04.442][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
```

- `TWS_EVENT_CONNECTION_DETACH`设置TWS断开灯效
- 其他限制同上

### 蓝牙连接灯效

```c
[00:00:10.624][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_CONNECTED
[00:00:10.628][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_CONNECTED
[00:00:15.642][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:15.642][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
```

- `BT_STATUS_FIRST_CONNECTED`设置为蓝牙连接灯效

- 可以覆盖蓝牙连接灯效的case:

  - BT_STATUS_SNIFF_STATE_UPDATE 已经加了限制了

  - TWS灯效，已经加了限制了

  - ```c
    [00:07:24.995][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_PHONE_HANGUP----BT_STATUS_A2DP_MEDIA_START----BT_STATUS_PHONE_ACTIVE 
    [00:07:30.756][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_A2DP_MEDIA_STOP
    [00:10:50.252][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY
    ```

    - 这一些要保证两点，进入时开启了某种灯效，退出时一定要还原到蓝牙连接灯效
      - 这一些灯效都是蓝牙连接后才有的。

### 蓝牙断开灯效

```c
单耳
[00:00:35.731][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SNIFF_STATE_UPDATE
[00:00:35.995][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:00:36.298][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE  

双耳
[00:03:20.872][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:03:20.873][PWM_LED]led_name = 10, disp_mode = 0xa
[00:03:20.875][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_SECOND_DISCONNECT
```

- `BT_STATUS_FIRST_DISCONNECT`设置蓝牙断开灯效
  - `APP_MSG_BT_IN_PAIRING_MODE`这个不设置灯效避免打断
- 断开时一般是要区分TWS状态的，因为一般TWS连接与断开都是有不同灯效的。那么蓝牙断开时必然要区分TWS状态。

### 游戏模式灯效

```c
[00:10:50.252][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_LOW_LANTECY
```

- 进入时开启了某种灯效，退出时一定要还原到蓝牙连接灯效

### 超距离断开灯效

```c
中途单耳超距断开直到自动关机
耳机回连手机超时后，不会主动连接手机，除了手机手动回连
[00:08:03.629][LED_UI]MSG_FROM_BT_STACK----ui_bt_stack_msg_handler----BT_STATUS_FIRST_DISCONNECT
[00:08:03.652][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:08:17.686][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:08:31.712][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAGE_MODE
[00:10:09.884][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_BT_IN_PAIRING_MODE 
[00:13:09.906][LED_UI]MSG_FROM_APP----ui_app_msg_handler----APP_MSG_POWER_OFF
[00:13:12.740][LED_UI]MSG_FROM_PWM_LED----ui_pwm_led_msg_handler----LED_MSG_STATE_END
[00:13:12.741][LED_UI]LED_STATE_END: name = 9  
```

- `BT_STATUS_FIRST_DISCONNECT`会先有一个蓝牙断开
- `APP_MSG_BT_IN_PAGE_MODE`会覆盖蓝牙断开，而且超级快，几乎看不出问题
  - 这个设置为超距离断开回连灯效，开机后连接蓝牙之前这个case不会有灯效
  - 连接蓝牙之后，这个case有灯效。但是只会在超距离断开时出现，其他不会
- `APP_MSG_BT_IN_PAIRING_MODE`没有设置灯效，不会覆盖

### 充电状态相关灯效

#### 关机状态插入充电到充满

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

#### 充电拔出

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
