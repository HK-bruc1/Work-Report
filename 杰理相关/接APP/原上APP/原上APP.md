# 添加标识凭证PID VID CID

`apps\common\third_party_profile\jieli\rcsp\adv\ble_rcsp_adv.c`

```C
rcsp_make_set_adv_data
```

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

```C
//*********************************************************************************//
//                                 自定义配置                                       //
//*********************************************************************************//
#define YUANSHANG_APP_ENABLE                 1  //接入原上APP
#define DHF_MUSIC_SWITCH_PLAY_EN             1  //在播放状态下切换上下曲
#define DHF_CALL_FIXED_EN                    1//带APP固定通话按键功能
#define DHF_CALL_NOMODE_SWITCH               1//通话状态下不允许切换模式（ANC/游戏模式）
#define USER_COLOR_PID_SET                   0x18 //不同颜色PID设置[black:0x39/cotton candy:0x4D/white:0x4E] 
```

# 也是走RCSP流程

# 广播格式

![image-20260123161224190](./原上APP.assets/image-20260123161224190.png)

![image-20260123161055147](./原上APP.assets/image-20260123161055147.png)

- 广播格式不对的话就连接不上。

```c
int rcsp_make_set_adv_data(void)
{
    u8 i;
    u8 *buf = adv_data;
    buf[0] = 0x1E;
    buf[1] = 0xFF;

#if YUANSHANG_APP_ENABLE  //cid  公司编码（固定）
    buf[2] = 0x22;	// JL ID
    buf[3] = 0xB8;
#else
    buf[2] = 0xD6;	// JL ID
    buf[3] = 0x05;
#endif

#if YUANSHANG_APP_ENABLE
    buf[4] = 0x00;	// VID
    buf[5] = 0x48;

    buf[6] = 0x00;	// PID
    buf[7] = USER_COLOR_PID_SET;
#else
    u16 vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
    buf[4] = vid & 0xFF;
    buf[5] = vid >> 8;

    u16 pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
    buf[6] = pid & 0xFF;
    buf[7] = pid >> 8;
#endif

#if RCSP_MODE == RCSP_MODE_EARPHONE

#if YUANSHANG_APP_ENABLE
    buf[8] = 0x04;	//   2:TWS耳机类型   |  protocol verson
    if (bt_get_connect_status() >=  BT_STATUS_CONNECTING) {
        buf[8] = 0x05;
    }
#else
    buf[8] = 0x20;	//   2:TWS耳机类型   |  protocol verson

#if (BT_AI_SEL_PROTOCOL & LE_AUDIO_CIS_RX_EN)
    buf[8] |= 4;
#else
    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[8] |= 2;
    }
#endif
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
#if YUANSHANG_APP_ENABLE
    buf[9]^=0xad;
    buf[10]^=0xad;
    buf[11]^=0xad;
    buf[12]^=0xad;
    buf[13]^=0xad;
    buf[14]^=0xad;
    for (i = 0; i <= 15; i++) {
        buf[15 + i] = 0x00;
    }
#endif
#if RCSP_MODE == RCSP_MODE_WATCH
    if (RCSP_USE_SPP == get_defalut_bt_channel_sel()) {
        buf[15] = 1;
    }
#else

#if !YUANSHANG_APP_ENABLE
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

#if (BT_AI_SEL_PROTOCOL & LE_AUDIO_CIS_RX_EN)
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
#endif
#endif // RCSP_MODE == RCSP_MODE_WATCH

    __this->modify_flag = 0;
    adv_data_len = 31;
    /* ble_op_set_adv_data(31, buf); */
    app_ble_adv_data_set(rcsp_server_ble_hdl, buf, 31);
    app_ble_adv_data_set(rcsp_server_ble_hdl1, buf, 31);

    log_info("ADV data():");
    log_info_hexdump(buf, 31);
    return 0;
}
```

# OTA开双备份flash不够

16M足够了，F6的。改一下可视化工具配置。

OTA软件APP后台一般只能上传一个软件，意味着只能出一个软件。如果分左右的话，APP升级后两个耳机变成了同一个声道！就会出现声道错乱。一般都是硬件上给一个下拉电阻做左右区分。
