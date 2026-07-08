# 疑问

**在TWS蓝牙耳机中大都使用触摸按键来控制。而挂脖式耳机，一般都有实体按键。比如一个三个实体按键分别承担不同的功能。挂脖式耳机为啥都用AD按键？而不是IO按键呢？AD按键与IO按键的区别与占用引脚的优势比较？IO按键的话，就需要三个不同的IO口的实现。AD按键可以根据不同的电压值来区分是哪一个按键？**

挂脖式耳机使用AD按键的主要原因是**引脚资源的限制和成本考量**：

1. **引脚数量限制**：挂脖式耳机的主控芯片（如蓝牙音频SOC）的可用GPIO引脚往往比较紧张，需要分配给多个功能（LED指示灯、充电检测、音频控制等）

2. **线材长度因素**：挂脖式耳机的按键通常位于线控部分，距离主控有一定距离，使用AD按键可以减少线材中的导线数量（只需2-3根线：GND、VCC、AD信号线）

3. **成本优势**：减少PCB走线和连接器引脚数量，降低整体BOM成本

## **AD按键 vs IO按键对比**

**AD按键（模拟按键）**

**工作原理**：

- 多个按键通过不同阻值的电阻分压，产生不同的电压值
- 单个ADC引脚检测电压，通过电压范围判断哪个按键被按下
- 例如：按键1=0.5V，按键2=1.0V，按键3=1.5V

**优势**：

- ✅ **只占用1个ADC引脚**（可支持3-5个按键）
- ✅ 减少线材导线数量（适合线控设计）
- ✅ 节省GPIO资源
- ✅ PCB布线简单

**劣势**：

- ❌ 需要ADC转换时间（通常几ms）
- ❌ 抗干扰能力相对较弱
- ❌ 不支持多按键同时按下检测
- ❌ 需要软件做电压范围判断和去抖动处理

## **IO按键（数字按键）**

**工作原理**：

- 每个按键连接一个独立的GPIO引脚
- 通过检测GPIO电平（高/低）判断按键状态
- 通常配置为上拉输入，按下时接地

**优势**：

- ✅ 响应速度快（微秒级）
- ✅ 抗干扰能力强
- ✅ 可以检测多按键组合（如音量+/-同时按）
- ✅ 软件处理简单

**劣势**：

- ❌ **每个按键占用1个GPIO**（3个按键=3个IO）
- ❌ 线控需要更多导线（N个按键需要N+1根线）
- ❌ 增加连接器成本

## **实际应用场景**

**挂脖式耳机（多用AD按键）**：

- 按键较多（3-5个：音量+/-、播放/暂停、接听/挂断等）
- 线控位置远离主控
- 对按键响应速度要求不高（音乐控制）

**TWS耳机（多用触摸按键）**：

- 空间极度受限
  - 可以占用很小的空间位置，而AD按键则是实体按键，通过按压连通电路，ADC由模拟电压值转换为数值
- 需要防水防汗
- 触摸IC通过I²C/SPI通信，只占用2-3个引脚
- 用户体验更好（无需按压力度）

## 典型AD按键电路示例

```c
VCC (3.3V)
  |
  R0 (10kΩ)
  |
  +--- 按键1 ---[R1=10kΩ]--- GND
  |
  +--- 按键2 ---[R2=22kΩ]--- GND
  |
  +--- 按键3 ---[R3=47kΩ]--- GND
  |
  +---> ADC引脚
```

不同按键按下时产生不同分压，MCU通过ADC采样电压值识别按键。

## ADC与DAC的区别

### **DAC（Digital-to-Analog Converter）**

**数字到模拟转换器**

- **功能**：将离散的数字信号转换为连续的模拟信号
- **方向**：数字 → 模拟
- 举例：
  - 数字音乐文件 → 模拟音频信号输出到扬声器
  - PWM波形 → 平滑的控制电压
  - 数字控制信号 → 电机调速的模拟电压

在蓝牙耳机中的应用

```c
【播放音乐流程】
手机数字音频 → 蓝牙传输 → 耳机解码芯片 
                                ↓
                              [DAC]
                                ↓
                          模拟音频信号 → 放大器 → 喇叭

【按键检测流程】
按键按下 → 分压电路产生电压(如1.5V)
              ↓
            [ADC]
              ↓
        数字值(如3072) → MCU判断是哪个按键
```

### **形象比喻**

- **ADC**：就像"翻译员"把你说的话（模拟）写成文字（数字）
- **DAC**：就像"播音员"把文字稿（数字）读出声音（模拟）

# 配置流程

## 板级文件选择

常规板级配置选择：

`apps\earphone\board\br36\board_config.h`

```c
/*
 *  板级配置选择
 */
#define CONFIG_BOARD_AC700N_DEMO
// #define CONFIG_BOARD_AC7006F_EARPHONE
// #define CONFIG_BOARD_AC700N_SD_PC_DEMO
//#define CONFIG_BOARD_AC7003F_DEMO
//  #define CONFIG_BOARD_AC700N_ANC
//#define CONFIG_BOARD_AC700N_DMS
```

## 按键选择与配置

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

```c
//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   1//DISABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_ADKEY_PORT                     IO_PORTC_05         //AD按键端口(需要注意选择的IO口是否支持AD功能)

#define TCFG_ADKEY_AD_CHANNEL               AD_CH_PC5
#define TCFG_ADKEY_EXTERN_UP_ENABLE        0// ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    220                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 30   / (30   + R_UP))     //3k
#define TCFG_ADKEY_AD2      (0x3ffL * 62   / (62   + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 91   / (91   + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 150  / (150  + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 240  / (240  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL * 330  / (330  + R_UP))     //33k
#define TCFG_ADKEY_AD7      (0x3ffL * 510  / (510  + R_UP))     //51k
#define TCFG_ADKEY_AD8      (0x3ffL * 1000 / (1000 + R_UP))     //100k
#define TCFG_ADKEY_AD9      (0x3ffL * 2200 / (2200 + R_UP))     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   1//4
#define TCFG_ADKEY_VALUE5                   1//5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   2// 9
```

### 配置分析

**基本参数**

- **ADC引脚**：IO_PORTC_05（PC5口）
- **上拉电阻**：内部上拉 10kΩ（R_UP = 100，实际100 × 100Ω = 10kΩ）
- **ADC精度**：10位（0x3ffL = 1023）
  - 就是总体分成多少份，在一个范围之内属于某一个按键。

电路拓扑结构

```c
VCC (VDDIO)
    |
   10kΩ (内部上拉)
    |
    +---> ADC引脚 (PC5)
    |
    +--- 按键0 --- 0Ω --- GND
    +--- 按键1 --- 3kΩ --- GND
    +--- 按键2 --- 6.2kΩ --- GND
    +--- 按键3 --- 9.1kΩ --- GND
    +--- 按键6 --- 33kΩ --- GND
    +--- 按键7 --- 51kΩ --- GND
    +--- 按键8 --- 100kΩ --- GND
    +--- 按键9 --- 220kΩ --- GND
```

### 电压检测阈值计算示例

根据代码定义，判定逻辑应该是：

```c
// 当ADC采样值 < VOLTAGE0 时 → 按键0
// 当ADC采样值 < VOLTAGE1 时 → 按键1
// 当ADC采样值 < VOLTAGE2 时 → 按键2
// ... 以此类推
```



**设计特点**

1. **容错设计**：使用中间值作为阈值，增加抗干扰能力
2. **灵活映射**：VALUE可以将多个物理按键映射到同一功能
3. **标准10位ADC**：0x3ffL = 1023，覆盖0-VDDIO电压范围

**典型按键功能推测**

根据挂脖式耳机的常见设计：

- **按键0 (0Ω - 短路)**：音量减（最常用，短路电阻最稳定）
- **按键1 (3kΩ)**：播放/暂停/接听挂断（多功能键）
- **按键2 (6.2kΩ)**：音量加
- **按键3 (9.1kΩ)**：下一曲
- **按键6 (33kΩ)**：上一曲或电源键

## 关闭TWS

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

```c
//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//
#define TCFG_USER_TWS_ENABLE                      0   //tws功能使能
```

## DAC与ANC以及麦的配置

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

```c
/*DAC硬件上的连接方式,可选的配置：
    DAC_OUTPUT_MONO_L               单左声道差分
    DAC_OUTPUT_MONO_R               单右声道差分
    DAC_OUTPUT_LR                   双声道差分
*/
#define TCFG_AUDIO_DAC_CONNECT_MODE            DAC_OUTPUT_LR
// #define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_MONO_R
// #define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_LR
```

- TWS一般都是单声道。7003为右声道，7006为左声道。
- 挂脖不是TWS，是双声道输出。
  - 外形耳机类似有线耳机造型，而且只有一块板子，单板估计是做不了ANC的。是两只耳机共同起作用的。

## 低功耗供电方式

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

```c
//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_DCDC15//PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			1   //芯片是否进入powerdown
/*强VDDIO等级配置,可选：
    VDDIOM_VOL_20V    VDDIOM_VOL_22V    VDDIOM_VOL_24V    VDDIOM_VOL_26V
    VDDIOM_VOL_30V    VDDIOM_VOL_30V    VDDIOM_VOL_32V    VDDIOM_VOL_36V*/
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_28V
/*弱VDDIO等级配置，可选：
    VDDIOW_VOL_21V    VDDIOW_VOL_24V    VDDIOW_VOL_28V    VDDIOW_VOL_32V*/
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_26V               //弱VDDIO等级配置
#define TCFG_LOWPOWER_OSC_TYPE              OSC_TYPE_LRC
#define TCFG_LOWPOWER_LIGHT_SLEEP_ATTRIBUTE 	LOWPOWER_LIGHT_SLEEP_ATTRIBUTE_KEEP_CLOCK 
```

- TWS一般都是选LDO。
  - Dongle也选。

# 按键流程

首先不使用配置工具的按键配置

`apps\earphone\user_cfg.c`

- 似乎公版都这样写。。。

```c
#define USE_CONFIG_BIN_FILE                  0

#define USE_CONFIG_STATUS_SETTING            1                          //状态设置，包括灯状态和提示音
#define USE_CONFIG_AUDIO_SETTING             USE_CONFIG_BIN_FILE        //音频设置
#define USE_CONFIG_CHARGE_SETTING            USE_CONFIG_BIN_FILE        //充电设置
#define USE_CONFIG_KEY_SETTING               USE_CONFIG_BIN_FILE        //按键消息设置
#define USE_CONFIG_MIC_TYPE_SETTING          USE_CONFIG_BIN_FILE        //MIC类型设置
#define USE_CONFIG_LOWPOWER_V_SETTING        USE_CONFIG_BIN_FILE        //低电提示设置
#define USE_CONFIG_AUTO_OFF_SETTING          USE_CONFIG_BIN_FILE        //自动关机时间设置
#define USE_CONFIG_COMBINE_VOL_SETTING       1					        //联合音量读配置
```

`apps\earphone\board\br36\board_ac700n_demo.c`

```c
/************************** KEY MSG****************************/
/*各个按键的消息设置，如果USER_CFG中设置了USE_CONFIG_KEY_SETTING为1，则会从配置文件读取对应的配置来填充改结构体*/
u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // SHORT                  LONG             HOLD                   UP                   DOUBLE          TRIPLE             四击              五击        六击            七击         八击         九击         十击     
    {KEY_MUSIC_PP,          KEY_POWEROFF,  KEY_POWEROFF_HOLD,        KEY_NULL,     KEY_CALL_LAST_NO,     KEY_OPEN_SIRI,    KEY_LOW_LANTECY,   KEY_NULL,    KEY_NULL,    KEY_DUT_MODE,  KEY_NULL,    KEY_NULL,   KEY_TENTH_CLICK},   //KEY_0
    {KEY_VOL_UP,            KEY_MUSIC_NEXT,    KEY_NULL,         KEY_NULL,          KEY_NULL,        KEY_NULL},   //KEY_1
    {KEY_VOL_DOWN,          KEY_MUSIC_PREV,  KEY_NULL,       KEY_NULL,          KEY_NULL,      KEY_NULL},   //KEY_2

    // 上滑               下滑          左滑               右滑
    {KEY_MUSIC_NEXT, KEY_MUSIC_PREV, KEY_NULL,           KEY_NULL,        KEY_NULL,             KEY_NULL},          //触摸按键滑动时的消息
};
```

整个工程的按键配置都在二维数组这里了。无论选择什么按键类型。这是怎么做到的？

## 二维数组

```c
enum {
    KEY_EVENT_CLICK,
    KEY_EVENT_LONG,
    KEY_EVENT_HOLD,
    KEY_EVENT_UP,
    KEY_EVENT_DOUBLE_CLICK,
    KEY_EVENT_TRIPLE_CLICK,
    KEY_EVENT_FOURTH_CLICK,
    KEY_EVENT_FIRTH_CLICK,
    KEY_EVENT_SIXTH_CLICK,
    KEY_EVENT_SEVEN_CLICK,
    KEY_EVENT_EIGHTH_CLICK,
    KEY_EVENT_NINTH_CLICK,
    KEY_EVENT_TENTH_CLICK,
    KEY_EVENT_USER,
    KEY_EVENT_MAX,
};
//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
#define KEY_NUM_MAX                        	10
#define KEY_NUM                            	3

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表

#define TCFG_KEY_TONE_EN					0//ENABLE		// 按键提示音。建议音频输出使用固定采样率
```

- 这两个参数决定了，整个SDK中可以有多少个按键，每个按键最多可以有多少种操作（单机，双击，长按。。。）。

- 每一个位置都有一个特定的按键事件映射，只需要把按键事件触发时的对应处理事件case填入即可。

## 按键处理流程

按键处理流程中直接调用这个二维数组：

`apps\earphone\key_event_deal.c`

```c
int app_earphone_key_event_handler(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;

    u8 key_event;

    if (key->type == KEY_DRIVER_TYPE_VOICE) {
        /* 语音消息 */
        ret = jl_kws_voice_event_handle(event);
        if (ret == true) {
            return ret;
        }
    }
#if TCFG_APP_MUSIC_EN
    if (event->arg == DEVICE_EVENT_FROM_CUSTOM) {
        log_e("is music mode msg\n");
        return false;
    }
#endif

#if (TCFG_EAR_DETECT_ENABLE && TCFG_EAR_DETECT_CTL_KEY)
    extern int cmd_key_msg_handle(struct sys_event * event);
    if (key->event == KEY_EVENT_USER) {
        cmd_key_msg_handle(event);
        return ret;
    }
#endif

#if TCFG_USER_TWS_ENABLE
    if (pbg_user_key_vaild(&key_event, event)) {
        ;
    } else
#endif
    {
        key_event = key_table[key->value][key->event];
    }

    void bt_sniff_ready_clean(void);
    bt_sniff_ready_clean();

#if RCSP_ADV_EN
    extern void set_key_event_by_rcsp_info(struct sys_event * event, u8 * key_event);
    set_key_event_by_rcsp_info(event, &key_event);
#endif

    log_info("key_event:%d %d %d\n", key_event, key->value, key->event);

#if LL_SYNC_EN
    extern void ll_sync_led_switch(void);
    if (key->value == 0 && key->event == KEY_EVENT_CLICK) {
        log_info("ll_sync_led_switch\n");
        ll_sync_led_switch();
        return 0;
    }
#endif

    switch (key_event) {
    case  KEY_MUSIC_PP:
#ifdef CONFIG_BOARD_AC700N_TEST
        tone_play_index(IDEX_TONE_BT_MODE, 0);
        mem_stats();
#endif
        /* tone_play_index(IDEX_TONE_NORMAL,1);
        break; */

        /*start_streamer_test();
        break;*/

        if ((get_call_status() == BT_CALL_OUTGOING) ||
            (get_call_status() == BT_CALL_ALERT)) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else if (get_call_status() == BT_CALL_INCOMING) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        } else if (get_call_status() == BT_CALL_ACTIVE) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else {
            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        }
        break;
```

- 直接根据二维数组映射出来的按键处理事件case进入不同的处理操作。

## 按键映射

```c
struct key_event {
    u8 init;
    u8 type;
    u16 event;
    u32 value;
    u32 tmr;
};
int app_earphone_key_event_handler(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;
	key_event = key_table[key->value][key->event];
```

- 调用映射时已经知道了是那个按键触发的什么事件。这里做的就是对应按键的对应事件触发的处理。
- 拿到按键的值（那个按键，key0,key1...）
- 拿到对应按键的事件（单机，双击，三击）

## 发送按键消息

`apps\earphone\earphone.c`

```c
/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    if (SYS_EVENT_REMAP(event)) {
        g_printf("****SYS_EVENT_REMAP**** \n");
        return 0;
    }

    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        if (bt_user_priv_var.fast_test_mode) {
            audio_adc_mic_demo_close();
            tone_play_index(IDEX_TONE_NORMAL, 1);
        }
#if CONFIG_BT_BACKGROUND_ENABLE
        if (bt_in_background()) {
            break;
        }
#endif
        app_earphone_key_event_handler(event);
        break;

    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
      
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            /*
             * tws事件处理函数
             */
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */

        break;

    case SYS_IR_EVENT:


        return 0;

    case SYS_PBG_EVENT:
        pbg_user_event_deal(&event->u.pbg);
        break;
//...
```

- 按键驱动识别成功后会发送按键消息，会有任务获取消息并处理。有轻量级操作系统的感觉了。
  - 回调函数形式。
- 首先对消息事件类型做判断什么样的类型事件进入什么case流程。
- 判断是按键消息事件时，就直接映射了`app_earphone_key_event_handler(event);`根据二维数组映射出具体的事件处理。

## 谁触发事件处理分发函数的

```c
static const struct application_operation app_earphone_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册earphone模式
 */
REGISTER_APPLICATION(app_earphone) = {
    .name 	= "earphone",
    .action	= ACTION_EARPHONE_MAIN,
    .ops 	= &app_earphone_ops,
    .state  = APP_STA_DESTROY,
};
```

- 是注册到OS中作为名称为`earphone`任务中的回调函数。
- 该任务有两个回调函数
  - 一个是当事件发生时触发回调函数
  - 一个是状态机，用来切换状态的。不同状态执行不同的操作。

## 按键扫描驱动

`apps\common\device\key\key_driver.c`

```c
//=======================================================//
// 按键初始化函数: 初始化所有注册的按键驱动
//=======================================================//
int key_driver_init(void)
{
    int err;

#if TCFG_IOKEY_ENABLE

#endif

#if TCFG_ADKEY_ENABLE
#ifdef CONFIG_ICRECORDER_CASE_ENABLE
    extern multi_adkey_init(const struct adkey_platform_data * multi_adkey_data);
    extern const struct adkey_platform_data multi_adkey_data[];
    extern struct key_driver_para multi_adkey_scan_para;
    err = multi_adkey_init(multi_adkey_data);
    if (err == 0) {
        sys_s_hi_timer_add((void *)&multi_adkey_scan_para, key_driver_scan, multi_adkey_scan_para.scan_time); //注册按键扫描定时器
    }
#else
    extern const struct adkey_platform_data adkey_data;
    extern struct key_driver_para adkey_scan_para;
    err = adkey_init(&adkey_data);
    if (err == 0) {
        sys_s_hi_timer_add((void *)&adkey_scan_para, key_driver_scan, adkey_scan_para.scan_time); //注册按键扫描定时器
    }
#endif
#endif

#if TCFG_IRKEY_ENABLE
    
#endif

#if TCFG_TOUCH_KEY_ENABLE
   
#endif

#if TCFG_ADKEY_RTCVDD_ENABLE
    extern const struct adkey_rtcvdd_platform_data adkey_rtcvdd_data;
    extern struct key_driver_para adkey_rtcvdd_scan_para;
    err = adkey_rtcvdd_init(&adkey_rtcvdd_data);
    if (err == 0) {
        sys_s_hi_timer_add((void *)&adkey_rtcvdd_scan_para, key_driver_scan, adkey_rtcvdd_scan_para.scan_time); //注册按键扫描定时器
    }
#endif

#if TCFG_RDEC_KEY_ENABLE
   
#endif

#if TCFG_CTMU_TOUCH_KEY_ENABLE

#endif /* #if TCFG_CTMU_TOUCH_KEY_ENABLE */

#if TCFG_SLIDE_KEY_ENABLE
   
#endif//TCFG_SLIDE_KEY_ENABLE

#if TCFG_6083_ADKEY_ENABLE
   
#endif
#if TCFG_TENT600_KEY_ENABLE
   
#endif

    return 0;
}
```

- 根据条件编译初始化各种按键类型下的按键扫描函数。
- 按键扫描事件判断函数都是一样的。

```c
//=======================================================//
// 按键扫描函数: 扫描所有注册的按键驱动
//=======================================================//
static void key_driver_scan(void *_scan_para)
```

## 按键扫描的参数

```c
    extern const struct adkey_platform_data adkey_data;
    extern struct key_driver_para adkey_scan_para;
    err = adkey_init(&adkey_data);
    if (err == 0) {
        sys_s_hi_timer_add((void *)&adkey_scan_para, key_driver_scan, adkey_scan_para.scan_time); //注册按键扫描定时器
    }
```

都已经配置好了，在板级配置文件中。

`apps\earphone\board\br36\board_ac700n_demo.c`

```c
/************************** AD KEY ****************************/
#if TCFG_ADKEY_ENABLE
const struct adkey_platform_data adkey_data = {
    .enable = TCFG_ADKEY_ENABLE,                              //AD按键使能
    .adkey_pin = TCFG_ADKEY_PORT,                             //AD按键对应引脚
    .ad_channel = TCFG_ADKEY_AD_CHANNEL,                      //AD通道值
    .extern_up_en = TCFG_ADKEY_EXTERN_UP_ENABLE,              //是否使用外接上拉电阻
    .ad_value = {                                             //根据电阻算出来的电压值
        TCFG_ADKEY_VOLTAGE0,
        TCFG_ADKEY_VOLTAGE1,
        TCFG_ADKEY_VOLTAGE2,
        TCFG_ADKEY_VOLTAGE3,
        TCFG_ADKEY_VOLTAGE4,
        TCFG_ADKEY_VOLTAGE5,
        TCFG_ADKEY_VOLTAGE6,
        TCFG_ADKEY_VOLTAGE7,
        TCFG_ADKEY_VOLTAGE8,
        TCFG_ADKEY_VOLTAGE9,
    },
    .key_value = {                                             //AD按键各个按键的键值
        TCFG_ADKEY_VALUE0,
        TCFG_ADKEY_VALUE1,
        TCFG_ADKEY_VALUE2,
        TCFG_ADKEY_VALUE3,
        TCFG_ADKEY_VALUE4,
        TCFG_ADKEY_VALUE5,
        TCFG_ADKEY_VALUE6,
        TCFG_ADKEY_VALUE7,
        TCFG_ADKEY_VALUE8,
        TCFG_ADKEY_VALUE9,
    },
};
#endif
```

`apps\common\device\key\adkey.c`

```c
u8 ad_get_key_value(void);
//按键驱动扫描参数列表
struct key_driver_para adkey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_AD,
    .get_value 		  = ad_get_key_value,
};
u8 ad_get_key_value(void)
{
    u8 i;
    u16 ad_data;

    if (!__this->enable) {
        return NO_KEY;
    }

    /* ad_data = adc_get_voltage(__this->ad_channel); */
    ad_data = adc_get_value(__this->ad_channel);
//     printf("ad_value ======================== %d \n", ad_data);
    for (i = 0; i < ADKEY_MAX_NUM; i++) {
        if ((ad_data <= __this->ad_value[i]) && (__this->ad_value[i] < 0x3ffL)) {
            return __this->key_value[i];
        }
    }
    return NO_KEY;
}
```

## 通用按键扫描函数

AD按键（基于ADC的多键复用按键）扫描流程是一个典型的**事件驱动 + 定时器 + 回调**架构，结合硬件采样与软件过滤逻辑，确保可靠检测按下/释放/长按等事件。整个过程可分为**初始化**、**周期扫描**和**事件上报**三个阶段：

1. 初始化阶段：
   - 配置硬件参数：通过adkey_data结构体设置ADC通道、电压阈值ad_value[]（基于电阻分压计算的采样值）和键值映射key_value[]（e.g., 键1对应KEY_UP）。
   - 定义扫描参数：adkey_scan_para包含通用行为（如scan_time=10ms、filter_time=2消抖、long_time=75长按阈值）和类型标识key_type=KEY_DRIVER_TYPE_AD，以及回调get_value=ad_get_key_value。
   - 调用adkey_init初始化ADC/引脚，成功后注册定时器sys_s_hi_timer_add，每10ms触发key_driver_scan。
2. 周期扫描阶段（每10ms执行key_driver_scan）：
   - **获取当前键值**：调用ad_get_key_value采样ADC值ad_data，遍历ad_value[]匹配阈值，返回对应key_value[i]（或NO_KEY）。
   - **通用过滤与判断**：比较当前键值与上次last_key，应用消抖（连续filter_time次确认）、按下计数press_cnt（长按/hold阈值）、连击计数click_cnt（等待click_delay_time后生成单击/双击等事件）。
   - **状态机逻辑**：处理按下（重置计数）、抬起（启动连击延时）、持续按下（累加长按）、无按下（检查连击超时）。
3. 事件上报阶段：
   - 生成事件（e.g., KEY_EVENT_CLICK, KEY_EVENT_LONG），填充sys_event结构体（包含key_type标识、键值、时间戳）。
   - 通过sys_event_notify上报上层，可选重映射key_event_remap、IR传感器过滤、键音播放。结束后更新last_key，重置计数。

整个流程高效、低功耗：定时器驱动避免轮询，ADC采样支持多键（最多10键）复用，阈值匹配简单可靠。潜在痛点如噪声可通过增加filter_time或hysteresis优化。

## AD按键的按键扫描流程

**怎么知道是哪个AD按键？**

函数内部动态调用cur_key_value = scan_para->get_value();（即ad_get_key_value）时才获取：

- ad_get_key_value基于当前ADC采样（adc_get_value(ad_channel)）和阈值ad_value[]，返回具体键值（如1表示第一个键）。
- 这是一个“懒加载”：入口知道“实例”，运行时才采样“键值”，支持实时变化（e.g., 按下不同键）。

流程示意图（进入函数瞬间）：

```c
定时器触发 → key_driver_scan(_scan_para = &adkey_scan_para)
            ↓
scan_para = (key_driver_para*)_scan_para  // 立即知道：类型=AD, get_value=ad_get_key_value, filter_time=2 等
            ↓
cur_key_value = scan_para->get_value();   // 动态调用ad_get_key_value() → ADC采样 → 返回具体键值 (e.g., KEY_UP)
            ↓
基于键值 + scan_para参数 → 消抖/事件生成
```

一直调用ADC按键采样函数，调用结果已经是知道那个AD按键了！

```c
u8 ad_get_key_value(void);
//按键驱动扫描参数列表
struct key_driver_para adkey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_AD,
    .get_value 		  = ad_get_key_value,
};
u8 ad_get_key_value(void)
{
    u8 i;
    u16 ad_data;

    if (!__this->enable) {
        return NO_KEY;
    }

    /* ad_data = adc_get_voltage(__this->ad_channel); */
    ad_data = adc_get_value(__this->ad_channel);
//     printf("ad_value ======================== %d \n", ad_data);
    for (i = 0; i < ADKEY_MAX_NUM; i++) {
        if ((ad_data <= __this->ad_value[i]) && (__this->ad_value[i] < 0x3ffL)) {
            return __this->key_value[i];
        }
    }
    return NO_KEY;
}
/************************** AD KEY ****************************/
#if TCFG_ADKEY_ENABLE
const struct adkey_platform_data adkey_data = {
    .enable = TCFG_ADKEY_ENABLE,                              //AD按键使能
    .adkey_pin = TCFG_ADKEY_PORT,                             //AD按键对应引脚
    .ad_channel = TCFG_ADKEY_AD_CHANNEL,                      //AD通道值
    .extern_up_en = TCFG_ADKEY_EXTERN_UP_ENABLE,              //是否使用外接上拉电阻
    .ad_value = {                                             //根据电阻算出来的电压值
        TCFG_ADKEY_VOLTAGE0,
        TCFG_ADKEY_VOLTAGE1,
        TCFG_ADKEY_VOLTAGE2,
        TCFG_ADKEY_VOLTAGE3,
        TCFG_ADKEY_VOLTAGE4,
        TCFG_ADKEY_VOLTAGE5,
        TCFG_ADKEY_VOLTAGE6,
        TCFG_ADKEY_VOLTAGE7,
        TCFG_ADKEY_VOLTAGE8,
        TCFG_ADKEY_VOLTAGE9,
    },
    .key_value = {                                             //AD按键各个按键的键值
        TCFG_ADKEY_VALUE0,
        TCFG_ADKEY_VALUE1,
        TCFG_ADKEY_VALUE2,
        TCFG_ADKEY_VALUE3,
        TCFG_ADKEY_VALUE4,
        TCFG_ADKEY_VALUE5,
        TCFG_ADKEY_VALUE6,
        TCFG_ADKEY_VALUE7,
        TCFG_ADKEY_VALUE8,
        TCFG_ADKEY_VALUE9,
    },
};
#endif
```

- 通过电压阈值判断得到具体是那个按键

```c
.key_type		  = KEY_DRIVER_TYPE_AD,
```

- 没啥鸡巴用，上层都不分，直接按照按键事件处理，部分具体按键类型

```c
    e.type = SYS_KEY_EVENT;
    e.u.key.init = 1;
    e.u.key.type = scan_para->key_type;//区分按键类型
```

## 为什么不同类型按键（如AD、IO、触摸）能使用同一个扫描函数key_driver_scan？

核心在于**参数化 + 回调设计**，实现“**硬件无关的通用逻辑** + **类型特有的输入适配**”。key_driver_scan不关心具体按键如何“感知”输入（e.g., AD用电压、IO用GPIO电平、触摸用电容），只需统一处理“键值变化 → 事件生成”的软件逻辑。这类似于MVC模式：回调封装“模型”（硬件），函数处理“控制器”（状态机），上层是“视图”（事件消费）。

### 关键机制解释

- 每个类型独立参数实例：不同按键（如AD的adkey_scan_para、IO的io_key_para）都是struct key_driver_para的实例，包含：
  - **共享通用参数**：filter_time（消抖）、long_time（长按阈值）、click_delay_time（连击延时）等，用于所有类型的状态判断。这些参数类型化（e.g., AD消抖短因噪声小，IO长因机械抖动），但逻辑统一。
  - **类型标识**：key_type（e.g., AD=1, IO=2），在上层事件中区分处理（e.g., AD事件可加电压日志，IO可忽略长按）。
  - **状态变量**：last_key、press_cnt、click_cnt等，实例私有，确保AD的计数不干扰IO。
- 回调隔离硬件细节：get_value()函数指针是“桥接”：
  - AD：ad_get_key_value → ADC采样 + 阈值匹配 → 返回键码。
  - IO：io_get_key_value → GPIO轮询 → 返回按下位掩码。
  - 触摸：touch_get_key_value → 电容阈值比较 → 返回手势码。
  - 扫描函数只需cur_key_value = scan_para->get_value()获取“抽象键值”（u8整数，如1=上键），后续判断（如cur_key_value != last_key）通用适用。
- **定时器多实例支持**：每个类型注册自己的定时器（周期可不同，e.g., 触摸10ms、IO20ms），回调统一指向key_driver_scan，传入各自_scan_para。这样并行扫描，无冲突。

这种设计符合开闭原则（对扩展开放、对修改关闭）：新增类型（如矩阵键盘）只需实现get_value和para，无需改扫描函数。实际中，它节省了~70%的代码重复（每个类型只需~50行特有逻辑），并确保一致的用户体验（e.g., 所有键长按阈值类似）。

# 回头看配置

```c
/************************** KEY MSG****************************/
/*各个按键的消息设置，如果USER_CFG中设置了USE_CONFIG_KEY_SETTING为1，则会从配置文件读取对应的配置来填充改结构体*/
u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // SHORT                  LONG             HOLD                   UP                   DOUBLE          TRIPLE             四击              五击        六击            七击         八击         九击         十击     
    {KEY_MUSIC_PP,          KEY_POWEROFF,  KEY_POWEROFF_HOLD,        KEY_NULL,     KEY_CALL_LAST_NO,     KEY_OPEN_SIRI,    KEY_LOW_LANTECY,   KEY_NULL,    KEY_NULL,    KEY_DUT_MODE,  KEY_NULL,    KEY_NULL,   KEY_TENTH_CLICK},   //KEY_0
    {KEY_VOL_UP,            KEY_MUSIC_NEXT,    KEY_NULL,         KEY_NULL,          KEY_NULL,        KEY_NULL},   //KEY_1
    {KEY_VOL_DOWN,          KEY_MUSIC_PREV,  KEY_NULL,       KEY_NULL,          KEY_NULL,      KEY_NULL},   //KEY_2

    // 上滑               下滑          左滑               右滑
    {KEY_MUSIC_NEXT, KEY_MUSIC_PREV, KEY_NULL,           KEY_NULL,        KEY_NULL,             KEY_NULL},          //触摸按键滑动时的消息
};
#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   1//4
#define TCFG_ADKEY_VALUE5                   1//5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   2// 9
```

- demo.c中只有三个按键，key0,1,2 滑动按键。
- 有三个范围指向key1,两个范围指向key2
- 实际有三个实体按键，但是他们的电压值不知道，所以不清楚。
- 但是处理是识别key->value,只配置了三个。
  - 映射还修改了TCFG_ADKEY_VALUE4以上的按键映射，估计是为了使用后期的误差。
  - 不然只处理key0,key1,key2,key3,且实际只有三个AD实体按键，生效了key0,1,2。感觉后面不用修改的，电阻分压中的电阻应该是依次递增的。

