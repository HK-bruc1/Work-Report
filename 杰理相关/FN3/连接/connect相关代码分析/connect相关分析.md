# 经典蓝牙的完整过程

## 系统初始化与应用启动

从`app_main.c`中的`app_main`函数开始：

```c
void app_main()
{
    // ...系统初始化
    struct intent it;
    
    // 初始化意图结构体
    init_intent(&it);
    it.name = "earphone";
    it.action = ACTION_EARPHONE_MAIN;
    start_app(&it); // 启动耳机应用
}
```

## 耳机应用入口

`start_app`会调用应用的状态机函数。在`earphone.c`中，应用注册为：

```c
REGISTER_APPLICATION(app_earphone) = {
    .name    = "earphone",
    .action  = ACTION_EARPHONE_MAIN,
    .ops     = &app_earphone_ops,
    .state   = APP_STA_DESTROY,
};
```

其中`app_earphone_ops`定义了状态机函数：

```c
static const struct application_operation app_earphone_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};
```

## 蓝牙初始化过程

在`state_machine`函数中，处理`APP_STA_START`状态：

```c
case APP_STA_START:
    if (!it) {
        break;
    }
    switch (it->action) {
    case ACTION_EARPHONE_MAIN:
        // 设置系统时钟
        clk_set("sys", BT_NORMAL_HZ);
        u32 sys_clk = clk_get("sys");
        bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);
        
        // 蓝牙功能初始化
        bt_function_select_init(); 
        bredr_handle_register();
        EARPHONE_STATE_INIT();
        DHFAppCommand_init();
        btstack_init();
        
        // 设置蓝牙版本
        set_bt_version(BLUETOOTH_CORE_SPEC_54);//不可见，跳转是直接填写宏定义的
        
        #if TCFG_USER_TWS_ENABLE
        tws_profile_init();
        sys_key_event_filter_disable();
        #endif
        
        // 系统功能初始化
        sys_key_event_enable();
        sys_auto_shut_down_enable();
        bt_sniff_feature_init();
        sys_auto_sniff_controle(MY_SNIFF_EN, NULL);
        break;
    // ...其他情况处理
    }
```

## 蓝牙功能选择初始化

此函数在蓝牙模块初始化阶段（如设备启动或模式切换）被调用，负责动态配置蓝牙协议栈的关键参数，确保设备在不同硬件平台、编译选项和使用场景下能够正常运行。其核心目标是：

1. **适配硬件差异**（如芯片型号、MAC 地址分配）。
2. **优化性能与功耗**（如 QoS、增强电源控制）。
3. **支持多协议功能**（如 BLE、mSBC/AAC 编码）。
4. **提供调试与测试能力**（如电量上报、SDK 版本获取）。

通过条件编译（如 `#if TCFG_USER_BLE_ENABLE`）实现功能开关，确保代码灵活性与可移植性。

`bt_function_select_init`设置蓝牙参数：

```c
void bt_function_select_init()
{
    // 设置蓝牙连接参数
    __set_user_ctrl_conn_num(TCFG_BD_NUM);//设置最大连接设备数量（TCFG_BD_NUM定义）
    __set_support_msbc_flag(1);//启用mSBC编码
    __set_aac_bitrate(131 * 1000);//设置AAC编码比特率为131kbps
    __set_support_aac_flag(1);//强制启用AAC音频编码支持
    
    // 设置蓝牙连接超时参数
    __set_page_timeout_value(8000);//设置蓝牙页面超时时间为8000ms（回连搜索时间）
    __set_super_timeout_value(8000);// 设置超级定时器超时时间为8000ms（主机连接超时参数）
    
    // 配对参数设置
    __set_simple_pair_param(3, 0, 2);//设置简单配对参数
    
    // 其他BLE相关初始化
    #if TCFG_USER_BLE_ENABLE
    {
        u8 tmp_ble_addr[6];
        memcpy(tmp_ble_addr, (void *)bt_get_mac_addr(), 6);
        le_controller_set_mac((void *)tmp_ble_addr);
    }
    #endif
    
    // 设置增强功率控制
    set_bt_enhanced_power_control(1);// 启用增强型电源控制（LE Enhanced Power Control）
    
    // 设置延迟报告时间
    set_delay_report_time(CONFIG_A2DP_DELAY_TIME);//设置A2DP媒体数据延迟报告时间
}
```

- 设置蓝牙连接参数
  - 设置最大连接设备数量（**宏定义**）
- 设置蓝牙连接超时参数
  - 设置蓝牙页面超时时间为8000ms（回连搜索时间）
  - 设置超级定时器超时时间为8000ms（主机连接超时参数）
    - **主机回连也会超时？？？不是什么都是都会回连从机吗？？？**
- 配对参数设置

## 蓝牙事件处理注册

从 `bredr_handle_register` 的现有代码看，它注册的是**上层业务逻辑的回调**，而非底层事件处理函数：

```c
// 注册SPP数据处理回调（仅限SPP Profile）
spp_data_deal_handle_register(...);

// 注册快速测试接口（测试盒专用）
bt_fast_test_handle_register(...);

// 音量同步回调（音乐/通话音量联动）
music_vol_change_handle_register(...);

// 电量显示回调（手机端显示电量）
get_battery_value_register(...);

// DUT模式处理函数（产线测试用）
bt_dut_test_handle_register(...);
```

这些是**特定功能模块的回调**，不涉及蓝牙协议栈事件（如连接、配对、HCI状态）。

~~`bredr_handle_register`注册蓝牙事件处理函数：~~

```c
void bredr_handle_register()
{
    // 注册HCI事件处理函数
    __set_user_hci_event_handler(bt_hci_event_handler);
    
    // 注册SPP数据处理函数
    __set_user_spp_data_handler(user_spp_data_handler);
    
    // 注册其他事件处理函数
    spp_data_deal_handle_register(user_spp_data_handler);
    // ...
}
```

### 蓝牙事件处理的实际位置

在 `earphone.c` 中，蓝牙事件的处理由以下几个关键函数完成：

- 主事件分发函数：`event_handler`

这是整个应用层的系统事件处理入口，负责将不同类型的事件（按键、蓝牙、设备等）分发到对应的处理函数。

- **文件位置**：`c:\Users\admin\Desktop\工作\FN3\apps\earphone\earphone.c`
- **行号范围**：L2446 - L2640
- 作用：
  - 处理按键事件 (`SYS_KEY_EVENT`)
  - 处理蓝牙事件 (`SYS_BT_EVENT`)，包括连接状态变化和底层HCI事件
  - 处理设备事件 (`SYS_DEVICE_EVENT`)，如充电、电源、TWS、耳检测等
  - 处理红外传感器事件 (`SYS_IR_EVENT`)
  - 处理PB消息事件 (`SYS_PBG_EVENT`)

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
        if (bt_in_background()) {
            break;
        }
        app_earphone_key_event_handler(event);
        break;

    case SYS_BT_EVENT:
        // 蓝牙事件处理
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            bt_hci_event_handler(&event->u.bt);
        }
#if TCFG_ADSP_UART_ENABLE
        else if (!strcmp(event->arg, "UART")) {
            extern void bt_uart_command_handler(struct uart_event * event);
            bt_uart_command_handler(&event->u.uart);
        } else if (!strcmp(event->arg, "UART_CMD")) {
            extern void adsp_uart_command_event_handle(struct uart_cmd_event * uart_cmd);
            adsp_uart_command_event_handle(&event->u.uart_cmd);
        }
#endif
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            bt_tws_connction_status_event_handler(&event->u.bt);
        }
#endif
        break;

    case SYS_DEVICE_EVENT:
        // 设备相关事件处理
        if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            return app_charge_event_handler(&event->u.dev);
        }
        ...
        break;

    default:
        return false;
    }

    SYS_EVENT_HANDLER_SPECIFIC(event); // 可能调用模块特定处理
#ifdef CONFIG_BT_BACKGROUND_ENABLE
    if (app) {
        default_event_handler(event);
    }
#endif
    return false;
}
```

**蓝牙连接状态事件处理：`bt_connction_status_event_handler`**

- **功能**：处理蓝牙连接状态的变化，如连接成功、断开、来电、通话开始/结束等。
- **文件位置**：`earphone.c`（定义在别处，通过头文件引用）
- **调用点**：在 `event_handler` 中当事件类型为 `SYS_BT_EVENT_TYPE_CON_STATUS` 时被调用。

**HCI 底层事件处理：`bt_hci_event_handler`**

- **功能**：处理蓝牙协议栈底层的 HCI 事件，如配对请求、页面超时、连接失败等。
- **文件位置**：`earphone.c`（定义在别处，通过头文件引用）
- **调用点**：在 `event_handler`中当事件类型为`SYS_BT_EVENT_TYPE_HCI_STATUS`时被调用。

**TWS 对耳事件处理：`bt_tws_connction_status_event_handler`**

- **功能**：处理与对耳设备相关的 TWS 事件，如连接、断开、角色切换等。
- **文件位置**：`earphone.c`（定义在别处，通过头文件引用）
- **调用点**：在 `event_handler`中当事件类型为 `SYS_BT_EVENT_FROM_TWS`时被调用。

这些函数构成了蓝牙事件处理的核心流程。如果你需要扩展或修改蓝牙行为（如新增配对逻辑、修改连接策略），应重点查看并修改这几个函数。

## 蓝牙协议栈初始化

`btstack_init`启动蓝牙协议栈，并开始运行蓝牙任务。这会最终触发蓝牙初始化完成事件。

- **看不到源码**

`include_lib\btstack\btstack_task.h`

```c
/**
 * @brief 蓝牙协议栈核心初始化
 * 该函数负责：
 * 1. 初始化蓝牙协议栈核心数据结构
 * 2. 创建蓝牙任务/线程处理协议栈事件
 * 3. 初始化HCI接口（UART/USB等物理接口）
 * 4. 注册协议栈事件回调函数
 * 5. 加载蓝牙配置参数（如MAC地址、安全数据库等）
 * @return 成功返回0，失败返回错误码
 */
int btstack_init();

/**
 * @brief 蓝牙协议栈资源释放
 * 该函数负责：
 * 1. 停止所有蓝牙协议栈线程/任务
 * 2. 释放动态分配的内存资源
 * 3. 关闭并释放HCI接口
 * 4. 清理协议栈状态机
 * 5. 保存必要的持久化配置数据
 * @return 成功返回0，失败返回错误码
 */
int btstack_exit();

/**
 * @brief BQB认证测试专用线程初始化
 * 该函数用于：
 * 1. 创建专门处理BQB认证测试的线程
 * 2. 初始化测试专用的GATT服务和特征值
 * 3. 注册测试模式下的事件处理回调
 * 4. 配置协议分析仪需要的调试接口
 * 5. 设置测试所需的特殊协议参数
 */
void ble_bqb_test_thread_init(void);
```

这些函数在蓝牙协议栈中的典型调用场景：

1. **btstack_init()** 在系统启动时调用，完成蓝牙协议栈的初始化
2. **ble_bqb_test_thread_init()** 仅在工程编译为BQB认证测试模式时调用，用于创建测试专用线程
3. **btstack_exit()** 在系统关机或蓝牙功能关闭时调用，确保资源正确释放

## 蓝牙初始化完成事件处理

当蓝牙初始化完成后，会触发`BT_STATUS_INIT_OK`事件，在`bt_connction_status_event_handler`函数中处理：

```c
case BT_STATUS_INIT_OK:
    log_info("BT_STATUS_INIT_OK\n");
    
    // 初始化按键
    EARPHONE_CUSTOM_EARPHONE_KEY_INIT();
    
    // 设置SBC编码参数
    __set_sbc_cap_bitpool(38);
    
    // 初始化BLE
    #if (TCFG_USER_BLE_ENABLE)
    bt_ble_init();
    #endif
    
    // 初始化搜索索引
    bt_init_ok_search_index();
    ui_update_status(STATUS_BT_INIT_OK);
    
    // 根据条件决定连接模式
    if (is_dac_power_off()) {
        #if TCFG_USER_TWS_ENABLE
        bt_tws_poweron();
        #else
        bt_wait_connect_and_phone_connect_switch(0);
        #endif
    } else {
        app_var.wait_timer_do = sys_timeout_add(NULL, play_poweron_ok_timer, 100);
    }
    break;
```

1. **按键初始化**`EARPHONE_CUSTOM_EARPHONE_KEY_INIT();`
   - 初始化耳机的物理按键功能，可能涉及按键事件注册、防抖设置等。
2. **DUT模式设置**`#if TCFG_NORMAL_SET_DUT_MODE`
   - 如果启用`TCFG_NORMAL_SET_DUT_MODE`，进入DUT（Device Under Test）模式：
     - `bredr_set_dut_enble(1, 1);`：启用BR/EDR模式的DUT测试功能。
     - `ble_standard_dut_test_init();`：初始化BLE标准DUT测试环境。
3. **音频数据导出功能**`#if TCFG_AUDIO_DATA_EXPORT_ENABLE`
   - 如果启用音频数据导出功能：
     - `lmp_hci_set_role_switch_supported(0)`：禁用角色切换（仅在SPP导出模式下需要）。
     - `audio_capture_init();`：初始化音频捕获模块，用于将音频数据通过SPP或其它接口导出。
4. **ANC（主动降噪）初始化**`#if TCFG_AUDIO_ANC_ENABLE`
   - 启动ANC模块电源：`anc_poweron();`。
5. **SBC编码参数设置**`__set_sbc_cap_bitpool(38);`
   - 设置SBC编码的位池参数为38，优化音频传输质量。
6. **BLE模块初始化**`#if TCFG_USER_BLE_ENABLE`
   - 如果启用BLE支持：
     - 在BQB测试模式下初始化BLE BQB测试线程。
     - 在普通模式下初始化BLE协议栈：`bt_ble_init();`。
7. **TWS（True Wireless Stereo）初始化**`#if TCFG_USER_TWS_ENABLE`
   
   - 初始化TWS功能：`pbg_demo_init();`，用于双耳同步连接。
8. **搜索索引与UI更新**
   
   - `bt_init_ok_search_index();`：初始化蓝牙设备搜索索引，用于快速回连历史设备。
   - `ui_update_status(STATUS_BT_INIT_OK);`：更新用户界面状态为“蓝牙初始化完成”。
   - `chargestore_set_bt_init_ok(1);`：通知充电仓模块蓝牙已初始化。
9. **连接模式决策**
   
   - 如果处于BQB或PER测试模式：`bt_wait_phone_connect_control(1);`开启可发现可连接状态。
   - 否则根据DAC电源状态决定连接策略：
     - DAC关闭时：进入TWS模式或直接回连设备。
     - DAC开启时：延迟播放启动提示音（通过`sys_timeout_add`调用 `play_poweron_ok_timer`）。
     
   - 策略解析：
     
     1. **测试模式** (`BT_BQB`/`BT_PER`)：
        - 直接开启可发现/可连接状态，用于测试场景
     2. **正常模式**：
        - **DAC电源关闭时**：
          - 双耳模式使用TWS专用连接流程
          - 单耳模式直接发起主动连接
        - **DAC电源工作时**：
          - 延迟100ms后执行连接操作，主要考虑：
            - 需等待DAC电源稳定
            - 给用户更流畅的连接体验（如播放开机提示音后再连接）
     3. **策略优势**：
        - 智能选择连接时机，兼顾电源管理和用户体验
        - 支持单双耳模式自动适配
        - 测试模式与正常模式分离，保证测试准确性
     
     这种设计既满足了测试需求，又能根据不同硬件配置（是否支持TWS）和系统状态（DAC电源状态）智能选择最合适的连接方式。
10. **开机提示音（可选）**
    - 注释代码段显示可通过`tone_play_index` 播放开机提示音，但实际功能被条件编译控制。

### 关键函数说明

- `EARPHONE_CUSTOM_EARPHONE_KEY_INIT()`定义在定制头文件中，负责按键硬件配置。
- `bt_ble_init()`：初始化BLE协议栈，包括广播、连接等核心功能。
- `pbg_demo_init()`：TWS配对/连接演示模块初始化。
- `anc_poweron()`：ANC模块上电及初始化流程。

### 配置宏影响

- `TCFG_AUDIO_DATA_EXPORT_ENABLE`启用音频数据导出功能（如调试或测试）。
- `TCFG_USER_BLE_ENABLE`启用BLE功能，支持BLE连接与广播。
- `TCFG_USER_TWS_ENABLE`启用TWS双耳连接功能，实现主从耳同步。

此分支完整实现了蓝牙设备从协议栈初始化到功能模块配置的全流程，确保设备进入可连接、可交互的就绪状态。

## 等待连接模式

**在哪里被调用？？？**

- play_poweron_ok_timer
- bt_connction_status_event_handler
- bt_hci_event_connection_timeout
- bt_hci_event_connection_exist

在`bt_wait_connect_and_phone_connect_switch`函数中设置等待连接模式：

```c
int bt_wait_connect_and_phone_connect_switch(void *p)
{
    int ret = 0;
    int timeout = 0;
    s32 wait_timeout;

    // 当自动连接计时器归零时，结束连接尝试流程
    if (bt_user_priv_var.auto_connection_counter <= 0) {
        bt_user_priv_var.auto_connection_timer = 0;
        bt_user_priv_var.auto_connection_counter = 0;

        // 取消蓝牙页面扫描（停止寻找新设备）
        EARPHONE_STATE_CANCEL_PAGE_SCAN();
        log_info("auto_connection_counter = 0\n");
        // 发送取消页面扫描命令
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);

        // 如果存在开机记忆的设备索引
        if (get_current_poweron_memory_search_index(NULL)) {
            // 重新初始化设备搜索索引
            bt_init_ok_search_index();
            // 递归调用自身进行下一台设备连接尝试
            return bt_wait_connect_and_phone_connect_switch(0);
        } else {
            // 进入可发现/可连接状态，等待新设备连接
            bt_wait_phone_connect_control(1);
            return 0;
        }
    }

    // 根据参数p决定连接模式：
    // p=0: 主动连接记忆设备；p!=0: 被动等待新连接
    if ((int)p == 0) {
        if (bt_user_priv_var.auto_connection_counter) {
            // 主动连接模式：设置较长的超时时间
            timeout = 5600;
            // 关闭可发现/可连接状态（专注连接指定设备）
            bt_wait_phone_connect_control(0);
            // 发送通过地址连接指定设备的命令
            user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 
                                 6, bt_user_priv_var.auto_connection_addr);
            ret = 1; // 返回1表示已发起连接请求
        }
    } else {
        // 被动连接模式：设置较短的超时时间
        timeout = 400;
        // 取消当前页面扫描
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
        // 启用可发现/可连接状态（进入配对模式）
        bt_wait_phone_connect_control(1);
    }

    // 更新剩余自动连接时间
    if (bt_user_priv_var.auto_connection_counter) {
        bt_user_priv_var.auto_connection_counter -= timeout;
        log_info("do=%d\n", bt_user_priv_var.auto_connection_counter);
    }

    // 设置新的超时定时器，再次调用本函数形成循环
    // 参数说明：(!(int)p)用于区分模式（0:主动连接 1:被动等待）
    bt_user_priv_var.auto_connection_timer = sys_timeout_add(
        (void *)(!(int)p),
        bt_wait_connect_and_phone_connect_switch, 
        timeout);

    return ret;
}
```

[bt_wait_connect_and_phone_connect_switch](javascript:void(0)) 是蓝牙连接管理的核心函数，主要负责以下功能：

1. **自动连接记忆设备**
   - 当参数 `p=0` 时，主动发起连接存储的设备地址（通过 [USER_CTRL_START_CONNEC_VIA_ADDR](javascript:void(0)) 命令）
   - 使用 5600ms 超时机制，持续尝试连接直到超时
   - 连接成功后通过 `ret=1` 返回状态
2. **被动等待新连接**
   - 当参数 `p≠0` 时，进入可发现模式等待新设备连接
   - 使用 400ms 短超时快速响应新连接请求
   - 通过 [bt_wait_phone_connect_control(1)](javascript:void(0)) 启用设备可发现性
3. **超时重试机制**
   - 使用 [sys_timeout_add](javascript:void(0)) 创建递归定时器，实现连接重试循环
   - 主动连接模式传递 `!(int)p` 标志保持连续尝试
   - 被动模式传递 `!(int)p` 标志维持低功耗监听
4. **多设备回连逻辑**
   - 当检测到开机记忆设备索引时，递归调用自身实现多设备回连（如双耳模式下的第二台设备连接）
   - 通过 [get_current_poweron_memory_search_index()](javascript:void(0)) 判断记忆设备存在性
5. **状态清理**
   - 当计时器归零时：
     - 取消页面扫描（[EARPHONE_STATE_CANCEL_PAGE_SCAN](javascript:void(0))）
     - 若有记忆设备则重新初始化搜索索引
     - 无记忆设备时进入可发现状态等待新连接

该函数是蓝牙连接流程的关键控制单元，协调着设备开机后的自动回连、连接失败后的重试、以及新设备配对的切换，是实现蓝牙设备稳定连接的核心逻辑。

**通过定时器实现循环执行机制**

1. **循环触发点**

```c
// ... existing code ...
bt_user_priv_var.auto_connection_timer = sys_timeout_add(
    (void *)(!(int)p),
    bt_wait_connect_and_phone_connect_switch, 
    timeout);
```

2. **循环终止条件**

- 当 `auto_connection_counter <= 0` 时：

```c
if (bt_user_priv_var.auto_connection_counter <= 0) {
    bt_user_priv_var.auto_connection_timer = 0;
    bt_user_priv_var.auto_connection_counter = 0;
    // ... existing code ...
}
```

3. **循环模式切换**

   - 主动连接模式：持续尝试记忆设备连接（p=0）

   - 被动等待模式：快速轮询新设备（p≠0）

4. **递归执行逻辑** 通过 [sys_timeout_add](javascript:void(0)) 实现异步递归调用，每次执行后：

```c
bt_user_priv_var.auto_connection_counter -= timeout;
// ... existing code ...
```

5. **特殊退出路径** 当检测到记忆设备存在时：

```c
if (get_current_poweron_memory_search_index(NULL)) {
    return bt_wait_connect_and_phone_connect_switch(0); // 新的递归调用
}
```

这种设计实现了：

- 开机时持续尝试记忆设备连接
- 连接失败后自动切换到可发现模式
- 支持多设备回连场景
- 动态调整超时时间（5600ms vs 400ms）

### 关键函数关联说明

- `bt_init_ok_search_index()`
  - 初始化蓝牙设备连接搜索索引，用于管理多设备连接顺序

- `user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL)`
  - 停止蓝牙页面扫描（终止设备发现过程）

- `bt_wait_phone_connect_control(0/1)`
  - 参数0：关闭可发现/可连接状态
  
  - 参数1：开启可发现/可连接状态
  
- `sys_timeout_add()`
  - 创建定时器，实现周期性的连接尝试

### 功能总结

该函数实现了蓝牙设备的**智能连接调度机制**，包含两个核心模式：

1. **主动连接模式**（p=0）：
   - 按预设地址连接历史设备
   - 使用5600ms超时重试机制
   - 连接失败时自动切换到下一个记忆设备
2. **被动等待模式**（p≠0）：
   - 提供400ms的快速连接窗口
   - 进入可发现状态等待新设备连接
   - 用于设备首次配对或所有记忆设备连接失败后

通过递归调用和定时器，形成了一个**动态连接状态机**，在主动连接和被动等待模式之间智能切换，确保设备既能快速恢复历史连接，又能及时响应新设备配对需求。

## 蓝牙连接事件处理

当手机发起连接请求时，会触发HCI事件，在`bt_hci_event_handler`函数中处理：

```c
int bt_hci_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case HCI_EVENT_CONNECTION_REQUEST:
        // 接受连接请求
        user_send_cmd_prepare(USER_CTRL_ACCEPT_CONN, 1, bt->args);
        break;
    
    case HCI_EVENT_CONNECTION_COMPLETE:
        // 连接完成事件处理
        bt_hci_event_connection(bt);
        break;
    
    // ...其他事件处理
    }
    
    // 处理连接状态事件
    bt_connction_status_event_handler(bt);
    
    return 0;
}
```

[bt_hci_event_handler](javascript:void(0)) 函数是蓝牙协议栈的底层事件处理函数，主要负责处理蓝牙主机控制器接口（HCI）触发的各种事件，其核心功能包括：

**厂商自定义事件处理**

- 远程测试事件
  - HCI_EVENT_VENDOR_REMOTE_TEST:
    - 当设备因远端测试断开连接时，清除测试状态并复位系统（用于产测场景）。
    - 在经典蓝牙模式下连接时，禁用BLE广播；非TWS连接时关闭TWS功能。

**标准HCI事件处理**

通过 `switch` 分支处理以下核心事件：

- 元事件
  - [HCI_EVENT_VENDOR_META](javascript:void(0)): 调用厂商自定义事件处理函数 `bt_vendor_meta_event_handle`（如耳检测测试指令）。
- 设备发现
  - [HCI_EVENT_INQUIRY_COMPLETE](javascript:void(0)): 查询设备完成后的处理（如关闭发射器搜索）。
- 配对交互
  - `HCI_EVENT_USER_CONFIRMATION_REQUEST/PIN_CODE_REQUEST`: 自动接受配对请求（无需用户交互）。
- 连接状态管理
  - HCI_EVENT_CONNECTION_COMPLETE: 处理连接完成事件（成功或失败）。
    - 成功（[ERROR_CODE_SUCCESS](javascript:void(0))）：记录连接地址并触发连接状态更新。
    - 超时（[ERROR_CODE_CONNECTION_TIMEOUT](javascript:void(0))）：标记远端断开并尝试重连。
  - [HCI_EVENT_DISCONNECTION_COMPLETE](javascript:void(0)): 断开连接时更新设备状态（如切换LED模式、进入低功耗）。
- 错误处理
  - 处理配对失败、连接拒绝、资源不足等错误码（如 [ERROR_CODE_PIN_OR_KEY_MISSING](javascript:void(0))）。

**关键逻辑说明**

- 自动连接机制
  - 在连接成功后，通过 `user_conn_addr` 记录设备地址，支持后续自动重连（[USER_AUTO_CONN_A2DP](javascript:void(0)) 宏启用时生效）。
- 低功耗控制
  - 在断开连接或超时后，调用 [sys_auto_shut_down_enable()](javascript:void(0)) 启动自动关机定时器。
- TWS同步
  - 在多设备场景中，通过 [bt_tws_poweroff()](javascript:void(0)) 和 [bt_tws_phone_disconnected()](javascript:void(0)) 维护双耳同步状态。

**核心作用**

该函数是蓝牙设备连接管理的核心逻辑，负责：

1. 响应手机/设备的连接请求。
2. 处理配对、连接、断开等底层协议交互。
3. 维护设备状态（如低功耗、TWS同步）。
4. 错误恢复（如超时重试、自动重连）。

此函数直接影响蓝牙设备的连接稳定性与用户体验（如自动配对、断开提示音播放等）。

```c
case ERROR_CODE_SUCCESS:
            // 连接成功 - 记录设备地址
            #if USER_AUTO_CONN_A2DP
            memcpy(user_conn_addr, bt->args, 6);
            #endif
            testbox_in_ear_detect_test_flag_set(0);
            bt_hci_event_connection(bt);
            break;
```

## 连接完成事件处理

在`bt_hci_event_connection`中处理连接完成事件：

```c
/**
 * 连接完成事件处理函数 - 处理蓝牙设备连接完成后的协议栈交互
 * 参数:
 *   bt - 指向蓝牙事件结构体的指针
 * 功能描述:
 *   1. 维护TWS双耳同步状态
 *   2. 清理自动连接定时器
 *   3. 退出Sniff低功耗模式
 *   4. 管理设备发现/连接使能状态
 */
static void bt_hci_event_connection(struct bt_event *bt)
{
    //【调试信息】记录当前TWS连接状态（主从角色、连接标志位）
    log_info("tws_conn_state=%d\n", bt_user_priv_var.tws_conn_state);

#if TCFG_USER_TWS_ENABLE
    //【TWS模式】通知TWS模块连接建立事件
    bt_tws_hci_event_connect();

#ifndef CONFIG_NEW_BREDR_ENABLE
    //【传统EDR模式】关闭TWS自动重连机制（防止连接后重复触发）
    tws_try_connect_disable();
#endif
#else
    //【单耳模式】清理自动连接资源
    if (bt_user_priv_var.auto_connection_timer) {
        // 删除自动连接定时器
        sys_timeout_del(bt_user_priv_var.auto_connection_timer);
        bt_user_priv_var.auto_connection_timer = 0;
    }
    // 重置自动连接计数器
    bt_user_priv_var.auto_connection_counter = 0;
    
    // 关闭手机的可发现/可连接状态（连接完成后停止广播）
    bt_wait_phone_connect_control(0);
    
    // 退出Sniff低功耗模式（保持正常通信状态）
    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
#endif

}
```

1. **TWS模式处理**
   - 通过[bt_tws_hci_event_connect()](javascript:void(0))同步双耳连接状态
   - 在传统EDR模式下禁用[tws_try_connect_disable()](javascript:void(0))防止重复连接
2. **单耳模式资源管理**
   - 清理自动连接定时器与计数器
   - 关闭设备发现/连接使能状态
   - 强制退出Sniff低功耗模式
3. **系统状态维护**
   - 记录调试日志用于连接状态追踪
   - 确保连接建立后维持稳定通信链路

注释清晰解释了不同编译配置下的差异化处理逻辑，以及连接完成后的协议栈状态维护机制。

## 连接状态事件处理

- `bt_connction_status_event_handler`

当设备连接状态变化时，会触发`BT_STATUS_FIRST_CONNECTED`或`BT_STATUS_SECOND_CONNECTED`事件：

```c
case BT_STATUS_SECOND_CONNECTED:
    clear_current_poweron_memory_search_index(0);
case BT_STATUS_FIRST_CONNECTED:
    log_info("BT_STATUS_CONNECTED\n");
    
    // 调整电源模式
    earphone_change_pwr_mode(PWR_DCDC15, 3000);
    
    // 禁用自动关机
    sys_auto_shut_down_disable();
    
    // 更新UI状态
    #if TCFG_USER_TWS_ENABLE
    if (!get_bt_tws_connect_status()) {
        ui_update_status(STATUS_BT_CONN);
    }
    #else
    ui_update_status(STATUS_BT_CONN);
    #endif
    
    // 更新耳机状态
    EARPHONE_STATE_BT_CONNECTED();
    
    // 播放连接提示音
    tone_play_index(p_tone->bt_connect, 1);
    break;
```

已为蓝牙连接状态处理分支添加专业注释，重点说明：

1. **双设备管理逻辑**
   - 通过[clear_current_poweron_memory_search_index](javascript:void(0))维护开机记忆索引
   - 使用[get_current_poweron_memory_search_index](javascript:void(0))控制多设备连接策略
2. **TWS同步机制**
   - [tws_conn_switch_role](javascript:void(0))实现主从角色切换
   - [bt_tws_phone_connected](javascript:void(0))判断对耳连接状态
3. **BLE图标控制**
   - 根据[get_auto_connect_state](javascript:void(0))区分首次连接与重连场景
   - 使用[ICON_TYPE_RECONNECT/ICON_TYPE_CONNECTED](javascript:void(0))显示不同连接状态
4. **电源管理**
   - [earphone_change_pwr_mode](javascript:void(0))切换到高效供电模式
   - [sys_auto_shut_down_disable](javascript:void(0))防止连接状态下自动关机
5. **用户提示**
   - 通过[play_bt_connect_dly](javascript:void(0))实现延迟播放连接提示音

注释清晰解释了在[BT_STATUS_FIRST_CONNECTED](javascript:void(0))和[BT_STATUS_SECOND_CONNECTED](javascript:void(0))事件中，系统如何协调TWS双耳状态、管理BLE广播、控制电源模式以及提供用户反馈的完整处理流程。

## 连接断开处理

当蓝牙连接断开时，会触发`HCI_EVENT_DISCONNECTION_COMPLETE`事件：

```c
	bt_hci_event_handler()
        
	case HCI_EVENT_VENDOR_NO_RECONN_ADDR:
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        // 连接断开事件处理
        bt_hci_event_disconnect(bt);
        break;


/*
 * 蓝牙连接断开事件处理函数 - 处理蓝牙连接断开后的协议栈行为
 * 参数:
 *   bt - 指向蓝牙事件结构体的指针，包含断开事件详细信息
 * 功能描述:
 * 1. 管理电源模式切换
 * 2. 更新UI状态指示
 * 3. 控制自动关机功能
 * 4. 处理测试盒模式下的特殊需求
 * 5. 管理页面扫描定时器
 * 6. 处理双设备连接时的回连逻辑
 */
static void bt_hci_event_disconnect(struct bt_event *bt)
{
    // 如果设备正在关机流程中，直接返回
    if (app_var.goto_poweroff_flag) {
        return;
    }

    // 断开连接后切换到LDO15电源模式（低功耗模式）
    // 参数0表示立即执行电源模式切换
    earphone_change_pwr_mode(PWR_LDO15, 0);

    // 获取当前连接设备数量
    log_info("<<<<<<<<<<<<<<total_dev: %d>>>>>>>>>>>>>\n", get_total_connect_dev());

    // 如果当前没有活动的通道连接
    if (!get_curr_channel_state()) {
        // 非TWS模式下处理
#if (TCFG_USER_TWS_ENABLE == 0)
        // 如果不是关机流程，更新UI为蓝牙断开状态
        if (!app_var.goto_poweroff_flag) {
            ui_update_status(STATUS_BT_DISCONN);
        }
#endif
        // 启用自动关机功能（若配置了自动关机时间）
        sys_auto_shut_down_enable();
    }

    // 双设备模式下的连接状态检查
#if (TCFG_BD_NUM == 2)
    log_info("get_bt_connect_status = 0x%x,%x\n", get_bt_connect_status(), get_curr_channel_state());
    // 如果当前没有活动通道连接
    if (!get_curr_channel_state()) {
        // 启用自动关机功能
        sys_auto_shut_down_enable();
    }
#endif

    // 测试盒模式特殊处理
#if TCFG_TEST_BOX_ENABLE
    // 如果处于测试盒特殊DUT模式
    extern u8 chargestore_get_ex_enter_dut_flag(void);
    if (chargestore_get_ex_enter_dut_flag()) {
        // 保持设备可发现/可连接状态
        bt_discovery_and_connectable_using_loca_mac_addr(1, 1);
        return;
    }

    // 如果处于测试盒模式
    if (chargestore_get_testbox_status()) {
        // 保持设备可连接但不可发现
        bt_discovery_and_connectable_using_loca_mac_addr(0, 1);
        return;
    }
#endif

    // 双设备模式下的页面扫描管理
#if (TCFG_AUTO_STOP_PAGE_SCAN_TIME && TCFG_BD_NUM == 2)
    // 当前有一台设备连接
    if (get_total_connect_dev() == 1) {
        // 创建定时器停止页面扫描
        if (app_var.auto_stop_page_scan_timer == 0) {
            app_var.auto_stop_page_scan_timer = sys_timeout_add(NULL, bt_close_page_scan, (TCFG_AUTO_STOP_PAGE_SCAN_TIME * 1000)); // 延迟TCFG_AUTO_STOP_PAGE_SCAN_TIME秒后停止页面扫描
        }
    } else {
        // 删除定时器
        if (app_var.auto_stop_page_scan_timer) {
            sys_timeout_del(app_var.auto_stop_page_scan_timer);
            app_var.auto_stop_page_scan_timer = 0;
        }
    }
#endif

    // 双设备模式错误处理
#if (TCFG_BD_NUM == 2)
    // 处理特定错误码导致的连接失败
    if ((bt->event == ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR) ||
        (bt->event == ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED) ||
        (bt->event == ERROR_CODE_ROLE_SWITCH_FAILED) ||
        (bt->event == ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS)) {
        /*
         * 连接接受超时等错误处理
         * 如果支持1t2连接，尝试回连下一台设备
         * get_current_poweron_memory_search_index()检查是否还有待连接设备
         */
        if (get_current_poweron_memory_search_index(NULL)) {
            // 准备重新连接下一台设备
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            return;
        }
    }
#endif

    // TWS模式下的特殊处理
#if TCFG_USER_TWS_ENABLE
    // 根据错误码类型调用TWS专用处理函数
    if (bt->value == ERROR_CODE_CONNECTION_TIMEOUT) {
        // 连接超时处理
        bt_tws_phone_connect_timeout();
    } else {
        // 普通断开连接处理
        bt_tws_phone_disconnected();
    }
#else
    // 非TWS模式下允许重新连接
    bt_wait_phone_connect_control(1);
#endif
}
```

1. 电源模式管理
   - 切换电源模式
   - 断开连接后调用 earphone_change_pwr_mode(PWR_LDO15, 0)，将设备切换至低功耗电源模式（LDO15），以降低功耗。

2. UI状态更新

   - 非TWS模式
     - 如果未启用 TWS（True Wireless Stereo）功能，且设备未处于关机流程，则通过 ui_update_status(STATUS_BT_DISCONN) 更新用户界面状态为“蓝牙断开”。

   - 双设备模式
     - 检查当前连接设备数量（get_total_connect_dev()）和通道状态（get_curr_channel_state()），若无活动连接则触发自动关机逻辑。

3. 自动关机控制
   - 启用自动关机
     - 若配置了自动关机时间（TCFG_AUTO_SHUT_DOWN_TIME），通过 sys_auto_shut_down_enable() 启动定时器，超时后执行关机操作。

4. 测试盒模式处理

- 特殊DUT模式
  - 若设备处于测试盒的特殊DUT模式（通过 chargestore_get_ex_enter_dut_flag() 判断），保持设备可发现且可连接状态（调用 bt_discovery_and_connectable_using_loca_mac_addr(1, 1)）。

- 普通测试盒模式
  - 若仅处于测试盒模式（非DUT），保持设备可连接但不可发现状态（调用 bt_discovery_and_connectable_using_loca_mac_addr(0, 1)）。

5. 页面扫描定时器管理

   - 双设备模式下的扫描控制

     - 若当前仅剩一台设备连接：

     - 创建定时器（sys_timeout_add），延迟 TCFG_AUTO_STOP_PAGE_SCAN_TIME 秒后关闭页面扫描（调用 bt_close_page_scan）。

   - 若无设备连接：
     - 删除定时器（sys_timeout_del），停止页面扫描管理。

6. 双设备回连逻辑
   - 错误码处理
     - 针对特定错误码（如连接拒绝、超时等），检查是否支持多设备回连（通过 get_current_poweron_memory_search_index() 判断），若存在待连接设备则触发重新连接（调用 user_send_cmd_prepare(USER_CTRL_START_CONNECTION)）。

7. TWS模式特殊处理

   - TWS专用逻辑

     - 若断开原因为连接超时（ERROR_CODE_CONNECTION_TIMEOUT），调用 bt_tws_phone_connect_timeout() 处理超时逻辑。

     - 其他断开情况调用 bt_tws_phone_disconnected() 更新TWS状态。

   - 非TWS模式
     - 调用 bt_wait_phone_connect_control(1) 允许设备重新进入可连接状态。

8. 其他功能

   - 日志记录

   - 输出当前连接设备数量（get_total_connect_dev()）和连接状态（get_bt_connect_status()）的调试信息。

**关键配置影响**

- 宏定义控制功能
  - [TCFG_USER_TWS_ENABLE](javascript:void(0))：启用/禁用TWS双耳同步逻辑。
  - [TCFG_TEST_BOX_ENABLE](javascript:void(0))：启用/禁用测试盒模式下的特殊处理。
  - `TCFG_BD_NUM == 2`：启用双设备连接管理逻辑。
  - [TCFG_AUTO_STOP_PAGE_SCAN_TIME](javascript:void(0))：控制页面扫描定时器行为。

该函数通过上述处理确保设备在蓝牙断开后的行为符合预期，包括低功耗管理、用户交互、自动重连策略及测试场景适配。

## 断开延迟处理与回连

- 谁来调用？
- 什么时候调用？

当断开连接一段时间后，执行回连操作：

```c
static void bt_discon_dly_handle(void *priv)
{
    // 清除延迟断开定时器
    app_var.phone_dly_discon_time = 0;

    STATUS *p_tone = get_tone_config();

    // 双设备模式下的断开处理
#if(TCFG_BD_NUM == 2)
    /* tone_play(TONE_DISCONN); */
    // 播放蓝牙断开提示音（单设备模式）
    tone_play_index(p_tone->bt_disconnect, 1);
#else

    // 单设备模式下：
    // 仅当TWS未连接或未处于延迟断开状态时播放提示音
#if TCFG_USER_TWS_ENABLE
    if (!get_bt_tws_connect_status() && !get_bt_tws_discon_dly_state())
#endif
    {
        tone_play_index(p_tone->bt_disconnect, 1);
    }
#endif

    // TWS相关处理
#if TCFG_USER_TWS_ENABLE
    STATUS *p_led = get_led_config();
    if (get_bt_tws_connect_status()) {
#if TCFG_CHARGESTORE_ENABLE
        // 通知充电盒手机已断开
        chargestore_set_phone_disconnect();
#endif
        // 主耳通过TWS同步播放断开提示音
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            bt_tws_play_tone_at_same_time(SYNC_TONE_PHONE_DISCONNECTED, 400);
        }
    } else {
        // 单耳模式下特殊处理：
        // 切换LED时钟到RC模式以降低功耗
        pwm_led_clk_set((!TCFG_LOWPOWER_BTOSC_DISABLE) ? PWM_LED_CLK_RC32K : PWM_LED_CLK_BTOSC_24M);
        // 更新UI状态为蓝牙断开
        ui_update_status(STATUS_BT_DISCONN);
    }
#endif

}
```

1. **清除延迟断开定时器**：停止与断开相关的定时器。
2. 播放提示音：
   - 在双设备模式下直接播放断开提示音。
   - 在单设备模式下根据 TWS 状态决定是否播放提示音。
3. TWS 相关处理：
   - 如果对耳仍连接，主耳通过 TWS 同步播放断开提示音。
   - 如果对耳未连接，更新 LED 时钟模式以降低功耗，并将 UI 状态更新为蓝牙断开。
4. **充电盒支持**：通知充电盒手机已断开。

该函数专注于断开后的清理和状态更新，并不直接参与回连逻辑。回连通常由其他函数（如 [bt_wait_connect_and_phone_connect_switch](javascript:void(0))）管理。

## 完整调用链小结

经典蓝牙的完整调用链：

系统启动: `main() → app_main() → start_app(&it)`

耳机初始化: `state_machine(APP_STA_START, ACTION_EARPHONE_MAIN)`

蓝牙功能初始化: `bt_function_select_init() → bredr_handle_register() → btstack_init()`

蓝牙初始化完成: `BT_STATUS_INIT_OK → bt_wait_connect_and_phone_connect_switch(0)`

等待连接: 开启可发现可连接 → `USER_CTRL_WRITE_SCAN_ENABLE + USER_CTRL_WRITE_CONN_ENABLE`

连接建立: `HCI_EVENT_CONNECTION_REQUEST → HCI_EVENT_CONNECTION_COMPLETE → bt_hci_event_connection()`

连接状态更新: `BT_STATUS_FIRST_CONNECTED → EARPHONE_STATE_BT_CONNECTED()`

连接断开: `HCI_EVENT_DISCONNECTION_COMPLETE → bt_hci_event_disconnect() → bt_discon_dly_handle()`

回连或搜索: `USER_CTRL_START_CONNEC_VIA_ADDR` 或 重新进入可被发现状态