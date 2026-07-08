# BLE HID 实现文档：RDX 共存与 Windows 11 连通性验证

## 1. 目标

在现有 RDX BLE 协议栈上追加标准 BLE HID Keyboard（HOGP），实现：

- **PC 侧**：Windows 11 识别为标准 BLE 键盘，接收按键和组合键输入
- **手机 APP 侧**：复用现有 RDX APP，通过配置模式连接设备修改按键映射（V1）
- **共存策略**：同一 GATT 数据库承载 RDX + HID 两组 Service（V0），同一时间只连一个 Central。V1 再加 Key Config Service

### V0 范围（本次实施）

**包含：**
- HID Service (0x1812)：Protocol Mode / Report Map / HID Information / HID Control Point / Keyboard Input Report / Consumer Control Report / Boot KB Input & Output
- 固定 HID Report Map，固定语音键发送 `Win + H`
- 关闭经典蓝牙 HID（`TCFG_BT_SUPPORT_HID=0`）避免双键盘
- HID 模式下跳过 RDX 20s 强制断开逻辑，避免连接后录音/文件同步副作用

**不包含：**
- Key Config Service (128-bit UUID) — 推迟到 V1
- APP 按键配置 / 映射表存储 — 推迟到 V1
- 配置模式状态机（三键长按、广播切换）— 推迟到 V1
- `keymap_manager.c` / `keymap_storage.c` — 推迟到 V1
- 麦克风 / 录音功能

### V0 前置验证

编码前必须先完成 **Step 0：`le_hogp_*` 库函数验证**，确认 SDK 的 `le_hogp_set_ReportMap()` / `le_hogp_set_icon()` 是否能在当前 RDX BLE handle 下生成或接管 HID Service。若可用则减少大量手工 GATT DB 编码；若不可用则按本文档手写路径执行。

---

## 2. 架构总览

```text
GATT 数据库（单一 profile_data[] 数组，V0）:
  ┌─────────────────────────────────────────┐
  │ 0x0001  GAP Service (设备名)             │  ← RDX Service：handle/UUID/属性不变
  │ 0x0004  RDX Main Service (128-bit)       │  ← RDX Service：handle/UUID/属性不变
  │ 0x000C  Battery Service (0x180F)         │  ← RDX Service：handle/UUID/属性不变
  │ 0x0010  OTA Service (128-bit)            │  ← RDX Service：handle/UUID/属性不变
  │ 0x0016  HID Service (0x1812)             │  ← 新增：标准 BLE HID（V0）
  │ 0x002C  Key Config Service (128-bit)     │  ← V1 预留，V0 不实现
  └─────────────────────────────────────────┘

连接模型（单路 BLE，分时复用）:
  V0:  PC ←→ 设备 (HID keyboard input)  — 不做模式切换
  V1:  键盘模式 + 配置模式（手机 APP ←→ 设备，RDX protocol + Key Config）
```

### 文件拆分（方案 B，修订版）

```
新增文件（独立 HID 子目录，不污染 RDX 根目录）:
  SDK/apps/common/third_party_profile/rdx_protocol/hid/
    ├── hid_gatt_db.inc          HID Service GATT DB 片段（.inc 片段，非 .c）— V0
    ├── hid_gatt_db.h            对应 handle 宏定义 + 路由表声明 — V0
    ├── hid_report_map.c         HID Report Map 字节数组 — V0
    ├── hid_report_map.h         Report ID / Report 长度宏定义 — V0
    ├── hid_report_sender.c      HID Report 构建与发送（含 Press/Release 时序）— V0
    ├── hid_report_sender.h      发送接口 — V0
    ├── hid_keyboard.c           HID 模块总入口：ATT 路由、Report 发送、连接参数 — V0
    ├── hid_keyboard.h           TCFG_MINI_HID_KEYBOARD_ENABLE=0 时空展开的宏封装 — V0
    ├── hid_config_service.c     Config Service ATT handler — V1
    ├── hid_config_service.h     配置服务接口 — V1
    ├── keymap_manager.c         按键映射管理 — V1
    ├── keymap_manager.h         映射表结构体与接口 — V1
    └── keymap_storage.c         双槽 VM/Flash 读写 — V1

修改文件:
    SDK/apps/common/third_party_profile/rdx_protocol/
    ├── rdx_ble_server.c         ATT 回调头部插入 HID 路由表分发
    ├── rdx_app.c                条件初始化 HID 模块
    └── multi_protocol_main.c    编译期 profile 选择

    SDK/apps/common/third_party_profile/rdx_protocol/
    └── rdx_ble_server.c         HID 模式下跳过 20s 强制断开（rdx_ble_server_connected_handle）

    SDK/apps/earphone/include/
    └── t2620_project_config.h   新增 TCFG_MINI_HID_KEYBOARD_ENABLE（V0 设为 1）

    SDK/apps/common/config/
    └── bt_profile_config.c      关闭经典 HID SDP（TCFG_BT_SUPPORT_HID=0）

注意：
  - 不在 sdk_config.h 加任何新宏（该文件由 JL 工具自动同步，AGENTS.md 禁止手写项目宏）
  - GATT DB 片段命名为 .inc 而非 .c，明确表示它是被 #include 的头文件片段
  - HID 文件全部收在 hid/ 子目录下，与 RDX 的 rdx_*.c 物理隔离
  - V0 不创建 keymap_manager.* / keymap_storage.* / hid_config_service.*
```

---

## 3. GATT 数据库结构

### 3.1 完整 handle 分配表

V0 阶段在 `rdx_profile_data[]` 数组末尾追加 HID Service，RDX 各 Service 的 handle、UUID、属性保持不变。V1 重构时将 HID + Config 部分独立到 `hid_gatt_db.c`。

```
Handle    类型                        UUID                         属性
──────────────────────────────────────────────────────────────────────────────
现有 RDX（不动）:
0x0001    PRIMARY_SERVICE            0x1800 (GAP)                 —
0x0002    CHARACTERISTIC             0x2A00 (Device Name)         R
0x0003    VALUE                      0x2A00                       R, DYNAMIC
0x0004    PRIMARY_SERVICE            06068D0C-... (RDX Main)      —
0x0005    CHARACTERISTIC             06068D1C-... (RDX Cmd)       WWR
0x0006    VALUE                      06068D1C-...                 WWR, DYNAMIC
0x0007    CHARACTERISTIC             06068D2C-... (RDX Notify)    N
0x0008    VALUE                      06068D2C-...                 N
0x0009    CCCD                       0x2902                       RW
0x000A    CHARACTERISTIC             06068D3C-... (RDX Read)      R
0x000B    VALUE                      06068D3C-...                 R, DYNAMIC
0x000C    PRIMARY_SERVICE            0x180F (Battery)             —
0x000D    CHARACTERISTIC             0x2A19 (Battery Level)       R+N
0x000E    VALUE                      0x2A19                       R+N, DYNAMIC
0x000F    CCCD                       0x2902                       RW
0x0010    PRIMARY_SERVICE            00239A6F-... (OTA)           —
0x0011    CHARACTERISTIC             00239A7F-... (OTA Write)     WWR
0x0012    VALUE                      00239A7F-...                 WWR, DYNAMIC
0x0013    CHARACTERISTIC             00239A8F-... (OTA Notify)    N
0x0014    VALUE                      00239A8F-...                 N
0x0015    CCCD                       0x2902                       RW
──────────────────────────────────────────────────────────────────────────────
新增 HID Service (0x1812):
0x0016    PRIMARY_SERVICE            0x1812 (HID)                 —
0x0017    CHARACTERISTIC             0x2A4E (Protocol Mode)       R+WWR
0x0018    VALUE                      0x2A4E (default=0x01)        R+WWR, DYNAMIC
0x0019    CHARACTERISTIC             0x2A4B (Report Map)          R
0x001A    VALUE                      0x2A4B                       R, DYNAMIC
0x001B    CHARACTERISTIC             0x2A4A (HID Information)     R
0x001C    VALUE                      0x2A4A                       R, DYNAMIC
0x001D    CHARACTERISTIC             0x2A4C (HID Control Point)   WWR
0x001E    VALUE                      0x2A4C                       WWR, DYNAMIC
0x001F    CHARACTERISTIC             0x2A4D (Report)              R+N
0x0020    VALUE                      0x2A4D (Keyboard Input)      R+N, DYNAMIC
0x0021    DESCRIPTOR                 0x2908 (Report Ref: ID=0x01,TYPE=INPUT)
0x0022    CCCD                       0x2902                       RW
0x0023    CHARACTERISTIC             0x2A4D (Report)              R+N
0x0024    VALUE                      0x2A4D (Consumer Control)    R+N, DYNAMIC
0x0025    DESCRIPTOR                 0x2908 (Report Ref: ID=0x02,TYPE=INPUT)
0x0026    CCCD                       0x2902                       RW
0x0027    CHARACTERISTIC             0x2A22 (Boot KB Input)       R+N
0x0028    VALUE                      0x2A22                       R+N, DYNAMIC
0x0029    CCCD                       0x2902                       RW
0x002A    CHARACTERISTIC             0x2A32 (Boot KB Output)      R+W+WWR
0x002B    VALUE                      0x2A32                       R+W+WWR, DYNAMIC
──────────────────────────────────────────────────────────────────────────────
V1 预留 Key Config Service (128-bit UUID，待与 APP 对齐) — V0 不实现:
0x002C    PRIMARY_SERVICE            <CONFIG_SERVICE_UUID_128>    —
0x002D    CHARACTERISTIC             <PROTO_VER_UUID_128>         R
0x002E    VALUE                      <PROTO_VER_UUID_128>         R, DYNAMIC
0x002F    CHARACTERISTIC             <DEV_STATUS_UUID_128>        R+N
0x0030    VALUE                      <DEV_STATUS_UUID_128>        R+N, DYNAMIC
0x0031    CCCD                       0x2902                       RW
0x0032    CHARACTERISTIC             <KEYMAP_UUID_128>            R+W
0x0033    VALUE                      <KEYMAP_UUID_128>            R+W, DYNAMIC
0x0034    CHARACTERISTIC             <COMMIT_UUID_128>            W
0x0035    VALUE                      <COMMIT_UUID_128>            W, DYNAMIC
0x0036    CHARACTERISTIC             <RESET_UUID_128>             W
0x0037    VALUE                      <RESET_UUID_128>             W, DYNAMIC
0x0038    CHARACTERISTIC             <REBOOT_UUID_128>            W
0x0039    VALUE                      <REBOOT_UUID_128>            W, DYNAMIC

V0 END: 0x00, 0x00（HID Service 结束后即结束，不含 Config Service）
```

### 3.2 handle 宏定义

为消除魔数，在 `hid_gatt_db.h` 中为每个新增 handle 定义宏：

```c
// HID Service handles — V0
#define HID_HANDLE_SERVICE              0x0016
#define HID_HANDLE_PROTOCOL_MODE        0x0018
#define HID_HANDLE_REPORT_MAP           0x001A
#define HID_HANDLE_HID_INFO             0x001C
#define HID_HANDLE_HID_CTRL_POINT       0x001E
#define HID_HANDLE_KB_INPUT             0x0020
#define HID_HANDLE_KB_INPUT_REPORT_REF  0x0021  // 0x2908: [Report ID=0x01][Type=INPUT(0x01)]
#define HID_HANDLE_KB_INPUT_CCCD        0x0022
#define HID_HANDLE_CC_INPUT             0x0024
#define HID_HANDLE_CC_INPUT_REPORT_REF  0x0025  // 0x2908: [Report ID=0x02][Type=INPUT(0x01)]
#define HID_HANDLE_CC_INPUT_CCCD        0x0026
#define HID_HANDLE_BOOT_KB_INPUT        0x0028
#define HID_HANDLE_BOOT_KB_INPUT_CCCD   0x0029
#define HID_HANDLE_BOOT_KB_OUTPUT       0x002B

// HID handle 范围（路由表用）
#define HID_HANDLE_BEGIN                0x0016
#define HID_HANDLE_END                  0x002B

#if TCFG_MINI_HID_CONFIG_SERVICE_ENABLE
// Config Service handles — V1
#define CFG_HANDLE_SERVICE              0x002C
#define CFG_HANDLE_PROTO_VER            0x002E
#define CFG_HANDLE_DEV_STATUS           0x0030
#define CFG_HANDLE_KEYMAP               0x0033
#define CFG_HANDLE_COMMIT               0x0035
#define CFG_HANDLE_RESET                0x0037
#define CFG_HANDLE_REBOOT               0x0039

#define CFG_HANDLE_BEGIN                0x002C
#define CFG_HANDLE_END                  0x0039
#endif
```

### 3.3 BTstack GATT 数据库编码格式

当前 SDK 使用 BTstack 格式。每条记录为一个变长字节数组，以 2 字节小端长度开头，后跟 handle、GATT 声明类型 UUID、属性及 payload。

具体编码格式需参照 `rdx_profile_data[]` 现有记录逐字节反推，不建议根据文档描述直接写。关键参照点：

- PRIMARY_SERVICE 声明：参照 handle 0x0001 / 0x0004 / 0x000C / 0x0010
- CHARACTERISTIC 声明：参照 handle 0x0002 / 0x0005 / 0x0007 等
- VALUE 声明：参照 handle 0x0003 / 0x0006 / 0x0008 等
- CCCD：参照 handle 0x0009 / 0x000F / 0x0015
- DESCRIPTOR（0x2908 Report Reference）：payload 为固定 2 字节 `[Report ID(1)] [Report Type(1)]`，其中 Report Type = 0x01(Input)/0x02(Output)/0x03(Feature)。**不含长度字段**——Report 数据长度由 Report Map 中的 REPORT_SIZE × REPORT_COUNT 定义
- 16-bit UUID vs 128-bit UUID 记录的长度差异：对比 GAP Service（16-bit）和 RDX Service（128-bit）

例如添加 Protocol Mode Characteristic（0x2A4E, handle 0x0017）：

```c
// 格式: [Length 2B LE] [Type 1B] [Properties 1B] [Handle 2B LE] [UUID nB]
// Type 编码及完整字节结构需参照 rdx_profile_data 现有记录逐字节验证。
// 建议先对照现有 RDX Characteristic 声明记录（如 handle 0x0005/0x0007）
// 反推正确的 length、type byte、properties 和 UUID 编码方式，
// 避免直接抄文档中的示例而未经验证。
```

---

## 4. HID Report Map

### 4.1 完整字节数组

```c
const uint8_t hid_report_map[] = {
    // ── Usage Page: Generic Desktop (0x01) ──
    0x05, 0x01,

    // ── Usage: Keyboard (0x06) ──
    0x09, 0x06,

    // ── Collection: Application ──
    0xA1, 0x01,

    // ── Report ID 0x01: Keyboard Input (8 bytes) ──
    0x85, 0x01,                         // Report ID = 1

    // Modifier byte (8 bits)
    0x05, 0x07,                         // Usage Page: Keyboard/Keypad
    0x19, 0xE0,                         // Usage Minimum: Left Ctrl (224)
    0x29, 0xE7,                         // Usage Maximum: Right GUI (231)
    0x15, 0x00,                         // Logical Minimum: 0
    0x25, 0x01,                         // Logical Maximum: 1
    0x75, 0x01,                         // Report Size: 1
    0x95, 0x08,                         // Report Count: 8
    0x81, 0x02,                         // Input: Data, Variable, Absolute

    // Reserved byte
    0x75, 0x08,                         // Report Size: 8
    0x95, 0x01,                         // Report Count: 1
    0x81, 0x01,                         // Input: Constant

    // Keycode array (6 keys)
    0x05, 0x07,                         // Usage Page: Keyboard/Keypad
    0x19, 0x00,                         // Usage Minimum: 0
    0x29, 0x65,                         // Usage Maximum: 101 (Keyboard Right GUI)
    0x15, 0x00,                         // Logical Minimum: 0
    0x25, 0x65,                         // Logical Maximum: 101
    0x75, 0x08,                         // Report Size: 8
    0x95, 0x06,                         // Report Count: 6
    0x81, 0x00,                         // Input: Data, Array, Absolute

    // V0 无 LED Output Report — 不定义 0x91 Output。
    // LED 状态仅通过 Boot Keyboard Output (0x2A32) 在 Boot Protocol 下接收，
    // write callback 接受并返回 0 但不执行任何硬件操作。
    // Report Protocol 下 Report Map 不含 Output Report，Windows 不会尝试写入 LED。

    0xC0,                               // End Collection

    // ── Report ID 0x02: Consumer Control (2 bytes) ──
    0x05, 0x0C,                         // Usage Page: Consumer
    0x09, 0x01,                         // Usage: Consumer Control
    0xA1, 0x01,                         // Collection: Application
    0x85, 0x02,                         // Report ID = 2
    0x15, 0x00,                         // Logical Minimum: 0
    0x25, 0x01,                         // Logical Maximum: 1
    0x75, 0x01,                         // Report Size: 1
    0x95, 0x10,                         // Report Count: 16
    0x0A, 0xCD, 0x00,                   // Usage: Play/Pause
    0x0A, 0xE9, 0x00,                   // Usage: Volume Up
    0x0A, 0xEA, 0x00,                   // Usage: Volume Down
    0x0A, 0xE2, 0x00,                   // Usage: Mute
    0x0A, 0xB6, 0x00,                   // Usage: Scan Next Track
    0x0A, 0xB5, 0x00,                   // Usage: Scan Previous Track
    0x0A, 0xB7, 0x00,                   // Usage: Stop
    0x0A, 0x94, 0x00,                   // Usage: My Computer
    0x0A, 0x23, 0x02,                   // Usage: WWW Home
    0x0A, 0x24, 0x02,                   // Usage: WWW Back
    0x0A, 0x25, 0x02,                   // Usage: WWW Forward
    0x0A, 0x26, 0x02,                   // Usage: WWW Stop
    0x0A, 0x27, 0x02,                   // Usage: WWW Refresh
    0x0A, 0x21, 0x02,                   // Usage: WWW Search
    0x0A, 0x2A, 0x02,                   // Usage: AC Bookmarks
    0x0A, 0x8A, 0x01,                   // Usage: Mail
    0x81, 0x02,                         // Input: Data, Variable, Absolute
    0xC0,                               // End Collection
};
```

### 4.2 Report 格式总结

| Report ID | 类型 | ATT 载荷长度（Report Protocol） | 数据字段 |
|----------:|------|-----:|------|
| `0x01` | Input | 8 bytes | modifiers(1) + reserved(1) + keycode[6] |
| `0x02` | Input | 2 bytes | Consumer bitmap(2) |
| Boot | Input | 8 bytes | modifiers(1) + reserved(1) + keycode[6]（发到 handle 0x0028） |
| Boot | Output | 1 byte | LED 状态（接收自 PC，handle 0x002B，忽略数据并返回 0） |

**payload 不含 Report ID 的原因**：当前设计为每个 Report 单独一个 0x2A4D characteristic + 各自 0x2908 Report Reference 描述符。0x2908 已告知 host 该 characteristic 对应哪个 Report ID / Report Type，notification payload 只需发送 Report 数据本体。
- Report Protocol → 发到 `0x2A4D` Report characteristic（handle 0x0020 / 0x0024），**不含** Report ID 前缀
- Boot Protocol → 发到 `0x2A22` Boot Keyboard Input（handle 0x0028），收从 `0x2A32` Boot Keyboard Output（handle 0x002B）

### 4.3 `Win + H` 发送时序

Report Protocol（handle 0x0020，8B payload，不含 Report ID——0x2908 已指定 ID=0x01）：

```text
Step 1: Keyboard Report (8 bytes)
  byte[0] = 0x08       // Left GUI
  byte[1] = 0x00       // reserved
  byte[2] = 0x0B       // Keyboard H
  byte[3..7] = 0x00

  → app_ble_att_send_data(hdl, 0x0020, report, 8, ...)

Step 2: 20ms sys_timer 回调

Step 3: All-release Report (8 bytes)
  byte[0..7] = 0x00    // 全部释放

  → app_ble_att_send_data(hdl, 0x0020, report, 8, ...)
```

Boot Protocol（handle 0x0028，8B，无 Report ID）：
```text
  byte[0] = 0x08       // Left GUI
  byte[1] = 0x00       // reserved
  byte[2] = 0x0B       // Keyboard H
  byte[3..7] = 0x00
  → app_ble_att_send_data(hdl, 0x0028, report, 8, ...)
```

### 4.4 HID Information 值

```c
uint8_t hid_info[] = {
    0x11, 0x01,     // bcdHID = 1.11 (小端: 0x0111)
    0x00,           // bCountryCode = 0 (Not localized)
    0x02            // Flags: bit0=RemoteWake(0), bit1=NormallyConnectable(1)
};
```

---

## 5. 回调分发重构

### 5.1 当前问题

当前 `rdx_ble_server_att_read_callback` 和 `rdx_ble_server_att_write_callback` 通过 switch-case 直接按 handle 分发。追加 HID Service 后 switch 会膨胀。同时 if+switch 嵌套模式不利于后续裁剪。

### 5.2 路由表方案

用静态路由表替代 if+switch。每个模块在 `hid_gatt_db.h` 中声明自己的路由项，`rdx_ble_server.c` 只做一次查表分发的改造。

**路由表定义（hid_gatt_db.h）：**

```c
// 每个 ATT 子模块注册一组 handle 范围 + handler 函数指针
typedef uint16_t (*att_read_fn)(void *hdl, hci_con_handle_t conn,
    uint16_t handle, uint16_t offset, uint8_t *buf, uint16_t buf_size);
typedef int (*att_write_fn)(void *hdl, hci_con_handle_t conn,
    uint16_t handle, uint16_t trans_mode, uint16_t offset,
    uint8_t *buf, uint16_t buf_size);

typedef struct {
    uint16_t     handle_begin;
    uint16_t     handle_end;
    att_read_fn  read_handler;
    att_write_fn write_handler;
} att_route_entry_t;

#if TCFG_MINI_HID_KEYBOARD_ENABLE
extern const att_route_entry_t hid_att_routes[];
extern const int hid_att_route_count;
#endif
```

**路由表实例（hid_gatt_db.inc 或 hid_keyboard.c）：**

```c
const att_route_entry_t hid_att_routes[] = {
    {
        .handle_begin = HID_HANDLE_BEGIN,
        .handle_end   = HID_HANDLE_END,
        .read_handler  = hid_att_read_callback,
        .write_handler = hid_att_write_callback,
    },
#if TCFG_MINI_HID_CONFIG_SERVICE_ENABLE
    {
        .handle_begin = CFG_HANDLE_BEGIN,
        .handle_end   = CFG_HANDLE_END,
        .read_handler  = hid_config_att_read_callback,
        .write_handler = hid_config_att_write_callback,
    },
#endif
};
const int hid_att_route_count = ARRAY_SIZE(hid_att_routes);
```

**RDX 侧改造（rdx_ble_server.c）：**

```c
static uint16_t rdx_ble_server_att_read_callback(void *hdl,
    hci_con_handle_t conn_handle, uint16_t att_handle,
    uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
#if TCFG_MINI_HID_KEYBOARD_ENABLE
    // HID 路由表优先匹配，命中后直接返回
    for (int i = 0; i < hid_att_route_count; i++) {
        const att_route_entry_t *r = &hid_att_routes[i];
        if (att_handle >= r->handle_begin && att_handle <= r->handle_end
            && r->read_handler) {
            return r->read_handler(hdl, conn_handle, att_handle,
                                   offset, buffer, buffer_size);
        }
    }
#endif
    // 原有 RDX switch-case 完全不变
    switch (att_handle) {
        case ATT_CHARACTERISTIC_2A00_01_VALUE_HANDLE: ...
        case ATT_CHARACTERISTIC_06068D3C_..._VALUE_HANDLE: ...
        case ATT_CHARACTERISTIC_2A19_01_VALUE_HANDLE: ...
        default: return 0;
    }
}
```

写回调同样模式——在 RDX switch 前插入路由表遍历。

### 5.3 路由表的优势

- RDX 代码只加一个 `for` 循环，不感知 HID 细节
- `TCFG_MINI_HID_KEYBOARD_ENABLE=0` 时路由表遍历被预处理器完全消除
- 新增 handle 段只需在 `hid_att_routes[]` 中追加一行，不改 RDX 回调
- V1 裁剪 RDX 时，HID 模块独立可复用

### 5.4 连接事件路由

HCI 连接/断开事件仍由 `rdx_ble_server_cbk_packet_handler` 统一处理。新增逻辑：

- 连接建立后，根据当前产品模式决定是否允许特定 CCC 配置
- 键盘模式下，PC 配置 HID CCC 不受限制
- 配置模式下，拒绝 PC 对 HID CCC 的写入

---

## 6. 配置模式状态机（V1 — V0 不做）

### 6.1 状态迁移

```text
┌──────────────┐
│ Keyboard Mode │  ← 默认模式
│ ADV: Mini Voice│    广播 HID Service UUID
│      Keyboard │    PC 可连接做 BLE 键盘
└──────┬───────┘
       │ KEY0 + KEY1 + KEY4 三键长按 5s
       ▼
┌──────────────┐
│ Config Entry  │  LED 快闪蓝 3 次 → 慢闪
│ disconnect PC │  断开当前 BLE 连接
│ switch ADV    │  广播名改为 "Mini Voice Keyboard CFG"
└──────┬───────┘  不广播 HID Service UUID
       │ 广播启动
       ▼
┌──────────────┐
│ Config Adv    │  等待 APP 连接，超时 120s
│               │  超时 → 回到 Keyboard Mode
└──────┬───────┘
       │ APP 连接
       ▼
┌──────────────┐
│ Config Conn   │  LED 常亮 1s → 呼吸
│               │  APP 读写 Key Map / Commit / Reset
└──────┬───────┘  APP 空闲 60s → 断开回到 Keyboard Mode
       │
       ├─── Commit 成功 ──→ LED 绿闪 2 次 → Keyboard Mode
       ├─── Reset 成功  ──→ LED 绿闪 2 次 → Keyboard Mode
       ├─── 写入失败    ──→ LED 红闪 3 次 → 等待重试或退出
       └─── 长按 KEY0+KEY1+KEY4 5s ──→ 强制退出 → Keyboard Mode
```

### 6.2 状态机数据结构

```c
typedef enum {
    PRODUCT_MODE_KEYBOARD = 0,    // 键盘模式（默认）
    PRODUCT_MODE_CONFIG_ENTRY,    // 进入配置模式（断开中）
    PRODUCT_MODE_CONFIG_ADV,      // 配置模式广播中
    PRODUCT_MODE_CONFIG_CONN,     // 配置模式已连接
} product_mode_t;

typedef struct {
    product_mode_t mode;
    sys_timer config_entry_timer;    // KEY0+KEY1+KEY4 三键长按检测
    sys_timer config_adv_timeout;    // 广播超时 120s
    sys_timer config_idle_timeout;   // 连接空闲超时 60s
    bool config_dirty;               // 有未提交的配置修改
} product_mode_ctx_t;
```

### 6.3 模式切换函数

```c
void product_mode_enter_config(void);
void product_mode_exit_config(void);
void product_mode_commit_done(void);
void product_mode_reset_done(void);
void product_mode_force_exit(void);
```

### 6.4 配对与安全策略

**V0 安全策略（L0/L1 分层，与 §13.3 对齐）：**

| 级别 | SM 配置 | 说明 |
|------|---------|------|
| **L0（V0 基线）** | `IO_CAPABILITY_NO_INPUT_NO_OUTPUT`, `SM_AUTHREQ_MITM_PROTECTION \| SM_AUTHREQ_BONDING` | Just Works + Bonding。当前 RDX 已启用，是 V0 最低安全基线。Bonding 是 Windows 回连的关键 |
| **L1（V0 推荐）** | L0 + `SM_AUTHREQ_SECURE_CONNECTION` | LE Secure Connections。需 §14 Step 1 验证 RDX APP 兼容性后启用 |

**关键认知**：
- `IO_CAPABILITY_NO_INPUT_NO_OUTPUT + MITM` **不提供真正的 MITM 交互保障**——设备无显示屏和键盘，无法完成数值比较或密码输入。实际退化为 Just Works 配对。`SM_AUTHREQ_MITM_PROTECTION` flag 在此配置下的作用是指示"期望 MITM 但实际不可用"，BTstack 会降级处理。
- **Bonding 是 Windows 回连的关键**（不是 SC）。当前 RDX 已开启 bonding（`SM_AUTHREQ_BONDING`），V0 基线保留。
- **SC 建议验证后启用**：L1 提供更强的加密。但需 §14 Step 1 先验证 RDX APP 能否在 SC 下正常配对和回连。若不兼容则 V0 使用 L0。

**配置模式安全策略：**

- 写 Key Map / Commit / Reset 仅在 `PRODUCT_MODE_CONFIG_CONN` 状态下接受
- 写入 payload 校验：长度、协议版本、key_index 范围 0~4、CRC
- 物理按键进入配置模式为安全门槛（V0 无法被远程触发）
- V1 评估配置模式单独 bonding 或应用层二次认证

---

## 7. 广播策略

### 7.1 两种模式的广播数据

| 字段 | 键盘模式 | 配置模式 |
|------|---------|---------|
| ADV Flags | 沿用 RDX 现有 flags（当前为 0x0A: LE General Discoverable） | 同左 |
| Complete Local Name | `Mini Voice Keyboard` | `Mini Voice Keyboard CFG` |
| Service UUID (ADV) | 0x1812 (HID) — 16-bit UUID | 不广播 HID UUID |
| Scan Response | 同现有 RDX 厂商数据 | 仅 Config Service 128-bit UUID（或不发 RSP） |

### 7.2 实现

复用现有 `rdx_ble_server_fill_adv_data` / `rdx_ble_server_fill_rsp_data` 框架，新增模式参数：

```c
static u8 rdx_ble_server_fill_adv_data(u8 *adv_data, product_mode_t mode)
{
    u8 offset = 0;
    const char *name_p = (mode == PRODUCT_MODE_KEYBOARD)
        ? "Mini Voice Keyboard"
        : "Mini Voice Keyboard CFG";

    offset += make_eir_packet_val(&adv_data[offset], offset,
        HCI_EIR_DATATYPE_FLAGS, 0x0A, 1);
    offset += make_eir_packet_data(&adv_data[offset], offset,
        HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)name_p, strlen(name_p));

    if (mode == PRODUCT_MODE_KEYBOARD) {
        // 广播 HID Service UUID (0x1812)
        uint8_t hid_uuid16[] = {0x12, 0x18};
        offset += make_eir_packet_data(&adv_data[offset], offset,
            HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS,
            (void *)hid_uuid16, sizeof(hid_uuid16));
    }
    // 配置模式不广播 service UUID，降低 Windows 自动回连概率

    return offset;
}
```

### 7.3 广播数据长度核算

键盘模式 ADV 数据预估：

| 字段 | 字节 |
|------|-----:|
| AD Length + Type (Flags) | 3 |
| Flags value | 1 |
| AD Length + Type (Complete Local Name) | 2 |
| Name "Mini Voice Keyboard" | 20 |
| AD Length + Type (16-bit Service UUIDs) | 2 |
| HID UUID 0x1812 | 2 |
| **合计** | **30** |

30B 在 BLE 4.x ADV 31 字节限制内，但已很紧凑。注意：
- 如果后续产品名加长，需要缩减或拆分到 SCAN_RSP
- 现有 `rdx_ble_server_fill_adv_data` 如果原本还塞了 Manufacturer Specific Data，必须改掉，不能和 HID UUID 共存于 ADV 中
- 厂商数据（Product Code / MAC / Factory Code 等）继续放在 SCAN_RSP 中，不受影响

### 7.4 重新启用广播

进入配置模式前先 `rdx_ble_server_adv_enable(0)`，更新 ADV/RSP 数据后重新 `rdx_ble_server_adv_enable(1)`。退出时同样流程。

---

## 8. HID Report 发送

### 8.1 发送接口

```c
// hid_report_sender.h

// 发送键盘 Report（普通按键 / 组合键）
// modifiers: LeftCtrl=0x01, LeftShift=0x02, LeftAlt=0x04, LeftGUI=0x08
// keycode: HID keycode 数组，长度 0~6，0x00 表示空
// 返回: 0=成功, 非0=失败
int hid_send_keyboard_report(uint8_t modifiers, const uint8_t *keycodes, uint8_t count);

// 发送 Consumer Control Report（媒体键）
// usage_id: 例如 0x00CD=Play/Pause, 0x00E9=Vol+, 0x00EA=Vol-
int hid_send_consumer_report(uint16_t usage_id);

// 发送全释放 Report（防粘键）
int hid_send_all_release(void);

// 发送 Win + H（语音键专用）
int hid_send_voice_key(void);
```

### 8.2 `hid_send_voice_key` 实现

`Win + H` 需要 Press → Delay → Release 时序。**不允许在 BLE callback / 发送链路中调用 `os_time_dly()` 等阻塞函数**——`app_ble_att_send_data` 运行在 BLE 事件回调上下文，阻塞会导致整个 BLE 协议栈卡死。

正确做法：使用 sys_timer 或投递到 app task 分两步发送。

```c
// 语音键 Press/Release 状态机
static uint8_t g_voice_key_state = 0;  // 0=idle, 1=waiting_release

static void hid_voice_key_release_timer_cb(void *p)
{
    // 通过统一接口发送 Release，内部根据 g_hid_protocol_mode 选择 handle
    hid_send_all_release();
    g_voice_key_state = 0;
}

int hid_send_voice_key(void)
{
    if (g_voice_key_state != 0) return -1;  // busy

    // 通过统一接口发送 Press（Left GUI + H）
    // 底层 hid_send_keyboard_report() 根据 g_hid_protocol_mode
    // 自动选择 0x0020（Report）或 0x0028（Boot）
    uint8_t keys[] = {0x0B};  // Keyboard H
    int ret = hid_send_keyboard_report(0x08 /* Left GUI */, keys, 1);
    if (ret) return ret;

    g_voice_key_state = 1;

    // 注册 ~20ms 一次性定时器，到时自动发送 Release
    // 具体 API 以项目使用的 sys_timer 接口为准（如 sys_timeout_add / sys_timer_add 等）
    sys_timeout_add(NULL, hid_voice_key_release_timer_cb, 20);

    return 0;
}

// hid_report_sender.c 内部实现关键点：
// hid_send_keyboard_report() 根据 g_hid_protocol_mode 分发：
//   mode=0x01 → handle HID_HANDLE_KB_INPUT  (0x0020), 8B payload
//   mode=0x00 → handle HID_HANDLE_BOOT_KB_INPUT (0x0028), 8B payload
// hid_send_all_release() 同理，发全零 8B payload 到对应 handle
// hid_send_voice_key() 不直接碰 handle 宏，全部走统一接口
```

调用前应通过 `hid_can_send()` 检查 BLE 连接状态和 HID CCC 是否已配置。发送失败时需处理重试或丢弃，避免阻塞按键事件队列。

### 8.3 按键到 HID Report 的调用链

```text
IO key driver (rdx_key.c)
  → 按键事件（短按/长按等）
  → V0 固定映射 switch(physical_key_index):  // V1 替换为 keymap_manager_lookup()
      KEY0 → hid_send_voice_key()
      KEY1 → hid_send_keyboard_report(0, {KEY_ENTER}, 1)
      KEY2 → hid_send_keyboard_report(0, {KEY_BACKSPACE}, 1)
      KEY3 → hid_send_keyboard_report(0, {KEY_SPACE}, 1)
      KEY4 → hid_send_consumer_report(CONSUMER_MUTE)
  → hid_report_sender 构建 Report 并调用 app_ble_att_send_data()
  → BLE 栈发送 Notification
```

`voice_key`（report_type=2）为特殊项。`hid_send_voice_key()` 自行处理 Press(8B) + Timer(20ms) + Release(8B) 时序，不走 `hid_report_sender` 的通用单次 Report 构建路径。

### 8.4 发送前提条件检查

**HID CCC 独立追踪。** 不复用 RDX 的 `ccc_configured`——RDX 的 CCC 对应 RDX Notify handle (0x0009/0x0015)，而 HID 有独立的 CCC：
- 0x0022：Keyboard Input Report CCC
- 0x0026：Consumer Control Report CCC
- 0x0029：Boot Keyboard Input Report CCC

HID 模块在 ATT write callback 中自行记录 CCC 状态，不依赖 RDX 全局变量：

```c
// hid_keyboard.c — HID 模块本地状态
static bool g_hid_kb_input_ccc = false;      // handle 0x0022 被 Central 配置为 notify
static bool g_hid_boot_kb_input_ccc = false; // handle 0x0029
static bool g_hid_consumer_ccc = false;      // handle 0x0026

// 在 ATT write callback 中，收到 CCCD handle 写入时更新：
// CCC value 为 16-bit 小端，buffer[0..1] 组成 u16：
//   0x0001 = notifications enabled, 0x0000 = disabled
// 注意：必须调用 SDK 的 att_set_ccc_config(handle, value) 更新 BTstack 内部 CCC 状态，
// 否则 ATT_OP_AUTO_READ_CCC 模式下 notify 会被 BTstack 拦截。参照 rdx_ble_server.c 现有 CCC 处理。
// 写入长度异常时忽略（CCC value 固定 2 字节）：
//
// case HID_HANDLE_KB_INPUT_CCCD:
//     if (buffer_size >= 2) {
//         u16 ccc = little_endian_read_16(buffer, 0);
//         g_hid_kb_input_ccc = (ccc == 0x0001);
//         att_set_ccc_config(HID_HANDLE_KB_INPUT_CCCD, ccc);
//     }
//     break;
// case HID_HANDLE_BOOT_KB_INPUT_CCCD:
//     if (buffer_size >= 2) {
//         u16 ccc = little_endian_read_16(buffer, 0);
//         g_hid_boot_kb_input_ccc = (ccc == 0x0001);
//         att_set_ccc_config(HID_HANDLE_BOOT_KB_INPUT_CCCD, ccc);
//     }
//     break;
// case HID_HANDLE_CONSUMER_CCCD:
//     if (buffer_size >= 2) {
//         u16 ccc = little_endian_read_16(buffer, 0);
//         g_hid_consumer_ccc = (ccc == 0x0001);
//         att_set_ccc_config(HID_HANDLE_CC_INPUT_CCCD, ccc);
//     }
//     break;

// 断开连接时（HCI_EVENT_DISCONNECTION_COMPLETE）清空所有 CCC 状态：
// g_hid_kb_input_ccc = g_hid_boot_kb_input_ccc = g_hid_consumer_ccc = false;

// RDX 侧提供连接状态查询（rdx_ble_server.h）
bool rdx_ble_server_is_connected(void);

// HID 侧使用 — 按当前 protocol mode 判断对应 CCC
bool hid_can_send(void)
{
    // V0 只有键盘模式，无 product_mode 状态机
    return rdx_ble_server_is_connected()
        && ((g_hid_protocol_mode == 0x01 && g_hid_kb_input_ccc)
            || (g_hid_protocol_mode == 0x00 && g_hid_boot_kb_input_ccc));
}
```

注：
- V0 不做 `g_product_mode_ctx`——那是 V1 配置模式的概念。V0 始终处于键盘模式。
- RDX `ccc_configured` 不能被 HID 复用。绕开此耦合最简单的做法是：HID ATT write callback 在收到 CCCD handle 写入时自行记录 CCC 状态。
- `rdx_ble_server_is_connected()` 仅用于查询 BLE 连接状态（con_handle 是否有效），不引入 RDX 的录音/文件同步语义。

### 8.5 Protocol Mode 切换处理

BLE HID 支持两种协议模式：

| Mode | Protocol Mode Value | Report 行为 |
|------|-------------------:|-------------|
| Boot Protocol | `0x00` | 固定 8 字节 Boot Keyboard Input Report，无 Report ID 前缀 |
| Report Protocol | `0x01` | 按 Report Map 定义，Report ID 由 0x2908 标识，ATT payload 不含 Report ID |

Windows 可能在启动阶段（如 BitLocker PIN 输入、Pre-Login）通过 HID Control Point 发送 `Suspend`/`Exit Suspend`，或直接写 Protocol Mode Characteristic 切换到 Boot Protocol。

**固件行为要求：**

```c
static uint8_t g_hid_protocol_mode = 0x01;  // 默认 Report Protocol

// Protocol Mode write callback
static void hid_protocol_mode_write_handler(uint16_t value_len, uint8_t *value)
{
    if (value_len == 1 && (value[0] == 0x00 || value[0] == 0x01)) {
        g_hid_protocol_mode = value[0];
    }
}

// 发送时根据当前 mode 选择 Report 格式
int hid_send_keyboard_report(uint8_t modifiers, const uint8_t *keycodes, uint8_t count)
{
    if (g_hid_protocol_mode == 0x00) {
        // Boot Protocol: 8B payload，发到 Boot KB Input (0x0028)
        // 格式: modifiers(1) + reserved(1) + keycode[6]
        return hid_send_boot_report(modifiers, keycodes, count);
    } else {
        // Report Protocol: 8B payload，发到 Report (0x0020)
        // 格式: modifiers(1) + reserved(1) + keycode[6]
        // 不含 Report ID — 0x2908 已标注此 characteristic 对应 Report ID 0x01
        return hid_send_report_report(modifiers, keycodes, count);
    }
}
```

- 上电默认 Protocol Mode = 0x01（Report Protocol）
- Boot Protocol 下发送 Boot Keyboard Input Report（handle 0x0028），不含 Report ID
- Consumer Control Report（handle 0x0024）不属于 Boot Protocol 范畴；Boot Protocol 下优先保证 Boot Keyboard Input Report 可用。设备端仍可按 Report Protocol 方式发送 Consumer Control，但 Windows 在 Boot Mode 下不保证响应
- Boot Keyboard Output Report（handle 0x002B）用于接收 PC 的 LED 状态（Caps Lock 等），设备如无 LED 可忽略数据，但 write callback **必须返回 0 表示成功**（当前 SDK 所有 ATT write callback 示例均以 `return 0` 表示正常处理完成，包括 RDX、custom_protocol、trans_data_demo。返回非零值可能被 BTstack 解释为错误，导致 Windows 认为写入失败并影响设备枚举）

### 8.6 连接参数策略

当前 RDX 连接参数表第一档为 `{6, 10, 4, 600}`（interval 7.5ms~12.5ms, latency=4），interval 已接近 HID 键盘需求，但 latency=4 对按键响应不理想。此外 RDX 目前未主动调用 `rdx_ble_server_check_connetion_updata_deal()`。

**V0 HID 模式连接参数要求：**

| 参数 | 推荐值 | 说明 |
|------|-------|------|
| Interval Min | 6 (~7.5ms) | HOGP 典型值，保证按键响应 |
| Interval Max | 9~10 (~11.25~12.5ms) | Windows 中央设备常用上限 |
| Latency | 0 或 1 | 低延迟，按键不应被跳过 |
| Timeout | ≥300ms | 保证连接稳定性 |

**实现方式：**

- 优先使用现有 RDX API `rdx_ble_server_send_request_connect_parameter()`（若存在）或 `ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, ...)`。使用 `TCFG_MINI_HID_CONN_*` 宏填充连接参数结构体
- HID 连接建立后需覆盖 **两种连接完成事件**：
  - `HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE`（BLE 5.0+，当前 rdx_ble_server.c 已有 switch，但分支只打印日志）
  - `HCI_SUBEVENT_LE_CONNECTION_COMPLETE`（legacy BLE 4.x）
- 注意：Windows 作为 Central 可能主动发起连接参数更新。如果 Central 已发起请求（`L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE`），设备端应接受而非拒绝
- 若当前 RDX 框架已有连接参数更新检查（`check_connetion_updata_deal`），HID 模式下跳过或修改其判断逻辑

---

## 9. 按键映射数据结构（V0 固定默认映射，V1 扩展为可配置 keymap）

### 9.1 keymap_item_t

```c
#define KEYMAP_MAGIC 0x4B4D4150u  // "KMAP"

typedef struct {
    uint8_t  key_index;      // 物理按键编号 0~4
    uint8_t  action;         // 0=click, 1=long_press, 2=double_click
    uint8_t  report_type;    // 0=keyboard, 1=consumer, 2=voice_key
    uint8_t  modifiers;      // 组合键修饰符（键盘模式下）
    uint16_t usage_page;     // HID Usage Page
    uint16_t usage_id;       // HID Usage ID
    uint8_t  flags;          // bit0=enable, bit1=repeat, others=reserved
    uint8_t  reserved[7];
} keymap_item_t;  // 16 bytes（自然对齐后），需与 APP 确认最终长度
```

### 9.2 默认映射（固件内置）

```c
const keymap_item_t default_keymap[5] = {
    // KEY0: 语音键 → Win + H
    { .key_index=0, .action=0, .report_type=2, .modifiers=0,
      .usage_page=0, .usage_id=0, .flags=0x01 },
    // KEY1: Enter
    { .key_index=1, .action=0, .report_type=0, .modifiers=0,
      .usage_page=0x07, .usage_id=0x28, .flags=0x01 },
    // KEY2: Backspace
    { .key_index=2, .action=0, .report_type=0, .modifiers=0,
      .usage_page=0x07, .usage_id=0x2A, .flags=0x01 },
    // KEY3: Space
    { .key_index=3, .action=0, .report_type=0, .modifiers=0,
      .usage_page=0x07, .usage_id=0x2C, .flags=0x01 },
    // KEY4: 静音
    { .key_index=4, .action=0, .report_type=1, .modifiers=0,
      .usage_page=0x0C, .usage_id=0x00E2, .flags=0x01 },
};
```

---

## 10. APP 协议对齐清单（V1 阻塞项）

V0 阶段只需固定映射，不需要 Config Service。进入 V1 编码前必须与 APP 端逐项对齐：

| # | 对齐项 | 状态 |
|---|--------|------|
| 1 | Config Service UUID (128-bit) | 待 APP 端确认 |
| 2 | Protocol Version / Device Status / Key Map / Commit / Reset / Reboot 的 Characteristic UUID (128-bit) | 待确认 |
| 3 | `keymap_item_t` 结构体字段定义和字节序 | 待确认，推荐小端 |
| 4 | 单个 keymap_item_t 的长度（含 padding） | 待确认，当前 16 bytes（reserved[7]） |
| 5 | Commit 命令 payload 格式和返回码 | 待确认 |
| 6 | Reset 命令：恢复默认映射还是清空全部 | 待确认 |
| 7 | Reboot/Apply 命令：立即生效还是重启后生效 | 待确认 |
| 8 | Notify 返回码定义（成功/CRC错误/版本不兼容/权限不足） | 待确认 |
| 9 | 协议版本号及 APP 不兼容时的行为 | 待确认 |
| 10 | Device Status Notify 的 payload 格式 | 待确认 |

---

## 11. V0 验证计划

### 11.1 验证前提

- Step 0（`le_hogp_*` 验证）已完成，已确定走"调库"还是"手写 GATT DB"
- 固件已编译并烧录
- HID Service 已在 BTstack 中注册（通过库函数或手写 `rdx_profile_data[]`）
- BLE 广播包含 HID Service UUID (0x1812)
- `t2620_project_config.h` 中 `TCFG_MINI_HID_KEYBOARD_ENABLE` 设为 `1`
- `TCFG_BT_SUPPORT_HID` 设为 `0`（关闭经典蓝牙 HID）

### 11.2 BLE HID 连通性验证

| 步骤 | 预期结果 | 判定标准 |
|------|---------|---------|
| 1. Windows 11 打开「蓝牙和设备」→「添加设备」→「蓝牙」 | 搜索列表中显示 `Mini Voice Keyboard` | 设备名正确出现 |
| 2. 点击设备名配对 | Windows 显示「已连接」 | 连接成功，无配对错误 |
| 3. 查看 nRF Connect / BLE 嗅探器中的连接参数 | interval 7.5~12.5ms, latency 0~1 | 连接参数满足 HID 低延迟 |
| 4. 打开记事本，按 KEY1 (Enter) 键 | 记事本出现换行 | 键盘 Report 被 Windows 正确解析 |
| 5. 按 KEY0 (语音键) | 弹出 Windows 语音键入工具栏 | `Win + H` 组合键正确发送 |
| 6. 连续快速按 KEY3 20 次 | 记事本出现 20 个空格 | 不丢键、不乱码 |
| 7. Windows 锁屏后再解锁，按 KEY1 | 记事本出现换行 | Suspend/Exit Suspend 恢复后键盘仍可用 |
| 8. BitLocker / 登录界面按 KEY1 | 输入 PIN 时按键正常 | Boot Protocol 正常工作 |
| 9. 断开 BLE 连接后重新上电 | Windows 自动回连 | bonding 正常；若 V0 未启用 SC 则手动重连可接受 |
| 10. 按 KEY4 (静音) | Windows 系统音量静音 | Consumer Control Report 正常 |
| 11. 在设备管理器中查看 | 出现 BLE HID Keyboard 设备 | Windows 正确识别为键盘 |

### 11.3 RDX APP 共存验证（V1 配置模式 — V0 不测试本节

V0 只验证 PC 键盘模式。配置模式（APP 连接、按键映射修改）推迟到 V1。以下为 V1 预留：

| 步骤 | 预期结果 | 判定标准 |
|------|---------|---------|
| 1. 长按 KEY0+KEY1+KEY4 5s | LED 快闪蓝 3 次 → 慢闪，PC 断开 | 进入配置模式 |
| 2. 手机 APP 扫描 | 看到 `Mini Voice Keyboard CFG` | 配置广播名正确 |
| 3. APP 连接设备 | LED 常亮 1s → 呼吸，连接成功 | 配置连接正常 |
| 4. APP 读取各 RDX Characteristic | 返回正确的设备信息（版本/电量等） | RDX 协议仍正常工作 |
| 5. APP 断开或超时 60s | LED 熄灭，设备恢复键盘广播 | 退出配置模式 |
| 6. PC 重新连接 | 按键功能恢复正常 | 键盘模式恢复 |

### 11.4 回归验证

| 步骤 | 预期结果 | 判定标准 |
|------|---------|---------|
| 1. RDX 20s 强制断开不会在 HID 模式下误触发 | PC 连接保持 >20s | `rdx_ble_server_connected_handle()` 中 HID 模式跳过超时断开 |
| 2. 现有 RDX APP 配对和回连 | APP 仍可正常连接设备 | RDX 协议不受 HID 新增影响 |
| 3. 经典蓝牙 HFP 连接 | Windows 仍可识别 Hands-Free 麦克风 | HFP 功能不受 BLE 改动影响 |

---

## 12. 配置宏规划

### 12.1 配置位置

**所有 HID 相关宏写入 `SDK/apps/earphone/include/t2620_project_config.h`，不在 `sdk_config.h` 中新增任何宏。**

依据：
- `sdk_config.h` 由 JL 视觉配置工具自动同步，手动修改会被工具回写覆盖（`AGENTS.md:127`）
- `t2620_project_config.h` 在 `app_config.h:5` 中 include，位于 `sdk_config.h` 之后，`#undef` + 重定义能被后续代码正确看到
- 项目已有先例：`TCFG_DIP_SWITCH_POWER_ENABLE` 等宏已通过此方式管理（`AGENTS.md:224`）

### 12.2 宏设计

```c
// SDK/apps/earphone/include/t2620_project_config.h（新增）

/*
 * MINI 语音键盘 — V0 BLE HID 验证
 * 设为 1 启用 BLE HID Keyboard；设为 0 完全回退到纯 RDX 模式
 */
#ifndef TCFG_MINI_HID_KEYBOARD_ENABLE
#define TCFG_MINI_HID_KEYBOARD_ENABLE    0   // V0 验证时改为 1
#endif

#if TCFG_MINI_HID_KEYBOARD_ENABLE

// ── 设备标识（可按需覆盖）──
#ifndef TCFG_MINI_HID_DEVICE_NAME
#define TCFG_MINI_HID_DEVICE_NAME        "Mini Voice Keyboard"
#endif
#ifndef TCFG_MINI_HID_APPEARANCE
#define TCFG_MINI_HID_APPEARANCE         0x03C1  // BLE_APPEARANCE_HID_KEYBOARD (961)
#endif

// ── 功能开关 ──
#ifndef TCFG_MINI_HID_BOOT_PROTOCOL_ENABLE
#define TCFG_MINI_HID_BOOT_PROTOCOL_ENABLE   1   // Boot Protocol（BitLocker/登录界面）。V0 固定=1，暂不支持关闭
#endif
#ifndef TCFG_MINI_HID_CONSUMER_ENABLE
#define TCFG_MINI_HID_CONSUMER_ENABLE        1   // Consumer Control（媒体键）。V0 固定=1，暂不支持关闭
#endif

// 注意：以上两个宏在 V0 阶段固定为 1。V2+ 如需按宏裁剪 GATT DB / Report Map / handle 定义，
// 需完整补齐条件编译路径，包括：
//   - hid_gatt_db.inc 中对应 characteristic 的 #if 包裹
//   - hid_gatt_db.h 中对应 handle 宏的 #if 包裹
//   - hid_report_map.c 中 Report Map 分段（如有 ID 0x02 的 Consumer 被关闭则删除对应段）
//   - §11 验证计划中相应测试项的条件存在
// 在条件编译路径补齐之前，这两个宏不可设为 0。
#ifndef TCFG_MINI_HID_CONFIG_SERVICE_ENABLE
#define TCFG_MINI_HID_CONFIG_SERVICE_ENABLE  0   // V1 配置服务
#endif

// ── 连接参数 ──
#ifndef TCFG_MINI_HID_CONN_INTERVAL_MIN
#define TCFG_MINI_HID_CONN_INTERVAL_MIN      6   // 7.5ms (unit: 1.25ms)
#endif
#ifndef TCFG_MINI_HID_CONN_INTERVAL_MAX
#define TCFG_MINI_HID_CONN_INTERVAL_MAX      9   // 11.25ms
#endif
#ifndef TCFG_MINI_HID_CONN_LATENCY
#define TCFG_MINI_HID_CONN_LATENCY           0   // 不跳过连接事件
#endif
#ifndef TCFG_MINI_HID_CONN_TIMEOUT
#define TCFG_MINI_HID_CONN_TIMEOUT           300 // 300ms
#endif

// ── 蓝牙 profile 裁剪 ──
// 关闭经典蓝牙 HID，避免 BR/EDR HID 与 BLE HID 双键盘
#undef  TCFG_BT_SUPPORT_HID
#define TCFG_BT_SUPPORT_HID              0

#ifndef TCFG_MINI_HID_DISABLE_A2DP
#define TCFG_MINI_HID_DISABLE_A2DP           1   // 产品不做蓝牙播放，默认关闭 A2DP
#endif
#if TCFG_MINI_HID_DISABLE_A2DP
#undef  TCFG_BT_SUPPORT_A2DP
#define TCFG_BT_SUPPORT_A2DP             0
#endif

#endif // TCFG_MINI_HID_KEYBOARD_ENABLE
```

V1 追加：

```c
#define TCFG_MINI_HID_CONFIG_SERVICE_ENABLE  0   // V1 时改为 1
```

### 12.3 条件编译约定

所有 HID 代码用以下 guard 包裹：

```c
#if TCFG_MINI_HID_KEYBOARD_ENABLE
// ... HID 模块代码 ...
#endif
```

当 `TCFG_MINI_HID_KEYBOARD_ENABLE=0` 时，HID 模块完全不存在——GATT DB 不含 HID Service、路由表为空展开、回调不增加任何开销。

**禁止**在新的 `MINI_KEYBOARD_V0`/`MINI_KEYBOARD_V1` 宏名——统一用 `TCFG_MINI_HID_KEYBOARD_ENABLE` 作为主开关，与 SDK 现有 `TCFG_` 前缀风格一致。

### 12.4 GATT DB 片段引用方式

```c
// rdx_ble_server.c — profile_data 组合

const uint8_t rdx_profile_data[] = {
    // ... 现有 RDX Service 条目 ...

#if TCFG_MINI_HID_KEYBOARD_ENABLE
    #include "hid/hid_gatt_db.inc"       // .inc 片段，不是 .c
#endif
    // END
    0x00, 0x00,
};
```

使用 `.inc` 而非 `.c` 命名，表示这是被 `#include` 的头文件片段，不参与独立编译。

---

## 13. 关键风险

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| `le_hogp_*` 库函数能力未知 | 若库函数已实现 HOGP，大量手写代码变为浪费 | 编码前先做 Step 0 验证：调 `le_hogp_set_ReportMap` + `le_hogp_set_icon`，nRF Connect 检查 0x1812 是否自动出现 |
| Windows 对 Report Map 解析失败 | 键盘无法使用 | 先用 nRF Connect / BLE 嗅探器验证 Report Map 格式；参考已知可用的 HOGP Report Map |
| 经典 HID (BR/EDR) 与 BLE HID 共存导致 Windows 出现两个键盘 | 设备列表混乱 | V0 即在 `t2620_project_config.h` 中 `#undef TCFG_BT_SUPPORT_HID` + `#define 0` |
| RDX 20s 强制断开逻辑在键盘模式下误杀 BLE 连接 | PC 连接后 20s 被设备侧断开 | V0 在 `rdx_ble_server.c` 的 `rdx_ble_server_connected_handle()` 中检查 HID 模式，跳过超时断开逻辑 |
| GATT 数据库变大导致 RAM 压力 | 栈溢出或连接失败 | 确认 ATT_RAM_BUFSIZE 足够；监控编译后内存占用 |
| 广播数据超过 31 字节限制 | ADV 截断 | §7.3 已核算 30B 在限制内 |
| 配置模式下 Windows 自动回连抢占 BLE | APP 无法配 | 配置模式不广播 HID UUID + 改名 + 超时兜底（V1） |
| RDX APP 与 HID Service 在同一 GATT DB 中互联 | APP 意外触发 HID 行为 | 路由表按 handle 段严格隔离 |
| 配置模式三键组合（KEY0+KEY1+KEY4）仍可能被误触 | 用户意外进入配置模式 | 长按 5s 门槛较高（V1）；后续可评估改为需要电源键参与的更复杂组合 |
| BLE 连接参数不满足 HID 低延迟需求 | 按键延迟大或 Windows 拒绝连接 | §8.6：HID 模式主动请求 7.5ms~12.5ms, latency=0~1 |
| `os_time_dly` 在 BLE 上下文阻塞 | BLE 协议栈卡死 | §8.2 已改为 timer 回调两段发送 |

### 13.1 最大坑：GATT handle 手写编码

`rdx_profile_data[]` 是手写十六进制字节数组，一处 UUID 长度或属性位写错就会导致全部 handle 漂移。HID Service 从 handle 0x0016 开始连续排布 20+ 条记录。

**缓解：**
- HID GATT DB 片段先写最小测试版本（只加一个自定义 Service），用 nRF Connect 扫描确认所有 handle 与头文件宏一致
- 编写 handle 自检脚本，解析编译后的 `rdx_profile_data[]` 字节，与 `hid_gatt_db.h` 中宏做 diff
- 参照 `tests/host/test_rdx_playback_static.ps1` 风格写 PowerShell 校验脚本

### 13.2 最大坑：Report Map 分段读取

HID Report Map 通常 80+ 字节。Windows 必然通过 Read Blob 分段读取。当前 RDX read callback 中只有 Device Name 正确处理了 offset，其他 handler 都没按 offset 分段返回。

**缓解：**
- HID 的 read handler 必须实现标准 offset 逻辑：
  ```c
  if (offset >= total_len) return 0;
  uint16_t copy_len = MIN(buffer_size, total_len - offset);
  if (buffer) memcpy(buffer, &data[offset], copy_len);
  return copy_len;
  ```
- 用 nRF Connect 模拟分段读取验证（先读前 22 字节 MTU，再 offset=22 读剩余）

### 13.3 最大坑：安全参数与 RDX APP 冲突

`multi_protocol_main.c` 中 `app_ble_sm_init()` 设置的安全参数是全局的，所有 BLE 连接共享。

**V0 安全策略（分层建议，按验证结果逐级启用）：**

| 级别 | SM 配置 | 作用 | 风险 |
|------|---------|------|------|
| L0（当前 RDX） | `SM_AUTHREQ_MITM_PROTECTION \| SM_AUTHREQ_BONDING` | Just Works + Bonding，保证 Windows 回连 | 无 |
| L1（推荐 V0） | L0 + `SM_AUTHREQ_SECURE_CONNECTION` | LE Secure Connections，增强 bonding 安全性 | 可能影响旧版 RDX APP 配对/回连 |
| L2（V1 评审） | L1 + `IO_CAPABILITY_DISPLAY_YES_NO` | 带确认的 MITM | 设备无显示屏，需评估可行性 |

**关键认知：**
- **Bonding 是 Windows 回连的关键**（不是 SC）。无 bonding 时 Windows 每次断开后可能要求重新配对。当前 RDX 已开启 bonding，V0 基线应保留。
- **SC 建议验证后启用**，但不是 V0 必须项。注意 `IO_CAPABILITY_NO_INPUT_NO_OUTPUT + MITM` 不能提供真正的 MITM 交互保障——Just Works 的安全性已足够应对 BLE 键盘场景。
- **缓解**：§14 Step 1 先验证 SC 兼容性。若 RDX APP 兼容则启用 L1；若不兼容则 V0 使用 L0（Just Works + Bonding），Windows 回连仍正常。

---

## 14. 实施前验证清单

进入固件编码前，必须按顺序完成以下验证，降低返工风险：

### Step 0：`le_hogp_*` 库函数能力验证（最高优先级，1-2h）

**目的**：确认 Jieli SDK 库层的 `le_hogp_set_ReportMap()` / `le_hogp_set_icon()` 能否直接接管 HID Service 注册。

**方法**：
1. 在隔离分支，在现有 RDX BLE init 流程中（`multi_protocol_profile_init` 或 `app_ble_init` 之后）调用：
   ```c
   #include "third_party_profile/jieli/le_common.h"
   extern const uint8_t hid_report_map[];
   extern const uint16_t hid_report_map_size;
   le_hogp_set_ReportMap((u8 *)hid_report_map, hid_report_map_size);
   le_hogp_set_icon(TCFG_MINI_HID_APPEARANCE);  // 0x03C1 = BLE_APPEARANCE_HID_KEYBOARD (961)
   ```
2. 编译、烧录，用 nRF Connect 扫描设备，查看 GATT 数据库：
   - 是否出现 0x1812 HID Service？
   - 其下是否有 Report Map / HID Information / Protocol Mode / Report / Boot KB Input / Boot KB Output 等 characteristic？
   - handle 是否在 RDX profile 之后？
   - 是否与现有 RDX Service 共存（同一 hdl 下）？

**判定标准**：
- ✅ **库函数可用**：0x1812 自动出现且 characteristic 齐全 → 手写 GATT DB 可省略，仅需 Report Map 数据 + 按 Fork A 判定结果补 glue code（ATT callback / CCC 管理 / ADV 按实际判定决定是否需要）
- ⚠️ **部分可用**：Service 出现但 characteristic 不全 → 库做基础框架，仍需手写部分 GATT DB
- ❌ **不可用**：无任何 0x1812 迹象 → 完全按本文档手写路径执行

### Step 1：安全参数兼容性验证

| 项目 | 内容 |
|------|------|
| 方法 | 在隔离分支将 `app_ble_sm_init` 改为 `SM_AUTHREQ_SECURE_CONNECTION \| SM_AUTHREQ_MITM_PROTECTION \| SM_AUTHREQ_BONDING`（L1），分别用 Windows 11 和现有 RDX APP 测试配对和回连 |
| 判定 | 若 RDX APP 兼容 → V0 启用 SC（L1）；若失败 → V0 保持 L0（`SM_AUTHREQ_MITM_PROTECTION \| SM_AUTHREQ_BONDING`），Windows bonding 回连仍正常 |

### Step 2：GATT handle 编码验证

| 项目 | 内容 |
|------|------|
| 方法 | 先写最小 HID Service 的 `.inc` 片段（仅 0x1812 + Report Map + HID Info），编译后在 nRF Connect 中读出全部 handle |
| 判定 | handle 与 `hid_gatt_db.h` 宏一一对应，无错位 |

### Step 3：Report Map 分段读取验证

| 项目 | 内容 |
|------|------|
| 方法 | nRF Connect 执行 Read Blob，分段读取 Report Map（先读 22 字节，再 offset=22 读剩余），拼接后与原始数组对比 |
| 判定 | 分段数据拼接后与 `hid_report_map[]` 原始数组一致 |

### Step 4：连接参数验证

| 项目 | 内容 |
|------|------|
| 方法 | HID 连接建立后用 BLE 嗅探器或 nRF Connect 查看当前连接参数（interval/latency/timeout） |
| 判定 | Interval 在 7.5~12.5ms 范围，Latency ≤ 1

---

## 15. V0 实施顺序

### Fork A：`le_hogp_*` 库函数可用

**Step 0 判定项（除 GATT 是否出现 HID Service 外，还需确认）：**

| 判定项 | 方法 | 影响 |
|--------|------|------|
| HID ATT callback 由谁接管？ | nRF Connect 写入 0x2A4C（HID Control Point），看设备 log 是否经过 rdx_ble_server write callback | 若库内部接管 → 不需要在 rdx_ble_server.c 插 HID 路由表；若 handle 仍进入 RDX callback → 需要路由表 |
| HID CCC 由谁管理？ | nRF Connect 写入 0x0022（Keyboard Input CCC）为 0x0001，发送 Win+H Report，看 notify 是否发出 | 若库内部管理 CCC → 本地 g_hid_*_ccc 状态和 att_set_ccc_config 可能不需要或冲突；若库不管 CCC → 需自行管理 |
| 库是否处理 Protocol Mode / HID Control Point？ | 查看 SDK 文档或在库符号中搜索 `hids_*` / `hogp_*` 相关函数 | 决定 §8.5 的 switch 逻辑是否还需要 |
| 广播数据是否自动包含 0x1812 UUID？ | 用 nRF Connect 查看 ADV 数据 | 若已包含 → 不需要修改 ADV 填充 |

**Fork A 实施顺序：**
1. 调用 `le_hogp_set_ReportMap()` + `le_hogp_set_icon()` 注册 HID Service
2. 根据 Step 0 判定结果，决定是否在 `rdx_ble_server.c` 插入路由表
3. 写 `hid_report_sender.c`：`hid_send_keyboard_report()` / `hid_send_consumer_report()` / `hid_send_voice_key()` / `hid_send_all_release()`（统一通过 `hid_send_keyboard_report()` 按 protocol mode 选 handle）
4. 若库不管 HID CCC → 在 write callback 中实现 CCC tracking + `att_set_ccc_config()`（同 Fork B）
5. 若库不管连接参数 → 实现 HID 低延迟连接参数请求
6. 改安全参数（按 §14 Step 1 结果选 L0 或 L1）
7. 若 ADV 不含 0x1812 → 手工添加
8. 关闭经典 HID（`TCFG_BT_SUPPORT_HID=0`）
9. HID 模式下跳过 RDX 20s 强制断开
10. 按 §11.2 逐项验证

### Fork B：手写 GATT DB

1. 写 `hid_report_map.c`（Report Map 字节数组）
2. 写 `hid_gatt_db.inc`（最小 HID Service：0x1812 + Protocol Mode + Report Map + HID Information + HID Control Point + Keyboard Input Report + Consumer Control Report + Boot KB Input/Output）
3. 写 `hid_gatt_db.h`（handle 宏 + CCCD/Report Ref 宏 + BEGIN/END + 路由表声明）
4. 在 `rdx_profile_data[]` 末尾 `#include "hid/hid_gatt_db.inc"`
5. 写 `hid_report_sender.c`（`hid_send_keyboard_report()` / `hid_send_consumer_report()` / `hid_send_voice_key()` / `hid_send_all_release()` — 全部通过统一接口按 protocol mode 选择 handle）
6. 写 `hid_keyboard.c`（ATT read/write callback 含 CCC tracking + `att_set_ccc_config()` + HID Control Point 处理 + 连接参数请求）
7. 在 `rdx_ble_server.c` 插入路由表
8. nRF Connect 验证 handle 对齐（§14 Step 2）
9. nRF Connect 验证 Read Blob（§14 Step 3）
10. 其余步骤同 Fork A 的 5-10

---

## 16. 参考资源

- [Bluetooth HID Profile Spec 1.1.1](https://www.bluetooth.com/specifications/specs/hid-profile-1-1-1/)
- [HID Usage Tables 1.5](https://www.usb.org/document-library/hid-usage-tables-15)
- [Device Class Definition for HID 1.11](https://www.usb.org/document-library/device-class-definition-hid-111)
- [BLE HOGP (HID over GATT Profile)](https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile-1-0/)
- 工程内现有 GATT 数据库格式参考：`rdx_ble_server.c` 中的 `rdx_profile_data[]` 数组
- 工程内 ATT 回调模式参考：`rdx_ble_server.c` 中的 `rdx_ble_server_att_read_callback()` / `rdx_ble_server_att_write_callback()`
- 独立 BLE Service 模板参考：`custom_protocol_demo/custom_protocol.c`（独立 profile_data[] + 独立 callback + 独立 init）
- 技术栈方案：`docs/mini-voice-keyboard-tech-stack.md`
- 配置宏管理规则：`AGENTS.md:127,224`
