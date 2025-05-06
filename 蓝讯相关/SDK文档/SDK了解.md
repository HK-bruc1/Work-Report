# SDK介绍

**以AB5656A3芯片为例**

## SDK目录结构

![image-20250426104041330](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426104041330.png)

![image-20250426104058156](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426104058156.png)

## 开发流程

![image-20250426104226301](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426104226301.png)

## 新建一个project

![image-20250426104336233](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426104336233.png)

- setting中的配置是通过工具的。
- 相似的功能，需要代码实现的，工程复制在修改。

## 修改配置文件

`projects\earphone\xcfg.h`

![image-20250426160846005](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426160846005.png)

## 修改按键、显示

![image-20250426161155286](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426161155286.png)

## 修改提示音

![image-20250426161412965](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426161412965.png)

- 如果是新建的资源文件夹的话，要跟其他文件夹结构一样

![image-20250426161616341](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426161616341.png)

# SDK代码解析

## SDK目录结构

```c
└─app

    ├─platform       

    │  ├─bsp        //底层外设相关

    │  ├─functions    //功能相关

    │  ├─gui        //显示功能

    │  ├─header

    │  └─libs

    └─projects //调用API

        └─earphone

            ├─display    //显示

            ├─message  //消息处理

            ├─Output    //文件输出

            │  └─bin    //音乐文件，配置

            │      ├─res

            │      │  ├─en

            │      │  ├─eq

            │      │  └─zh

            │      └─Settings

            │          └─Resources

            │              ├─S6

            │              │  ├─en

            │              │  └─zh

            │              └─TWS

            │                  ├─en

            │                  └─zh

            ├─plugin //插件

            └─port //移植
```

### bsp目录

该目录下，包含一些和底层硬件相关的数据，函数初始化

```c
├─bsp

│      bsp.h

│      bsp_audio.c

│      bsp_audio.h

│      bsp_aux.c

│      bsp_aux.h

│      bsp_ble.c

│      bsp_ble.h

│      bsp_bpap.c

│      bsp_bt.c

│      bsp_bt.h

│      bsp_charge.c

│      bsp_charge.h

│      bsp_cm.c

│      bsp_dac.c

│      bsp_dac.h

│      bsp_dump_buf_huart.c

│      bsp_eq.c

│      bsp_eq.h

│      bsp_fmrx.c

│      bsp_fmrx.h

│      bsp_fmtx.c

│      bsp_fmtx.h

│      bsp_fs.c

│      bsp_fs.h

│      bsp_hdmi.c

│      bsp_hdmi.h

│      bsp_hdmi_cec_msg.c

│      bsp_i2c.c

│      bsp_i2c.h

│      bsp_i2s.c

│      bsp_i2s.h

│      bsp_i2s_ta5711.c

│      bsp_i2s_ta5711.h

│      bsp_i2s_wm8978.c

│      bsp_i2s_wm8978.h

│      bsp_iap.c

│      bsp_id3_tag.c

│      bsp_id3_tag.h

│      bsp_iic_putchar.c

│      bsp_iis_ext.c

│      bsp_iis_ext.h

│      bsp_ir.c

│      bsp_ir.h

│      bsp_karaok.c

│      bsp_karaok.h

│      bsp_key.c

│      bsp_key.h

│      bsp_led.c

│      bsp_led.h

│      bsp_lrc.c

│      bsp_lrc.h

│      bsp_map.c

│      bsp_music.c

│      bsp_music.h

│      bsp_param.c

│      bsp_param.h

│      bsp_piano.c

│      bsp_piano.h

│      bsp_record.c

│      bsp_record.h

│      bsp_spiflash1.c

│      bsp_spiflash1.h

│      bsp_spiflash1_music_bin.c

│      bsp_spi_audio.h

│      bsp_spp.c

│      bsp_sys.c

│      bsp_sys.h

│      bsp_tkey.c

│      bsp_tkey.h

│      bsp_uart.c
```

### functions目录

```c
├─functions

│      func.c

│      func.h

│      func_aux.c

│      func_aux.h

│      func_bt.c

│      func_bt.h

│      func_bt_dut.c

│      func_bt_dut.h

│      func_bt_hid.c

│      func_bt_hid.h

│      func_clock.c

│      func_clock.h

│      func_exspiflash_music.c

│      func_exspiflash_music.h

│      func_fmrx.c

│      func_fmrx.h

│      func_hdmi.c

│      func_hdmi.h

│      func_i2s.c

│      func_i2s.h

│      func_idle.c

│      func_idle.h

│      func_lowpwr.c

│      func_lowpwr.h

│      func_music.c

│      func_music.h

│      func_spdif.c

│      func_spdif.h

│      func_speaker.c

│      func_speaker.h

│      func_update.c

│      func_update.h

│      func_usbdev.c

│      func_usbdev.h

│      sfunc_bt_call.c

│      sfunc_bt_ota.c

│      sfunc_bt_ring.c

│      sfunc_record.c

│      sfunc_record.h

│      sfunc_record_play.c

│      sfunc_record_play.h
```

### Message目录（重要）

主要包括按键消息处理，是蓝牙方案经常需要改动的目录

```c
 ├─message

    │      msg_aux.c

    │      msg_bt.c

    │      msg_clock.c

    │      msg_exspiflash_music.c

    │      msg_fmrx.c

    │      msg_hdmi.c

    │      msg_i2s.c

    │      msg_idle.c

    │      msg_music.c

    │      msg_record.c

    │      msg_spdif.c

    │      msg_speaker.c

    │      msg_usbdev.c

```

### Plugin目录

```c
音乐文件的调用，基本不会去修改这里

    ├─plugin

    │      eq_table.c

    │      multi_lang.c

    │      multi_lang.h

    │      plugin.c

    │      plugin.h

```

### Port目录

主要包括硬件外设的调用，mani函数

```c
└─port

            port_earphone.c

            port_earphone.h

            port_ir.c

            port_key.c

            port_led.c

            port_led.h

            port_ledseg.c

            port_linein.c

            port_linein.h

            port_mic.c

            port_mic.h

            port_mute.c

            port_pwm.c

            port_pwm.h

            port_sd.c

            port_sd.h

            port_sd1.c

            port_tkey.c

            port_tkey.h

            port_update.c
```

## 代码运行流程

### 初始化

`app\projects\earphone\main.c`

```c
//正常启动Main函数
int main(void)
{
    printf("Hello AB565XA3: %x\n", LVDCON);
    bsp_sys_init();
    func_run();
    return 0;
}
```

- bsp_sys_init();函数主要包括各种功能的初始化，获取download工具的配置。

- func_run();主要是处理蓝牙消息和硬件的消息。

### 各模式循环

初始化之后，进入一个FUN函数，蓝牙耳机的FUN函数基本上都在跑func_bt。（**其他应用模式开启需要去config.h中使能，不然就算状态切换到对应模式应用也不会执行**）

- 循环模型的状态机，只要func_cb.sta的状态不变就会一直跑某一个模式。在某个模式中func_cb.sta发生了改变，就会退到大循环中进行模式选择。前提是对应宏使能了，不然没有用。

```c
AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);

    func_bt_chk_off();
    while (1) {
        func_clear();

        switch (func_cb.sta) {
#if FUNC_BT_EN
        case FUNC_BT:
            func_bt();
            break;
#endif

#if BT_DUT_TEST_EN
        case FUNC_BT_DUT:
            func_bt_dut();
            break;
#endif

#if FUNC_BTHID_EN
        case FUNC_BTHID:
            func_bthid();
            break;
#endif // FUNC_BTHID_EN

#if FUNC_AUX_EN
        case FUNC_AUX:
            func_aux();
            break;
#endif // FUNC_AUX_EN

#if FUNC_SPEAKER_EN
        case FUNC_SPEAKER:
            func_speaker();
            break;
#endif // FUNC_SPEAKER_EN

        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwrdwn_tone_en);
            break;

#if BT_FCC_TEST_EN
        case FUNC_BT_FCC:
            func_bt_fcc();
            break;
#endif // BT_FCC_TEST_EN

        default:
            func_exit();
            break;
        }
    }
}
```

### 蓝牙模式

蓝牙功能函数

- 大循环选择一个模式后，模式中是一个小循环。除非func_cb.sta 状态改变退出到大循环中进行模式选择，不然一直在模式的小循环中跑。

```c
AT(.text.bfunc.bt)
void func_bt(void)
{
    printf("%s\n", __func__);

    func_bt_enter();

    while (func_cb.sta == FUNC_BT) {
        func_bt_process();
        func_bt_message(msg_dequeue());
    }

    func_bt_exit();
}
```

在程序跑到func_bt();的时候，SDK留给开发者处理的**只有消息处理和电量检测，来电检测等**，蓝牙耳机接收音频信号，解码那些都是屏蔽起来的。

- **感觉修改的都是UI界面，和定制化的各种消息处理以及灯效处理**

#### 进入蓝牙模式

- 蓝牙入口函数

  - func_bt_enter();

  - 主要执行蓝牙初始化，播报提示音

#### 蓝牙模式大循环

`func_bt_process();`

- 包括响铃，通话两种状态的消息处理，他们也有按键消息处理

  - sfunc_bt_ring();

  - sfunc_bt_call();

`func_bt_message`

- 普通蓝牙模式下的消息处理，音乐播放感觉也在这
  - func_bt_message_do

#### C语言的特殊宏

- LINE 表示正在编译的文件的行号

- FILE 表示正在编译的文件的名字

- DATE_ 表示编译时刻的日期字符串，例如： “25 Dec 2007”

- TIME 表示编译时刻的时间字符串，例如： “12:30:55”

```c
#include <stdio.h>

int main(void)
{
        printf("%s\r\n",__FILE__);

        printf("%d\r\n",__LINE__);

        printf("%s\r\n",__DATE__);

        printf("%s\r\n",__TIME__);

        return 0;
}

打印结果：
speci_define.c
6
Jul  6 2019
00:46:39
```

##### 开启打印

**去config.h中打印配置中选一个串口的引脚，常用PB3，但是有一个灯效也是用PB3所以使用时需要避免冲突，不需要打印时，把宏修改回去。**

### 消息

经常需要改动的部分

#### 消息处理

二次开发中修改最多的部分就是消息处理这一块，按键消息处理的修改最多。按键消息有长按，短按，双击，三击，四击，五击等等

```c
void func_bt_message_do(u16 msg)
{
    int klu_flag = 0;
    u8 ku_sel = xcfg_cb.user_def_ks_sel;

    switch (msg) {
    case KU_PLAY:
        ku_sel = UDK_PLAY_PAUSE;
    case KU_PLAY_USER_DEF:
    case KU_PLAY_PWR_USER_DEF:
//        key_voice_play(501, 100, 3);                                        //按键音
        if (!bt_nor_is_connected()) {
            bt_tws_pair_mode(3);                                            //单击PLAY按键手动配对
            break;
        }
        user_def_key_msg(ku_sel);
        break;

    case KL_PLAY_PWR_USER_DEF:
        if (!xcfg_cb.kl_pwrdwn_dis) {
            klu_flag = 1;                                                   //长按抬键的时候呼SIRI
        }
    case KL_PLAY_USER_DEF:
        f_bt.user_kl_flag = 0;
        if (xcfg_cb.user_def_kl_sel == UDK_GAME_SWITCH) {
            klu_flag = 0;
        }
        if (!bt_tws_pair_mode(4)) {                                         //是否长按配对功能
            if (user_def_lkey_tone_is_enable(xcfg_cb.user_def_kl_sel)) {
                sys_warning_play(T_WARNING_NEXT_TRACK, 1);                  //长按“滴”一声
//                tws_res_play(TWS_RES_TONE);                                 //tws同步播放
            }
            if (klu_flag) {
                f_bt.user_kl_flag = user_def_func_is_ready(xcfg_cb.user_def_kl_sel);     //长按抬键的时候再处理
            } else {
                user_def_key_msg(xcfg_cb.user_def_kl_sel);
            }
        }
        break;

        //SIRI, NEXT, PREV在长按抬键的时候响应,避免关机前切歌或呼SIRI了
    case KLU_PLAY_PWR_USER_DEF:
        if (f_bt.user_kl_flag) {
            user_def_key_msg(xcfg_cb.user_def_kl_sel);
            f_bt.user_kl_flag = 0;
        }
        break;

        //长按调音量
    case KH_PLAY_PWR_USER_DEF:
        if (!xcfg_cb.kl_pwrdwn_dis) {
            break;
        }
    case KH_PLAY_USER_DEF:
        func_message(get_user_def_vol_msg(xcfg_cb.user_def_kl_sel));
        break;
```

#### 消息来源

以按键为例

void msg_enqueue(u16 msg);//消息队列

- 将不同按键事件放入消息队列，等待调用msg_dequeue取出事件根据所处状态做对应的处理。

```c
放入消息队列，使用定时器定时扫描app\platform\bsp\bsp_sys.c
AT(.com_text.bsp.key)
u8 bsp_key_scan(void)
{
    u8 key_val;
    u16 key = NO_KEY;

    key_val = key_scan();
#if VBAT_DETECT_EN
    sys_cb.vbat = get_vbat_val();
#endif // VBAT_DETECT_EN

#if USER_TKEY_SHORT_SLIDE
    key = bsp_key_slide_process(key_val);
#else
    key = bsp_key_process(key_val);
#endif

#if USER_TKEY_SLIDE
    key = bsp_tkey_slide_process(key);
#endif
    if ((key != NO_KEY) && (!bsp_key_pwron_filter(key))) {
        //防止enqueue多次HOLD消息
        if ((key & KEY_TYPE_MASK) == KEY_LONG) {
            sys_cb.kh_vol_msg = (key & 0xff) | KEY_HOLD;
        } else if ((key & KEY_TYPE_MASK) == KEY_LONG_UP) {
            msg_queue_detach(sys_cb.kh_vol_msg, 0);
            sys_cb.kh_vol_msg = NO_KEY;
        } else if (sys_cb.kh_vol_msg == key) {
            msg_queue_detach(key, 0);
        }
#if WAV_KEY_VOICE_QUICK_EN
        if (key == K_PLAY_PWR_USER_DEF) {
            sys_cb.tws_res_brk = 1;
        }
#endif
#if LED_188LED_DISP_EN
        if (key == K_PLAY_PWR_USER_DEF) {
            bsp_188led_disp_set_on();
        }
#endif
//        printf(key_msg_str, key);
        msg_enqueue(key);
    }
    return key_val;
}

从消息队列中取出消息事件做对应的处理
func_bt_message(msg_dequeue());
传递到func_bt_message_do(msg);
```

#### 按键消息的注意事项

下面的宏都是按键消息：

以PLAY按键为例

```c
#define K_PLAY                  (KEY_PLAY | KEY_SHORT) //下降沿

#define KU_PLAY                 (KEY_PLAY | KEY_SHORT_UP) //上升沿

#define KL_PLAY                 (KEY_PLAY | KEY_LONG) //长按

#define KLU_PLAY                (KEY_PLAY | KEY_LONG_UP) //长按上升沿

#define KH_PLAY                 (KEY_PLAY | KEY_HOLD) //长按2秒左右

#define KD_PLAY                 (KEY_PLAY | KEY_DOUBLE) //双击

#define KTH_PLAY                (KEY_PLAY | KEY_THREE) //三击

#define KFO_PLAY                (KEY_PLAY | KEY_FOUR) //四击

#define KFI_PLAY                (KEY_PLAY | KEY_FIVE) //五击
```

除了KU_PLAY按键，配置工具中还可以将按键消息类型定义为KU_PLAY_USER_DEF，KU_PLAY_PWR_USER_DEF。但是除了个例好像都是指向同一个处理函数，每一种按键操作的类型都可以分为以上三种，一般三种都指向同一个处理函数，但是从不同分支进来会做一些处理，可以看上面的代码。

**注意！每次按键都会触发下降沿。**

以蓝牙模式为例：

程序先在func_bt_message函数做判断，如果在该函数没有找到一致的case，则会跑到公共的消息处理函数中 void func_message(u16 msg) 再做判断。

- 在`user_def_key_msg`没有符合的功能代码，就会进入公共消息处理函数

```c
    } else if (func_sel == UDK_MODE) {                  //MODE
        func_message(KU_MODE);
    } else {                                            //VOL+, VOL-
        func_message(get_user_def_vol_msg(func_sel));
```

#### 应用：1S消息

在定时器中，每隔一秒发送一个消息MSG_SYS_1S

在蓝牙消息或者公共消息做处理，常用的1秒消息处理有报告电量，连接蓝牙自动播放。

- 一般会在实质消息处理前，会先处理空消息和定时消息

```c
AT(.text.bfunc.bt)
void func_bt_message(u16 msg)
{
    if (msg == NO_MSG || msg == MSG_SYS_1S) {       //减少flash缺页
        func_bt_message_m(msg);
        if (msg == MSG_SYS_1S) {
        }
    } else {
        func_bt_message_do(msg);
    }
}
```

#### 蓝牙消息函数

三个状态的消息处理，蓝牙模式比较特殊，除了一个func_bt_message还有两个，响铃，通话。

- 先判断当前的状态，根据状态进入不同的处理分支。比如响铃的长按和普通模式下的长按处理不一样。配置工具的设置都不一样。

**响铃:void sfunc_bt_ring_message(u16 msg)**

- 来电响铃的时候执行消息处理，主要包括接/挂电话，电量报告和按键消息公共处理。

**通话中:sfunc_bt_call_message();**

- 通话过程的按键消息处理，主要包括音量调整，三方通话，电量报告

**Music: void func_bt_message(u16 msg)**

- 蓝牙音乐模式的消息处理，上下曲切换，暂停播放，音量调整，报告电池电量等

**所以为什么周期消息是单独处理的，不进入那种switch case语句，可能太费时间。**

## TWS 组队和蓝牙配对过程分析

在对中科蓝讯 SDK 的二次开发过程中，我们时常需要对 TWS 组队和蓝牙配对功能进行修改，或在此过程中添加功能。

### 灯效显示函数概览

- **核心函数**：`func_bt_disp_status()`
- **作用**：根据不同蓝牙状态（TWS 组队/蓝牙配对的各个阶段），在左右耳机分别调用对应的灯效子函数。
  - **烧录两版软件，这个不用担心找不到左右耳自己的灯效子函数**
- **实现方式**：通过 `switch(status)` 结构，每个 `case` 对应一个阶段，由下面的子函数完成具体的红/蓝灯闪烁逻辑

### 灯效流程详解

1.**首次上电（无程序差异）**

- **显示**：开机灯效。
- **进入扫描**：状态 `BT_STA_SCANNING`，调用 `led_bt_scan()`，灯效为“300 ms 亮、450 ms 灭”，红蓝交替闪

2.**TWS 组队（双击 USER_DEF 键触发）**

- **并行**：TWS 组队与蓝牙模块开启同时进行
- **主耳（发起方）**
  - 调用 `bt_tws_search_slave(15000)`，成功后进入 `BT_STA_IDLE`，调用 `led_bt_idle()`，灯效同扫描阶段（300 ms 亮/450 ms 灭，红蓝交替）。
  - 播放“组队完成”提示音。
- **副耳**
  - 进入 `BT_STA_CONNECTED`，调用 `led_bt_connected()`，“蓝灯亮 100 ms，灭 5 s”

3.**蓝牙配对完成**

- 主、副耳均进入 `BT_STA_CONNECTED`，调用 `led_bt_connected()`，统一“蓝灯亮 100 ms，灭 5 s”

### 功能修改需求

**目标**：在 TWS 组队完成后，先红蓝灯同时亮 1 s，再由主耳单独以 100 ms 频率交替闪烁，副耳熄灯。

**自定义灯效结构体**

```c
typedef struct {
  uint8_t red_pattern;   // 红灯位图（倒序处理）
  uint8_t blue_pattern;  // 蓝灯位图（倒序处理）
  uint8_t t1;            // 位间时间 = t1 × 50 ms
  uint8_t t2;            // 周期间时间 = t2 × 50 ms
} led_cfg_t;
```

例：红蓝同时亮 1 s + 主耳 100 ms 交替闪

```c
// 倒序后位图：0b01100000 = 0x06
led_cfg_t cfg = { 0x06, 0x06, 10, 255 };
```

- 在配置工具中，可通过填写 `{0x00,0x55,10,0}` 等方式定义更多自定义效果

**同步/不同步灯效函数**

- 新增 `led_set_sta_choice(const void *cfg, int sync)`：
  - `sync = 1`：主、副耳同步闪；
  - `sync = 0`：分别执行各自灯效

**100 ms 队列消息机制**

- 在消息队列中加入 100 ms 定时消息，用于驱动主耳快速闪烁，副耳保持灭灯

**逻辑实现要点**

- TWS 完成判定：

```c
if (bt_tws_is_connected() && !tws_connected) {
  tws_connected = 1;
  delay_5ms(200);  // 保证红蓝同时亮 1 s
  led_set_sta_choice(&cfg, bt_tws_is_slave() ? 0 : 1);
}
```

- 蓝牙配对完成判定

```c
if (bt_nor_is_connected()) {
  // 避免副耳在未配对时误入
}
```

- 断开重置

```c
case BT_NOTICE_DISCONNECT:
  tws_connected = 1;  // 重新进入主耳快速闪烁模式
  break;
```

#### 按“先 TWS，后配对”顺序的蓝牙流程调整

1.关闭上电自动回连

```c
bt_set_reconnect_times(0);
```

- 防止组队后立即回连手机

2.初始不可被发现

```c
bt_set_scan(0x00);  // 不可发现
```

- 延迟至组队超时后再开放扫描

3.计时控制

- 在 `sys_cb` 中新增 `tws_connect_start` 记录组队开始时钟。

- 在 `msg_bt.c` 中检测：

  - 若超过 5 s，允许回连或可见；

  - 若超过 10 s 且无配对记录，调用 `bt_set_scan(0x03)` 开放扫描

## TWS左右声道分配

### SDK setting 配置左右声道

![image-20250506231358784](./SDK了解.assets/image-20250506231358784.png)

- 第一种“不分配”，即不对耳机声道进行指定，均可以输出双声道，**这种方式在 TWS 耳机中通常不会采用，可以用做蓝牙音箱的开发；**
  - 这相当于两个耳机各放各的，互不打扰

- 第二种“自动分配”，这种方式也没有对耳机的声道进行直接的指定，根据选择“主右声道副左声道”或“主左声道副右声道”，通过 TWS 之间主副耳机来确定声道，**但是使用这种方式没法保证稳定的主从关系就会导致左右耳机之间声道混乱，也不适合 TWS 耳机的使用；**
  - 机器的左右耳是固定的，主从也应该定下来，到时候把灯效修改一下即可。

- 第三种“硬件选择”，这种方式在硬件设计时，通过左右耳机的硬件上的连接来做左右声道的分配，其中又可以通过两种配置来确定，一个是在左耳的 PWRKEY 引脚接 820K 欧姆的电阻到地，另一种是使用较多的方式，即选择一个 IO 脚位接地，来配置为左，对应的 IO 没有接地的一侧则为右，IO 口的配置同样可以在 setting 中选择，硬件上设计上可以在该 IO 口预留一个 0 欧姆电阻接地，左边耳机焊接，右边耳机 NC。
- 第四种“固定配置”，除了硬件上做选择，还有软件上做选择，在声道分配中，选择“配置选择为左声道”或“配置选择为右声道”，软件烧录后会固定该耳机的声道分配，**这种方式需要分两个配置文件，即两版烧录软件，对应左右耳机，但不需要在硬件上做更改**，上一种方式**通过硬件来做区分则是可以左右耳机烧录同一版软件**，但在设计上会占用一个 IO 资源，在 IO 口有空余时可以使用。
  - 第三种需要使用一个IO口。第四种使用两版烧录软件即可。

### 根据蓝牙地址配置左右声道（不可行）

以上四种方式在 SDK 中可以找到对应的检测的位置，在 xcfg.h 中可以看到配置的变量，在工程代码中追这个变量就可以找到配置生效的地方；

![image-20250506232321330](./SDK了解.assets/image-20250506232321330.png)

可以看到在软件中不过是读取了 setting 中配置的内容，从而进行相应的硬件 IO 检测或软件设置；

![image-20250506232511287](./SDK了解.assets/image-20250506232511287.png)

在AB5656A3中，此函数不可见。在bsp_sys_init初始化函数中调用。TWS声道初始化。

~~那么同样的在这里也可以增加一种新的左右声道分配方式，使用蓝牙地址去配置，使用过蓝讯芯片的小伙伴应该知道，芯片的蓝牙地址是可以在 setting 中配置，烧录后生效。~~

- **现在函数不可见，此方法无效**

蓝牙地址的配置方式也有几种，这里不做赘述，主要可以看蓝牙地址的单次递增方式，或区间循环方式，这样在烧录配置蓝牙地址的时候，耳机蓝牙地址会根据配置形成一定的奇偶关系；

![image-20250506232920132](./SDK了解.assets/image-20250506232920132.png)

xcfg.h 中，同样可以找到对应配置里的蓝牙地址项；

app\projects\earphone\xcfg.h

```c
u8 bt_addr[6];                              //蓝牙地址
```

在 tws_lr_xcfg_sel() 中可以去掉 SDK 中原有的配置，添加通过耳机蓝牙地址的奇偶性质来对左右声道进行分配，如下；

![image-20250506233146557](./SDK了解.assets/image-20250506233146557.png)

**这样的话确实就可以烧录一版软件，但是还是不方便，因为有时候左右耳机的触摸按键不是完全相同的。烧录同一版软件的话，无法区分左右的个性化操作。**

# 工具使用

## X-link连接

![image-20250426161819919](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426161819919.png)

啥设备有USB接口。。。有个3.5耳机接口就不错了。

## EQ工具

![image-20250426162235917](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426162235917.png)

## 测试盒

![image-20250426162317379](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250426162317379.png)

# 耳机充电配置

## 电池充电过程

在TWS耳机中，通常使用锂电池。锂电池的充电过程可以分为四个阶段:

1. **涓流充电**: 当电池电压过低（如过放后）时，使用较小的电流进行预充电.
2. **恒流充电**: 当电池电压涓流充电到一定电压阈值（2.9V或3V）时，进入恒流充电模式，此时电压范围通常是3V - 4.2V.
3. **恒压充电**: 当电压达到4.2V后进入恒压充电模式，此时充电电流会逐步降低.
4. **充电截止**: 充电停止.

充电停止的判断通常有两种方式:

1. **定时涓流充电时间**: 恒压充电电压达到后开始计时恒压充电的时间，达到设定时间后停止充电.
2. **根据电流判断**: 在恒压充电过程中，电流会逐步降低，当电流低于设定的低阈值时停止充电.

## SDK 中充电配置

**中科蓝讯芯片中内置了 charger，相关的充电配置已经给出来，可以看到 config.h 中这里定义了充电相关参数的配置，值对应 xcfg_cb 中的值，所以充电实际上可以在 Downloader 上位机中去配置。**

![image-20250506180306200](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250506180306200.png)

![image-20250506180323717](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250506180323717.png)

![image-20250506180811115](./SDK%E4%BA%86%E8%A7%A3.assets/image-20250506180811115.png)

基本的充电配置都可以在这个页面中进行，例如前面介绍锂电池充电过程时提到的几个充电阶段电流的配置，以及相对应的充电截止电流的阈值。其他的配置则可以根据实际的功能需求改动，**建议都使能涓流充电控制**，当锂电池过放时，必须使用涓流进行预充，避免电池损坏；插入 DC 复位和插入 DC 禁止软开机，则按照实际需求配置，这里的 DC 插入通常指芯片 VUSB 引脚接入 5V。同样的满电自动关机和充电仓的类型都是根据需求的功能来进行配置 。

在这里实际上判断充电结束的方式是前面提到的两种方式的结合，即芯片在**充电截止电压到达后，会去计时，同时判断充电截止电流，当充电截止电流达到后，停止充电，或充电截止电流没达到，充电截止的时间达到了，同样停止充电**。

## 充电控制

除了前面提到的基础充电配置，实际上对于充电有时候会有其他的控制需求，在耳机中的体现就是 NTC 功能，**根据充电环境的温度，去控制充电达到保护电池延长电池使用的目的**。通常通过 ADC 功能去采集热敏电阻的电压，换算得到相应的温度值，实现比较简单，**只要得出 ADC 值对应的温度列表就可以实现**，这里主要讲充电的调整控制部分，例如在某温度下需要对充电电流进行调控，通常采用的实现方式如下，实际上就是先停止充电，在修改恒流充电的电流配置在重新初始化进行充电。

- **检测到温度过高后，把充电电流改小降低发热？**

# 灯效复制

哈喽，大家好。在对中科蓝讯 SDK 的二次开发过程中，我们时常需要对 TWS 组队和蓝牙配对功能进行修改，或在此过程中添加功能。下面我将从两个大方面对此进行分享。第一，将分析 TWS 组队和蓝牙配对过程，旨在先对 SDK 中的TWS 组队和蓝牙配对有所了解，方便下一步的进行；第二，将列举此过程两个功能的修改或添加。

https://www.sunsili.com/html/support/specialtopic/207.html

一、TWS 组队和蓝牙配对过程灯效分析



1、准备工作A）接线准备：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图1)](./SDK了解.assets/232316fecmca2e9mm1h9mm.png)

B）打印信息输出 IO 更改为PA7：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图2)](./SDK了解.assets/232316zpjufc4ecncpfpjw.png)

C）配置工具配置准备：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图3)](./SDK了解.assets/232316t91kf1911f1n1a97.png)

2、TWS 组队和蓝牙配对灯效函数func_bt_disp_status() 函数将显示 TWS 组队和蓝牙配对各个阶段的灯效，每个阶段的灯效通过该函数 switch 语句中的对应 case 下的灯效函数进行显示，又分主副耳灯效。

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图4)](./SDK了解.assets/232316bldneymnxynlksap.png)

3、TWS 组队和蓝牙配对过程在左右耳机都新下程序的情况下，首次上电左右耳机首先显示开机灯效，然后进入 case BT_STA_SCANNING（func_bt_disp_status() 函数中的 case 语句）调用 led_bt_scan() 函数，TWS 未组队和蓝牙未配对灯效，亮300ms灭450ms，蓝红交替闪：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图5)](./SDK了解.assets/232316uvyt96a499k67k9f.png)

双击 USER_DEF 键将进行 TWS 组队：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图6)](./SDK了解.assets/232316yqgc1qf8qu2of1y8.png)

注意，此时耳机的蓝牙模块亦同时打开，即 TWS 组队与蓝牙配对是同时进行的。当 TWS 组队完成，蓝牙未配对，主耳（双击按键一方，最终将调用函数 bt_tws_search_slave(15000) ，然后被判定为主耳）进入 case BT_STA_IDLE 调用 led_bt_idle() 函数，亮300ms灭450ms，蓝红交替闪；并播放 TWS 组队提示音。副耳进入 case BT_STA_CONNECTED 调用 led_bt_connected() 函数，蓝灯亮100ms, 灭5S：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图7)](./SDK了解.assets/232316aw593wnnl9li8ml4.png)

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图8)](./SDK了解.assets/232316baxzu4zsxm9r41sy.png)

当 TWS 组队完成，且蓝牙配对完成，主副耳都进入 case BT_STA_CONNECTED 调用 led_bt_connected() 函数，蓝灯亮100ms, 灭5S。

二、TWS 组队和蓝牙配对过程功能修改1、TWS 组队完成主副耳红蓝灯亮一秒后，副耳熄灭，主耳 100ms 闪烁A）自定义红蓝灯亮 1s 和 100ms 闪烁结构体（配置工具中没有的灯效）：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图9)](./SDK了解.assets/232316n9h99edskbbo3ehs.png)

第一个 0x06 将以 0110 0000 倒序来控制红灯亮灭（0：灭，1：亮）；第二个 0x06 将以0110 0000 倒序来控制蓝灯亮灭；10 指两个二进制位间的时间是 10*50ms；255 指两个字节间的时间间隔是无限长，它的时间单位同样是 50ms。SDK已定义的灯效可通过配置工具来修改，如下开机状态配置 LED：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图10)](./SDK了解.assets/232316wpwz1yhvzo9y97g7.png)

t1 是指两颗 LED 之间闪烁的间隔（上图即为 10*50ms）；每 8 颗 LED 可以看做是一个周期，写成代码即对应 8 个二进制位；t2 是指两个周期之间的间隔（上图即为 0*50ms）。上图红灯全部熄灭，对应二进制数：0000 0000，若写成灯效结构体须倒序，倒序后值没有变化；蓝灯灯效对应二进制数：1010 1010，但若写成灯效结构体须先倒序为：0101 0101 ，因此，上图若写成灯效结构体其各个成员值为：{0x00,0x55,10,0}。B）新建使主副耳灯效同步的灯效函数：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图11)](./SDK了解.assets/232316fjz8t28udsp5kva2.png)

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图12)](./SDK了解.assets/232316g7mxrzgttnm00xi5.png)




![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图13)](http://bbs.sunsili.com/data/attachment/forum/202212/12/232316lmlb11rqrsb8wr0p.png)

led_set_sta_choice(const void *cfg,int cnt) 函数是自定义函数，其中，cfg 是灯效结构体指针；cnt 为真，主副耳的灯效将同步，为假，则不同步。

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图14)](./SDK了解.assets/232316s3p82d3dr2n8nav0.png)

bt_tws_is_slave() 用于判断是否是副耳。C）增加 100ms 队列消息：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图15)](./SDK了解.assets/232316clp5ppy1m7hr6ydf.png)

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图16)](./SDK了解.assets/232316c7nzybown7j6a80z.png)

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图17)](./SDK了解.assets/232316nfzxe2dy7dnxqzx7.png)

D）灯效实现：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图18)](./SDK了解.assets/232316mgccmg4mxgctbnnt.png)

bt_tws_is_connected() 函数用于判断 TWS 组队是否完成；tws_connected 是全局变量，做自加是避免两次进入 case BT_STA_IDLE 重复跑灯效函数，并作为主耳 100ms 红蓝灯交替闪烁的标记位；delay_5ms(200) 保证红蓝灯亮一秒而不被覆盖。

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图19)](./SDK了解.assets/232316z50n5njetwgawuze.png)

bt_nor_is_connected() 函数用于判断蓝牙配对是否完成，此处为防止副耳在完成 TWS 组队但未完成蓝牙配对的情况下跑此灯效。

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图20)](./SDK了解.assets/232316xsvft060za7f1vjq.png)

在 100ms 队列消息中使主耳 100ms 红蓝灯交替闪烁，副耳保持熄灯状态。

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图21)](./SDK了解.assets/232316jknrbb2yy4kx3dyl.png)

蓝牙断开将进入 case BT_NOTICE_DISCONNECT，令 tws_connected = 1 可使耳机进入主耳 100ms 红蓝灯交替闪烁副耳熄灭的蓝牙配对灯效。

- 先进行 TWS 组队再进行蓝牙配对

A）设置上电回连手机次数为 0 次：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图22)](./SDK了解.assets/232316d3fk22o0dq2t22w2.png)

B）蓝牙初始化完成后设置不可被发现：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图23)](./SDK了解.assets/232316epfv89v9z7ll89w9.png)

bt_set_scan() 函数的参数为 0x00 时，可以设置耳机蓝牙不被发现。C）进入 FUNC_ BT 前获取当前时钟：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图24)](./SDK了解.assets/232316f1hauw5611d1wh4k.png)

在 sys_cb 中增加变量 tws_connect_start 用以记录时间；

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图25)](http://bbs.sunsili.com/data/attachment/forum/202212/12/232316hl80z4z4bij93b4y.png)

双击按键开始 TWS 组队，则记录下开始组队的时刻；

D）msg_bt.c 文件里面判断计时是否到 10S，到5S 后设置回连或者可被手机发现：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图26)](./SDK了解.assets/232316hcm9js9jjj28jij8.png)

tick_check_expire() 函数用于判断记录的组队时刻开始到现在是否有 10s；bt_nor_get_link_info(NULL) 判断是否有蓝牙配对信息；bt_set_scan() 函数的参数为 0x03 时，耳机蓝牙可被发现、可被连接。

E）如 10s 内 TWS 配上对设置可被发现或者回连手机：

![中科蓝讯SDK TWS 组队和蓝牙配对过程分析(图27)](./SDK了解.assets/232316owwvo1vy0w1f2w10.png)

内容介绍到这里，欢迎大家批评指正。对于其他的组队以及配对的功能，可以借鉴上面几个点去延伸，如果大家还有什么其他的问题或者功能想要询问，亦可以在评论区中提出，可以共同探讨，一起进步。

