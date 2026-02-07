# 修改RCSP的广播数据形成凭证

`apps\common\third_party_profile\jieli\rcsp\adv\ble_rcsp_adv.c`

```c
int rcsp_make_set_adv_data(void)
{
    u8 i;
    u8 *buf = adv_data;
    buf[0] = 0x1E;
    buf[1] = 0xFF;

    buf[2] = 0xD6;	// JL ID
    buf[3] = 0x05;

#if !(_THIRD_APP_ON_RCSP_ENABLE)
    u16 vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
    buf[4] = vid & 0xFF;
    buf[5] = vid >> 8;

    u16 pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
    buf[6] = pid & 0xFF;
    buf[7] = pid >> 8;
#else
    buf[4] = _BUF_4;	// VID
    buf[5] = _BUF_5;

    buf[6] = _BUF_6;	// PID
    buf[7] = _BUF_7;
#endif

#if RCSP_MODE == RCSP_MODE_EARPHONE

    buf[8] = 0x20;	//   2:TWS耳机类型   |  protocol verson

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN))
    buf[8] |= 4;
#else
    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[8] |= 2;
    }
#endif

#if (RCSP_ADV_HALTER_ENABLE)
    buf[8] = 0x23;
#endif

#endif // RCSP_MODE == RCSP_MODE_EARPHONE

#if RCSP_MODE == RCSP_MODE_SOUNDBOX
#if (SOUNDCARD_ENABLE)
    buf[8] = 0x40;	//   4:声卡类型   |  protocol verson
#else
    buf[8] = 0x0;	//   0:音箱类型   |  protocol verson
#endif
    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[8] |= 2;
    }
#endif


#if RCSP_MODE == RCSP_MODE_WATCH
    buf[8] = 0x50 | VER_FLAG_BLE_CTRL_BREDR | VER_FLAG_IOS_BLE_LINK_BREDR;	// 手表类型
#endif

    rcsp_adv_fill_mac_addr(&buf[9]);
#if RCSP_MODE == RCSP_MODE_WATCH
    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[15] = 1;
    }
#else
    /* printf("connect_flag %s, %s, %d, flag:%d\n", __FILE__, __FUNCTION__, __LINE__, __this->connect_flag); */
    buf[15] = __this->connect_flag;

    buf[16] = __this->bat_percent_L ? (((!!__this->bat_charge_L) << 7) | (__this->bat_percent_L & 0x7F)) : 0;
    buf[17] = __this->bat_percent_R ? (((!!__this->bat_charge_R) << 7) | (__this->bat_percent_R & 0x7F)) : 0;
    buf[18] = __this->bat_percent_C ? (((!!__this->bat_charge_C) << 7) | (__this->bat_percent_C & 0x7F)) : 0;

    memset(&buf[19], 0x00, 4);							// reserve

    buf[19] = __this->seq_rand;

    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[20] = 1;
    }

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_UNICAST_SINK_EN | LE_AUDIO_JL_UNICAST_SINK_EN))
    buf[20] |= BIT(2);  // 是否支持Le Audio功能
    if (is_cig_phone_conn()) {
        buf[20] |= BIT(3);  // Le Audio是否已连接
    }
#endif

    u8 t_buf[16];
    btcon_hash(&buf[2], 16, &buf[15], 4, t_buf);		// hash
    for (i = 0; i < 8; i++) {								// single byte
        buf[23 + i] = t_buf[2 * i + 1];
    }
#endif // RCSP_MODE == RCSP_MODE_WATCH

    __this->modify_flag = 0;
    adv_data_len = 31;
#if !TCFG_THIRD_PARTY_PROTOCOLS_SIMPLIFIED
    app_ble_adv_data_set(rcsp_server_ble_hdl, buf, 31);
    app_ble_adv_data_set(rcsp_server_ble_hdl1, buf, 31);
#else
    ble_op_set_adv_data(31, buf);
#endif

    log_info("ADV data():");
    log_info_hexdump(buf, 31);
    return 0;
}
```

其他完全一样。

# APP切换EQ(均衡器流程)

## 增加新的eq效果

- `interface\media\effects\audio_eq.h`

```c
typedef enum {
    EQ_MODE_NORMAL = 0,
    EQ_MODE_ROCK,
    EQ_MODE_POP,
    EQ_MODE_CLASSIC,
    EQ_MODE_JAZZ,
    EQ_MODE_COUNTRY,
#if 1//DHF_AYM_SITENG_APP_MORE_EQ_ENABLE
    EQ_MODE_BLUE,
    EQ_MODE_SITENG_CLASS,
    EQ_MODE_ELECTRC,
#endif
    EQ_MODE_CUSTOM,//自定义
    EQ_MODE_MAX,
} EQ_MODE;
```

- `audio\effect\eq_config.c`

```c
const struct eq_seg_info eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f},
};

const struct eq_seg_info eq_tab_rock[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    2, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -2, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -2, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, 0.7f},
};

const struct eq_seg_info eq_tab_pop[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     3, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     1, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -2, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -4, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -4, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  -2, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   1, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2, 0.7f},
};

const struct eq_seg_info eq_tab_classic[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     4, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    4, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   2, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2, 0.7f},

};

const struct eq_seg_info eq_tab_country[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    2, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    2, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, 0.7f},
};

const struct eq_seg_info eq_tab_jazz[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    4, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   4, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   2, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   3, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, 0.7f},
};
struct eq_seg_info eq_tab_custom[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f},
};

#if 1//DHF_AYM_SITENG_APP_MORE_EQ_ENABLE
struct eq_seg_info eq_tab_blue[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   3, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -3, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  2, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  5, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  3, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f},
};
struct eq_seg_info eq_tab_siteng_class[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 30,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 60,    13, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 120,   12, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   7, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -1, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   2, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  4, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 7, 0.7f},
};
struct eq_seg_info eq_tab_electrc[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 30,   -13, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 60,    -10, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 120,   -11, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -6, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  10, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  17, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  11, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  5, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, -1, 0.7f},
};
#endif

#if 1//DHF_AYM_SITENG_APP_MORE_EQ_ENABLE
const struct eq_seg_info *eq_type_tab[EQ_MODE_MAX] = {
    eq_tab_normal, eq_tab_rock, eq_tab_pop, eq_tab_classic, eq_tab_jazz, eq_tab_country,eq_tab_blue,eq_tab_siteng_class,eq_tab_electrc, eq_tab_custom
};
// 默认系数表，每个表对应的总增益,用户可修改
float global_gain_tab[EQ_MODE_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const u8 tab_section_num[] = {
    ARRAY_SIZE(eq_tab_normal), ARRAY_SIZE(eq_tab_rock),
    ARRAY_SIZE(eq_tab_pop), ARRAY_SIZE(eq_tab_classic),
    ARRAY_SIZE(eq_tab_jazz), ARRAY_SIZE(eq_tab_country),
    ARRAY_SIZE(eq_tab_blue),ARRAY_SIZE(eq_tab_siteng_class),
    ARRAY_SIZE(eq_tab_electrc),ARRAY_SIZE(eq_tab_custom)
};
#else
// 默认系数表,用户可修改
const struct eq_seg_info *eq_type_tab[EQ_MODE_MAX] = {
    eq_tab_normal, eq_tab_rock, eq_tab_pop, eq_tab_classic, eq_tab_jazz, eq_tab_country, eq_tab_custom
};
// 默认系数表，每个表对应的总增益,用户可修改
float global_gain_tab[EQ_MODE_MAX] = {0, 0, 0, 0, 0, 0, 0};
static const u8 tab_section_num[] = {
    ARRAY_SIZE(eq_tab_normal), ARRAY_SIZE(eq_tab_rock),
    ARRAY_SIZE(eq_tab_pop), ARRAY_SIZE(eq_tab_classic),
    ARRAY_SIZE(eq_tab_jazz), ARRAY_SIZE(eq_tab_country),
    ARRAY_SIZE(eq_tab_custom)
};
#endif
```

## 带APP切回`eq_tab_normal`还能回到外部eq文件效果吗？

## 使用APP切换均衡器效果日志

## 使用APP切换均衡器效果调用链

```c

```



# 修改蓝牙名

- 是主耳回调函数，只有主耳执行。
- 一起复位有概率不行，只能同时关机了。

## 日志

```c
[00:01:26.093]JL_rcsp_adv_cmd_resp
[00:01:26.093] JL_OPCODE_SET_ADV
[00:01:26.094]JL_opcode_set_adv_info:
07 01 38 37 36 35 34 33 
[00:01:26.095]deal_bt_name_setting(58)ES<>wS<>w
[00:01:26.153]att disconn
[00:01:26.154][LE-RCSP]RCSP HCI_EVENT_DISCONNECTION_COMPLETE: 50
 
[00:01:26.155][LE-RCSP]ble_work_st:21->5
 
[00:01:26.156]get_update_flag:0
[00:01:26.157][LE-RCSP])))))))) 1
 
[00:01:26.157]rcsp_ble_adv_enable_with_con_dev, rets=0xfe40f08
[00:01:26.158]apps/common/third_party_profile/jieli/rcsp/ble_rcsp_server.c, rcsp_ble_adv_enable_with_con_dev, 492, max:1, conn_num:0
[00:01:26.160]ble_module_enable, rets=0xfe40b24
[00:01:26.161][LE-RCSP]ble_module_enable:1
 
[00:01:26.161]rcsp_bt_ble_adv_enable, rets=0xfe40b24
[00:01:26.162]set_adv_enable, en:1, rets=0xfe407ec
[00:01:26.163][LE-RCSP]adv_en:1
 
[00:01:26.164][LE-RCSP]ble_work_st:5->20
 
[00:01:26.165]ADV data():
1E FF D6 05 7E 51 03 21 22 20 C7 F6 6C 5C 13 01 
64 00 00 26 01 00 00 90 DC 29 D6 E7 F5 04 7F 
[00:01:26.166]rsp_data(13):
02 01 0A 09 09 38 37 36 35 34 33 32 00 
[00:01:26.168]set_address_for_adv_handle:0

13 5C 6C F6 C7 20 
[00:01:26.175]get_update_flag:0
```



## 调用链

- `apps\common\third_party_profile\jieli\rcsp\server\functions\rcsp_setting_opt\settings\adv_bt_name_setting.c`
- 直接是回调函数形式

```c
static RCSP_SETTING_OPT adv_bt_name_opt = {
    .data_len = 32,
    .setting_type =	ATTR_TYPE_EDR_NAME,
    .syscfg_id = CFG_BT_NAME,
    .deal_opt_setting = deal_bt_name_setting,
    .set_setting = set_bt_name_setting,
    .get_setting = get_bt_name_setting,
    .custom_setting_init = NULL,
    .custom_vm_info_update = NULL,
    .custom_setting_update = NULL,
    .custom_sibling_setting_deal = NULL,
    .custom_setting_release = NULL,
    .set_setting_extra_handle = bt_name_set_setting_extra_handle,
    .get_setting_extra_handle = bt_name_get_setting_extra_handle,
};

REGISTER_APP_SETTING_OPT(adv_bt_name_opt);

static void deal_bt_name_setting(u8 *bt_name_setting, u8 write_vm, u8 tws_sync)
{
    printf("%s(%d)",__func__,__LINE__);
    u8 write_vm_ret = 0;
    if (!bt_name_setting) {
        get_bt_name_setting(g_edr_name);
    } else {
        memcpy(g_edr_name, bt_name_setting, 32);
    }
    if (write_vm) {
        write_vm_ret = update_bt_name_vm_value(g_edr_name);
    }
    if (tws_sync) {
        bt_name_sync(g_edr_name);
    }
    if (write_vm_ret) {
        //这里只能主耳执行，使用tws_sync_poweroff()以及sys_enter_soft_poweroff(POWEROFF_NORMAL_TWS)会重复报一次提示音
        //power_set_soft_poweroff()只会单耳关机
        //一起复位有概率不行
        tws_api_sync_call_by_uuid('T', SYNC_CMD_POWER_OFF_TOGETHER, 300);
    }
}

// 1、写入VM
static int update_bt_name_vm_value(u8 *bt_name_setting)
{
    u8 ret = syscfg_write(CFG_BT_NAME, bt_name_setting, 32);
    return ret;
}
```

# APP中的恢复出厂设置

## 调用链

- `apps\common\third_party_profile\jieli\rcsp\server\rcsp_cmd_recieve.c`

```c
void rcsp_cmd_recieve(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr)
{
    switch (OpCode) {
        case JL_OPCODE_CUSTOMER_USER:
        rcsp_user_cmd_recieve(priv, OpCode, OpCode_SN, data, len, ble_con_handle, spp_remote_addr);
        break;  
```

- `apps\common\third_party_profile\jieli\rcsp\server\rcsp_cmd_user.c`

```c
//*----------------------------------------------------------------------------*/
/**@brief    rcsp自定义命令数据接收处理
   @param    priv:全局rcsp结构体， OpCode:当前命令， OpCode_SN:当前的SN值， data:数据， len:数据长度
   @return
   @note	 二次开发需要增加自定义命令，通过JL_OPCODE_CUSTOMER_USER进行扩展，
  			 不要定义这个命令以外的命令，避免后续SDK更新导致命令冲突
*/
/*----------------------------------------------------------------------------*/
void rcsp_user_cmd_recieve(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr)
{
    //自定义数据接收
    rcsp_printf("%s:", __FUNCTION__);
    rcsp_put_buf(data, len);
#if 0
    ///以下是发送测试代码
    u8 test_send_buf[] = {0x04, 0x05, 0x06};
    rcsp_user_cmd_send(test_send_buf, sizeof(test_send_buf));
#endif

    //调用自定义数据解析接口
    extern void rscp_user_cmd_recv(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr);
    rscp_user_cmd_recv(priv, OpCode, OpCode_SN, data, len, ble_con_handle, spp_remote_addr);

    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0, ble_con_handle, spp_remote_addr);

}

//--------------------------数据解析 (APP发过来的数据)------------------------------
extern void dhf_factory_reset_deal(void);
void rscp_user_cmd_recv(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr){

    printf("接收到APP自定义指令----%s(%d)",__func__,__LINE__);
    if(len == 4 && (data[0] == 0xFF) && (data[1] == 0xFD) && (data[2] == 0xF5) && (data[3] == 0x01)){
        //接收到APP的恢复出厂设置命令
        //恢复出厂设置之前把音乐停止掉，不然按键触发对应提示音后，音乐还会继续播放
        bt_cmd_prepare(USER_CTRL_AVCTP_OPID_STOP, 0, NULL);
        //自定义的处理函数
        dhf_factory_reset_deal();
    }
}
```

# RCSP广播

`apps\common\third_party_profile\jieli\rcsp\adv\ble_rcsp_adv.c`

```c
    rcsp_bt_ble_adv_enable(0);
    rcsp_make_set_adv_data();
    rcsp_make_set_rsp_data();
    rcsp_bt_ble_adv_enable(1);
```

## `rcsp_make_set_adv_data()` - 设置广播数据包（ADV Data）

**作用**：生成并设置BLE的**主广播数据包**，包含设备的核心识别信息。

**包含的关键信息**：

- **厂商ID**（JL ID: 0x05D6）
- **VID/PID**（设备型号识别）
- **设备类型**（耳机/音箱/手表等）
- **MAC地址**
- **电量信息**（左耳、右耳、充电仓）
- **连接状态标志**
- **TWS配对状态**
- **Hash校验码**（防伪验证）

**特点**：

- 固定31字节长度
- **主动广播**，无需手机扫描即可接收
- 包含快速识别设备所需的最小信息集

## `rcsp_make_set_rsp_data()` - 设置扫描响应数据包（Scan Response）

**作用**：生成并设置BLE的**扫描响应数据包**，补充广播包无法容纳的信息。

**包含的关键信息**：

- **设备名称**（通过 `bt_get_local_name()` 获取）
- **FLAGS**（设备能力标志）

**特点**：

- 动态长度（取决于设备名称长度）
- **被动响应**，只有当手机主动发起扫描请求（Scan Request）时才返回
- 主要用于显示人类可读的设备名称

## 在NRF Connect工具中的体现

当你在NRF Connect中看到一个BLE设备时：

| 数据包类型   | 对应函数                   | 你看到的内容                                       |
| ------------ | -------------------------- | -------------------------------------------------- |
| **ADV_IND**  | `rcsp_make_set_adv_data()` | 原始广播数据（Manufacturer Data，包含电量、MAC等） |
| **SCAN_RSP** | `rcsp_make_set_rsp_data()` | 设备名称（Complete Local Name）                    |

**查看方式**：

- 点击设备后，**RAW** 标签页会显示两个数据包
- **ADV** 部分 = `adv_data`（31字节）
- **SCAN RSP** 部分 = `scan_rsp_data`（包含设备名）

## 为什么要分开？

1. **空间限制**：BLE 4.x 广播包最大31字节，无法同时容纳所有信息
2. **省电优化**：主广播包高频发送，扫描响应包按需发送
3. **快速识别**：手机可以先通过ADV包判断设备类型/电量，再决定是否获取完整名称

## 异常分析

### BLE名称没有超出限制日志

```c
[00:00:08.887]ADV data():
1E FF D6 05 7E 51 03 21 22 D3 38 DF 6E 32 91 00 
64 00 00 10 01 00 00 E0 C7 C9 01 5A 92 84 96 
[00:00:08.889]rsp_data(31):
02 01 0A 1B 09 44 45 56 49 41 20 55 4C 54 52 41 
20 41 49 52 20 50 52 4F 2D 45 4D 34 31 30 00 

[00:00:08.891]<error> [APP_BLE]app_ble_adv_enable 1 faild !
```

### BLE名称超出限制日志

```c
[00:00:02.842]ADV data():
1E FF D6 05 7E 51 03 21 22 D3 38 DF 6E 32 91 00 
64 00 00 10 01 00 00 E0 C7 C9 01 5A 92 84 96 
[00:00:02.843]***rsp_data overflow!!!!!!
[00:00:02.844]advertisements_setup_init fail !!!!!!
[00:00:02.845]set_address_for_adv_handle:0

91 32 6E DF 38 D3 
```

### 自动截断继续往下跑

```c
int rcsp_make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;
    const char *edr_name = bt_get_local_name();

#if DOUBLE_BT_SAME_MAC || TCFG_BT_BLE_BREDR_SAME_ADDR
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x0A, 1);
#else
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x06, 1);
#endif

    u8 name_len = strlen(edr_name) + 1;
    //修改1：超长时自动截断，而不是返回失败
    if (offset + 2 + name_len > ADV_RSP_PACKET_MAX) {
        name_len = ADV_RSP_PACKET_MAX - offset - 2;  // 计算最大可用长度
        puts("***rsp_data overflow, auto truncate!!!!!!\n");
        // return -1;  // 删除这行，不再返回失败
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)edr_name, name_len);
    scan_rsp_data_len = offset;
    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
#if !TCFG_THIRD_PARTY_PROTOCOLS_SIMPLIFIED
    app_ble_rsp_data_set(rcsp_server_ble_hdl, buf, 31);
    app_ble_rsp_data_set(rcsp_server_ble_hdl1, buf, 31);
#else
    ble_op_set_rsp_data(offset, buf);
#endif
    return 0;
}
```

### 是否可以加长

- 31字节是BLE协议的硬性规定，不能加长！

**BLE 4.0/4.1/4.2 - 严格31字节**

- **技术原因：**
  - 总包长最大 47 bytes
  - MAC地址头 6 bytes
  - PDU头 2 bytes
  - **剩余 = 31 bytes**（这是协议栈能用的最大空间）

**BLE 5.0 扩展广播（Extended Advertising）**

- BLE 5.0引入了**扩展广播**，可以突破31字节限制：
- **但有3个前提条件：**
  1. ✅ **芯片支持** BLE 5.0
  2. ✅ **手机支持** BLE 5.0（iOS 13+, Android 8+）
  3. ✅ **代码启用** Extended Advertising

### 最长多少

**所有可见字符（包括字母、数字、空格、标点符号）都占用字节**。

在 26 字节限制内，你需要计算：

- 字母：每个 1 字节
- 数字：每个 1 字节
- **空格：每个 1 字节** ⬅️ 重点
- 标点（-）：每个 1 字节

# 改名后苹果复位

## 死机复位日志

```c
=======================================
Chip Exception, Current CPU0
USP : 0x00400ED8 
SSP : 0x00100C80 
SP  : 0x00100C20 
RETS : 0x000FEB9E 
RETI : 0x000FEBC6 
ICFG : 0x07010280 

CPU0 Trace: 0x0010811E -->0x00108140 -->0x000FEB9E -->0x000FEBC6 

CPU0 EMU_CON = 0xE000001F 
CPU0 EMU_MSG = 0x80000000 
CPU1 EMU_CON = 0x00000000 
CPU1 EMU_MSG = 0x00000000 
JL_CEMU->MSG0: 0x01000000 
JL_CEMU->MSG1: 0x00000000 
JL_CEMU->MSG2: 0x00000000 
JL_CEMU->ID: 0x000000F1 
JL_HEMU->MSG0: 0x00000000 
JL_HEMU->ID: 0x00000000 
JL_LEMU->MSG0: 0x00000000 
JL_LEMU->ID: 0x00000076 

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Exception at CPU0 info:

CPU0 EMU_CON = 0xE000001F 
CPU0 EMU_MSG = 0x80000000 

Exception In User Mode

[0-CPU] emu err msg : system excption
 JL_CEMU->MSG0: 0x01000000 
 JL_CEMU->MSG1: 0x00000000 
 JL_CEMU->MSG2: 0x00000000 
[1-HCORE] hmem err msg : cpu0 read    mmu excption

CPU0 Trace: 0x0010811E -->0x00108140 -->0x000FEB9E -->0x000FEBC6 

USP : 0x00400ED8 
SSP : 0x00100C80 
SP  : 0x00100C20 
SSP_LIMIT_L : 0x00100080 
SSP_LIMIT_H : 0x00100C80 
USP_LIMIT_L : 0x0040004C 
USP_LIMIT_H : 0x00401000 

RETS : 0x000FEB9E 
RETI : 0x000FEBC6 
RETX : 0x00000000 
RETE : 0x0FE136D8 
PSR  : 0x00000004 
ICFG : 0x07010280 

R00 = 0x00000000 
R01 = 0x00000000 
R02 = 0x00000000 
R03 = 0x00000040 
R04 = 0x00000040 
R05 = 0x00000000 
R06 = 0x0013BB0C 
R07 = 0x00000003 
R08 = 0x00400F70 
R09 = 0x0013BB0C 
R10 = 0x00000010 
R11 = 0x00000000 
R12 = 0x00400F70 
R13 = 0x00100003 
R14 = 0x00100004 
R15 = 0x00100002 
USP : 
40 00 00 00 88 29 10 00 00 00 00 00 01 00 00 00 
70 0F 40 00 0C BB 13 00 E6 74 10 00 40 00 00 00 
88 29 10 00 B0 D1 E3 0F 70 0F 40 00 00 00 00 00 
02 00 00 00 86 11 E4 0F 28 E4 0F 00 00 00 00 00 
50 88 41 00 80 88 41 00 58 88 41 00 A4 E4 0F 00 
70 0F 40 00 F0 1E 10 00 74 0F 40 00 00 15 E6 0F 
09 00 00 00 00 15 E6 0F 80 D1 E7 0F 90 51 E7 0F 
FC ED E0 0F 00 00 00 00 F0 1E 10 00 74 0F 40 00 
00 15 E6 0F 09 00 00 00 0D 00 10 00 BE D7 E1 0F 
13 00 00 00 75 0F 40 00 89 00 30 00 7E 11 E4 0F 
01 01 00 00 00 00 00 00 30 28 10 00 20 33 10 00 
5B C9 E8 0F 20 33 10 00 D0 45 10 00 DD B2 1A 90 
54 55 E0 0F 00 33 10 00 20 33 10 00 38 EF E0 0F 
0D 00 00 00 00 00 00 00 0E 1A F5 AD 67 B2 10 01 
09 00 00 00 02 00 00 00 00 00 00 00 0A 00 00 00 
CF 8A E8 0F 00 33 10 00 D0 45 10 00 DD B2 1A 90 
C6 23 D4 F9 DB 36 D0 F7 5E 29 0E 29 A2 73 7E 48 
13 35 24 2E 39 9F 7B 0F 52 42 E2 34 70 AD 9F 1B 
D8 EE E0 0F BD 4E 35 20 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 

SSP : 
29 DE D0 6F 58 6E C8 6E 47 20 CF 6F D0 EC 59 A3 
D0 EC D0 02 01 18 00 E1 5C D0 42 28 BF EA 3D FA 
50 EE 50 A0 4E 40 4A 45 50 ED 5C C1 44 66 51 6F 
C0 EF 01 F0 35 E1 00 FB D7 60 D7 61 02 4A 02 F8 
0C 04 82 F8 12 08 43 20 06 F9 15 06 03 E1 30 90 
04 89 C3 FF 00 08 00 04 04 8E 43 20 06 F9 0B 06 
03 E1 3C 90 D8 EC 3A 36 04 86 53 20 B6 E8 00 00 
C3 FF 40 08 00 04 11 1F 44 21 B6 E8 02 00 44 20 
44 BD 22 A4 62 19 30 E1 BF 2F D4 63 80 F9 06 04 
30 E1 DF 2F B0 EC 01 00 4F 38 04 81 47 38 C0 F1 
A5 20 50 60 32 19 20 19 C2 FF 03 40 7E 00 20 19 
D0 60 64 E8 04 57 50 60 50 E8 FD F1 20 00 D1 64 
D0 EC 55 C1 50 60 72 E1 04 00 41 E1 04 00 3C E8 
00 00 21 16 D1 60 BF EA 27 F5 BF EA 1F F5 86 F8 
08 04 40 20 BF EA 0E F5 01 EF 01 50 40 20 BF EA 
0B F5 60 00 C0 FF 00 BE 13 00 04 16 48 88 01 E1 
4C D0 02 16 10 85 13 05 A3 05 D0 EC DC 14 02 E1 
20 40 C2 62 10 9F 13 07 A3 07 C8 00 20 00 00 E1 
04 52 00 EF 00 0E 40 22 BF EA 0E F5 BF EA 0E F5 
00 00 F7 9E C1 22 C0 22 B8 63 A9 63 80 17 91 17 
49 FF 00 10 C8 FD D4 E9 7C E0 E6 85 40 20 80 00 
C1 FF 80 27 10 00 10 85 02 05 92 05 80 00 00 00 
00 07 FE 00 00 00 00 00 00 00 00 00 00 00 00 00 
53 50 49 00 55 54 52 58 00 50 53 52 41 4D 00 00 
00 07 FE 00 00 0B FD 00 75 62 6F 6F 74 5F 7A 6F 
6E 65 00 55 41 52 54 55 50 44 41 54 45 00 00 00 
00 08 00 14 00 04 00 1C 00 04 00 1C 00 08 00 10 
00 00 00 18 00 00 00 18 61 70 70 5F 64 69 72 5F 
68 65 61 64 00 46 4C 41 53 48 5F 51 45 5F 50 4F 
53 00 61 70 70 5F 61 72 65 61 5F 68 65 61 64 00 
61 70 70 5F 64 69 72 5F 68 65 61 64 32 00 69 73 
64 5F 63 6F 6E 66 69 67 2E 69 6E 69 00 55 41 52 

SP : 
00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 
40 00 00 00 00 00 00 00 0C BB 13 00 03 00 00 00 
70 0F 40 00 0C BB 13 00 10 00 00 00 00 00 00 00 
70 0F 40 00 03 00 10 00 04 00 10 00 02 00 10 00 
C6 EB 0F 00 D8 36 E1 0F 00 00 00 00 9E EB 0F 00 
04 00 00 00 80 02 01 07 D8 0E 40 00 80 0C 10 00 
29 DE D0 6F 58 6E C8 6E 47 20 CF 6F D0 EC 59 A3 
D0 EC D0 02 01 18 00 E1 5C D0 42 28 BF EA 3D FA 
50 EE 50 A0 4E 40 4A 45 50 ED 5C C1 44 66 51 6F 
C0 EF 01 F0 35 E1 00 FB D7 60 D7 61 02 4A 02 F8 
0C 04 82 F8 12 08 43 20 06 F9 15 06 03 E1 30 90 
04 89 C3 FF 00 08 00 04 04 8E 43 20 06 F9 0B 06 
03 E1 3C 90 D8 EC 3A 36 04 86 53 20 B6 E8 00 00 
C3 FF 40 08 00 04 11 1F 44 21 B6 E8 02 00 44 20 
44 BD 22 A4 62 19 30 E1 BF 2F D4 63 80 F9 06 04 
30 E1 DF 2F B0 EC 01 00 4F 38 04 81 47 38 C0 F1 
A5 20 50 60 32 19 20 19 C2 FF 03 40 7E 00 20 19 
D0 60 64 E8 04 57 50 60 50 E8 FD F1 20 00 D1 64 
D0 EC 55 C1 50 60 72 E1 04 00 41 E1 04 00 3C E8 
00 00 21 16 D1 60 BF EA 27 F5 BF EA 1F F5 86 F8 
08 04 40 20 BF EA 0E F5 01 EF 01 50 40 20 BF EA 
0B F5 60 00 C0 FF 00 BE 13 00 04 16 48 88 01 E1 
4C D0 02 16 10 85 13 05 A3 05 D0 EC DC 14 02 E1 
20 40 C2 62 10 9F 13 07 A3 07 C8 00 20 00 00 E1 
04 52 00 EF 00 0E 40 22 BF EA 0E F5 BF EA 0E F5 
00 00 F7 9E C1 22 C0 22 B8 63 A9 63 80 17 91 17 
49 FF 00 10 C8 FD D4 E9 7C E0 E6 85 40 20 80 00 
C1 FF 80 27 10 00 10 85 02 05 92 05 80 00 00 00 
00 07 FE 00 00 00 00 00 00 00 00 00 00 00 00 00 
53 50 49 00 55 54 52 58 00 50 53 52 41 4D 00 00 
00 07 FE 00 00 0B FD 00 75 62 6F 6F 74 5F 7A 6F 
6E 65 00 55 41 52 54 55 50 44 41 54 45 00 00 00 

OSTCBCur :app_core
OSTCBHighRdy :app_core

cpu run: 0x027ED31E 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x00000034 0x0259A578 0x0259AEF4 0x00000000 0x0259AEF4 lp_touch_key
6A 81 10 00 DE D9 0F 00 06 00 00 00 00 00 00 00 
00 00 00 00 A0 E1 FF 00 00 00 00 00 00 00 00 00 
38 93 41 00 68 93 41 00 40 93 41 00 B0 33 40 00 
0C BB 13 00 10 00 00 00 00 00 00 00 00 00 00 00 
FF FF FF FF 01 00 00 00 1A 00 00 00 E8 E4 0F 00 
00 12 00 00 1A 00 00 00 00 00 00 00 04 00 00 00 
01 00 00 00 5C 49 10 00 FF FF 00 00 FE FF FF FF 
FA 02 E5 0F 78 73 E7 0F A8 60 0C 92 92 E1 5E 80 
01 00 10 00 08 00 00 00 00 00 00 00 00 00 00 00 
95 4D 66 BA 85 22 3B 9F 33 8A 67 91 7D 7E 8D 83 
F4 BE F5 4C C5 6E 77 F5 0F C1 C0 9C 23 B4 35 C3 
C6 C6 07 D4 A7 0B 39 C4 6B 83 95 C3 BD 3A 42 DE 
6D 48 00 12 0E 12 FA 11 01 00 00 00 60 19 B9 AA 

lp_touch_key task backtrace : 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x0000002E 0x00000000 0x00000000 0x00000000 0x00000000 anc
6A 81 10 00 DE D9 0F 00 06 00 00 00 00 00 00 00 
00 00 00 00 A0 E1 FF 00 24 8B 41 00 00 00 00 00 
04 91 41 00 34 91 41 00 0C 91 41 00 BC 2F 40 00 
0C BB 13 00 10 00 00 00 00 00 00 00 EB 18 A0 65 
A1 80 FA 92 01 00 00 00 C0 92 E7 0F E8 E4 0F 00 
BC 2F 40 00 90 11 E7 0F 81 B0 1F EA 44 26 10 00 
83 98 92 60 2B 5E E7 0F 00 00 00 00 92 A1 E9 29 
60 FA E1 0F 00 00 00 00 A2 DF DC 67 2C 05 15 B3 
00 81 60 E6 0B 80 D5 A2 E9 E2 A3 3A 81 B0 1F EA 
0E 6B 31 90 83 98 92 60 13 A9 F9 18 6A CA 7E 63 
92 A1 E9 29 EB 18 A0 65 A1 80 FA 92 C9 90 68 DE 
CA 86 1A D8 C4 A1 F7 11 

anc task backtrace : 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x0000002E 0x027AEB96 0x027AEC5F 0x000001E2 0x027AEC5F led_driver
6A 81 10 00 DE D9 0F 00 06 00 00 00 00 00 00 00 
00 00 00 00 A0 E1 FF 00 58 93 41 00 00 00 00 00 
14 8D 41 00 44 8D 41 00 1C 8D 41 00 BC 27 40 00 
0C BB 13 00 10 00 00 00 00 00 00 00 F0 C9 3A BA 
4A 24 73 CF AA 03 FA 95 DA 66 1A FA E8 E4 0F 00 
85 13 E7 0F BC 27 40 00 01 00 10 00 6C F1 67 10 
32 BE DD 5B D4 20 3E CE 5A 2F 3D 71 8A 00 15 6C 
D4 21 E0 0F 01 00 10 00 74 37 10 00 05 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 60 4F 3B F8 

led_driver task backtrace : 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x0000002E 0x01C1934F 0x01C35AB1 0x00000000 0x01C35AB1 jlstream
6A 81 10 00 DE D9 0F 00 06 00 00 00 00 00 00 00 
00 00 00 00 A0 E1 FF 00 90 9F 41 00 00 00 00 00 
04 8B 41 00 34 8B 41 00 0C 8B 41 00 DC 23 40 00 
0C BB 13 00 08 00 00 00 00 00 00 00 5B 03 81 71 
41 5E 0E 5C EA BE D4 34 30 3C 43 D5 E8 E4 0F 00 
00 00 00 00 48 59 10 00 BC 23 40 00 7C D8 41 00 
E0 23 40 00 48 59 10 00 BE 23 40 00 2E 46 83 16 
50 EA E0 0F 00 00 00 00 00 00 9A FF 00 00 00 00 
00 00 00 00 00 00 00 00 50 A2 8A 01 6C D8 41 00 
00 00 00 00 7A 00 30 00 6A BB E2 0F 01 01 00 00 
00 00 00 00 30 28 10 00 41 5E 0E 5C EA BE D4 34 
30 3C 43 D5 D3 C8 A9 00 

jlstream task backtrace : 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x00000016 0x027E59DD 0x027ECCED 0x000B046E 0x027ECCED IDLE0
BC F8 0F 00 C4 7F 10 00 06 00 00 00 50 D0 F2 00 
40 00 00 00 01 01 00 00 00 01 00 00 A8 B2 9D 11 
AF 58 9F C8 AF C9 36 97 4F 00 1E 19 BA CE 12 8E 
5D F2 D6 F6 E8 54 49 43 B5 14 1E 1A 19 0E 4C DD 
2A CC 73 9C A9 12 25 AC 98 75 A2 42 A9 12 25 00 
E8 79 10 00 89 7C B4 BA 

IDLE0 task backtrace : 
StackPush / SwInTime / SwOutTime / RunTime / LastRunTime / Name
0x0000004A 0x027ECCED 0x027E59DD 0x0001C37E 0x027E59DD app_core
40 00 00 00 88 29 10 00 00 00 00 00 01 00 00 00 
70 0F 40 00 0C BB 13 00 E6 74 10 00 40 00 00 00 
88 29 10 00 B0 D1 E3 0F 70 0F 40 00 00 00 00 00 
02 00 00 00 86 11 E4 0F 28 E4 0F 00 00 00 00 00 
50 88 41 00 80 88 41 00 58 88 41 00 A4 E4 0F 00 
70 0F 40 00 F0 1E 10 00 74 0F 40 00 00 15 E6 0F 
09 00 00 00 00 15 E6 0F 80 D1 E7 0F 90 51 E7 0F 
FC ED E0 0F 00 00 00 00 F0 1E 10 00 74 0F 40 00 
00 15 E6 0F 09 00 00 00 0D 00 10 00 BE D7 E1 0F 
13 00 00 00 75 0F 40 00 89 00 30 00 7E 11 E4 0F 
01 01 00 00 00 00 00 00 30 28 10 00 20 33 10 00 
5B C9 E8 0F 20 33 10 00 D0 45 10 00 DD B2 1A 90 
54 55 E0 0F 00 33 10 00 20 33 10 00 38 EF E0 0F 
0D 00 00 00 00 00 00 00 0E 1A F5 AD 67 B2 10 01 
09 00 00 00 02 00 00 00 00 00 00 00 0A 00 00 00 
CF 8A E8 0F 00 33 10 00 D0 45 10 00 DD B2 1A 90 
C6 23 D4 F9 DB 36 D0 F7 5E 29 0E 29 A2 73 7E 48 
13 35 24 2E 39 9F 7B 0F 52 42 E2 34 70 AD 9F 1B 
D8 EE E0 0F BD 4E 35 20 

app_core task backtrace : 
[00:00:00.100]~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[00:00:00.100]         setup_arch
[00:00:00.100]~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[00:00:00.100]get_boot_rom(): 0
[00:00:00.100][PMU]msys_rst_src: 0x1
[00:00:00.100][PMU]MSYS_P11_RST
[00:00:00.100][PMU]p11_rst_src: 0x40
[00:00:00.100][PMU]P11_P33_RST
[00:00:00.100][PMU]P33_LEVEL2_RST
[00:00:00.100][PMU]p33_rst_src: 0x20
[00:00:00.100][PMU]P33_SOFT_RST
[00:00:00.100][PMU]P33_EXCEPTION_SOFT_RST
[00:00:00.100][PMU]reset_source_value: 0x2000401
[00:00:00.100][PMU]reset_source_value1: 0x0
[00:00:00.100][PMU]sfc_bit_mode: 2, port: 0
```

## 分析

**MMU读取异常** - CPU0在用户模式下访问内存时触发了MMU（内存管理单元）异常：

```c
[1-HCORE] hmem err msg : cpu0 read mmu excption
Exception In User Mode
```

### 异常发生位置

- **PC追踪**: `0x0010811E -> 0x00108140 -> 0x000FEB9E -> 0x000FEBC6`
- **当前任务**: `app_core`
- **RETS**: `0x000FEB9E` (子程序返回地址)
- **RETI**: `0x000FEBC6` (中断返回地址)

### 可能的原因

1. 非法内存访问
   - 访问了未映射或无权限的内存地址
   - 栈溢出（USP: 0x00400ED8 接近上限 0x00401000）
   - 野指针或空指针解引用
2. 堆栈问题
   - 用户栈指针(USP)和系统栈指针(SSP)可能存在越界
   - 注意 R00-R02 都是 0x00000000，可能涉及空指针访问
3. **复位来源**

```c
[PMU]P33_EXCEPTION_SOFT_RST
reset_source_value: 0x2000401
```

系统检测到异常后触发了软件复位

### 建议排查方向

1. 检查地址 `0x000FEB9E` 和 `0x000FEBC6` 附近的代码
2. 检查 `app_core` 任务中的指针使用和内存访问
3. 检查是否存在栈溢出（当前栈使用接近上限）
4. 使用调试器在异常地址设置断点，查看具体的内存访问操作
5. 检查 MMU 页表配置是否正确

这是一个典型的内存访问违规导致的系统保护性复位。

## 改名后的关机方式接口有问题

```c
void rcsp_reset_name_deal(void){
    printf("rcsp_reset_name_deal");
    tws_api_sync_call_by_uuid('T', SYNC_CMD_POWER_OFF_TOGETHER, 300);
}

void rcsp_reset_name_deal_os(void){
    int argv[3];
    argv[0] = (int)rcsp_reset_name_deal;
    argv[1] = 1;
    argv[2] = 0;
    os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
}

static void deal_bt_name_setting(u8 *bt_name_setting, u8 write_vm, u8 tws_sync)
{
    printf("%s(%d)",__func__,__LINE__);
    u8 write_vm_ret = 0;
    if (!bt_name_setting) {
        get_bt_name_setting(g_edr_name);
    } else {
        memcpy(g_edr_name, bt_name_setting, 32);
    }
    if (write_vm) {
        write_vm_ret = update_bt_name_vm_value(g_edr_name);
    }
    if (tws_sync) {
        bt_name_sync(g_edr_name);
    }
    if (write_vm_ret) {
        //这里只能主耳执行，使用tws_sync_poweroff()以及sys_enter_soft_poweroff(POWEROFF_NORMAL_TWS)会重复报一次提示音
        //power_set_soft_poweroff()只会单耳关机
        //一起复位有概率不行
        //tws_api_sync_call_by_uuid('T', SYNC_CMD_POWER_OFF_TOGETHER, 300);
        rcsp_reset_name_deal_os();
    }
}

    case SYNC_CMD_POWER_OFF_TOGETHER:
        printf("SYNC_CMD_POWER_OFF_TOGETHER\n");
        //sys_enter_soft_poweroff(POWEROFF_NORMAL);//毫不夸张的说会死机！！！注意使用这个接口有潜在风险
        //dac_power_off();使用这个接口会有波声。。。
        power_set_soft_poweroff();//不带提示音的
        break;
```

# 苹果APP界面中耳机出入仓配对后直接关机

```c
-deal_bt_name_setting回调中-不-进行RCSP改名后的关机操作
-adv_sync_reset_sync_func_t回调中进行IOS改名后的关机操作(有作用)
-rcsp_wait_reboot_dev回调中进行安卓改名后的关机操作（无作用，不走这里）
-RCSP改名后以及恢复出厂设置都改为复位(deal_bt_name_setting)
```



