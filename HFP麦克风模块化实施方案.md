# HFP 麦克风模块化实施方案（HOGP 并发最佳实践）

> 适用项目：VibeCoding Keyboard / T2620 / JL AC701N（BR28）  
> 文档定位：第一版 BLE HOGP 键盘与 Classic HFP 麦克风并发的设计、实施和验收依据。  
> 当前状态：代码路径具备 Classic + BLE 双模和 HFP/eSCO 能力；经典蓝牙可发现功能正在接入，但 HFP 产品控制、并发策略和量产验证尚未模块化完成。

## 1. 结论与实施原则

当前工程可以实现以下第一版产品组合：

```text
同一台 T2620 设备
├─ BLE Peripheral / HOGP：连接 PC，持续提供键盘输入
├─ Classic Bluetooth / HFP HF：连接 PC，提供麦克风上行
└─ RDX BLE App Config：与 HOGP 广播身份互斥，不进入首版并发
```

HFP 麦克风应当模块化，但不能重复实现 JL SDK 已有的 HFP、SCO/eSCO、编码器、AEC/CVP 和麦克风采集链路。推荐边界是：

- JL SDK 原生通话模块继续拥有 HFP 音频生命周期。
- 新增 RDX HFP Mic 模块，负责产品策略、状态机、经典蓝牙可发现/可连接控制、事件归一化、HOGP 共存状态和诊断。
- HOGP 模块继续只负责 BLE HID，不感知 HFP 编解码或音频节点。
- `rdx_app.c` 只做事件和产品按键路由，不直接操作 HFP、eSCO 或底层 LMP。
- 所有 T2620 专属编译覆盖放在 `t2620_project_config.h`，不直接修改工具生成的 `sdk_config.h/c`。

第一版的稳定性优先级固定为：

1. HOGP 键盘连接和按键正确性。
2. HFP 麦克风链路连续性。
3. 配对、回连和功耗体验。
4. RDX App 配置能力；首版不与 PC HOGP 同时连接。

## 2. 当前代码事实

### 2.1 已具备的能力

当前编译配置和运行路径已经具备并发的基础条件：

- `TCFG_USER_BLE_ENABLE=1`，控制器同时启用 `BT_MODULE_CLASSIC | BT_MODULE_LE`。
- RDX 的 LE role 为 `LE_SLAVE | LE_ADV`，可以作为 HOGP Peripheral。
- `TCFG_BT_SUPPORT_HFP=1`，`bt_profile_config.c` 注册 HFP SDP Record。
- `TCFG_BT_MSBC_EN=1`，工程编译了 mSBC 与 eSCO 音频模块。
- `TCFG_RDX_HOGP_ENABLE=1`，T2620 默认进入 HOGP 广播身份。
- `tws_phone_call.c` 在 SCO/eSCO 打开时调用 `esco_player_open()` 和 `esco_recoder_open()`，已经存在标准 HFP 全双工音频路径。
- HOGP Input Report 通过 `app_ble_att_send_data(..., ATT_OP_NOTIFY)` 发送，与 HFP 音频链路没有应用层互斥。
- RDX 当前收到 `BT_STATUS_SCO_STATUS_CHANGE` 等事件时只打印日志，没有主动断开 HOGP。

### 2.2 当前实现需要修正的边界

#### 2.2.1 经典蓝牙开关与板级电源动作耦合

原有 `rdx_app_bt_shutdown()` 不只关闭经典蓝牙，还执行：

- `dual_conn_close()`；
- `USER_CTRL_POWER_OFF`；
- `sd_set_power(0)`；
- `dac_power_off()`。

当前为了保留经典蓝牙，在 `BT_STATUS_INIT_OK` 后跳过整个 `rdx_app_bt_shutdown()`。这能避免 Classic Stack 被关闭，但也同时跳过 SD、DAC 等板级电源动作。

正式实现必须拆开：

```text
Classic Bluetooth 策略
    -> 只决定 Classic Stack、Inquiry Scan、Page Scan 和配对状态

Board Power 策略
    -> 独立决定 SD、DAC 和其他外设是否断电
```

不得继续使用“是否开启 HFP”间接控制 SD/DAC 电源。

#### 2.2.2 当前没有独立 HFP 产品模块

当前 HFP 相关产品行为散落在：

- `rdx_app.c`：Classic 初始化、扫描开关、BT/SCO 事件日志；
- `tws_phone_call.c`：JL SDK 通话音频生命周期；
- `bt_profile_config.c`：HFP SDP 注册；
- `sdk_config.h`：HFP、mSBC、BLE 优先级等生成配置。

其中 `tws_phone_call.c` 属于 SDK 通用通话实现，不适合承载 VibeCoding Keyboard 的配对策略、语音键策略或 HOGP 共存策略。

#### 2.2.3 BLE 高优先级尚未开启

当前 `TCFG_BLE_HIGH_PRIORITY_ENABLE=0`。JL 控制器配置注释明确指出，在 eSCO 工作时若要保证 BLE 建链和调度，应开启该选项。

第一版产品以键盘为最高优先级，因此建议 T2620 overlay 设置为 1；但开启后必须重新验证 HFP 音频丢包和断续，不能只验证键盘。

#### 2.2.4 Classic HID 不应与 BLE HOGP 重复开放

当前 `TCFG_BT_SUPPORT_HID=1` 会注册 Classic HID SDP。产品方案已经选择 BLE HOGP 作为键盘，Classic 侧只需要 HFP。

如果没有明确的 Classic HID 产品需求，应在 T2620 overlay 中关闭 Classic HID，避免 Windows 同时枚举 Classic Keyboard 和 BLE Keyboard，造成双键盘、配对入口和故障定位混乱。

#### 2.2.5 HFP 不是标准 Mic-only Profile

HFP 的标准设备形态是 Hands-Free/Headset，不是纯麦克风。Windows 打开 HFP 麦克风时通常同时创建或切换到 Hands-Free 音频端点。

第一版应保持标准 HFP HF 行为，不修改 SDP 伪装成 Mic-only：

- 上行：设备麦克风 -> eSCO -> PC；
- 下行：PC -> eSCO -> 设备通话播放路径；
- 若产品没有扬声器需求，应在产品音频策略层静音或安全丢弃下行，不能破坏 HFP 状态机。

## 3. 目标与非目标

### 3.1 本方案目标

1. 提供独立、可关闭、可诊断的 RDX HFP Mic 产品模块。
2. 保留 JL SDK 原生 HFP/eSCO 音频实现，不复制底层通话代码。
3. 支持同一台 PC 同时连接 BLE HOGP 和 Classic HFP。
4. HFP 音频 active 时保持 HOGP 连接并正常发送 8-byte Keyboard Report。
5. 明确 Classic 配对、可发现、可连接和回连策略。
6. 将产品配置集中在 T2620 overlay，并增加 host 契约防止工具重新生成后退化。
7. 为后续语音触发键、PC Agent 和状态 UI 留出稳定接口。

### 3.2 第一版不做

- 不实现第二个 BLE GATT Server 或第二个 HOGP handle。
- 不允许 PC HOGP、手机 RDX Config、Classic HFP 三方并发。
- 不在 HFP 模块中直接调用 `esco_recoder_open/close()`。
- 不复制或 fork `tws_phone_call.c`。
- 不把音频数据搬到 HID、RDX GATT 或自定义 BLE characteristic。
- 不定义私有 Mic-only SDP。
- 不在 SCO/eSCO 回调中写 Flash、做阻塞延时或执行复杂业务。
- 不在第一版动态调整 BLE connection interval、eSCO packet type 或 RF 时隙参数；这些参数只在实测证明需要时单独评审。

## 4. 推荐模块架构

### 4.1 分层结构

```text
产品业务层
┌──────────────────────────────────────────────┐
│ rdx_app.c / Product Key Policy              │
│ - 语音键意图                                 │
│ - UI/LED 路由                                │
│ - HOGP 与 Config 模式产品决策                │
└──────────────────────┬───────────────────────┘
                       │ public API / normalized event
┌──────────────────────▼───────────────────────┐
│ rdx_hfp_mic.c                                │
│ - 单线程状态机                               │
│ - enable/pairing/reconnect 产品策略          │
│ - HOGP 共存状态与错误统计                    │
│ - 对外状态查询和 observer                    │
└──────────────────────┬───────────────────────┘
                       │ platform operations
┌──────────────────────▼───────────────────────┐
│ rdx_hfp_mic_platform.c                       │
│ - BT_STATUS_* -> 内部事件                    │
│ - USER_CTRL_* 命令封装                       │
│ - SDK 查询接口适配                           │
└──────────────────────┬───────────────────────┘
                       │ JL SDK public interfaces
┌──────────────────────▼───────────────────────┐
│ JL Classic BT + phone_call                   │
│ - HFP SDP / RFCOMM / AT                      │
│ - SCO/eSCO                                   │
│ - esco_player / esco_recoder                 │
│ - mSBC/CVSD / AEC/CVP / audio state          │
└──────────────────────────────────────────────┘

并行且独立：
rdx_hogp_keyboard.c -> BLE ATT notify -> PC Keyboard
```

### 4.2 推荐新增文件

建议第一版仍放在现有 RDX 目录，减少 Makefile 和 include path 变化：

```text
SDK/apps/common/third_party_profile/rdx_protocol/
├─ rdx_hfp_mic.h
├─ rdx_hfp_mic.c
├─ rdx_hfp_mic_platform.h
├─ rdx_hfp_mic_platform.c
└─ rdx_hfp_mic_config.h
```

| 文件 | 职责 | 明确不负责 |
|---|---|---|
| `rdx_hfp_mic.h` | 稳定公共 API、状态、事件和统计结构 | JL SDK 私有结构、音频节点 |
| `rdx_hfp_mic.c` | 产品状态机、幂等控制、共存策略、observer | 直接调用 LMP、直接开关 recorder |
| `rdx_hfp_mic_platform.h` | 最小平台操作表和事件翻译声明 | 产品策略 |
| `rdx_hfp_mic_platform.c` | `BT_STATUS_*` 翻译、`USER_CTRL_*` 封装、SDK 状态查询 | 保存产品状态、HOGP report |
| `rdx_hfp_mic_config.h` | 模块默认配置、日志和编译期约束 | T2620 最终产品覆盖 |

### 4.3 音频生命周期唯一所有者

这是本方案最重要的边界：

```text
BT_STATUS_SCO_STATUS_CHANGE(open)
    -> SDK phone_call handler
    -> app_audio_state_switch(APP_AUDIO_STATE_CALL)
    -> esco_player_open()
    -> esco_recoder_open()

BT_STATUS_SCO_STATUS_CHANGE(close)
    -> SDK phone_call handler
    -> esco_recoder_close()
    -> esco_player_close()
```

`rdx_hfp_mic` 只观察 open/close 结果并更新状态。禁止它再次调用 recorder/player open/close，否则会产生：

- 双重初始化或重复释放；
- 音频节点引用计数错误；
- HFP 断开后麦克风仍占用；
- 与 SDK tone、A2DP 和通话音量状态机冲突。

## 5. 公共 API 设计

### 5.1 状态定义

```c
typedef enum {
    RDX_HFP_MIC_STATE_DISABLED = 0,
    RDX_HFP_MIC_STATE_IDLE,
    RDX_HFP_MIC_STATE_PAIRING,
    RDX_HFP_MIC_STATE_ACL_CONNECTED,
    RDX_HFP_MIC_STATE_AUDIO_STARTING,
    RDX_HFP_MIC_STATE_AUDIO_ACTIVE,
    RDX_HFP_MIC_STATE_AUDIO_STOPPING,
    RDX_HFP_MIC_STATE_ERROR,
} rdx_hfp_mic_state_t;
```

状态含义：

| 状态 | Classic ACL | SCO/eSCO | Inquiry Scan | Page Scan |
|---|---:|---:|---:|---:|
| `DISABLED` | 不要求 | 关闭 | 关闭 | 关闭 |
| `IDLE` | 未连接 | 关闭 | 关闭 | 开启，允许已配对 PC 回连 |
| `PAIRING` | 未连接 | 关闭 | 开启 | 开启 |
| `ACL_CONNECTED` | 已连接 | 关闭 | 关闭 | 由栈管理 |
| `AUDIO_STARTING` | 已连接 | 建链中 | 关闭 | 由栈管理 |
| `AUDIO_ACTIVE` | 已连接 | 已打开 | 关闭 | 由栈管理 |
| `AUDIO_STOPPING` | 已连接或断开中 | 关闭中 | 关闭 | 由栈管理 |
| `ERROR` | 未确定 | 未确定 | 关闭 | 按恢复策略处理 |

### 5.2 内部归一化事件

```c
typedef enum {
    RDX_HFP_MIC_EVT_BT_READY = 0,
    RDX_HFP_MIC_EVT_CLASSIC_CONNECTED,
    RDX_HFP_MIC_EVT_CLASSIC_DISCONNECTED,
    RDX_HFP_MIC_EVT_SCO_REQUEST,
    RDX_HFP_MIC_EVT_SCO_OPENED,
    RDX_HFP_MIC_EVT_SCO_CLOSED,
    RDX_HFP_MIC_EVT_CALL_ACTIVE,
    RDX_HFP_MIC_EVT_CALL_ENDED,
    RDX_HFP_MIC_EVT_HOGP_CONNECTED,
    RDX_HFP_MIC_EVT_HOGP_DISCONNECTED,
    RDX_HFP_MIC_EVT_TIMEOUT,
} rdx_hfp_mic_event_type_t;

typedef struct {
    rdx_hfp_mic_event_type_t type;
    u8 addr[6];
    u16 codec;
    s32 reason;
} rdx_hfp_mic_event_t;
```

模块内部不保存 `struct bt_event *` 指针。事件回调返回后原始消息内存可能失效，必须复制所需字段。

### 5.3 第一版公共函数

```c
int  rdx_hfp_mic_init(void);
void rdx_hfp_mic_deinit(void);

int  rdx_hfp_mic_set_enabled(u8 enable);
int  rdx_hfp_mic_pairing_start(void);
int  rdx_hfp_mic_pairing_stop(void);

void rdx_hfp_mic_handle_event(const rdx_hfp_mic_event_t *event);

rdx_hfp_mic_state_t rdx_hfp_mic_state_get(void);
u8   rdx_hfp_mic_is_connected(void);
u8   rdx_hfp_mic_is_audio_active(void);

void rdx_hfp_mic_dump_state(void);
```

第一版不提供 `mic_start()` 或 `esco_open()`。Windows/PC Audio Gateway 决定何时打开 HFP 音频端点；设备侧只响应标准 HFP/SCO 流程。

后续若语音键需要请求 PC Agent 开始录音，该请求应走 HID hotkey 或 Agent IPC，不应直接伪造 SCO 状态。

### 5.4 平台操作接口

产品状态机不直接使用 `lmp_hci_write_scan_enable()`，由平台适配层封装公开 SDK 命令：

```c
typedef struct {
    int (*set_discoverable)(u8 enable); /* Inquiry Scan */
    int (*set_connectable)(u8 enable);  /* Page Scan */
    int (*disconnect_classic)(void);
    int (*classic_power_set)(u8 enable);
    int (*get_total_connections)(void);
    int (*get_call_status)(void);
} rdx_hfp_mic_platform_ops_t;
```

使用 `USER_CTRL_WRITE_SCAN_ENABLE/DISABLE` 和 `USER_CTRL_WRITE_CONN_ENABLE/DISABLE` 时要分别表达“可发现”和“可连接”，不要用一个裸 `scan_enable=0` 同时切断已配对设备回连能力。

## 6. 状态机与事件处理

### 6.1 启动流程

```text
BT_STATUS_INIT_OK
    -> rdx_hfp_mic_init()
    -> enabled?
       ├─ no  -> DISABLED，关闭 Inquiry/Page Scan
       └─ yes -> 检查是否有 Classic bond
                ├─ 有 bond -> IDLE，Inquiry off，Page on
                └─ 无 bond -> PAIRING，Inquiry on，Page on
```

经典蓝牙启动不得依赖 HOGP 当前是否连接。HOGP 与 HFP 分别维护 BLE bond 和 Classic bond。

### 6.2 Classic 连接流程

```text
PAIRING / IDLE
    -> BT_STATUS_FIRST_CONNECTED
    -> ACL_CONNECTED
    -> Inquiry Scan off
    -> 保存运行态地址，仅使用 SDK/VM 的标准 bond 作为持久化来源
```

不得在 HFP 模块中建立第二份自定义 Classic bond 数据库。

### 6.3 麦克风 active 流程

```text
ACL_CONNECTED
    -> BT_STATUS_SCO_CONNECTION_REQ
    -> AUDIO_STARTING
    -> BT_STATUS_SCO_STATUS_CHANGE(open)
    -> AUDIO_ACTIVE
    -> SDK 原生模块打开 esco_player + esco_recoder
    -> HOGP 连接保持，按键继续 notify
```

HFP 模块的 open 事件表示“SDK 已报告音频链路打开”，不表示模块自己拥有 recorder。

### 6.4 音频停止与断开

```text
AUDIO_ACTIVE
    -> SCO close
    -> AUDIO_STOPPING
    -> SDK 关闭 recorder/player
    -> Classic ACL 仍连接 ? ACL_CONNECTED : IDLE

任意连接态
    -> Classic disconnected
    -> 清理 runtime address/codec/error
    -> 有 bond ? IDLE : PAIRING
```

所有 close/disconnect 路径必须幂等。重复的 `SCO_CLOSED` 或 `CLASSIC_DISCONNECTED` 不能触发 assert、重复释放或错误计数膨胀。

### 6.5 线程模型

- 状态机只在 `app_core` 上运行。
- BT stack handler 只复制事件并投递，不执行阻塞业务。
- ISR 或协议栈回调中禁止 `os_time_dly()`、Flash 写入、复杂日志 dump。
- HFP 状态 observer 不得反向同步调用状态机 API，避免重入。
- 超时器回调只投递 `RDX_HFP_MIC_EVT_TIMEOUT`，实际恢复动作仍在 `app_core` 执行。

## 7. HOGP + HFP 并发策略

### 7.1 连接模型

首版允许：

```text
PC BLE Central <---- BLE ACL ----> HOGP Keyboard
PC BT Audio Gateway <--- BR/EDR ACL + eSCO ---> HFP HF
```

首版禁止：

```text
PC BLE HOGP + 手机 RDX BLE Config + Classic HFP
```

RDX Config 与 HOGP 继续使用现有互斥广播身份和单 BLE owner，不因为 HFP 模块出现而改变。

### 7.2 调度优先级

建议 T2620 项目覆盖：

```c
#ifndef TCFG_RDX_HFP_MIC_ENABLE
#define TCFG_RDX_HFP_MIC_ENABLE              1
#endif

#if TCFG_RDX_HFP_MIC_ENABLE
#undef  TCFG_BT_SUPPORT_HFP
#define TCFG_BT_SUPPORT_HFP                  1

#undef  TCFG_BT_MSBC_EN
#define TCFG_BT_MSBC_EN                      1

#undef  TCFG_BLE_HIGH_PRIORITY_ENABLE
#define TCFG_BLE_HIGH_PRIORITY_ENABLE        1

/* Keyboard uses BLE HOGP; do not expose a second Classic HID service. */
#undef  TCFG_BT_SUPPORT_HID
#define TCFG_BT_SUPPORT_HID                  0
#endif
```

这些覆盖应放入 `SDK/apps/earphone/include/t2620_project_config.h`，不能写入 `sdk_config.h/c`。

`TCFG_BLE_HIGH_PRIORITY_ENABLE=1` 是推荐起点，不是免测试开关。它可能把更多时隙让给 BLE，因此必须同时观察：

- HOGP report 延迟和发送失败；
- eSCO 上/下行丢包；
- mSBC 音频爆音、卡顿和断链；
- HFP active 期间 BLE 断开后的回连时间。

### 7.3 HOGP 发送原则

- HFP active 时不暂停 HOGP Input Report。
- 不缓存无限数量的键盘报告；键盘状态应以当前按下集合为准。
- 若按下 report 成功而释放 report 失败，HOGP executor 必须执行已有的 release/reset 恢复策略，避免 Windows 卡键。
- HFP 状态变化不能修改 HID Report Map、CCC、Protocol Mode 或 connection owner。
- 不为 HFP active 单独提高按键 repeat rate。

### 7.4 推荐配对顺序

量产 UI 建议使用明确的两阶段引导：

1. 配对 BLE Keyboard。
2. 配对 Classic Headset/Microphone。
3. 两边都完成 bond 后退出配对模式。
4. 日常启动时同时允许两种已配对链路回连，但默认不再 Inquiry discoverable。

测试阶段应额外覆盖反向顺序，确保实现不依赖固定配对次序。

## 8. Classic 配对与扫描最佳实践

### 8.1 区分三种状态

Classic Bluetooth 中应明确区分：

- Discoverable：Inquiry Scan，允许新 PC 搜索到设备。
- Connectable：Page Scan，允许已知 PC 发起连接或回连。
- Connected：Classic ACL 已建立；HFP Service Level Connection 和 SCO/eSCO 仍是后续独立阶段。

不要把三者都称为“广播开关”，否则产品状态和日志难以定位。

### 8.2 推荐策略

| 产品状态 | Inquiry Scan | Page Scan | 说明 |
|---|---:|---:|---|
| 首次开机、无 bond | 开 | 开 | 允许首次配对 |
| 用户主动进入 HFP 配对 | 开 | 开 | 配对超时后自动退出 |
| 已 bond、未连接 | 关 | 开 | 不公开暴露，但允许回连 |
| 已连接 | 关 | 栈管理 | 不重复进入配对 |
| OTA/DUT/关机 | 关 | 关 | 禁止新连接 |

配对模式应有明确超时，例如 120 秒；超时后若已有 bond，回到 `IDLE`，不要永久保持 Inquiry Scan。

### 8.3 不推荐的做法

- 开机永久 Inquiry Scan。
- 用 HOGP 连接状态决定 Classic Page Scan。
- 连接 HFP 时主动断开 HOGP。
- 通过 `lmp_hci_write_scan_enable(0)` 无差别关闭所有 Classic 扫描。
- 在 `BT_STATUS_INIT_OK` 中通过跳过整套 board shutdown 来保留 HFP。
- HFP 断开后立即清除 bond 并重新进入公开配对。

## 9. 配置与产品选项

### 9.1 必选配置

| 配置 | 首版建议 | 原因 |
|---|---:|---|
| `TCFG_RDX_HFP_MIC_ENABLE` | 1 | 产品总开关 |
| `TCFG_BT_SUPPORT_HFP` | 1 | HFP HF profile |
| `TCFG_BT_MSBC_EN` | 1 | Windows 宽带语音优先 |
| `TCFG_USER_BLE_ENABLE` | 1 | HOGP |
| `TCFG_RDX_HOGP_ENABLE` | 1 | BLE Keyboard |
| `TCFG_BLE_HIGH_PRIORITY_ENABLE` | 1，需实测 | eSCO 下优先保障 HOGP |
| `TCFG_BT_SUPPORT_HID` | 0 | 避免 Classic HID 重复枚举 |
| `TCFG_USER_TWS_ENABLE` | 0 | 当前单板产品不需要 TWS 调度 |
| `TCFG_LE_AUDIO_APP_CONFIG` | 0 | 首版使用 Classic HFP |

### 9.2 A2DP/AVRCP 策略

当前工程还开启 A2DP/AVRCP。是否保留必须作为产品决策单独确认：

- 若设备需要耳机/扬声器播放，保留 A2DP/AVRCP。
- 若产品只需要键盘和 HFP 语音入口，可评估关闭 A2DP/AVRCP，减少 Windows 端点和回连复杂度。
- 关闭 A2DP 不会把 HFP 变成 Mic-only；HFP 下行通道依然存在。
- 第一次 HFP 并发验证建议先保持 SDK 默认音频 profile，基线稳定后再做 profile 精简，避免同时引入两个变量。

### 9.3 地址和名称

当前 BLE 与 BR/EDR 地址配置为不同地址，Windows 可能把它们显示成两个配对对象，这是正常的双模产品行为，但需要产品 UI 明确说明。

还应统一并验证：

- BLE 广播名、GAP Device Name 和 Windows 键盘显示名；
- Classic EIR/SDP 名称和 Windows Headset/Microphone 显示名；
- 不依赖名称相同来判断两个端点属于同一产品；PC Agent 应使用稳定标识和已确认的枚举规则。

HOGP 当前 `RDX_HOGP_NAME_SOURCE=0`，实际名称来自 RDX Server local name；`VibeKeyboard` custom 字符串并未生效。名称修正应单独处理并加入 host contract，不能在 HFP 模块中硬编码。

## 10. 与现有代码的集成点

### 10.1 `rdx_app.c`

保留：

- 产品按键和消息路由；
- 将现有 `BT_STATUS_*` 翻译入口转交给 platform adapter；
- 语音键未来对 HFP Mic/PC Agent 发出产品意图。

迁出：

- `rdx_app_bt_open()`；
- `rdx_app_bt_shutdown()` 中的 Classic 策略；
- `rdx_app_earphone_state_set_page_scan_enable()` 的底层 LMP 调用；
- `rdx_app_earphone_state_cancel_page_scan()` 的底层 LMP 调用；
- SCO/HFP 状态的散落日志。

板级 `sd_set_power()`、`dac_power_off()` 应移动到独立 power policy 或保留在明确的 board shutdown 路径，不进入 `rdx_hfp_mic`。

### 10.2 `tws_phone_call.c`

第一版不修改其音频生命周期，只把它视为平台能力：

- HFP/SCO 事件；
- `esco_player_open/close()`；
- `esco_recoder_open/close()`；
- A2DP 抢占、tone 和 call audio state；
- mSBC/CVSD、音量、AEC/CVP。

只有硬件实测发现明确 SDK 缺陷时，才允许小范围修复，并为修复增加注释和回归用例。不得为了“模块化”机械搬运该文件。

### 10.3 `rdx_hogp_keyboard.c`

不新增 HFP 业务依赖。允许提供只读诊断查询，例如：

- HOGP connected；
- HOGP ready；
- 最近一次 notify 返回码；
- report success/failure counter。

HFP Mic dump 可以查询这些状态，但 HOGP 不反向 include HFP 头文件。

### 10.4 `SDK/Makefile`

新增：

```make
apps/common/third_party_profile/rdx_protocol/rdx_hfp_mic.c \
apps/common/third_party_profile/rdx_protocol/rdx_hfp_mic_platform.c \
```

两个源文件都应由 `(THIRD_PARTY_PROTOCOLS_SEL & RDX_EN) && TCFG_RDX_HFP_MIC_ENABLE` 门控；关闭总开关时提供轻量 stub 或完全不进入调用路径。

## 11. 错误处理与可观测性

### 11.1 统计结构

```c
typedef struct {
    u32 classic_connect_count;
    u32 classic_disconnect_count;
    u32 sco_open_count;
    u32 sco_close_count;
    u32 sco_open_timeout_count;
    u32 unexpected_event_count;
    s32 last_disconnect_reason;
    s32 last_error;
} rdx_hfp_mic_stats_t;
```

计数器只用于诊断，不参与状态机决策。

### 11.2 状态日志

状态变化采用单行结构化日志：

```text
[HFP_MIC] ACL_CONNECTED -> AUDIO_STARTING evt=SCO_REQ addr=xx:xx:xx:xx:xx:xx
[HFP_MIC] AUDIO_STARTING -> AUDIO_ACTIVE codec=mSBC hogp_conn=1 hogp_ready=1
[HFP_MIC] AUDIO_ACTIVE -> ACL_CONNECTED evt=SCO_CLOSE reason=0
```

建议 `rdx_hfp_mic_dump_state()` 输出：

- enabled、state、pairing；
- Classic ACL 地址和连接数；
- SCO/eSCO active、codec；
- HOGP mode、connected、ready；
- BLE high priority 配置值；
- 最近断开原因和累计计数。

### 11.3 恢复策略

- SCO open 超时：回到 `ACL_CONNECTED`，不重启整个 BT Stack。
- Classic ACL 断开：清 runtime 状态，保持 bond，恢复 Page Scan。
- HOGP 断开：HFP audio 可继续；HOGP 按现有 BLE Server 策略重新广播。
- HFP audio active 时 HOGP notify 失败：记录并由 HOGP executor 做 release/reset，不关闭 SCO。
- 连续异常达到阈值：只上报错误和允许用户重新配对，不自动清除两个 bond。

## 12. 分阶段实施计划

### Phase 0：冻结当前基线

1. 记录当前工作树中 Classic Scan 修改。
2. 固件验证 HOGP 单独连接、Classic HFP 单独连接。
3. 记录 Windows 设备名称、BLE/Classic 地址、HFP codec 和断开原因。
4. 保存一份 HFP active + HOGP 按键的原始日志作为对照。

完成标准：能够区分“Classic 已配对”“HFP ACL 已连接”“SCO/eSCO 已打开”。

### Phase 1：建立模块骨架，仅观察不控制

1. 新增 `rdx_hfp_mic*` 文件和总开关。
2. 从 `rdx_app` 转发 BT/HFP/SCO 事件。
3. 状态机只更新状态和日志，不发送控制命令。
4. 对照 SDK 实际事件顺序修正状态转换。

完成标准：开关关闭时行为完全不变；开启时日志能准确反映 HFP 生命周期。

### Phase 2：迁移 Classic 扫描和配对策略

1. 将 Inquiry/Page Scan 操作移入 platform adapter。
2. 实现首次配对、已 bond 回连、用户配对超时。
3. 拆分 `rdx_app_bt_shutdown()`：Classic Stack 与 SD/DAC power policy 解耦。
4. 移除 `rdx_app.c` 对 `lmp_hci_write_scan_enable()` 的直接调用。

完成标准：HFP 可配对和回连；退出配对后不可被新 PC 搜索，但已配对 PC 可回连；SD/DAC 电源行为与改造前一致。

### Phase 3：落实 HOGP 共存配置

1. T2620 overlay 开启 BLE high priority。
2. 关闭 Classic HID。
3. 保持 HOGP 与 RDX Config 的单连接 owner 策略不变。
4. 增加 HFP active 期间 HOGP 发送和释放恢复诊断。
5. 完成 Windows 双配对和并发压力测试。

完成标准：达到第 14 节验收门槛。

### Phase 4：接入语音键和 PC Agent

1. 语音键只发送标准 HID hotkey 或 Agent 可识别事件。
2. PC Agent 选择 HFP 麦克风并启动/停止录音。
3. Agent 完成 STT 和文本注入。
4. 设备根据标准 HFP/SCO 事件更新状态，不假定按键后 SCO 必然成功。

完成标准：Agent 不在线时键盘仍正常；HFP 失败不阻塞普通按键。

### Phase 5：清理与量产冻结

1. 删除旧 Classic 扫描和散落 HFP 日志路径。
2. 冻结配置契约、模块边界和状态机测试。
3. 完成 Windows 版本、不同蓝牙芯片组、休眠唤醒和长稳测试。
4. 文档记录已验证 codec、PC 平台和已知限制。

## 13. Host 软件测试设计

建议新增：

```text
tests/host/test_rdx_hfp_mic_config.ps1
tests/host/test_rdx_hfp_mic_module_contract.ps1
tests/host/test_rdx_hfp_hogp_coexistence_contract.ps1
```

### 13.1 配置契约

验证：

- T2620 overlay 开启 `TCFG_RDX_HFP_MIC_ENABLE`、HFP、mSBC、BLE high priority。
- T2620 overlay 关闭 Classic HID。
- LE Audio、TWS 保持关闭。
- 项目宏不写入 `sdk_config.h/c`。
- Makefile 只在 RDX/HFP Mic 开关有效时编译新增模块。

### 13.2 模块边界契约

验证：

- `rdx_hfp_mic.c` 不出现 `esco_recoder_open/close`、`esco_player_open/close`。
- `rdx_hfp_mic.c` 不出现 `lmp_hci_write_scan_enable`。
- `rdx_hogp_keyboard.c` 不 include `rdx_hfp_mic.h`。
- `rdx_app.c` 不直接操作 SCO/eSCO 音频资源。
- Classic 扫描只通过 platform adapter 控制。
- `rdx_app_bt_shutdown()` 不再把 HFP 开关与 SD/DAC power policy 绑定。

### 13.3 状态机 host 测试

状态机应使用可注入 platform ops，以便 host stub 验证：

- disabled/init；
- 首次配对与配对超时；
- ACL connect/disconnect；
- SCO request/open/close；
- 重复 close/disconnect 幂等；
- SCO open timeout；
- HOGP disconnect 不关闭 HFP；
- HFP disconnect 不切换 HOGP/Config mode；
- 错序事件进入可恢复状态而不是 assert。

统一入口仍为：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\host\run_host_tests.ps1
```

## 14. 硬件与 Windows 验收标准

以下为第一版建议门槛，若产品另有正式指标，应以产品指标替换并冻结。

### 14.1 基础功能

- Windows 能分别完成 BLE Keyboard 和 Classic HFP 配对。
- HOGP 显示为键盘，Classic 侧显示为 Hands-Free/Headset 麦克风。
- Windows 打开录音后，日志确认 SCO/eSCO active，优先协商 mSBC。
- 录音关闭后 HOGP 不断开，Classic ACL 可保持连接。
- 关机、OTA、DUT 时两类连接按既有电源策略正确关闭。

### 14.2 并发稳定性

- HFP active 连续 60 分钟，HOGP 无非预期断开。
- HFP active 期间执行至少 10,000 次按下/释放，零丢键、零重复键、零卡键。
- HOGP 输入期间 HFP 不发生非预期 SCO 断链。
- 人工听测无持续爆音、明显周期性断续；同时记录 eSCO 丢包或 PLC 指标。
- HFP active 期间主动断开并回连 HOGP，能够恢复输入，不影响正在进行的 HFP 音频。
- HOGP active 期间断开并回连 Classic HFP，键盘输入不中断。

### 14.3 时延建议值

- HFP idle 和 active 两种状态分别测量按键到 PC 收到 HID event 的时延。
- 建议 HFP active 时 P95 不超过 50 ms、单次不超过 100 ms。
- 若硬件测量条件不足，至少保证 active 相对 idle 无肉眼可感知的持续延迟和排队突发。

### 14.4 恢复场景

- PC 重启后两种链路可恢复。
- PC 睡眠/唤醒后两种链路可恢复。
- 设备超距再回到范围内，两种链路不会互相阻塞回连。
- 删除 BLE bond 不应误删 Classic bond，反之亦然。
- HFP active 时设备异常断电，重新上电后没有麦克风资源占用或 HOGP 卡键状态。

### 14.5 测试矩阵

至少覆盖：

| 维度 | 建议范围 |
|---|---|
| Windows | Windows 10、Windows 11 当前量产版本 |
| PC 蓝牙 | Intel、Realtek、MediaTek/其他目标客户常见芯片 |
| Codec | mSBC、回退 CVSD |
| 配对顺序 | HOGP first、HFP first |
| 距离 | 近距离、临界距离、2.4 GHz 干扰环境 |
| 电源 | 冷启动、热重启、睡眠唤醒、异常断电 |
| 时长 | 10 分钟功能、60 分钟压力、8 小时长稳 |

## 15. Code Review 检查表

### 架构

- [ ] HFP Mic 模块不拥有 `esco_player`/`esco_recoder` 生命周期。
- [ ] SDK 通话模块仍是唯一音频 owner。
- [ ] HOGP 不依赖 HFP Mic。
- [ ] RDX App 不直接访问 LMP/SCO 细节。
- [ ] 状态机只在 `app_core` 运行。

### 配置

- [ ] 产品覆盖只在 `t2620_project_config.h`。
- [ ] HFP、mSBC、BLE HOGP 开启。
- [ ] Classic HID 关闭。
- [ ] BLE high priority 开启并完成双向回归。
- [ ] TWS、LE Audio 保持首版既定状态。

### 配对与连接

- [ ] Inquiry Scan 与 Page Scan 分开控制。
- [ ] 已 bond 状态不会永久 discoverable。
- [ ] 配对模式有超时。
- [ ] BLE bond 与 Classic bond 独立。
- [ ] HFP/HOGP 任一断开不会强制断开另一条链路。

### 可靠性

- [ ] open/close/disconnect 路径幂等。
- [ ] 回调中无阻塞延时、Flash 写入和大段业务。
- [ ] HOGP release 失败有 reset 恢复。
- [ ] 错误不会自动清除 bond。
- [ ] 日志能区分 ACL、HFP SLC、SCO/eSCO 和 HOGP 状态。

## 16. 最终推荐形态

第一版量产推荐冻结为：

```text
BLE：
  HOGP Keyboard
  单 BLE Peripheral 连接
  RDX Config 与 HOGP 模式互斥
  BLE high priority（完成 HFP 音频回归后冻结）

Classic：
  HFP HF + mSBC/CVSD fallback
  不开放 Classic HID
  Inquiry 只在首次配对或用户主动配对时开启
  Page Scan 保留已配对 PC 回连能力

模块：
  rdx_hfp_mic        = 产品状态与策略
  rdx_hfp_mic_platform = JL SDK 适配
  phone_call         = 唯一 HFP/eSCO 音频 owner
  rdx_hogp_keyboard  = 唯一 BLE HID transport owner

PC：
  Windows 使用 HFP 麦克风
  PC Agent 负责语音转文字和文本注入
  设备不在 HID 或 RDX GATT 上传输麦克风音频
```

该结构在不 fork JL SDK 通话实现的前提下，把 HFP 产品策略从 `rdx_app.c` 中独立出来；同时保持 HOGP 和 HFP 松耦合，使两条链路可以独立配对、独立恢复和独立诊断，适合作为第一版实现与后续量产维护基线。
