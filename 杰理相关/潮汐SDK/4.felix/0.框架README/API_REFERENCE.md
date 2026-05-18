# TideX AICAP SDK — API 参考手册

> **SDK 版本**: 1.0.0 &nbsp; | &nbsp; **日期**: 2026-04

---

## 1. 快速开始

应用层只需一行即可访问 SDK 全部公共 API：

```c
#include "tdx_api.h"
```

`tdx_api.h` 是 facade 头文件，内部自动包含所有核心 API 和 HAL 入口。
也可按需单独引用 `tdx_api_xxx.h`。

---

## 2. 公共头文件一览

### 2.1 核心 API（通过 `tdx_api.h` 自动包含）

| 头文件 | 模块 | 说明 |
|--------|------|------|
| `tdx_api_types.h` | 类型定义 | 公共枚举、结构体、错误码 |
| `tdx_api_event.h` | 事件系统 | 事件注册、派发 |
| `tdx_api_protocol.h` | 协议引擎 | TideX 协议命令收发 |
| `tdx_api_transport.h` | 传输层 | 多通道传输管理 |
| `tdx_api_file.h` | 文件管理 | 录音/用户文件操作 |
| `tdx_api_record.h` | 录音控制 | 录音启停、状态查询 |
| `tdx_api_ota.h` | OTA 升级 | 固件升级全流程 |
| `tdx_api_auth.h` | 设备认证 | 绑定、授权校验 |
| `tdx_api_util.h` | 工具函数 | 字符串处理、RTC |
| `tdx_callbacks.h` | 回调接口 | SDK→App 事件通知（3 类回调表） |
| `tdx_ui_types.h` | UI 类型 | 显示事件枚举 |
| `tdx_platform.h` | HAL 入口 | facade，包含全部 `tdx_hal_*.h` |
| `tdx_common_def.h` | 基础类型 | 跨平台类型别名、工具宏 |

### 2.2 扩展 API（按需单独引用）

| 头文件 | 模块 | 启用宏 |
|--------|------|--------|
| `tdx_api_ble_client.h` | BLE Client | `TDX_HAS_BLE_CLIENT` |
| `tdx_api_ble_client_transfer.h` | BLE 命令传输 | `TDX_HAS_BLE_CLIENT` |
| `tdx_api_ble_client_file.h` | BLE 文件同步 | `TDX_HAS_BLE_CLIENT` |
| `tdx_api_cellular.h` | 4G 蜂窝模块 | `TDX_HAS_CELLULAR` |
| `tdx_api_cellular_http.h` | 4G HTTP 客户端 | `TDX_HAS_CELLULAR` |
| `tdx_api_cellular_upload.h` | 4G 上传队列 | `TDX_HAS_CELLULAR` |
| `tdx_version.h` | SDK 版本 | 始终可用 |
| `tdx_log.h` | 日志系统 | 始终可用 |
| `tdx_log_cfg.h` | 日志配置 | 始终可用 |

### 2.3 内部头文件（不对外）

| 头文件 | 说明 |
|--------|------|
| `tdx_auth_internal.h` | 认证内部实现 |
| `tdx_brand_select.h` | 品牌路由（被 `tdx_sdk_config.h` 包含） |
| `tdx_ble_server_stack_events.h` | BLE Server 栈事件（SDK 内部） |
| `tdx_ble_client_stack_events.h` | BLE Client 栈事件（SDK 内部） |
| `tdx_spi.h` | SPI 传输内部（SDK 内部） |

---

## 3. 核心模块详解

### 3.1 类型定义 — `tdx_api_types.h`

**连接状态** (`tdx_conn_state_t`)：

| 枚举值 | 说明 |
|--------|------|
| `TDX_CONN_STATE_DISCONNECTED` | 未连接 |
| `TDX_CONN_STATE_CONNECTED` | 已连接 |
| `TDX_CONN_STATE_CONNECTING` | 连接中 |

**设备基础信息** (`tdx_dev_base_info_t`)：

回调 `get_dev_base_info()` 填充的结构体，包含型号、固件版本、MAC 等。

**常用常量**：

| 宏 | 说明 |
|----|------|
| `TDX_CUSTOM_CMD_MAX_LENGTH` | 自定义命令最大长度 |
| `TDX_CUSTOM_VALUE_MAX_LENGTH` | 自定义命令值最大长度 |

### 3.2 协议引擎 — `tdx_api_protocol.h`

负责 TideX 私有协议（`*DEV#cmd#...` / `*APP#cmd#...`）的命令收发和事件通知。

**公开类型**：

| 类型 | 说明 |
|------|------|
| `tdx_proto_event_t` | 协议事件枚举 |
| `tdx_proto_record_info_t` | 录音状态信息 |
| `tdx_proto_battery_info_t` | 电池信息 |
| `tdx_proto_edr_info_t` | EDR 状态信息 |

**常用函数**：

```c
ReqFileInfo *tdx_protocol_get_uploadfileInfo(void);
```

### 3.3 传输抽象 — `tdx_api_transport.h`

统一管理多种传输通道（BLE / SPP / WiFi / SPI / Socket），支持多通道并发路由。

**公开类型**：

| 类型 | 说明 |
|------|------|
| `tdx_transport_type_t` | 传输类型（BLE_SERVER / SPP / SOCKET / SPI …） |
| `tdx_channel_role_t` | 通道角色（CTRL / DATA / OTA） |
| `tdx_ble_server_info_t` | BLE Server 连接信息 |

**核心函数**：

```c
// 绑定通道到传输类型
int  tdx_transport_bind_channel(tdx_channel_role_t role,
                                 tdx_transport_type_t type);

// 在指定通道上发送数据
int  tdx_transport_send_on(tdx_channel_role_t role,
                            const uint8_t *data, uint32_t len);

// 查询通道连接状态
bool tdx_transport_channel_is_connected(tdx_channel_role_t role);

// 获取 BLE Server 连接信息
tdx_ble_server_info_t *tdx_ble_server_get_info(void);
```

### 3.4 文件管理 — `tdx_api_file.h`

**核心类型**：

| 类型 | 说明 |
|------|------|
| `ReqFileInfo` / `tdx_file_req_info_t` | 文件传输请求信息 |

**核心函数**：

```c
void tdx_file_record_data_sendbuf_free(void);
void tdx_file_record_data_send_finish(tdx_file_req_info_t *rf_info);
```

### 3.5 录音控制 — `tdx_api_record.h`

控制录音启停、格式、采样率等。底层通过 `tdx_hal_audio_*` HAL 驱动。

### 3.6 OTA 升级 — `tdx_api_ota.h`

固件 OTA 升级全流程：初始化 → 数据写入 → CRC 校验 → 升级应用。

### 3.7 设备认证 — `tdx_api_auth.h`

设备绑定、授权密钥、工厂认证信息管理。

### 3.8 事件系统 — `tdx_api_event.h`

事件注册与派发，用于 SDK 与应用层之间的异步通信。

### 3.9 SDK 版本 — `tdx_version.h`

```c
#include "tdx_version.h"

const char *ver = tdx_sdk_get_version();      // "1.0.0"
uint32_t    hex = tdx_sdk_get_version_hex();   // 0x010000

// 编译期版本宏
#define TDX_SDK_VERSION_MAJOR   1
#define TDX_SDK_VERSION_MINOR   0
#define TDX_SDK_VERSION_PATCH   0
#define TDX_SDK_VERSION_HEX     0x010000
#define TDX_SDK_VERSION_STR     "1.0.0"

#if TDX_SDK_VERSION_HEX >= 0x010100
  // SDK 1.1.0+ 新特性
#endif
```

---

## 4. 扩展模块详解

### 4.1 BLE Client 系列

**`tdx_api_ble_client.h`**：

| 类型 | 说明 |
|------|------|
| `tdx_ble_channel_t` | BLE 通道类型 |
| `tdx_ble_client_state_t` | 连接状态 |
| `tdx_ble_cli_event_t` | 客户端事件 |
| `tdx_ble_client_config_t` | 客户端配置 |

**`tdx_api_ble_client_transfer.h`**：BLE 命令收发管理（心跳、握手、超时重传）。

**`tdx_api_ble_client_file.h`**：

| 类型 | 说明 |
|------|------|
| `tdx_ble_file_status_t` | 文件传输状态 |
| `tdx_ble_file_info_t` | 文件信息 |
| `tdx_ble_file_transfer_ctx_t` | 传输上下文 |

### 4.2 4G Cellular 系列

**`tdx_api_cellular.h`**：

| 类型 | 说明 |
|------|------|
| `tdx_at_io_ops_t` | AT 通信后端 vtable |
| `tdx_cellular_config_t` | 模块配置 |
| `tdx_cellular_state_t` | 模块状态 |
| `tdx_cellular_event_t` | 模块事件 |

**`tdx_api_cellular_http.h`**：

| 类型 | 说明 |
|------|------|
| `tdx_http_server_config_t` | 服务器配置 |
| `tdx_http_token_t` | Token 管理 |
| `tdx_http_response_t` | HTTP 响应 |

**`tdx_api_cellular_upload.h`**：

| 类型 | 说明 |
|------|------|
| `tdx_upload_task_t` | 上传任务 |
| `tdx_upload_config_t` | 上传配置 |
| `tdx_upload_status_t` | 上传状态 |

---

## 5. 回调接口 — `tdx_callbacks.h`

SDK 通过回调函数表与应用层交互，共 3 类：

### 5.1 应用回调 (`tdx_app_callbacks_t`)

```c
void tdx_register_app_callbacks(const tdx_app_callbacks_t *cbs);
const tdx_app_callbacks_t *tdx_get_app_callbacks(void);
```

回调表分为四大类。所有字段可选，未使用的设为 `NULL`。

**GETTERS — SDK 向应用层查询信息**：

| 字段 | 签名 | 说明 |
|------|------|------|
| `get_dev_base_info` | `int (*)(tdx_dev_base_info_t*)` | 获取设备基础信息 |
| `get_auth_key` | `int (*)(char*, uint32_t)` | 获取认证密钥 |
| `get_factory_sn` | `int (*)(char*, uint32_t)` | 获取工厂序列号 |
| `get_bound_status` | `int (*)(void)` | 查询绑定状态 |
| `get_bt_name` | `int (*)(char*, uint32_t)` | 获取经典蓝牙名 |
| `get_ble_name` | `int (*)(char*, uint32_t)` | 获取 BLE 广播名 |
| `get_auto_off_time` | `int (*)(void)` | 获取自动关机超时(秒) |
| `get_mic_gain` | `void (*)(uint8_t, uint8_t*, uint8_t*)` | 查询麦克风增益 |
| `get_customer_app_key` | `const char* (*)(void)` | 获取客户 AppKey |
| `get_admin_app_key` | `const char* (*)(void)` | 获取管理员 AppKey |
| `get_dut_status` | `int (*)(void)` | 是否处于产测模式 |
| `get_poweroff_flag` | `int (*)(void)` | 是否正在关机 |
| `is_unbounding` | `int (*)(void)` | 是否正在解绑 |
| `get_app_is_idle` | `int (*)(void)` | 应用层是否空闲 |
| `is_ios_system` | `int (*)(void)` | 是否连接 iOS 设备 |
| `get_wifi_info` | `int (*)(tdx_wifi_info_t*)` | WiFi 连接状态 |
| `get_record_mode` | `uint8_t (*)(void)` | 当前录音模式 |
| `get_power_ready_flag` | `int (*)(void)` | 电源子系统是否就绪 |
| `get_readchardata` | `const char* (*)(void)` | BLE 读特征值缓冲 |

**SETTERS — SDK 向应用层推送配置变更**：

| 字段 | 签名 | 说明 |
|------|------|------|
| `set_bound_status` | `void (*)(int)` | 更新绑定状态 |
| `set_bt_name` | `void (*)(const char*)` | 更新蓝牙名 |
| `set_ble_name` | `void (*)(const char*)` | 更新 BLE 名 |
| `set_auto_off_time` | `void (*)(int)` | 更新自动关机时间 |
| `set_mic_gain` | `void (*)(uint8_t, uint8_t, uint8_t)` | 更新麦克风增益 |
| `set_wifi_onoff` | `void (*)(uint8_t)` | 设置 WiFi 开关 |
| `set_wifi_conn_state` | `void (*)(uint8_t)` | 设置 WiFi 连接状态 |
| `set_power_ready_flag` | `void (*)(void)` | 标记电源就绪 |

**ACTIONS — SDK 请求应用层执行操作**：

| 字段 | 签名 | 说明 |
|------|------|------|
| `request_power_off` | `void (*)(void)` | 请求关机 |
| `request_factory_reset` | `void (*)(void)` | 请求恢复出厂设置 |
| `emmc_poweron` | `void (*)(uint8_t)` | eMMC 上电 |
| `emmc_poweroff` | `void (*)(void)` | eMMC 下电 |
| `emmc_poweroff_check` | `void (*)(void)` | 延迟下电检查 |
| `emmc_poweroff_check_timer_stop` | `void (*)(void)` | 停止下电定时器 |
| `clk_lock` | `void (*)(const char*, int)` | 锁定最低时钟频率 |
| `clk_unlock` | `void (*)(const char*)` | 释放时钟锁 |

**NOTIFICATIONS — SDK 通知应用层异步事件**：

| 字段 | 签名 | 说明 |
|------|------|------|
| `on_bound_result` | `void (*)(int)` | 绑定操作结果 |
| `on_unbound` | `void (*)(void)` | 已解绑 |
| `on_record_state_changed` | `void (*)(uint8_t, uint8_t, uint8_t)` | 录音状态变更 |
| `on_record_trigger` | `void (*)(uint8_t, uint8_t, uint8_t)` | 远程录音触发 |
| `on_conn_state_changed` | `void (*)(uint8_t)` | BLE/SPP 连接状态变更 |
| `on_rtc_synced` | `void (*)(uint32_t)` | RTC 已同步 |
| `on_wifi_ctrl` | `void (*)(uint8_t)` | WiFi 控制命令 |
| `on_ota_start` | `void (*)(const char*, uint32_t)` | OTA 开始 |
| `on_ota_progress` | `void (*)(uint32_t, uint32_t)` | OTA 进度 |
| `on_ota_complete` | `void (*)(int)` | OTA 完成 |
| `on_sd_format_result` | `void (*)(uint8_t)` | SD 格式化结果 |
| `on_file_delete_result` | `void (*)(int, int)` | 文件删除结果 |
| `on_custom_cmd` | `void (*)(const char*, const char*)` | 自定义命令 |
| `on_phone_os_type` | `void (*)(uint8_t)` | 手机系统类型 |

### 5.2 显示回调 (`tdx_ui_callbacks_t`)

可选，仅有显示屏的产品需要。

```c
void tdx_register_ui_callbacks(const tdx_ui_callbacks_t *cbs);
const tdx_ui_callbacks_t *tdx_get_ui_callbacks(void);
```

| 字段 | 签名 | 说明 |
|------|------|------|
| `show_recording` | `void (*)(uint8_t)` | 显示/隐藏录音指示 |
| `show_connected` | `void (*)(void)` | 显示"已连接" |
| `show_disconnected` | `void (*)(void)` | 显示"已断开" |
| `show_ota_progress` | `void (*)(uint8_t)` | OTA 进度百分比 |
| `show_battery` | `void (*)(uint8_t)` | 电池电量 |
| `show_qrcode` | `void (*)(const char*)` | 二维码 |
| `show_text` | `void (*)(const char*)` | 文本信息 |
| `clear` | `void (*)(void)` | 清屏 |
| `post_event` | `void (*)(tdx_ui_event_t)` | 触发 UI 状态页 |
| `post_event_ex` | `void (*)(tdx_ui_event_t, int, int)` | 触发带参数的 UI 事件 |
| `get_cur_event` | `tdx_ui_event_t (*)(void)` | 查询当前 UI 状态 |

### 5.3 环形缓冲区 (`tdx_cbuf_ops_t`)

大块数据传输时 SDK 使用的平台无关环形缓冲区。

```c
void tdx_register_cbuf_ops(const tdx_cbuf_ops_t *ops);
const tdx_cbuf_ops_t *tdx_get_cbuf_ops(void);
```

| 字段 | 签名 | 说明 |
|------|------|------|
| `create` | `void* (*)(uint8_t*, uint32_t)` | 创建缓冲区 |
| `destroy` | `void (*)(void*)` | 销毁缓冲区 |
| `write` | `int (*)(void*, const uint8_t*, uint32_t)` | 写入数据 |
| `read` | `int (*)(void*, uint8_t*, uint32_t)` | 读取数据 |
| `data_len` | `uint32_t (*)(void*)` | 已有数据长度 |
| `free_space` | `uint32_t (*)(void*)` | 剩余空间 |
| `clear` | `void (*)(void*)` | 清空缓冲区 |

---

## 6. HAL 接口

移植方需实现的硬件抽象层。入口：`#include "tdx_platform.h"` 或直接引用
`tdx_hal_<domain>.h`。

### 6.1 OS 抽象 — `tdx_hal_os.h`（必须实现）

```c
/* Mutex */
int      tdx_hal_os_mutex_create(tdx_hal_mutex_t *mutex);
int      tdx_hal_os_mutex_lock(tdx_hal_mutex_t mutex);
int      tdx_hal_os_mutex_unlock(tdx_hal_mutex_t mutex);
void     tdx_hal_os_mutex_destroy(tdx_hal_mutex_t mutex);

/* Semaphore */
int      tdx_hal_os_sem_create(tdx_hal_sem_t *sem, int init_val);
int      tdx_hal_os_sem_wait(tdx_hal_sem_t sem, uint32_t timeout_ms);
int      tdx_hal_os_sem_post(tdx_hal_sem_t sem);
void     tdx_hal_os_sem_destroy(tdx_hal_sem_t sem);

/* Task */
int      tdx_hal_os_task_create(const char *name, void (*fn)(void*),
                                 void *arg, uint32_t stack, uint32_t extra,
                                 uint32_t prio);

/* Timing */
void     tdx_hal_os_delay_ms(uint32_t ms);
uint32_t tdx_hal_os_get_tick_ms(void);
```

### 6.2 日志输出 — `tdx_hal_log.h`（必须实现）

```c
void tdx_hal_log_printf(const char *fmt, ...);
void tdx_hal_log_write(const char *buf, uint32_t len);
```

### 6.3 软件定时器 — `tdx_hal_timer.h`（必须实现）

```c
int  tdx_hal_timer_create(tdx_hal_timer_t *timer, void (*cb)(void*),
                           void *arg, uint32_t period_ms, uint8_t repeat);
int  tdx_hal_timer_start(tdx_hal_timer_t timer);
int  tdx_hal_timer_stop(tdx_hal_timer_t timer);
void tdx_hal_timer_destroy(tdx_hal_timer_t timer);
```

### 6.4 GPIO — `tdx_hal_gpio.h`

```c
void tdx_hal_gpio_init(tdx_hal_gpio_id_t id);
void tdx_hal_gpio_set_high(tdx_hal_gpio_id_t id);
void tdx_hal_gpio_set_low(tdx_hal_gpio_id_t id);
void tdx_hal_gpio_set_highz(tdx_hal_gpio_id_t id);
```

### 6.5 SPI / UART / I2C

```c
/* SPI */
int  tdx_hal_spi_init(uint8_t mode, uint32_t clock_hz);
int  tdx_hal_spi_write(const uint8_t *data, uint32_t len);
int  tdx_hal_spi_read(uint8_t *data, uint32_t len);
int  tdx_hal_spi_transfer(const uint8_t *tx, uint8_t *rx, uint32_t len);

/* UART */
int  tdx_hal_uart_init(uint32_t baudrate);
int  tdx_hal_uart_write(const uint8_t *data, uint32_t len);
int  tdx_hal_uart_read(uint8_t *data, uint32_t max_len, uint32_t timeout_ms);

/* I2C */
int  tdx_hal_i2c_init(void);
int  tdx_hal_i2c_write(uint8_t addr, const uint8_t *data, uint32_t len);
int  tdx_hal_i2c_read(uint8_t addr, uint8_t *data, uint32_t len);
```

### 6.6 BLE Server — `tdx_hal_ble_server.h`

```c
int  tdx_hal_ble_send(uint8_t *data, uint32_t len);
int  tdx_hal_ble_get_mtu(void);
void tdx_hal_ble_get_mac(uint8_t mac[6]);
int  tdx_hal_ble_gatt_server_init(void);
int  tdx_hal_ble_gatt_notify(uint16_t handle, const uint8_t *data, uint32_t len);
int  tdx_hal_ble_set_adv_data(const uint8_t *data, uint32_t len);
int  tdx_hal_ble_set_adv_enable(uint8_t enable);
int  tdx_hal_ble_disconnect_conn(uint16_t conn_handle);
```

### 6.7 文件系统 — `tdx_hal_fs.h`

```c
tdx_hal_file_t tdx_hal_fs_open(const char *path, const char *mode);
int      tdx_hal_fs_close(tdx_hal_file_t f);
int      tdx_hal_fs_read(tdx_hal_file_t f, void *buf, uint32_t len);
int      tdx_hal_fs_write(tdx_hal_file_t f, const void *buf, uint32_t len);
int      tdx_hal_fs_seek(tdx_hal_file_t f, int offset, int whence);
int      tdx_hal_fs_tell(tdx_hal_file_t f);
int      tdx_hal_fs_remove(const char *path);
int      tdx_hal_fs_rename(const char *old_path, const char *new_path);
uint32_t tdx_hal_fs_get_free_space(void);
uint32_t tdx_hal_fs_get_total_space(void);
```

### 6.8 其他 HAL 域

| 头文件 | 说明 |
|--------|------|
| `tdx_hal_bt.h` | 经典蓝牙（名称管理、可发现性控制） |
| `tdx_hal_spp.h` | SPP 串口（发送、连接状态） |
| `tdx_hal_socket.h` | TCP/UDP Socket（连接、收发） |
| `tdx_hal_ble_client.h` | BLE Client（扫描、连接、收发） |
| `tdx_hal_usb_host.h` | USB Host（设备枚举、CDC 通信） |
| `tdx_hal_flash.h` | Flash 读写（直接扇区操作） |
| `tdx_hal_vm.h` | 非易失参数（KV 存储: read/write by ID） |
| `tdx_hal_rtc.h` | RTC 时钟（设置/读取日期时间） |
| `tdx_hal_audio.h` | 音频录放（编解码器控制、MIC 数据获取） |
| `tdx_hal_power.h` | 电源管理（电量查询、充电状态、关机） |
| `tdx_hal_ota.h` | OTA 升级（Flash 写入、校验、应用） |
| `tdx_hal_sys.h` | 系统复位、堆信息查询 |
| `tdx_hal_wifi_bus.h` | WiFi 总线（SPI/UART 到 ESP32 模组通信） |
| `tdx_hal_wifi_netif.h` | WiFi 网络接口（IP 配置、连接管理） |

各接口的完整函数签名和参数说明见头文件内注释。

---

## 7. 日志系统 — `tdx_log.h`

### 7.1 日志级别

| 宏 | 值 | 含义 |
|----|----|------|
| `TDX_LOG_LVL_ASSERT` | 0 | 断言（最高优先级） |
| `TDX_LOG_LVL_ERROR` | 1 | 错误 |
| `TDX_LOG_LVL_WARN` | 2 | 警告 |
| `TDX_LOG_LVL_INFO` | 3 | 信息 |
| `TDX_LOG_LVL_DEBUG` | 4 | 调试 |
| `TDX_LOG_LVL_VERBOSE` | 5 | 详细（最低优先级） |

### 7.2 用户宏

```c
TDX_LOGA(fmt, ...)              // Assert 级别
TDX_LOGE(fmt, ...)              // Error
TDX_LOGW(fmt, ...)              // Warn
TDX_LOGI(fmt, ...)              // Info
TDX_LOGD(fmt, ...)              // Debug
TDX_LOGV(fmt, ...)              // Verbose

TDX_LOG_HEX(data, len)          // Hexdump (DEBUG 级别)
TDX_LOG_HEX_LVL(lvl, data, len) // Hexdump (指定级别)
TDX_LOG_FRAME(frame, len)       // 协议帧美化打印
TDX_LOG_RAW(fmt, ...)           // 无前缀原始输出

TDX_LOG_ASSERT(expr)            // 断言宏
```

### 7.3 ISR 安全日志

ISR 上下文中使用延迟日志，零格式化、零锁、零堆：

```c
TDX_LOG_ISR(evt_id, p0, p1)     // INFO 级别
TDX_LOG_ISR_E(evt_id, p0, p1)   // ERROR
TDX_LOG_ISR_W(evt_id, p0, p1)   // WARN
TDX_LOG_ISR_D(evt_id, p0, p1)   // DEBUG

tdx_log_isr_flush();             // 在任务上下文中格式化并输出
```

### 7.4 运行时 API

```c
/* 初始化与反初始化 */
int  tdx_log_init(void);
void tdx_log_deinit(void);

/* 全局级别控制 */
void    tdx_log_set_global_level(uint8_t level);
uint8_t tdx_log_get_global_level(void);

/* 按 tag 级别过滤 */
void    tdx_log_set_tag_level(const char *tag, uint8_t level);
uint8_t tdx_log_get_tag_level(const char *tag);

/* 输出通道控制 */
void    tdx_log_set_channels(uint8_t ch_mask);
uint8_t tdx_log_get_channels(void);
void    tdx_log_set_output_enabled(uint8_t enabled);

/* 后端管理 */
int  tdx_log_register_backend(const tdx_log_backend_t *be);
void tdx_log_unregister_backend(uint8_t channel);

/* 异步日志 */
int  tdx_log_async_init(void);
void tdx_log_async_deinit(void);

/* Flash 离线日志 */
int  tdx_log_flash_init(void);
void tdx_log_flash_deinit(void);
int  tdx_log_flash_read(uint8_t *buf, uint32_t buf_size, uint32_t *out_len);
int  tdx_log_flash_clear(void);

/* 刷新所有缓冲 */
void tdx_log_flush(void);
```

### 7.5 输出通道位掩码

| 宏 | 值 | 说明 |
|----|----|------|
| `TDX_LOG_CH_UART` | `1<<0` | 串口输出 |
| `TDX_LOG_CH_BLE` | `1<<1` | BLE 透传输出 |
| `TDX_LOG_CH_SPP` | `1<<2` | SPP 输出 |
| `TDX_LOG_CH_WIFI` | `1<<3` | WiFi 输出 |
| `TDX_LOG_CH_FLASH` | `1<<4` | Flash 离线存储 |

### 7.6 后端描述符

```c
typedef struct {
    uint8_t            channel;     // TDX_LOG_CH_xxx
    uint8_t            min_level;   // 此后端接受的最低级别
    tdx_log_write_fn   write;       // 已格式化字符串写入函数
    tdx_log_flush_fn   flush;       // 可选刷新函数（可为 NULL）
} tdx_log_backend_t;
```

---

## 8. 编译配置 — `tdx_sdk_config.h`

### 8.1 WiFi 配置

| 宏 | 说明 | 默认值 |
|----|------|--------|
| `TDX_WIFI_MODE` | WiFi 工作模式 | `TDX_WIFI_MODE_AT` (0) |
| `TDX_WIFI_MODE_AT` | ESP32 AT 固件模式 | 0 |
| `TDX_WIFI_MODE_LWIP` | ESP32 网卡模式 (lwIP) | 1 |
| `WIFI_CTRL_BUS_SELECT` | WiFi 总线选择 | `WIFI_CTRL_BUS_SPI` |
| `WIFI_COMMUNICATION_TYPE` | 传输协议 | `TRANSFER_BY_TCP` |
| `TDX_WIFI_AP_SSID` | WiFi AP 名称 | `"AINote"` |
| `TDX_WIFI_AP_PWD` | WiFi AP 密码 | `"88888888"` |

### 8.2 通信包大小

| 宏 | 说明 | 默认值 |
|----|------|--------|
| `BLE_SEND_SINGLE_PACK_SIZE` | BLE 单包大小 | `4080 * 6` |
| `BLE_DAT_SINGLE_PACK_SIZE` | BLE 数据帧大小 | `460` |
| `WIFI_SEND_SINGLE_PACK_SIZE` | WiFi 单包大小 | `8000 * 5` |
| `WIFI_DAT_SINGLE_PACK_SIZE` | WiFi 数据帧大小 | `7200` |
| `AUDIO_PACK_BUF_SIZE` | 音频包缓冲区 | `640` |

### 8.3 SPI 传输配置

| 宏 | 说明 | 默认值 |
|----|------|--------|
| `TDX_SPI_DMA_MAX_LEN` | DMA 最大传输长度 | `4092 * 2` |
| `TDX_SPI_TX_BUF_SIZE` | 发送缓冲区大小 | `8000 * 15` |
| `TDX_SPI_RETRY_MAX` | 最大重试次数 | `5` |
| `TDX_SPI_STUCK_TIMEOUT_MS` | 卡死超时(ms) | `5000` |

---

## 9. 错误码 — `tdx_hal_defs.h`

| 范围 | 模块 | 代表错误码 |
|------|------|-----------|
| `0` | 成功 (`TDX_OK`) | — |
| `-0x0001 ~ -0x00FF` | 通用错误 | — |
| `-0x0100 ~ -0x01FF` | 传输层 | `TDX_ERR_TRANSPORT_DISCONNECTED`, `_SEND_FAIL`, `_RECV_FAIL`, `_MTU_EXCEEDED` |
| `-0x0200 ~ -0x02FF` | 文件系统 | `TDX_ERR_FS_DISK_FULL`, `_NOT_FOUND`, `_IO_ERROR` |
| `-0x0300 ~ -0x03FF` | 认证 | `TDX_ERR_AUTH_INVALID_KEY`, `_NOT_BOUND` |
| `-0x0400 ~ -0x04FF` | 协议 | `TDX_ERR_PROTOCOL_PARSE`, `_CRC` |
| `-0x0500 ~ -0x05FF` | OTA | `TDX_ERR_OTA_VERIFY_FAIL`, `_SIZE_EXCEEDED` |
| `-0x0600 ~ -0x06FF` | Socket | `TDX_ERR_SOCKET_CONNECT_FAIL`, `_CLOSED`, `_DNS_FAIL` |
| `-0x0700 ~ -0x07FF` | BLE Client | `TDX_ERR_BLE_CLIENT_SCAN_FAIL`, `_CONN_FAIL` |

---

## 10. 工具宏 — `tdx_hal_defs.h`

```c
TDX_UNUSED(x)            // 消除未使用变量警告
TDX_ARRAY_SIZE(a)        // 数组元素个数
TDX_MIN(a, b)            // 取较小值
TDX_MAX(a, b)            // 取较大值
```
