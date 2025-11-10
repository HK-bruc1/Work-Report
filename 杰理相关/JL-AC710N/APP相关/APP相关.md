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

