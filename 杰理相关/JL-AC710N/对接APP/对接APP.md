# 宏使能

```c
#define TCFG_USER_BLE_ENABLE 1 // BLE
#if TCFG_USER_BLE_ENABLE
#define TCFG_BT_BLE_TX_POWER 9 // 最大发射功率
#define TCFG_BT_BLE_BREDR_SAME_ADDR 1 // 和2.1同地址
#define TCFG_BT_BLE_ADV_ENABLE 1 // 广播
#define TCFG_BLE_HIGH_PRIORITY_ENABLE 0 // 高优先级
#endif // TCFG_USER_BLE_ENABLE

#define TCFG_THIRD_PARTY_PROTOCOLS_ENABLE 1 // 第三方协议配置
#if TCFG_THIRD_PARTY_PROTOCOLS_ENABLE
#define TCFG_RCSP_DUAL_CONN_ENABLE 1 // 支持连接两路RCSP
#define TCFG_THIRD_PARTY_PROTOCOLS_SIMPLIFIED 0 // 三方协议轻量化
#define TCFG_THIRD_PARTY_PROTOCOLS_SEL RCSP_MODE_EN // 第三方协议选择
#endif // TCFG_THIRD_PARTY_PROTOCOLS_ENABLE
```

- 就可以使用手机APP控制耳机了。比如音量，触摸操作等。
- 流程是什么？
- 通信机制是什么？
- 是怎么融入原来TWS蓝牙耳机正常流程的？