# APP发送切换游戏模式命令位置

- 按键进入游戏模式有灯效，但是APP指令进入却没有。

按键进入游戏模式的APP层消息处理：

- 灯效流程中也是基于这个`case`

```c
case APP_MSG_LOW_LANTECY:
    if (bt_get_low_latency_mode() == 0) {
        //不是低延迟模式就进入否则退出
        bt_enter_low_latency_mode();
    } else {
        bt_exit_low_latency_mode();
    }
    break;
```

APP软件直接切换游戏模式

- 走上面
- 但是只有按键处理才有这两个函数的调用，其他没有看到。
- 如果走`APP_MSG_LOW_LANTECY`肯定有灯效的。

```c
void bt_enter_low_latency_mode()
{
    puts("enter_low_latency\n");
#if RCSP_MODE && RCSP_ADV_WORK_SET_ENABLE
    rcsp_set_work_mode(RCSPWorkModeGame);
#else
    bt_set_low_latency_mode(1, 1, 300);
#endif
}

void bt_exit_low_latency_mode()
{
    puts("exit_low_latency\n");
#if RCSP_MODE && RCSP_ADV_WORK_SET_ENABLE
    rcsp_set_work_mode(RCSPWorkModeNormal);
#else
    bt_set_low_latency_mode(0, 1, 300);
#endif
}
```

## 耳机接收APP指令切换游戏模式（待分析）

```c
[00:01:05.229]JL_rcsp_adv_cmd_resp
[00:01:05.230] JL_OPCODE_SET_ADV
[00:01:05.230]JL_opcode_set_adv_info:
02 05 01 
[00:01:05.236]update_work_setting_state, false
[00:01:05.237]bt_set_low_latency_mode=0
[00:01:05.238]tone_player: tone_en/game_out.*
    
static void deal_work_setting(u8 *work_setting_info, u8 write_vm, u8 tws_sync)
{
    if (work_setting_info) {
        set_work_setting(work_setting_info);
        //printf("rcsp_work %s, %s, %d, work_setting:%d\n", __FILE__, __FUNCTION__, __LINE__, work_setting_info); */
    }
    if (write_vm) {
        adv_work_setting_vm_value(&g_work_mode);
    }
#if TCFG_USER_TWS_ENABLE
    if (tws_sync) {
        if (get_bt_tws_connect_status()) {
            update_rcsp_setting(ATTR_TYPE_WORK_MODE);
        }
    }
#endif
    update_work_setting_state();
}
static void update_work_setting_state(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status() && (tws_api_get_role() == TWS_ROLE_SLAVE)) {
        return;
    }
#endif
    if (RCSPWorkModeNormal == g_work_mode) {
        printf("%s, false\n", __FUNCTION__);
        bt_set_low_latency_mode(0, 1, 300);
    } else if (RCSPWorkModeGame == g_work_mode) {
        printf("%s, true\n", __FUNCTION__);
        bt_set_low_latency_mode(1, 1, 300);
    } else {
        printf("%s, set deal none\n", __FUNCTION__);
    }
}
static RCSP_SETTING_OPT adv_work_opt = {
    .data_len = 1,
    .setting_type = ATTR_TYPE_WORK_MODE,
    .syscfg_id = CFG_RCSP_ADV_WORK_SETTING,
    .deal_opt_setting = deal_work_setting,
    .set_setting = set_work_setting,
    .get_setting = get_work_setting,
    .custom_setting_init = adv_work_opt_init,
    .custom_vm_info_update = NULL,
    .custom_setting_update = NULL,
    .custom_sibling_setting_deal = NULL,
    .custom_setting_release = NULL,
    .set_setting_extra_handle = work_set_setting_extra_handle,
    .get_setting_extra_handle = work_get_setting_extra_handle,
};
```

## APP自定义按键进入游戏模式

```c
[00:11:48.194]RCSP:KEY_ACTION_CLICK---->RCSP_KEY_ACTION_CLICK/n
[00:11:48.194]RCSP:RCSP_ADV_KEY_TYPE_LOW---->APP_MSG_LOW_LANTECY/n
[00:11:48.195][APP]rcsp_key_remap: 43
[00:11:48.196]enter_low_latency
[00:11:48.200]update_work_setting_state, true
[00:11:48.201]bt_set_low_latency_mode=1
[00:11:48.201]tone_player: tone_en/game_in.*
```

## 实现

```c
static void update_game_led(void){
    led_ui_set_state(_LED_APP_MSG_LOW_LANTECY_NAME, _LED_APP_MSG_LOW_LANTECY_DISP_MODE);
}
static void update_normal_led(void){
    //还原蓝牙连接灯效
#if _LED_BT_STATUS_CONNECTED_LEFT_RIGHT
        //灯效分左右
        if ((channel == 'L' && msg[1] != APP_KEY_MSG_FROM_TWS) ||
            (channel == 'R' && msg[1] == APP_KEY_MSG_FROM_TWS)) {
            led_ui_set_state(_LED_BT_STATUS_CONNECTED_LEFT_NAME, _LED_BT_STATUS_CONNECTED_LEFT_DISP_MODE);  
        }else {
            led_ui_set_state(_LED_BT_STATUS_CONNECTED_RIGHT_NAME, _LED_BT_STATUS_CONNECTED_RIGHT_DISP_MODE);    
        }
#elif _LED_BT_STATUS_CONNECTED_MASTER_SLAVE
        //灯效分主从
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            led_ui_set_state(_LED_BT_STATUS_CONNECTED_MASTER_NAME, _LED_BT_STATUS_CONNECTED_MASTER_DISP_MODE);
        }else if(tws_api_get_role() == TWS_ROLE_SLAVE){
            led_ui_set_state(_LED_BT_STATUS_CONNECTED_SLAVE_NAME, _LED_BT_STATUS_CONNECTED_SLAVE_DISP_MODE);
        }
#else
        //默认双耳同步灯效
        led_ui_set_state(_LED_BT_STATUS_CONNECTED_NAME, _LED_BT_STATUS_CONNECTED_DISP_MODE);
#endif
}
static void update_work_setting_state(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status() && (tws_api_get_role() == TWS_ROLE_SLAVE)) {
        return;
    }
#endif
    if (RCSPWorkModeNormal == g_work_mode) {
        printf("%s, false\n", __FUNCTION__);
        //APP直接切换时不会走灯效流程，这里直接设置退出游戏模式灯效
        update_normal_led();
        bt_set_low_latency_mode(0, 1, 300);
    } else if (RCSPWorkModeGame == g_work_mode) {
        printf("%s, true\n", __FUNCTION__);
        //APP直接切换时不会走灯效流程，这里直接设置游戏模式灯效
        update_game_led();
        bt_set_low_latency_mode(1, 1, 300);
    } else {
        printf("%s, set deal none\n", __FUNCTION__);
    }
}
```

## 改进

灯效直接写在最底层函数，无论从那里进来都可以触发。

```c
    case APP_MSG_LOW_LANTECY:
        if (bt_get_low_latency_mode() == 0) {
            //不是低延迟模式就进入否则退出
            bt_enter_low_latency_mode();
        } else {
            bt_exit_low_latency_mode();
        }
        break;
static void update_work_setting_state(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status() && (tws_api_get_role() == TWS_ROLE_SLAVE)) {
        return;
    }
#endif
    if (RCSPWorkModeNormal == g_work_mode) {
        //APP直接切换时不会走灯效流程，这里直接设置退出游戏模式灯效
        //update_normal_led();直接写里面，APP与不带APP都可以通用
        bt_set_low_latency_mode(0, 1, 300);
    } else if (RCSPWorkModeGame == g_work_mode) {
        //APP直接切换时不会走灯效流程，这里直接设置游戏模式灯效
        //update_game_led();
        bt_set_low_latency_mode(1, 1, 300);
    } else {
        printf("%s, set deal none\n", __FUNCTION__);
    }
}
void bt_set_low_latency_mode(int enable, u8 tone_play_enable, int delay_ms)
{
    /*
     * 未连接手机,操作无效
     */
    int state = tws_api_get_tws_state();
    // if (state & TWS_STA_PHONE_DISCONNECTED) {
    //     return;
    // }
    //这里改为conf配置，更加灵活，虽然未连接蓝牙，游戏模式没有意义，播放个提示音
    //使用APP自然可以直接切换游戏模式

    //不用默认的led游戏模式流程直接在这里更新灯效，APP与普通都可以
    if(enable){
        //游戏模式灯效
        extern void update_game_led(void);
        update_game_led();
    }else {
        //退出游戏模式灯效
        extern void update_normal_led(void);
        update_normal_led();
    }

    const char *fname = enable ? get_tone_files()->low_latency_in :
                        get_tone_files()->low_latency_out;
    g_printf("bt_set_low_latency_mode=%d\n", enable);
#if TCFG_USER_TWS_ENABLE
    if (state & TWS_STA_SIBLING_CONNECTED) {
        if (delay_ms < 100) {
            delay_ms = 100;
        }
        tws_play_tone_file_alone_callback(fname, delay_ms, 0x6F90E37B);
    } else
#endif
    {
        play_tone_file_alone(fname);
        tws_api_low_latency_enable(enable);
        a2dp_player_low_latency_enable(enable);
    }
    if (enable) {
        if (bt_get_total_connect_dev()) {
            lmp_hci_write_scan_enable(0);
        }

    } else {
#if TCFG_USER_TWS_ENABLE
        tws_dual_conn_state_handler();
#endif
    }

#if (THIRD_PARTY_PROTOCOLS_SEL&TUYA_DEMO_EN)
        void tuya_game_mode_indicate(u8 status);
        tuya_game_mode_indicate(enable);
#endif

}
```

# adv_key_setting的调用链

`apps\common\third_party_profile\jieli\rcsp\server\functions\rcsp_setting_opt\settings\adv_key_setting.c`

- 单耳连接APP后

```c
[00:00:21.097]key_get_setting_extra_handle() 395
[00:00:21.098]key_get_setting_extra_handle : 5 5 3 4 12 12 10 9
进入一次key设置界面就会调用一次key_get_setting_extra_handle()
    
修改自定义按键设置
[00:01:28.792]key_set_setting_extra_handle() 357
[00:01:28.793]key_set_setting_extra_handle : 5 5 3 4 13 12 10 9
[00:01:28.794]deal_key_setting() 175
[00:01:28.795]get_key_setting() 124
[00:01:28.796]get_key_setting : 5 5 3 4 13 12 10 9
[00:01:28.797]update_key_setting_vm_value() 151
[00:01:28.802]VM Write AFTER : 5 5 3 4 13 12 10 9
[00:01:28.803]key_setting_sync() 162
[00:01:28.804]key_setting_sync : 5 5 3 4 13 12 10 9
[00:01:28.805]enable_adv_key_event() 80
 
返回key界面
[00:01:32.361]key_get_setting_extra_handle() 395
[00:01:32.362]key_get_setting_extra_handle : 5 5 3 4 13 12 10 9
```

- 双耳中的从机

```c
只有设置自定义按键时才会调用：
[00:03:49.365]set_key_setting() 98
[00:03:49.366]set_key_setting : 5 5 3 4 13 13 13 9
FE DC BA C0 C0 00 06 04 04 02 01 04 0D EF 
[00:03:49.368]deal_key_setting() 175
[00:03:49.369]get_key_setting() 124
[00:03:49.370]get_key_setting : 5 5 3 4 13 13 13 9
[00:03:49.371]update_key_setting_vm_value() 151
[00:03:49.377]VM Write AFTER : 5 5 3 4 13 13 13 9
[00:03:49.378]enable_adv_key_event() 80
```

## APP自定义按键空白问题

两只耳机都在VM同步保存了自定义按键数据，可能从副耳中读取或者主耳中读取，如果从副耳中读取按键缓存列表的话，那么就只有一半了。

```c
static RCSP_SETTING_OPT adv_key_opt = {
    .data_len = 8,//这里可能就涉及对耳数据传递大小了，也要一致。
    .setting_type = ATTR_TYPE_KEY_SETTING,
    .syscfg_id = CFG_RCSP_ADV_KEY_SETTING,
    .deal_opt_setting = deal_key_setting,
    .set_setting = set_key_setting,
    .get_setting = get_key_setting,
    .custom_setting_init = NULL,
    .custom_vm_info_update = NULL,
    .custom_setting_update = NULL,
    .custom_sibling_setting_deal = NULL,
    .custom_setting_release = NULL,
    .set_setting_extra_handle = key_set_setting_extra_handle,
    .get_setting_extra_handle = key_get_setting_extra_handle,
};
REGISTER_APP_SETTING_OPT(adv_key_opt);
```

# RCSP按键与普通按键映射流程

`apps\earphone\app_main.c`

```c
int app_get_message(int *msg, int max_num, const struct key_remap_table *key_table)
{
    const struct app_msg_handler *handler;

    app_core_get_message(msg, max_num);

    //消息截获,返回1表示中断消息分发
    for_each_app_msg_prob_handler(handler) {
        if (handler->from == msg[0]) {
            int abandon = handler->handler(msg + 1);
            if (abandon) {
                return 0;
            }
        }
    }
#if RCSP_ADV_KEY_SET_ENABLE
    if (msg[0] == MSG_FROM_KEY) {
        int _msg = rcsp_key_event_remap(msg + 1);
        //出来时已经能得到APP层的映射消息了
        if (_msg != -1) {
            msg[0] = MSG_FROM_APP;
            msg[1] = _msg;
            log_info("rcsp_key_remap: %d\n", _msg);
        }
    }
#endif

    if (msg[0] == MSG_FROM_KEY && key_table) {
        /*
         * 按键消息映射成当前模式的消息
         */
        struct app_mode *mode = app_get_current_mode();
        if (mode) {
#if TCFG_AUDIO_WIDE_AREA_TAP_ENABLE
            audio_wide_area_tap_ignore_flag_set(1, 1000);
#endif
            int key_msg = app_key_event_remap(key_table, msg + 1);
            log_info(">>>>>key_msg = %d\n", key_msg);
            if (key_msg == APP_MSG_NULL) {
                return 1;
            }
            msg[0] = MSG_FROM_APP;
            msg[1] = key_msg;
#if TCFG_APP_KEY_DUT_ENABLE
            app_key_dut_msg_handler(key_msg);
#endif
        }
    }

    return 1;
}


/**
 * rcsp按键配置转换
 *
 * @param value 按键功能
 * @param msg 按键消息
 *
 * @return 是否拦截消息
 */
int rcsp_key_event_remap(int *msg)
{
    y_printf("%s() %d", __func__, __LINE__);
#if _APP_KEY_CALL_ENABLE
    //通话状态下不进行拦截，因为APP中不涉及有关通话相关的自定义按键
    //避免出现来电时普通转换流程是单机接听，但是被RCSP拦截转换为了单击语音助手
    if((bt_get_call_status() == BT_CALL_INCOMING) || (bt_get_call_status() == BT_CALL_OUTGOING) || (bt_get_call_status() == BT_CALL_ACTIVE)){
        //此时不拦截，走普通按键转换APP层消息流程
        y_printf("RCSP检测到通话状态不拦截/n");
        return -1;
    }
#endif

    //参照普通按键流程，从机直接返回
    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        //这里从机是直接返回的，那么所有的按键消息都是在主耳处理的。
        return -1;
    }

    //当开启rcsp时，没有进入APP改过按键的话，也不会走RCSP流程！！！
    if (0 == get_adv_key_event_status()) {
        y_printf("disable_adv_key_event/n");
        return -1;
    }
```



