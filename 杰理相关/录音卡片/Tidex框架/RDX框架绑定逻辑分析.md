# RDX 框架中的绑定逻辑分析

## 概述

RDX 协议框架（`SDK/apps/common/third_party_profile/rdx_protocol/`）中同时存在两套独立的绑定/配对相关的命令体系：

- **bound 链路**：设备与手机 App/云侧之间的账号绑定
- **devpair 链路**：充电仓与耳机/子设备之间的配对

两套体系在协议命令层完全独立，语义不同。其中 **bound** 链路的 NVM 持久化和解绑清理接口可以直接从当前源码确认；**devpair** 链路的协议定义、广播能力声明、BLE GATT 通道布局均可确认，但命令处理函数在预编译库中不可见。

---

## 一、bound — 设备与手机 App/云侧的绑定

### 1.1 协议命令

`rdx_protocol.h` 中定义了上下行各一对 bound/unbound 命令：

```c
// 上行：Device → App
#define CMD_UP_BOUND_SET    "*DEV#bound#"     // :56
#define CMD_UP_UNBOUND      "*DEV#unbound#"   // :75

// 下行：App → Device
#define CMD_DL_BOUND_SET    "*APP#bound#"     // :134
#define CMD_DL_UNBOUND      "*APP#unbound#"   // :153
```

### 1.2 NVM 持久化

`rdx_vm.h:44-46` 定义了绑定状态的存储结构：

```c
typedef struct {
    u8 bound_state;   // 0 = 未绑定, 1 = 已绑定
} rdx_bound_info_t;
```

`rdx_vm.c` 提供独立的读写接口：

- **读取**（`:157-170`）：`rdx_vm_get_bound_status()` — 通过 `syscfg_read(VM_RDX_NOTTA_BOUND_STATUS, ...)` 从 NVM 读取
- **写入**（`:179-197`）：`rdx_vm_set_bound_status(u8 d, u8 show_en)` — 通过 `syscfg_write(VM_RDX_NOTTA_BOUND_STATUS, ...)` 写入 NVM，并可选择更新 OLED 显示

存储项 `VM_RDX_NOTTA_BOUND_STATUS` 在 `syscfg_id.h:259` 定义为值 `149`。

状态常量（`rdx_vm.c:61-62`）：

```c
#define RDX_BOUND_STATE_UNBOUND  (0)
#define RDX_BOUND_STATE_BOUND    (1)
```

### 1.3 解绑机制

`rdx_vm.c:243-308` 的 `rdx_vm_unbound_cb()` 是解绑的核心回调。当解绑操作完成时执行以下清理：

1. `bt_tws_remove_pairs()` — 移除全部 TWS 配对
2. `rdx_app_reset_AI_mode_info()` — 重置 AI 模式信息（若启用）
3. `rdx_ble_server_app_disconnect()` — 断开 BLE 连接（若为主设备）
4. `USER_CTRL_DEL_ALL_REMOTE_INFO` — 删除全部远程 BT 设备信息
5. 重置 BT 名称为出厂默认值
6. 重置 BLE 名称为出厂默认值
7. 重置自动关机时间为默认值
8. 重置麦克风增益为默认值
9. `rdx_vm_set_bound_status(0, 0)` — 将绑定状态置为未绑定
10. `rdx_protocol_bound_result_indicate(0)` — 通知 App 解绑结果
11. `cpu_reset()` — 系统复位

解绑行为是**接近工厂复位级别**的，不仅清除绑定状态，还会重置一批关键用户配置和配对信息。当前可直接确认的包括 TWS 配对、远端 BT 设备信息、BT/BLE 名称、自动关机时间和麦克风增益等，但不能仅凭现有源码断言"所有"用户可配置参数都被覆盖。

### 1.4 BLE 广播中的绑定位

`rdx_ble_server.c:1341-1421` 的 `rdx_ble_server_fill_rsp_data()` 在构造 BLE 广播制造商数据时，将绑定状态编码到状态字节的 **bit 6**（`ADV_MODE_BIT_MASK_BOUND = 6`，`:91`）：

```c
u8 bd = rdx_vm_get_bound_status();   // :1361

if(bd == 0){
    manu_data[len++] &= bd << ADV_MODE_BIT_MASK_BOUND;  // :1393，清除 bit 6
}else{
    manu_data[len++] |= bd << ADV_MODE_BIT_MASK_BOUND;  // :1395，设置 bit 6
}
```

这样手机在扫描阶段即可判断设备是否已绑定，无需建立连接。

### 1.5 TWS 认证设置同步预留

`rdx_app.h:302-306` 定义了 TWS 同步结构：

```c
typedef struct {
    u8 ble_conn;
    rdx_ble_sys_settings_t rdx_sys_setting;  // 包含 bound_flag 等全套认证数据
    u8 neib_mac[6];                           // TWS 对端 MAC
} rdx_tws_sync_info_t;
```

`rdx_app.c` 中定义了 `TWS_FUNC_ID_RDX_AUTH_SYNC`（`'R','D','A','U'`）这一功能号，且 TWS 同步结构中包含 `rdx_sys_setting`。这说明框架**为 TWS 侧同步认证设置预留了数据结构和功能号**。但在当前可见源码中，尚未找到该功能号与 `rdx_tws_sync_info_t` 的完整发送、接收或落库链路，因此**不能直接断言**"一只耳机完成 bound 后，TWS 对端会自动继承绑定状态"。

### 1.6 云认证设置结构

`rdx_app.h:273-300` 的 `rdx_ble_sys_settings_t` 包含完整的云认证字段：

```c
typedef struct {
    u32  crc;
    u32  settings_version;
    u8   pid_len;
    u8   common_pid[RDX_BLE_PRODUCT_ID_MAX_LEN];  // 产品公共 PID
    u8   login_key[LOGIN_KEY_LEN];                //  6 字节 — 登录密钥
    u8   ecc_secret_key[ECC_SECRET_KEY_LEN];      // 32 字节 — ECC 密钥
    u8   device_virtual_id[DEVICE_VIRTUAL_ID_LEN]; // 22 字节 — 设备虚拟 ID
    u8   user_rand[PAIR_RANDOM_LEN];              //  6 字节 — 配对随机数
    u8   bound_flag;                               // 绑定标志位
    u8   factory_test_flag;
    u8   server_cert_pub_key[64];                  // 服务器证书公钥
    u8   beacon_key[BEACON_KEY_LEN];              // 16 字节
    u8   login_key_v2[LOGIN_KEY_V2_LEN];          // 16 字节
    u8   secret_key[SECRET_KEY_LEN];              // 16 字节
    u8   protocol_v2_enable;
    u8   res[14];
} rdx_ble_sys_settings_t;
```

**注意**：该结构中的 `bound_flag`（`:292`）和 VM 模块中 `rdx_bound_info_t.bound_state` 语义高度相关，都表示绑定状态，但两者是**不同的结构体成员**，当前可见源码中未找到两者之间的直接赋值映射代码。可以确认它们并存于框架内，但不能断言是同一存储位。

---

## 二、devpair — 充电仓与耳机/子设备的配对

### 2.1 协议命令

`rdx_protocol.h` 中定义了 devpair 及其关联命令：

```c
// 上行：Device → App（通过 NOTIFY 通道 0x0008 发送）
#define CMD_UP_DEVICE_PAIR        "*DEV#devpair#"    // :77
#define CMD_UP_DEVICE_UNPAIR      "*DEV#devunpair#"  // :78
#define CMD_UP_DEVICE_EPBT        "*DEV#epBT#"       // :80  耳机蓝牙状态
#define CMD_UP_CHILD_MSG          "*DEV#child#"      // :86  子设备消息
#define CMD_UP_CHILD_BLE_CONNECTED "*DEV#connstate#" // :88  子设备 BLE 连接状态
#define CMD_UP_TWS_INFO           "*DEV#twsinfo#"    // :92  子设备 TWS 信息

// 下行：App → Device（通过 WRITE 通道 0x0006 发送）
#define CMD_DL_DEVICE_PAIR        "*APP#devpair#"    // :155
#define CMD_DL_DEVICE_UNPAIR      "*APP#devunpair#"  // :156
#define CMD_DL_DEVICE_EPBT        "*APP#epBT#"       // :160
#define CMD_DL_CHILD_MSG          "*APP#child#"      // :162
```

### 2.2 BLE GATT 服务布局

`rdx_ble_server.c:140-219` 的 `rdx_profile_data[]` 定义了完整的 BLE GATT 服务表：

```
Service 1: 0x1800 (Generic Access)
  └─ 0x0003: Device Name (2A00) — READ | DYNAMIC

Service 2: 06068D0C-6B97-11EF-B864-0240AC120002 (主通信服务)
  ├─ 0x0006: 06068D1C-... — WRITE_WITHOUT_RESPONSE | DYNAMIC
  │          APP → 仓 的命令写入通道（承载所有 *APP#xxx# 协议命令）
  ├─ 0x0008: 06068D2C-... — NOTIFY | DYNAMIC
  │          仓 → APP 的通知通道（承载所有 *DEV#xxx# 协议上报）
  │    0x0009: CCC — APP 订阅后启用通知
  └─ 0x000b: 06068D3C-... — READ | DYNAMIC
             AuthKey 读取通道（24 字节设备认证密钥）

Service 3: 0x180F (Battery Service)
  └─ 0x000e: Battery Level (2A19) — READ | NOTIFY | DYNAMIC
       0x000f: CCC

Service 4: 00239A6F-C616-89BB-3374-F05AF588A7B3 (OTA 服务)
  ├─ 0x0012: 00239A7F-... — WRITE_WITHOUT_RESPONSE (OTA 数据通道)
  └─ 0x0014: 00239A8F-... — NOTIFY (OTA 响应通道)
       0x0015: CCC
```

### 2.3 Broadcast 中的控制位

#### 能力声明：ABILITY_CHILD_PARENT

`rdx_ble_server.h:51` 定义了父子拓扑的能力位：

```c
#define ABILITY_CHILD_PARENT  (1 << 13)  // 充电仓作为 Parent，耳机作为 Child
```

此位包含在广播 Manufacturer Data 的 4 字节 Ability 字段中（`rdx_ble_server.c:1400-1403`），APP 扫描时据此决定是否显示"子设备信息"区域。

当前编译配置 `RDX_DEVICE_ABILITY`（`:59-64`）中**原本未包含**此位，会导致 APP 不展示子设备相关 UI。需手动添加：

```c
#define RDX_DEVICE_ABILITY  (ABILITY_LOCAL_STORAGE | \
                             ABILITY_WIFI_AP | \
                             ABILITY_CONFERENCE_RECORDING | \
                             ABILITY_RTC | \
                             ABILITY_WIFI_AP_V2 | \
                             ABILITY_CHILD_PARENT)  // ← 需添加
```

#### 绑定状态：bound_state

广播状态字节 **bit 6**（`ADV_MODE_BIT_MASK_BOUND = 6`）携带 `bound_state`，与 bound 链路共享同一广播字段。

#### Manufacturer Data 完整布局

`rdx_ble_server.c:1374-1412` 构造的制造商数据（非 AI 模式）：

| 偏移 | 字段 | 长度 | 说明 |
|------|------|------|------|
| 0 | Hardware Code | 4 | 产品硬件码（`PRODUCT_CODE`） |
| 4 | BLE MAC | 6 | BLE 地址（反序） |
| 10 | Factory Code | 4 | 工厂标识（`FACTORY_CODE`） |
| 14 | Status Byte | 1 | bit 7=AI模式, bit 6=bound_state |
| 15 | Ability | 4 | 能力位（big-endian），含 CHILD_PARENT(bit13) |
| 19 | Protocol Version | 1 | 协议版本 |
| 20 | Product Type | 2 | 产品类型（如 "B0"） |
| 22 | Self Mark | 2 | "NV" 标识 |

### 2.4 Post-Connection 认证：AuthKey

BLE 连接建立后，APP 读取 characteristic **0x000b**（`ATT_CHARACTERISTIC_06068D3C`）获取设备 AuthKey。调用链：

```
APP 读 0x000b
  → rdx_ble_server.c:1114 read_callback
    → rdx_app_earphone_get_readchardata()      // :1132
      → 返回 ble_readchar_info
        → 内容来自 rdx_app_earphone_pack_readchardata()  // rdx_app.c:695
          → 复制 p_auth->AuthKey（24 字节）
```

AuthKey 的来源是 `rdx_vm_auth_info_init()`（`rdx_vm.c:530`），从 Flash 读取工厂烧录的产品认证信息，格式为：

```
AuthKey,MAC,FactorySN
```

示例：`MCMEMOAA2025031800004045,50EE407836FD,01A012530A000001`

其中 AuthKey 为 24 字符，MAC 为 12 字符（无冒号），FactorySN 为 16 字符，总长不超过 54 字节。

`rdx_auth_info_t` 结构（`rdx_vm.h:48-53`）：

```c
typedef struct {
    u8 AuthKey[RDX_BLE_DEVICE_AUTH_KEY_SIZE + 1];  // 25 字节（含 '\0'）
    u8 ble_mac_hex[6];
    char ble_mac_str[RDX_BLE_MAC_STRING_SIZE + 1];  // 13 字节
    u8 fac_sn[RDX_FACTORY_SN_SIZE + 1];             // 17 字节
} rdx_auth_info_t;
```

若 Flash 中未烧录 AuthKey（`AuthKey` 保持全零），APP 读到空值可能导致"绑定耳机"等功能不可用。

### 2.5 完整交互流程

devpair 绑定是一个**三方的协议中转架构**：

```
┌──────────┐         BLE GATT          ┌──────────────┐     BT/BLE     ┌──────────┐
│ 手机 APP  │ ◄──────────────────────► │ 充电仓 (Parent) │ ◄────────────► │ 耳机 (Child) │
│          │  0x0006: 写入命令         │              │                │          │
│          │  0x0008: 通知上报         │              │    epBT        │          │
│          │  0x000b: 认证读取         │              │    child       │          │
│          │                           │              │    twsinfo     │          │
└──────────┘                           └──────────────┘                └──────────┘
```

#### 阶段 1：扫描 → 能力发现

```
仓 ── BLE 广播/扫描响应 ──► APP
       Manufacturer Data:
         Ability bit 13 = CHILD_PARENT  ──► APP 显示"子设备信息"UI 区域
         Status bit 6 = bound_state     ──► APP 判断账号绑定状态
```

#### 阶段 2：连接 → 认证

```
BLE 连接建立
  ├─ APP 读 0x000b → AuthKey  ──► 设备身份认证（决定功能可用性）
  └─ APP 写 0x0009 (CCC=0x01) ──► 订阅 NOTIFY 通道 0x0008
```

#### 阶段 3：配对交互

```
APP 发送命令（写 0x0006）：
  *APP#devpair#   — 发起配对
  *APP#devunpair# — 解除配对  
  *APP#epBT#      — 查询/控制耳机蓝牙
  *APP#child#     — 子设备消息

仓上报状态（NOTIFY 0x0008）：
  *DEV#devpair#   — 配对结果
  *DEV#devunpair# — 解配结果
  *DEV#epBT#      — 耳机蓝牙状态
  *DEV#child#     — 子设备消息
  *DEV#connstate# — 子设备 BLE 连接状态变化
  *DEV#twsinfo#   — 子设备 TWS 信息
```

### 2.6 数据模型旁证

`rdx_app.h:202-206` 的电池信息结构区分了三个独立电量通道：

```c
typedef struct {
    u8 case_battery;   // 充电仓电量
    u8 left_battery;   // 左耳电量
    u8 right_battery;  // 右耳电量
} __battery_info;
```

这说明数据模型层面已经考虑了仓 + 左耳 + 右耳的三方设备关系。

### 2.7 产品类型区分

`rdx_app_config.h:60-73` 的 `DEVICE_` 宏为每个品牌同时定义了 EP（耳机，如 `DEVICE_FINDAI_EP_T2508 = 0x4000`）和 CC（充电仓，如 `DEVICE_FINDAI_CC_T2508 = 0x4010`）两种设备类型。当前项目选型为：

```c
#define RDX_SEL_DEVICE  DEVICE_FINDAI_CC_T2508  // 充电仓产品
```

### 2.8 仓-耳发现机制：经典蓝牙 Page Scan

充电仓与耳机之间的发现和连接走的是**经典蓝牙（Classic BT）**，而非 BLE。`rdx_app.c` 中找到了完整的接口定义：

#### 2.8.1 BT 扫描开启

`rdx_app.c:939-949` 的 `rdx_app_bt_open()` 在 BT 初始化时开启经典蓝牙的 Inquiry Scan + Page Scan：

```c
void rdx_app_bt_open(void)
{
    lmp_hci_write_scan_enable((1 << 1) | 1);
    // bit 0 = Inquiry Scan (可被发现)
    // bit 1 = Page Scan   (可被连接)
}
```

该函数在 `rdx_dut.c:966` 被调用，即在产测模式下也会触发 BT 开启。

#### 2.8.2 三个空桩函数

`rdx_app.c` 中定义了三个耳机状态管理接口，**当前均为空实现**（返回 0）：

**① `rdx_app_earphone_state_set_page_scan_enable()`**（`:629-638`）

```c
int rdx_app_earphone_state_set_page_scan_enable()
{
    return 0;  // ← 空桩！应在经典 BT 上启动 Page Scan 以发现耳机
}
```

**② `rdx_app_earphone_state_get_connect_mac_addr()`**（`:651-661`）

```c
int rdx_app_earphone_state_get_connect_mac_addr()
{
    return 0;  // ← 空桩！应返回已连接耳机的经典 BT MAC 地址
}
```

**③ `rdx_app_earphone_state_cancel_page_scan()`**（`:673-683`）

```c
int rdx_app_earphone_state_cancel_page_scan()
{
    return 0;  // ← 空桩！应停止 Page Scan
}
```

#### 2.8.3 空桩的调用时机

这些函数**不是死代码**，它们被 `rdx_app.c` 的消息分发函数实际调用（`:1672-1676`）：

```c
case APP_MSG_BT_OPEN_PAGE_SCAN:
    rdx_app_earphone_state_set_page_scan_enable();    // 触发：开启耳机发现
case APP_MSG_BT_CLOSE_PAGE_SCAN:
    rdx_app_earphone_state_cancel_page_scan();        // 触发：停止耳机发现
```

`APP_MSG_BT_OPEN_PAGE_SCAN` 消息由预编译库发出，即**库期望产品代码实现这三个函数的具体逻辑**。

#### 2.8.4 推断：仓-耳通信使用经典蓝牙

从函数命名和调用链推断：

```
充电仓 (Parent)                       耳机 (Child)
  │                                      │
  │  经典 BT: Inquiry Scan (广播)         │
  │  ◄───────────────────────────────────│ 耳机通过 BT 广播自己
  │                                      │
  │  经典 BT: Page Scan (连接)            │
  │  ◄──────────────────────────────────►│ 建立 ACL 链接
  │                                      │
  │  经典 BT: 获取 MAC / 交换数据         │
  │                                      │
  APP                                     │
  │  BLE 0x0008: *DEV#connstate#         │
  │  ◄────────────────────────────────── │ 上报连接状态
  │                                      │
  │  BLE 0x0008: *DEV#twsinfo#           │
  │  ◄────────────────────────────────── │ 上报 TWS 信息
```

`epBT` 命令名即 "earphone Bluetooth" 的缩写，进一步佐证仓-耳之间是经典 BT 通道。

### 2.9 实现层分层总结

| 层 | 位置 | 状态 | 可信度 |
|----|------|------|--------|
| 协议命令解析（`*APP#devpair#` → 分发） | 预编译库 `librdxApp.a` | **黑盒** | — |
| 协议命令上报（`*DEV#connstate#` 等） | 预编译库 `librdxApp.a` | **黑盒** | — |
| 经典 BT 扫描（`lmp_hci_write_scan_enable`） | `rdx_app.c:948` | **已实现** | 可确认 |
| 经典 BT Page Scan 启停 | `rdx_app.c:629,673` | **空桩，待实现** | 接口可确认 |
| 获取已连接耳机 MAC | `rdx_app.c:651` | **空桩，待实现** | 接口可确认 |
| GATT 通道（0x0006 / 0x0008 / 0x000b） | `rdx_ble_server.c` | **已实现** | 可确认 |
| 广播能力声明（CHILD_PARENT） | `rdx_ble_server.h` | **已配置** | 可确认 |
| AuthKey 认证数据 | `rdx_vm.c` | **已实现** | 可确认 |
| 电池三分区模型 | `rdx_app.h:202` | **已定义** | 可确认 |
| 产品 EP/CC 双类型 | `rdx_app_config.h` | **已配置** | 可确认 |

**核心结论**：devpair 命令的解析和上报在预编译库中（黑盒），但**仓-耳之间的经典蓝牙发现/连接/状态获取是公开的待实现接口**。三个空桩函数是产品代码需要填补的关键环节。当前"绑定耳机"按钮灰掉，很可能就是这三个空桩导致仓未实际发现耳机、未上报 `*DEV#connstate#`。预编译库负责任务调度和协议包装，但底层硬件操作留给产品代码实现。

---

## 三、TWS 双耳配对（第三方机制）

除上述两套 RDX 协议层的机制外，还存在第三套独立的配对机制——TWS（真无线立体声）左右耳配对，它不属于 RDX 协议本身，而是由 BT 协议栈管理：

- `bt_tws_remove_pairs()` — 清除 TWS 配对记录
- `tws_api_get_role()` — 查询 TWS 角色（Master/Slave）
- `APP_MSG_TWS_START_PAIR` — 启动 TWS 配对的事件消息
- `neighbour_mac[6]` — `rdx_app.c:145` 存储 TWS 对端 MAC

TWS 配对在 bound 解绑流程中被一并清除（`rdx_vm.c:262`）。

---

## 四、架构总结

```
┌──────────────────────────────────────────────────────────────────┐
│                        RDX 协议层                                  │
│                                                                  │
│  ┌──────────────────────────────┐  ┌────────────────────────────┐│
│  │  bound 链路                   │  │  devpair 链路               ││
│  │  设备 ↔ 手机App/云            │  │  充电仓 ↔ 耳机/子设备        ││
│  │                              │  │                            ││
│  │  广播: Status bit 6          │  │  广播: Ability bit 13      ││
│  │  NVM: 存储项 149             │  │  GATT: 0x0006(写)           ││
│  │  解绑: 11步工厂复位级别       │  │        0x0008(通知)         ││
│  │  确信度: ★★★ 全链路可确认     │  │        0x000b(AuthKey)      ││
│  │                              │  │  经典BT: Page Scan(空桩)    ││
│  │                              │  │  确信度: ★★☆ 部分可确认     ││
│  └──────────────────────────────┘  └────────────────────────────┘│
│                                                                  │
│  ┌──────────────────────────────────────────────────────────────┐│
│  │  BLE GATT 服务 (rdx_profile_data)                             ││
│  │  0x1800: Device Name                                         ││
│  │  06068D0C: 主通信 (CMD Write + NOTIFY + AuthKey Read)         ││
│  │  0x180F: Battery                                             ││
│  │  00239A6F: OTA                                               ││
│  └──────────────────────────────────────────────────────────────┘│
│                                                                  │
│  ┌──────────────────────────────────────────────────────────────┐│
│  │  公开接口 (待实现 / 空桩)                                      ││
│  │  rdx_app_earphone_state_set_page_scan_enable()   — 发现耳机   ││
│  │  rdx_app_earphone_state_cancel_page_scan()       — 停止发现   ││
│  │  rdx_app_earphone_state_get_connect_mac_addr()   — 获取MAC    ││
│  └──────────────────────────────────────────────────────────────┘│
├──────────────────────────────────────────────────────────────────┤
│                   底层 BT 协议栈                                    │
│  ┌──────────────────────────────────────────────────────────────┐│
│  │  TWS 双耳配对 (bt_tws_remove_pairs 等)                        ││
│  │  左右耳机之间，BT 协议栈原生管理                                ││
│  │                                                              ││
│  │  经典 BT: Inquiry Scan + Page Scan (lmp_hci_write_scan_enable)││
│  │  仓 ←→ 耳机 通信承载                                          ││
│  └──────────────────────────────────────────────────────────────┘│
└──────────────────────────────────────────────────────────────────┘
```

---

## 五、关键文件索引

| 文件 | 关键内容 |
|------|----------|
| `rdx_protocol/rdx_protocol.h:56,75,134,153` | bound/unbound 命令定义 |
| `rdx_protocol/rdx_protocol.h:77-78,155-156` | devpair/devunpair 命令定义 |
| `rdx_protocol/rdx_protocol.h:80,86,88,92,160,162` | epBT、child、connstate、twsinfo 命令定义 |
| `rdx_protocol/rdx_vm.h:44-46` | `rdx_bound_info_t` 结构定义 |
| `rdx_protocol/rdx_vm.h:48-53` | `rdx_auth_info_t` 认证信息结构（AuthKey/MAC/SN） |
| `rdx_protocol/rdx_vm.c:61-62` | 绑定状态常量 |
| `rdx_protocol/rdx_vm.c:133-149` | `rdx_vm_init()` — 初始化 bound_state=0 |
| `rdx_protocol/rdx_vm.c:157-170` | `rdx_vm_get_bound_status()` 实现 |
| `rdx_protocol/rdx_vm.c:179-197` | `rdx_vm_set_bound_status()` 实现 |
| `rdx_protocol/rdx_vm.c:249-308` | `rdx_vm_unbound_cb()` 解绑回调 |
| `rdx_protocol/rdx_vm.c:530-617` | `rdx_vm_auth_info_init()` — 从 Flash 读取 AuthKey/MAC/SN |
| `rdx_protocol/rdx_app.h:273-300` | `rdx_ble_sys_settings_t` 云认证结构 |
| `rdx_protocol/rdx_app.h:302-306` | `rdx_tws_sync_info_t` TWS 同步结构 |
| `rdx_protocol/rdx_app.h:202-206` | `__battery_info` 三分区电量结构 |
| `rdx_protocol/rdx_app.h:388,401` | `BLE_READCHAR_INFO_SIZE` 定义 |
| `rdx_protocol/rdx_app.c:629-638` | `rdx_app_earphone_state_set_page_scan_enable()` — **空桩**：启用经典 BT Page Scan 发现耳机 |
| `rdx_protocol/rdx_app.c:651-661` | `rdx_app_earphone_state_get_connect_mac_addr()` — **空桩**：获取已连接耳机的经典 BT MAC |
| `rdx_protocol/rdx_app.c:673-683` | `rdx_app_earphone_state_cancel_page_scan()` — **空桩**：停止 Page Scan |
| `rdx_protocol/rdx_app.c:939-949` | `rdx_app_bt_open()` — 开启经典 BT Inquiry Scan + Page Scan |
| `rdx_protocol/rdx_app.c:1672-1676` | 消息分发：`APP_MSG_BT_OPEN/CLOSE_PAGE_SCAN` → 调用空桩 |
| `rdx_protocol/rdx_app.c:695-721` | `rdx_app_earphone_pack_readchardata()` — 打包 AuthKey 到 characteristic |
| `rdx_protocol/rdx_app.c:733-743` | `rdx_app_earphone_get_readchardata()` — 返回 AuthKey 给 BLE 读取 |
| `rdx_protocol/rdx_ble_server.c:89-91` | `ADV_MODE_BIT_MASK_AI_MODE`、`ADV_MODE_BIT_MASK_BOUND` 宏 |
| `rdx_protocol/rdx_ble_server.c:140-219` | `rdx_profile_data[]` — BLE GATT 服务表 |
| `rdx_protocol/rdx_ble_server.c:1114-1139` | characteristic 0x000b 读回调（返回 AuthKey） |
| `rdx_protocol/rdx_ble_server.c:1259-1278` | characteristic 0x0006 写回调（接收 APP 命令） |
| `rdx_protocol/rdx_ble_server.c:1287-1297` | CCC 0x0009 写回调（APP 订阅 NOTIFY 通道） |
| `rdx_protocol/rdx_ble_server.c:1343-1421` | `rdx_ble_server_fill_rsp_data()` — BLE 广播数据构造 |
| `rdx_protocol/rdx_ble_server.c:1717-1753` | `rdx_ble_server_send()` — NOTIFY 通道 0x0008 发送 |
| `rdx_protocol/rdx_ble_server.h:39-63` | Ability 位定义与 `RDX_DEVICE_ABILITY` 宏 |
| `rdx_protocol/rdx_app_config.h:60-73,80` | EP/CC 设备类型与当前选型 |
| `interface/utils/syscfg_id.h:259` | `VM_RDX_NOTTA_BOUND_STATUS` 存储项编号 |
| `cpu/br28/tools/sdk.elf.resolution.txt:1035,1115-1127` | 预编译库中 `rdx_vm_bound_status_check`、`rdx_vm_get/set_bound_status`、`rdx_vm_unbound_cb` 的符号解析 |
