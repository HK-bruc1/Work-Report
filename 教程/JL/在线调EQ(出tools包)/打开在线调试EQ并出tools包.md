# 打开对应宏

在`apps\earphone\board\br36\board_ac700n_demo_cfg.h`板级文件中打开宏

![1f3a407969de52e6a35b58e2c3a6117](./打开在线调试EQ并出tools包.assets/1f3a407969de52e6a35b58e2c3a6117.png)

有一些SDK不一样，但是核心是开EQ总使能，找到在线EQ调试打开，后面有对应的宏说明

其他SDK：

![image-20250705143311195](./打开在线调试EQ并出tools包.assets/image-20250705143311195.png)

# 修改声道选择

`apps\earphone\include\app_config.h`

```c
/* 声道确定方式选择 */
#define CONFIG_TWS_MASTER_AS_LEFT             0 //主机作为左耳
#define CONFIG_TWS_AS_LEFT_CHANNEL            1 //固定左耳
#define CONFIG_TWS_AS_RIGHT_CHANNEL           2 //固定右耳
#define CONFIG_TWS_LEFT_START_PAIR            3 //双击发起配对的耳机做左耳
#define CONFIG_TWS_RIGHT_START_PAIR           4 //双击发起配对的耳机做右耳
#define CONFIG_TWS_EXTERN_UP_AS_LEFT          5 //外部有上拉电阻作为左耳
#define CONFIG_TWS_EXTERN_DOWN_AS_LEFT        6 //外部有下拉电阻作为左耳
#define CONFIG_TWS_SECECT_BY_CHARGESTORE      7 //充电仓决定左右耳
#define CONFIG_TWS_CHANNEL_SELECT             CONFIG_TWS_MASTER_AS_LEFT//CONFIG_TWS_AS_LEFT_CHANNEL //配对方式选择
```

# 编译打包TOOLS包

- 因为tools包中有配置工具和升级文件。

![image-20250625163809376](./打开在线调试EQ并出tools包.assets/image-20250625163809376.png)

# 问题

`apps\earphone\include\app_config.h`这个不打开的话，编译会失败

```c
#define USB_PC_NO_APP_MODE                        1
```

![image-20250625175531936](./打开在线调试EQ并出tools包.assets/image-20250625175531936.png)

**不是SPP进不去界面。**

# 使用手机连接调EQ

一样的操作。
