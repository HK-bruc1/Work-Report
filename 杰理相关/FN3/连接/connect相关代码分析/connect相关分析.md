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

在`bt_wait_connect_and_phone_connect_switch`函数中设置等待连接模式：

```c
int bt_wait_connect_and_phone_connect_switch(void *p)
{
    // 检查是否在低功耗模式
    if (check_in_sniff_mode()) {
        return -EINVAL;
    }
    
    // 清除配对信息
    user_send_cmd_prepare(USER_CTRL_DEL_ALL_REMOTE_INFO, 0, NULL);
    
    if ((u32)p == 0) {
        // 尝试回连历史配对设备
        int ret = user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 0, NULL);
        if (ret == 0) { // 没有配对记录或连接失败
            // 开启可被发现和连接模式
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        }
    }
    return 0;
}
```

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

## 蓝牙连接事件处理？？？？？？？？？

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

## 连接完成事件处理

在`bt_hci_event_connection`中处理连接完成事件：

```c
void bt_hci_event_connection(struct bt_event *bt)
{
    bt_wait_phone_connect_control(0);
    check_a2dp_connect_status_before_esco();//samson added on 20250514,for anc
    samrtbox_stop_timer(0);
    
    u8 *addr = bt->args;
    
    // 修改系统状态
    sys_auto_shut_down_disable();
    
    #if (TCFG_USB_CDC_BACKGROUND_RUN && !TCFG_PC_ENABLE)
    usb_cdc_resume_deal();
    #endif
}
```

## 连接状态事件处理

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

## 连接断开处理

当蓝牙连接断开时，会触发`HCI_EVENT_DISCONNECTION_COMPLETE`事件：

```c
void bt_hci_event_disconnect(struct bt_event *bt)
{
    if (app_var.goto_poweroff_flag) {
        return;
    }
    
    u8 *addr = bt->args;
    
    // 更新耳机状态
    EARPHONE_STATE_BT_DISCONN();
    
    // 处理断开连接
    if ((!get_total_connect_dev()) && (!app_var.goto_poweroff_flag)) {
        ui_update_status(STATUS_BT_DISCONN);
        
        if (app_var.background_goback) {
            // 处理后台应用
        } else {
            // 延时处理断开连接
            if (app_var.phone_dly_discon_time) {
                sys_timeout_del(app_var.phone_dly_discon_time);
                app_var.phone_dly_discon_time = 0;
            }
            app_var.phone_dly_discon_time = sys_timeout_add((void *)addr, bt_discon_dly_handle, 250);
        }
    }
}
```

## 断开延迟处理与回连

当断开连接一段时间后，执行回连操作：

```c
void bt_discon_dly_handle(void *priv)
{
    int ret = 0;
    
    // 尝试回连
    ret = user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, priv);
    if (ret == 0) {
        // 回连失败，开启可发现可连接
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
    }
    
    if (get_call_status() == BT_CALL_HANGUP) {
        // 播放断开连接提示音
        tone_play_select(p_tone->bt_disconnect, 1);
    }
    
    // 启动自动关机计时
    sys_auto_shut_down_enable();
}
```

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