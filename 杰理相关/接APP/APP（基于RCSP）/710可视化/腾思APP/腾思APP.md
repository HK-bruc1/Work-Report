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
    if (!bt_name_setting) {
        get_bt_name_setting(g_edr_name);
    } else {
        memcpy(g_edr_name, bt_name_setting, 32);
    }
    if (write_vm) {
        update_bt_name_vm_value(g_edr_name);
    }
    if (tws_sync) {
        bt_name_sync(g_edr_name);
    }
}

// 1、写入VM
static void update_bt_name_vm_value(u8 *bt_name_setting)
{
    syscfg_write(CFG_BT_NAME, bt_name_setting, 32);
    //直接复位刷新名称，放外面的话，开机回调时会有影响。
    cpu_reset();
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

# IOS自定义均衡器异常

一调就跳到其他均衡器效果。

