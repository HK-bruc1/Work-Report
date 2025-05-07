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

## TWS 组队和蓝牙配对灯效分析

在对中科蓝讯 SDK 的二次开发过程中，我们时常需要对 TWS 组队和蓝牙配对功能进行修改，或在此过程中添加功能。

- 第一，将分析 TWS 组队和蓝牙配对过程，旨在先对 SDK 中的TWS 组队和蓝牙配对有所了解，方便下一步的进行；

### 灯效函数分析

- **TWS 组队和蓝牙配对灯效函数**`func_bt_disp_status()` 函数将显示 TWS 组队和蓝牙配对各个阶段的灯效，每个阶段的灯效通过该函数 switch 语句中的对应 case 下的灯效函数进行显示，又分主副耳灯效。

`app\platform\functions\func_bt.c`

```c
AT(.text.bfunc.bt)
static void func_bt_disp_status(void)
{
    uint status = bt_get_disp_status();

    if(f_bt.disp_status != status) {
        f_bt.disp_status = status;
        func_bt_disp_status_do();
    }
    func_bt_dac_ctrl();
    func_bt_tws_bre_led_ctl();
}
```

- 先获取当前蓝牙显示状态 bt_get_disp_status()

- **如果状态有变化**，更新 f_bt.disp_status 并调用 func_bt_disp_status_do()

- 然后分别调用 func_bt_dac_ctrl() 和 func_bt_tws_bre_led_ctl()

#### 关键：func_bt_disp_status_do() 里的灯效处理

- **烧录两版软件，这个不用担心找不到左右耳自己的灯效子函数**

```c
void func_bt_disp_status_do(void)
{
    if(!bt_is_connected()) {
        en_auto_pwroff();
        sys_cb.sleep_en = BT_PAIR_SLEEP_EN;
    } else {
        dis_auto_pwroff();
        sys_cb.sleep_en = 1;
    }

    switch (f_bt.disp_status) {
    case BT_STA_CONNECTING:
        led_bt_reconnect();
        break;
    case BT_STA_INITING:
    case BT_STA_IDLE:
        led_bt_idle();
        // ... 省略 ...
        break;
    case BT_STA_SCANNING:
        led_bt_scan();
        break;
    case BT_STA_DISCONNECTING:
        led_bt_connected();
        break;
    case BT_STA_CONNECTED:
        led_bt_connected();
        break;
    case BT_STA_INCOMING:
        led_bt_ring();
        break;
    case BT_STA_PLAYING:
        led_bt_play();
        break;
    case BT_STA_OUTGOING:
    case BT_STA_INCALL:
        led_bt_call();
        break;
    }
    // ... 省略 ...
}
```

- 这里根据不同的蓝牙状态，调用不同的LED灯效函数（如 led_bt_reconnect()、led_bt_idle()、led_bt_scan()、led_bt_connected() 等）。

- 这些状态包括：正在连接、空闲、扫描、已连接、来电、播放、通话等。

#### TWS组队和配对的灯效

- TWS组队和蓝牙配对的过程，都会导致蓝牙状态的变化（如从“空闲”到“配对/连接/组队”）。

- 这些状态变化会触发 func_bt_disp_status_do()，从而调用不同的LED灯效函数。

- 例如：

  - 配对时，状态可能是 BT_STA_IDLE 或 BT_STA_CONNECTING，对应 led_bt_idle() 或 led_bt_reconnect()。

  - 组队成功后，状态变为 BT_STA_CONNECTED，对应 led_bt_connected()。

#### TWS专用灯效

- func_bt_tws_bre_led_ctl() 也会在 func_bt_disp_status() 里被调用，用于TWS呼吸灯等特殊效果。

- 但具体TWS组队的灯效，通常还是通过状态变化间接实现。

#### 总结

- func_bt_disp_status() 通过状态机机制，间接实现了TWS组队和蓝牙配对各阶段的灯效显示。

- 你可以通过扩展或重载 led_bt_xxx() 相关函数，来实现自定义的组队/配对灯效，而无需修改SDK核心流程。

### 灯效函数流程

1. TWS 组队和蓝牙配对过程在左右耳机都**新下程序**的情况下：
   - 首次上电左右耳机首先显示**开机灯效**，然后进入 case BT_STA_SCANNING（func_bt_disp_status() 函数中的 case 语句）调用 led_bt_scan() 函数，**TWS 未组队和蓝牙未配对灯效**，亮300ms灭450ms，蓝红交替闪：

2. 双击 USER_DEF 键将进行 TWS 组队：
   - 在配置工具中开启双击配对

```c
    ///双击按键处理
    case KD_PLAY_USER_DEF:
    case KD_PLAY_PWR_USER_DEF:
        if (xcfg_cb.user_def_kd_tone_en) {
            sys_warning_play(T_WARNING_NEXT_TRACK, 1);                  //2击“滴”一声
        }
        if ((xcfg_cb.user_def_kd_lang_en) && (!bt_nor_is_connected())) {
            bt_switch_voice_lang();
        } else if (user_def_key_msg(xcfg_cb.user_def_kd_sel)) {
#if BT_TWS_EN
        } else if(bt_tws_pair_mode(2)) {
#endif
        }
        break;

```

- 注意，此时耳机的蓝牙模块亦同时打开，**即 TWS 组队与蓝牙配对是同时进行的**。
  - 当 TWS 组队完成，蓝牙未配对，主耳（双击按键一方，最终将调用函数 bt_tws_search_slave(15000) ，然后被判定为主耳）
    - **烧录两版软件，谁双击开启搜索谁就是主机**
  - 主机进入 case BT_STA_IDLE 调用 led_bt_idle() 函数，亮300ms灭450ms，蓝红交替闪；并播放 TWS 组队提示音。
  - 副耳进入 case BT_STA_CONNECTED 调用 led_bt_connected() 函数，蓝灯亮100ms, 灭5S：
    - 从机已经连接上主机了

3. 当 TWS 组队完成，且蓝牙配对完成，主副耳都进入 case BT_STA_CONNECTED 调用 led_bt_connected() 函数，蓝灯亮100ms, 灭5S。
   - **这个时候肯定有统一灯效的需求，可能想蓝牙配对之前都是一个灯效，不管TWS是否组队完成**

#### 相关函数

- bt_tws_is_connected() 函数用于**判断 TWS 组队是否完成**；

- bt_nor_is_connected() 函数用于**判断蓝牙配对是否完成**

- 蓝牙断开将进入 `case BT_NOTICE_DISCONNECT`

### 没有连接手机蓝牙之前统一灯效（实现）

#### 背景

- 配置工具中可以设置对耳的连接LED灯效。但是TWS配对成功时，会导致一个耳机处于连接状态（**TWS连接成功了**）一个处于未连接状态（**未连接手机**），从而导致灯效不一致。
- 正常来讲的话，应该是以连接手机为准，没有连接手机时的灯效一样。

#### 解决方案概述

通过在原有代码基础上增加四个关键部分的逻辑，实现了TWS连接后左右耳机灯效的统一显示**。**

1.**灯效显示逻辑改进（app\platform\functions\func_bt.c）**

**原有逻辑**：

```c
static void func_bt_disp_status(void)
{
    uint status = bt_get_disp_status();
    
    if(f_bt.disp_status != status) {
        f_bt.disp_status = status;
        func_bt_disp_status_do();
    }
    func_bt_dac_ctrl();
    func_bt_tws_bre_led_ctl();
}
```

- 先获取当前蓝牙显示状态 bt_get_disp_status()

- **如果状态有变化**，更新 f_bt.disp_status 并调用 func_bt_disp_status_do()

- 然后分别调用 func_bt_dac_ctrl() 和 func_bt_tws_bre_led_ctl()

**改进逻辑**：

```c
AT(.text.bfunc.bt)
static void func_bt_disp_status(void)
{   
    //获取当前的蓝牙显示状态
    uint status = bt_get_disp_status();

#if Y90_UI_EN
    //静态变量，用于保存上一次的手机蓝牙连接状态
    static bool nor_connect_disp_sta = 0;
    //获取当前的手机蓝牙连接状态
    bool nor_connect_sta = bt_nor_is_connected();
    //如果当前的蓝牙显示状态与上一次不同，或者当前的手机蓝牙连接状态与上一次不同，则更新灯效
    if(f_bt.disp_status != status || nor_connect_disp_sta != nor_connect_sta) {
        //更新手机蓝牙状态
        nor_connect_disp_sta = nor_connect_sta;
#else
    if(f_bt.disp_status != status) {
#endif
        //更新蓝牙显示状态
        f_bt.disp_status = status;
        //执行灯效显示状态处理
        func_bt_disp_status_do();
    }
    //执行蓝牙DAC控制
    func_bt_dac_ctrl();
    //执行蓝牙TWS呼吸灯控制
    func_bt_tws_bre_led_ctl();
}
```

2.**特定状态下的灯效统一（app\platform\functions\func_bt.c）**

**原有逻辑**：

```c
case BT_STA_IDLE:
    led_bt_idle();
    break;
    
case BT_STA_CONNECTED:
    led_bt_connected();
    break;
```

- TWS配对但是没有连接手机蓝牙的状态
  - 主机进入BT_STA_IDLE灯效
  - 从机进入BT_STA_CONNECTED灯效

**改进逻辑**：

```c
case BT_STA_IDLE:
#if FA11_UI_EN
	//双重保证
    if(bt_tws_is_connected()) {
        // TWS已连接但未连接手机时的灯效处理（主耳会进入）
        if(xcfg_cb.bt_tws_lr_mode == 4) {
            led_set_sta(0x00,0x60,0x04,0x19);
        } else if (xcfg_cb.bt_tws_lr_mode == 5) {
            led_set_sta(0x00,0x60,0x04,0x19);
        }
    } else {
        led_bt_idle();
    }
#else
    led_bt_idle();
#endif
    break;
    
case BT_STA_CONNECTED:
#if FA11_UI_EN
	//这里副耳TWS组队成功后也会进入BT_STA_CONNECTED
	//主耳连接手机蓝牙后也会进入BT_STA_CONNECTED
    if(!bt_nor_is_connected()) {
        // TWS已连接但未连接手机的情况（副耳会进入这个）
        if(xcfg_cb.bt_tws_lr_mode == 4) {
            led_set_sta(0x00,0x60,0x04,0x19);
        } else if (xcfg_cb.bt_tws_lr_mode == 5) {
            led_set_sta(0x00,0x60,0x04,0x19);
        }
        break;
    }
#endif
	//这是主耳会进入的逻辑
    led_bt_connected();
    break;
```

**原本逻辑（未使能 Y90_UI_EN）**

- TWS配对但未连接手机蓝牙时：

- 主机（主耳）会进入BT_STA_IDLE，显示led_bt_idle()灯效。

- 从机（副耳）会进入BT_STA_CONNECTED，显示led_bt_connected()灯效。

- 这样左右耳在未连接手机蓝牙前，灯效是不一样的。

**修改后逻辑**

- 只要TWS已连接，且bt_tws_lr_mode为4或5：
  - 无论主机还是从机，只要处于BT_STA_IDLE、BT_STA_INITING、BT_STA_CONNECTED，且没有连接手机蓝牙（!bt_nor_is_connected()），都会统一调用led_set_sta(0x00,0x60,0x04,0x19)。

- 也就是说，只要TWS配对成功但还没连手机蓝牙，左右耳都会显示同一个灯效（即led_set_sta(0x00,0x60,0x04,0x19)）。

**3.强制更新灯效显示（app\platform\bsp\bsp_bt.c）**

**0xff的意义**

```c
void func_bt_status(void)
{
    while(1) {
        func_bt_disp_status();

        {
            func_bt_warning();
        }

        if(f_bt.disp_status != 0xff) {
            break;
        }
    }
}
```

- 这个函数是蓝牙主循环的一部分。

- 每次循环都会调用func_bt_disp_status()和func_bt_warning()。

- 只有当f_bt.disp_status != 0xff时，才会跳出循环，否则会一直循环。
  - func_bt_disp_status中状态发生变化时就会更新灯效。

**f_bt.disp_status = 0xff的意义**

- 0xff通常是一个无效/未初始化的状态，用来标记“需要重新获取和刷新状态”。

- 当某些情况下（比如休眠、异常、状态切换等），代码会把f_bt.disp_status设为0xff，表示“当前状态未知，需要强制刷新”。

**为什么会触发灯效更新？**

- 当f_bt.disp_status = 0xff时，下一次func_bt_disp_status()执行时：

- 会重新获取当前的蓝牙状态status。

- 由于f_bt.disp_status（0xff）和实际的status一定不同，所以会进入更新分支，调用func_bt_disp_status_do()，刷新灯效。

- 这样可以保证在状态不明或需要强制刷新的情况下，灯效能及时、准确地反映当前蓝牙状态。

**原有逻辑**：

```c
case BT_NOTICE_DISCONNECT:
    bt_emit_notice_disconnect((u8 *)param);
    break;

case BT_NOTICE_CONNECTED:
    bt_emit_notice_connected((u8 *)param);
    break;

case BT_NOTICE_TWS_DISCONNECT:
    f_bt.tws_status &= ~0xc0;
    f_bt.warning_status |= BT_WARN_TWS_DISCON;
    break;

case BT_NOTICE_TWS_CONNECTED:
    bt_emit_notice_tws_connected((u8 *)param);
    break;
```

**改进逻辑**：

```c
case BT_NOTICE_DISCONNECT:
    bt_emit_notice_disconnect((u8 *)param);
#if FA11_UI_EN
    // 蓝牙断开连接时，强制更新灯效显示
    f_bt.disp_status = 0xff;
#endif
    break;

case BT_NOTICE_CONNECTED:
    bt_emit_notice_connected((u8 *)param);
#if FA11_UI_EN
    // 蓝牙连接成功时，强制更新灯效显示
    f_bt.disp_status = 0xff;
#endif
    break;

case BT_NOTICE_TWS_DISCONNECT:
    f_bt.tws_status &= ~0xc0;
    f_bt.warning_status |= BT_WARN_TWS_DISCON;
#if FA11_UI_EN
    // TWS断开连接时，强制更新灯效显示
    f_bt.disp_status = 0xff;
#endif
    break;

case BT_NOTICE_TWS_CONNECTED:
    bt_emit_notice_tws_connected((u8 *)param);
#if FA11_UI_EN
    // TWS连接成功时，强制更新灯效显示
    f_bt.disp_status = 0xff;
#endif
    break;
```

**改进说明**：

- 在四个关键的蓝牙状态变化事件中添加了强制更新灯效的代码
- 通过设置`f_bt.disp_status = 0xff`（无效值），强制在下一次调用func_bt_disp_status函数时更新灯效
- 这确保了在状态变化时，左右耳机都会更新灯效，保持一致

**函数名：bt_emit_notice**

- 这是一个蓝牙事件通知处理函数，用于响应蓝牙协议栈/驱动层发出的各种事件（如连接、断开、TWS连接/断开等）。

- 其作用是：当蓝牙底层发生关键事件时，通知上层做出相应处理，比如更新状态、刷新灯效、同步数据等。

**事件类型举例**

- BT_NOTICE_INIT_FINISH：蓝牙初始化完成

- BT_NOTICE_DISCONNECT：蓝牙断开

- BT_NOTICE_CONNECTED：蓝牙连接

- BT_NOTICE_TWS_DISCONNECT：TWS断开

- BT_NOTICE_TWS_CONNECTED：TWS连接

- ...（还有其他事件）

#### 为什么在这里赋值f_bt.disp_status = 0xff？

**作用**

- 赋值f_bt.disp_status = 0xff的目的是强制让上层的显示/灯效逻辑在收到这些关键事件后，立即刷新一次。

- 这是因为：

  - 蓝牙底层的事件变化有时不会直接改变f_bt.disp_status，而是通过事件通知机制让上层感知。

  - 如果不强制刷新，可能会出现状态已经变化但灯效没变的情况，导致用户体验不一致。

**具体流程**

1. 蓝牙底层发生事件（如断开、连接、TWS变化等）。

2. 调用bt_emit_notice，进入对应的case分支。

3. 在#if FA11_UI_EN下，赋值f_bt.disp_status = 0xff。

4. 上层主循环（如func_bt_status）检测到f_bt.disp_status == 0xff，会强制刷新一次灯效和显示状态。

#### “各状态变化不是原本就有吗？”——为什么还要这样做？

- 原本的状态变化（比如func_bt_disp_status里根据bt_get_disp_status()自动切换）是定期轮询的。

- 但有些底层事件（比如TWS断开、TWS连接、蓝牙断开等）是异步发生的，可能不会立刻被上层感知。

- 通过事件通知+强制刷新，可以保证所有关键状态变化都能第一时间反映到灯效和UI上，避免遗漏。

#### 方案工作原理

1. **全面的状态检测**：不仅检测蓝牙状态变化，还检测手机连接状态变化，确保在任何状态变化时都能更新灯效
2. **统一的灯效参数**：在TWS连接的情况下，为左右耳机设置相同的灯效参数，确保视觉一致性
3. **强制更新机制**：在关键状态变化点强制更新灯效显示，避免因状态不同步导致的灯效不一致

#### 原始灯效与改进后灯效的区别

##### 原始灯效行为

在原始SDK中，TWS耳机的灯效显示完全基于每个耳机自身的状态，导致以下情况：

1. TWS配对阶段：
   - 两只耳机都显示配对/搜索状态的灯效（通常是快速闪烁）
2. TWS连接成功但未连接手机阶段：
   - **主耳机**：显示为"已连接TWS但未连接手机"的状态，通常是某种特定的闪烁模式
   - **从耳机**：显示为"已连接"状态，因为从耳机视角它已经连接到了主耳机
   - **结果**：两只耳机显示不同的灯效，用户会看到不一致的视觉反馈
3. 连接手机阶段：
   - 只有主耳机与手机建立连接
   - **主耳机**：显示为"已连接手机"的状态
   - **从耳机**：仍然只显示"已连接TWS"的状态
   - **结果**：两只耳机可能仍然显示不同的灯效
4. 完全连接阶段：
   - 当主耳机成功与手机连接，并且信息同步到从耳机后
   - 两只耳机才会显示相同的"已连接"状态灯效
5. 休眠影响：
   - 在某些状态下，一只耳机可能进入休眠模式而关闭LED灯，而另一只保持活跃状态
   - 这进一步加剧了灯效的不一致性

##### 改进后灯效行为

改进后的方案通过多种机制确保TWS耳机在各个阶段都显示一致的灯效：

1. TWS配对阶段：
   - 与原来相同，两只耳机都显示配对/搜索状态的灯效
2. TWS连接成功但未连接手机阶段：
   - **关键改进点**：无论主从耳机，都显示相同的统一灯效
   - 通过`led_set_sta(0x00,0x60,0x04,0x19)`设置特定的灯效参数
   - 两只耳机显示完全相同的灯效，提供一致的视觉反馈
3. 连接手机阶段：
   - **关键改进点**：强制更新机制确保状态变化时两只耳机同步更新灯效
   - 通过设置`f_bt.disp_status = 0xff`触发灯效更新
   - 两只耳机显示相同的"已连接手机"状态灯效
4. 完全连接阶段：
   - 两只耳机显示相同的"已连接"状态灯效，与原来相同

##### 具体灯效差异示例

##### 场景一：TWS连接成功但未连接手机

**原始行为**：

- 主耳机：可能显示蓝灯慢闪（表示待连接状态）
- 从耳机：可能显示蓝灯常亮或特定模式闪烁（表示已连接状态）
- 用户体验：看到两只耳机灯光不同，可能误以为有一只耳机出现问题

**改进后行为**：

- 主耳机和从耳机：都显示相同的灯效模式（由`led_set_sta(0x00,0x60,0x04,0x19)`设置）
- 用户体验：看到两只耳机灯光一致，理解为TWS已连接但等待连接手机的状态

##### 场景二：主耳机连接手机过程中

**原始行为**：

- 主耳机：可能显示蓝灯快闪（表示正在连接状态）
- 从耳机：可能仍显示之前的状态灯效
- 用户体验：看到不同步的灯效变化，感到困惑

**改进后行为**：

- 通过强制更新机制，两只耳机同步更新灯效状态
- 用户体验：看到两只耳机同步变化的灯效，理解为系统正在连接手机

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
