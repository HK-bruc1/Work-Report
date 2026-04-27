# TideX AICAP SDK — 移植指南

> **SDK 版本**: 1.0.0 &nbsp; | &nbsp; **日期**: 2026-04
> **适用平台参考**: JieLi JL7018 (BR28)

---

## 1. 概述

TideX AICAP SDK 是 TideX 产品线的公共协议与功能库，提供与芯片平台无关的
协议引擎、传输抽象、文件管理、OTA、加密、日志等能力。

SDK 以 **闭源静态库 + 公共头文件** 形式发布，各产品线通过实现 **HAL 接口** 并
编写 **应用胶水层（port/app/）** 完成与目标芯片平台的对接。

```
┌─────────────────────────────────────────────────────────────┐
│  Application（产品固件主应用）                                │
├─────────────────────────────────────────────────────────────┤
│  port/app/        应用胶水层（产品特定业务逻辑）              │
├─────────────────────────────────────────────────────────────┤
│  sdk/include/     公共 API 头文件                            │
│  sdk/include/hal/ HAL 接口定义（按功能域拆分, 26 个头文件）    │
│  sdk/lib/<chip>/  预编译 .a 静态库                           │
├─────────────────────────────────────────────────────────────┤
│  port/bsp/        板级外设组件（OLED, LED, 充电 IC, 振动）    │
├─────────────────────────────────────────────────────────────┤
│  port/platform/<chip>/  HAL 实现（寄存器级驱动）              │
└─────────────────────────────────────────────────────────────┘
```

层级职责对照（STM32Cube / ESP-IDF 惯例）：

| 层级 | 对应 | 职责 |
|------|------|------|
| `port/platform/` | HAL/LL 层 | 寄存器级驱动: SPI、I2C、USB、GPIO … |
| `port/bsp/` | BSP 层 | 板级外设: 显示屏、充电 IC、指示灯 … |
| `sdk/src/` (闭源) | 中间件 / 核心 | 协议、传输、加密、文件、OTA … |
| `port/app/` | 应用层 | 产品特定业务编排 |

---

## 2. 发布包结构

```
tidex_aicap/
├── sdk/
│   ├── include/                           ← 公共 API 头文件
│   │   ├── tdx_api.h                     ← 统一 API 入口 (facade)
│   │   ├── tdx_api_types.h               ← 公共枚举、结构体、错误码
│   │   ├── tdx_api_protocol.h            ← 协议引擎 API
│   │   ├── tdx_api_transport.h           ← 传输层 API
│   │   ├── tdx_api_file.h               ← 文件管理 API
│   │   ├── tdx_api_record.h             ← 录音控制 API
│   │   ├── tdx_api_ota.h               ← OTA 升级 API
│   │   ├── tdx_api_auth.h              ← 设备认证 API
│   │   ├── tdx_api_event.h             ← 事件系统 API
│   │   ├── tdx_api_util.h              ← 工具函数 API
│   │   ├── tdx_api_ble_client.h         ← BLE Client API（按需）
│   │   ├── tdx_api_ble_client_transfer.h ← BLE 命令传输（按需）
│   │   ├── tdx_api_ble_client_file.h    ← BLE 文件同步（按需）
│   │   ├── tdx_api_cellular.h           ← 4G 蜂窝模块 API（按需）
│   │   ├── tdx_api_cellular_http.h      ← 4G HTTP API（按需）
│   │   ├── tdx_api_cellular_upload.h    ← 4G 上传队列 API（按需）
│   │   ├── tdx_callbacks.h              ← SDK→App 回调接口
│   │   ├── tdx_ui_types.h              ← UI 事件类型
│   │   ├── tdx_platform.h              ← HAL 统一入口 (facade)
│   │   ├── tdx_version.h               ← SDK 版本号
│   │   ├── tdx_sdk_config.h            ← SDK 编译开关默认值
│   │   ├── tdx_common_def.h            ← 基础类型定义
│   │   ├── tdx_brand_select.h          ← 品牌配置选择器
│   │   ├── tdx_log.h                   ← 日志公共 API
│   │   ├── tdx_log_cfg.h              ← 日志编译期配置
│   │   └── hal/                        ← HAL 接口定义 (26 个文件)
│   │       ├── tdx_hal.h              ← 统一 HAL 入口
│   │       ├── tdx_hal_defs.h         ← 错误码 & 公共类型
│   │       ├── tdx_hal_config.h       ← 平台能力声明（默认模板）
│   │       ├── tdx_hal_os.h           ← OS 抽象（mutex/sem/task/delay）
│   │       ├── tdx_hal_timer.h        ← 软件定时器
│   │       ├── tdx_hal_log.h          ← 日志输出
│   │       ├── tdx_hal_gpio.h         ← GPIO
│   │       ├── tdx_hal_spi.h          ← SPI 总线
│   │       ├── tdx_hal_uart.h         ← UART
│   │       ├── tdx_hal_i2c.h          ← I2C 总线
│   │       ├── tdx_hal_flash.h        ← Flash 读写
│   │       ├── tdx_hal_fs.h           ← 文件系统
│   │       ├── tdx_hal_ble_server.h   ← BLE GATT Server
│   │       ├── tdx_hal_ble_client.h   ← BLE Client
│   │       ├── tdx_hal_bt.h           ← 经典蓝牙
│   │       ├── tdx_hal_spp.h          ← SPP 串口
│   │       ├── tdx_hal_socket.h       ← TCP/UDP Socket
│   │       ├── tdx_hal_usb_host.h     ← USB Host
│   │       ├── tdx_hal_audio.h        ← 录音/MIC
│   │       ├── tdx_hal_power.h        ← 电源管理
│   │       ├── tdx_hal_vm.h           ← 非易失参数
│   │       ├── tdx_hal_rtc.h          ← RTC 时钟
│   │       ├── tdx_hal_ota.h          ← OTA
│   │       ├── tdx_hal_sys.h          ← 系统复位/堆
│   │       ├── tdx_hal_wifi_bus.h     ← WiFi 总线抽象
│   │       └── tdx_hal_wifi_netif.h   ← WiFi 网络接口
│   ├── lib/
│   │   └── <chip>/                    ← 预编译静态库（按芯片分目录）
│   │       └── libtdx_sdk.a
│   └── docs/                          ← 本文档
│
└── port/                              ← 参考实现 & 移植模板
    ├── platform/
    │   ├── tdx_platform_jl7018.c      ← HAL 聚合文件（#include 各实现源文件）
    │   └── jl7018/                    ← JL7018 HAL 参考实现
    │       ├── tdx_hal_os_jl7018.c
    │       ├── tdx_hal_ble_server_jl7018.c
    │       ├── tdx_hal_bt_jl7018.c
    │       ├── tdx_hal_audio_jl7018.c
    │       ├── tdx_hal_timer_jl7018.c
    │       ├── tdx_hal_sys_jl7018.c
    │       ├── tdx_hal_fs_jl7018.c
    │       ├── tdx_hal_i2c_jl7018.c
    │       ├── tdx_hal_bus_jl7018.c
    │       ├── tdx_hal_spi_jl7018.c
    │       ├── tdx_hal_usb_host_jl7018.c
    │       ├── tdx_hal_wifi_bus_jl7018.c
    │       └── tdx_hal_wifi_netif_jl7018.c
    ├── bsp/                           ← 板级外设组件
    │   ├── display/ssd1306/           ← OLED 显示 (SSD1306)
    │   ├── indicator/pwm_led/         ← LED 指示灯
    │   ├── vibrate/                   ← 振动马达
    │   └── charge/sk4558/             ← 充电 IC (SK4558)
    ├── boards/                        ← 板级硬件配置
    │   ├── rc_v1_jl7018/board.h      ← Record Card V1
    │   ├── ep_v1_jl7018/board.h      ← Earphone V1
    │   └── wt_v1_jl7018/board.h      ← WT V1
    ├── products/                      ← 产品特性定义
    │   ├── product_select.h           ← 产品路由选择器
    │   ├── rc/product_def.h           ← Record Card
    │   ├── rc_v2/product_def.h        ← Record Card V2
    │   ├── rp/product_def.h           ← Reporter Pen
    │   └── wt/product_def.h           ← WT
    ├── brands/                        ← 客户品牌配置
    │   ├── tdx_config_select.h        ← 品牌路由选择器
    │   ├── tdx_config_notta.h
    │   ├── tdx_config_turing.h
    │   └── ...                        ← (20+ 品牌配置文件)
    └── app/                           ← 应用胶水层参考
        ├── tdx_app.c / .h            ← 主初始化和消息循环
        ├── tdx_app_record.c           ← 录音业务逻辑
        ├── tdx_app_power.c            ← 电源管理
        ├── tdx_app_callbacks_impl.c   ← SDK 回调实现
        ├── tdx_app_custom_ops.c       ← 自定义协议命令
        ├── tdx_key.c / .h            ← 按键处理
        ├── tdx_battery.c / .h        ← 电池管理
        ├── tdx_vm.c / .h             ← VM 持久存储
        ├── tdx_dut.c                  ← 产测模式
        ├── tdx_app_config.h           ← 应用配置
        ├── tdx_app_ble_client*.c/.h   ← BLE Client 业务（按需）
        └── tdx_app_cellular_upload.c  ← 4G 上传业务（按需）
```

---

## 3. 层间依赖规则

| 规则 | 说明 |
|------|------|
| `sdk/` 不能引用 `port/` | SDK 通过 HAL 回调与平台交互 |
| `sdk/` 不能引用芯片 SDK 头文件 | 全部通过 `tdx_hal_*.h` 抽象 |
| `port/` 可以引用 `sdk/include/` | 应用/HAL 层使用公共 API |
| `sdk/src/` 内部头文件不对外暴露 | 不要 include `sdk/src/` 下的任何文件 |
| `port/bsp/` 应使用 `tdx_hal_*` API | 不应直接调用芯片 SDK |

---

## 4. 移植步骤

### 4.1 准备工作

1. 将 `tidex_aicap/` 整个目录放入项目的第三方库位置
2. 参考 `port/platform/jl7018/` 已有实现作为起点

### 4.2 创建平台能力配置

新建 `port/platform/<chip>/tdx_hal_config.h`，声明芯片支持的能力：

```c
// port/platform/<your_chip>/tdx_hal_config.h
#ifndef __TDX_HAL_CONFIG_H__
#define __TDX_HAL_CONFIG_H__

/* 传输能力 */
#define TDX_HAL_HAVE_BLE_SERVER     1
#define TDX_HAL_HAVE_BLE_CLIENT     0
#define TDX_HAL_HAVE_BT_SPP        1
#define TDX_HAL_HAVE_WIFI_AT       1
#define TDX_HAL_HAVE_SOCKET        0
#define TDX_HAL_HAVE_CELLULAR      0
#define TDX_HAL_HAVE_USB_DEVICE    0
#define TDX_HAL_HAVE_USB_HOST      0

/* 总线 */
#define TDX_HAL_HAVE_SPI           1
#define TDX_HAL_HAVE_UART          1
#define TDX_HAL_HAVE_I2C           1

/* OS */
#define TDX_HAL_HAVE_MUTEX         1
#define TDX_HAL_HAVE_SEM           1
#define TDX_HAL_HAVE_TASK          1
#define TDX_HAL_HAVE_TIMER         1

/* 存储 */
#define TDX_HAL_HAVE_FS            1
#define TDX_HAL_HAVE_FLASH         1
#define TDX_HAL_HAVE_VM            1

/* 外设 */
#define TDX_HAL_HAVE_GPIO          1
#define TDX_HAL_HAVE_RTC           1
#define TDX_HAL_HAVE_AUDIO         1
#define TDX_HAL_HAVE_OTA           1

/* 平台限制 */
#define TDX_HAL_BLE_MTU_MAX        244
#define TDX_HAL_TASK_STACK_MIN     256
#define TDX_HAL_FS_PATH_MAX        64

#endif
```

**`TDX_HAL_HAVE_*` 与 `TDX_HAS_*` 区别**：

| 宏前缀 | 含义 | 定义位置 | 举例 |
|--------|------|---------|------|
| `TDX_HAL_HAVE_*` | 芯片**能不能**提供此接口 | `port/platform/<chip>/tdx_hal_config.h` | `TDX_HAL_HAVE_BLE_SERVER 1` |
| `TDX_HAS_*` | 产品**需不需要**此功能 | `port/products/*/product_def.h` 或 `port/boards/*/board.h` | `TDX_HAS_WIFI 1` |

### 4.3 实现 HAL 接口

HAL 接口定义在 `sdk/include/hal/` 目录下。只需实现 `tdx_hal_config.h` 中
声明为 `1` 的能力对应函数。

| HAL 头文件 | 说明 | 必选 |
|-----------|------|------|
| `tdx_hal_os.h` | mutex / sem / task / delay / tick | **是** |
| `tdx_hal_timer.h` | 软件定时器 | **是** |
| `tdx_hal_log.h` | 日志输出（printf / write） | **是** |
| `tdx_hal_gpio.h` | GPIO 高低电平控制 | 按需 |
| `tdx_hal_spi.h` | SPI 总线收发 | 按需 |
| `tdx_hal_uart.h` | UART 串口收发 | 按需 |
| `tdx_hal_i2c.h` | I2C 总线收发 | 按需 |
| `tdx_hal_flash.h` | Flash 擦写 | 按需 |
| `tdx_hal_fs.h` | 文件系统（open/read/write/seek） | 按需 |
| `tdx_hal_ble_server.h` | BLE GATT Server | 按需 |
| `tdx_hal_ble_client.h` | BLE Client | 按需 |
| `tdx_hal_bt.h` | 经典蓝牙 | 按需 |
| `tdx_hal_spp.h` | SPP 串口 | 按需 |
| `tdx_hal_socket.h` | TCP/UDP Socket | 按需 |
| `tdx_hal_usb_host.h` | USB Host | 按需 |
| `tdx_hal_audio.h` | 录音/MIC | 按需 |
| `tdx_hal_power.h` | 电源管理 | 按需 |
| `tdx_hal_vm.h` | 非易失参数存储 | 按需 |
| `tdx_hal_rtc.h` | RTC 时钟 | 按需 |
| `tdx_hal_ota.h` | OTA 升级 | 按需 |
| `tdx_hal_sys.h` | 系统复位 / 堆信息 | 按需 |
| `tdx_hal_wifi_bus.h` | WiFi 总线（SPI/UART 到 ESP 模组） | 按需 |
| `tdx_hal_wifi_netif.h` | WiFi 网络接口 | 按需 |

推荐使用 **聚合文件** 模式——用一个 `.c` 文件 `#include` 所有 HAL 实现源文件：

```c
// port/platform/tdx_platform_<chip>.c
#include "<chip>/tdx_hal_os_<chip>.c"
#include "<chip>/tdx_hal_gpio_<chip>.c"
#include "<chip>/tdx_hal_timer_<chip>.c"
#include "<chip>/tdx_hal_sys_<chip>.c"
#include "<chip>/tdx_hal_fs_<chip>.c"
// ...
#if TDX_HAS_WIFI
#include "<chip>/tdx_hal_wifi_bus_<chip>.c"
#include "<chip>/tdx_hal_wifi_netif_<chip>.c"
#endif
```

### 4.4 创建板级配置

新建 `port/boards/<board_name>/board.h`，定义引脚分配和产品特性覆盖：

```c
// port/boards/<board_name>/board.h
#ifndef __TDX_BOARD_H__
#define __TDX_BOARD_H__

/* 选择产品线 */
#define TDX_PRODUCT_RC

/* 在 product_def.h 默认值之前覆盖特性开关 */
// #define TDX_HAS_WIFI        0     /* 如果此板型不带 WiFi */
// #define TDX_HAS_DISPLAY     0     /* 如果此板型不带显示屏 */

/* 拉入产品默认特性 + 品牌配置 */
#include "product_select.h"
#include "tdx_config_select.h"

/* 驱动选型 */
#define TDX_DISPLAY_DRIVER  TDX_DRV_SSD1306
#define TDX_WIFI_DRIVER     TDX_DRV_ESP8684

/* 引脚分配 */
#define TDX_I2C_SCL_IO      IO_PORTB_04
#define TDX_I2C_SDA_IO      IO_PORTB_05
#define TDX_OLED_POWER_IO   IO_PORTA_00
#define TDX_SPI_CS_IO       IO_PORT_DM
// ...

#endif
```

Makefile 中使用 `-include` 注入板级配置：

```makefile
CFLAGS += -include $(TIDEX_DIR)/port/boards/<board_name>/board.h
```

### 4.5 创建产品定义（可选）

如果现有产品线（RC / EP / CC / RP / WT / RC_V2）不满足需求，可在
`port/products/<new>/product_def.h` 中定义新的产品默认特性。

所有 `TDX_HAS_*` 宏使用 `#ifndef` 保护，允许 `board.h` 在包含前覆盖。

当前支持的产品线：

| 产品宏 | 产品定义文件 | 说明 |
|--------|-------------|------|
| `TDX_PRODUCT_RC` | `rc/product_def.h` | Record Card |
| `TDX_PRODUCT_RC_V2` | `rc_v2/product_def.h` | Record Card V2 |
| `TDX_PRODUCT_EP` | `ep/product_def.h` | Earphone |
| `TDX_PRODUCT_CC` | `cc/product_def.h` | Charge Case |
| `TDX_PRODUCT_RP` | `rp/product_def.h` | Reporter Pen |
| `TDX_PRODUCT_WT` | `wt/product_def.h` | WT |

### 4.6 创建品牌配置

在 `port/brands/tdx_config_<brand>.h` 中定义品牌参数（设备名、厂商码、SKU 等）。
通过 `port/brands/tdx_config_select.h` 根据编译宏路由到对应配置。

品牌配置使用 `#undef` + `#define` 模式覆盖 `tdx_sdk_config.h` 中的默认值：

```c
// port/brands/tdx_config_mybrand.h
#undef  TDX_CUSTOMER_NAME
#define TDX_CUSTOMER_NAME   "MyBrand"

#undef  TDX_VENDOR_CODE
#define TDX_VENDOR_CODE     "MB"

#undef  BLE_LOCAL_NAME
#define BLE_LOCAL_NAME      "MyBrand-AiNote"
// ...
```

### 4.7 实现回调并注册

SDK 通过回调函数表（`tdx_callbacks.h`）与应用层交互，共 3 类回调表：

| 回调表 | 注册函数 | 必选 |
|--------|---------|------|
| `tdx_app_callbacks_t` | `tdx_register_app_callbacks()` | **是** |
| `tdx_ui_callbacks_t` | `tdx_register_ui_callbacks()` | 有显示屏时 |
| `tdx_cbuf_ops_t` | `tdx_register_cbuf_ops()` | 大块传输时 |

示例：

```c
// port/app/tdx_app_callbacks_impl.c
#include "tdx_callbacks.h"

static int my_get_bound_status(void) { return is_bound; }
static void my_on_conn_changed(uint8_t state) { /* ... */ }

static const tdx_app_callbacks_t my_cbs = {
    .get_bound_status       = my_get_bound_status,
    .on_conn_state_changed  = my_on_conn_changed,
    // ... 所有字段可选，未用的设为 NULL
};

void my_callbacks_init(void) {
    tdx_register_app_callbacks(&my_cbs);
}
```

### 4.8 编写应用胶水层

在 `port/app/` 中编写产品业务逻辑，参考现有文件：

| 文件 | 功能 |
|------|------|
| `tdx_app.c` | 主初始化和消息循环 |
| `tdx_app_record.c` | 录音业务逻辑 |
| `tdx_app_power.c` | 电源管理 |
| `tdx_app_callbacks_impl.c` | SDK 回调实现 |
| `tdx_app_custom_ops.c` | 自定义协议命令 |
| `tdx_key.c` | 按键处理 |
| `tdx_battery.c` | 电池管理 |
| `tdx_vm.c` | VM 参数管理 |
| `tdx_dut.c` | 产测模式 |
| `tdx_app_ble_client_*.c` | BLE Client 业务（按需） |
| `tdx_app_cellular_upload.c` | 4G 上传业务（按需） |

---

## 5. 构建集成

### 5.1 Include 路径

```makefile
TIDEX_DIR = apps/common/third_party_profile/tidex_aicap

# SDK 公共头文件（必须）
INCLUDES += -I$(TIDEX_DIR)/sdk/include
INCLUDES += -I$(TIDEX_DIR)/sdk/include/hal

# 平台 HAL 配置（必须，-I 优先于默认模板）
INCLUDES += -I$(TIDEX_DIR)/port/platform/<chip>

# Port 层头文件
INCLUDES += -I$(TIDEX_DIR)/port/app
INCLUDES += -I$(TIDEX_DIR)/port/platform
INCLUDES += -I$(TIDEX_DIR)/port/brands
INCLUDES += -I$(TIDEX_DIR)/port/products
INCLUDES += -I$(TIDEX_DIR)/port/products/$(TDX_PRODUCT_DIR)

# BSP 组件（按需）
INCLUDES += -I$(TIDEX_DIR)/port/bsp/display/ssd1306
INCLUDES += -I$(TIDEX_DIR)/port/bsp/vibrate
INCLUDES += -I$(TIDEX_DIR)/port/bsp/charge/sk4558
INCLUDES += -I$(TIDEX_DIR)/port/bsp/indicator/pwm_led
```

### 5.2 板级配置注入

```makefile
# 将 board.h 全局注入所有编译单元
CFLAGS += -include $(TIDEX_DIR)/port/boards/<board>/board.h
```

### 5.3 链接库

```makefile
LIBS += $(TIDEX_DIR)/sdk/lib/<chip>/libtdx_sdk.a
```

### 5.4 源文件

将以下 `port/` 源文件加入编译：

- `port/platform/tdx_platform_<chip>.c`（HAL 聚合文件——编译此文件即可）
- `port/app/*.c`（应用胶水层）
- `port/bsp/**/*.c`（板级外设组件，按需）

---

## 6. 配置层次

SDK 采用分层配置，优先级从高到低：

```
port/boards/<board>/board.h            ← 板级覆盖（最高优先级）
port/brands/tdx_config_<brand>.h       ← 品牌参数（#undef 后重定义）
port/products/<prod>/product_def.h     ← 产品默认值（#ifndef 保护）
sdk/include/tdx_sdk_config.h           ← SDK 安全默认值（最低优先级）
```

配置流程：
1. `board.h` 通过 `-include` 全局注入，定义 `TDX_PRODUCT_RC` 等产品宏
2. `board.h` 可在此时覆盖任何 `TDX_HAS_*` 宏
3. `board.h` 包含 `product_select.h` → 根据 `TDX_PRODUCT_*` 路由到正确的 `product_def.h`
4. `product_def.h` 中的 `#ifndef` 保护使得 `board.h` 的覆盖生效
5. `board.h` 包含 `tdx_config_select.h` → 注入品牌参数
6. 最后 `tdx_sdk_config.h` 提供所有未定义宏的安全默认值

---

## 7. 条件编译宏汇总

| 宏前缀 | 含义 | 定义位置 |
|--------|------|---------|
| `TDX_SDK_ENABLED` | SDK 主开关 | `tdx_sdk_config.h` |
| `TDX_PRODUCT_*` | 产品线选择 | `board.h`（`RC` / `EP` / `CC` / `RP` / `WT` / `RC_V2`） |
| `TDX_HAS_*` | 产品特性开关 | `board.h` 或 `product_def.h` |
| `TDX_HAL_HAVE_*` | 平台能力开关 | `tdx_hal_config.h` |
| `TDX_DRV_*` | 驱动选型常量 | `board.h` |
| `TDX_PIN_*` / `TDX_*_IO` | 引脚分配 | `board.h` |
| `TDX_LOG_MOD_LVL_*` | 模块日志级别 | `tdx_log_cfg.h` |

当前支持的 `TDX_HAS_*` 特性开关：

| 宏 | 说明 | 默认值 |
|----|------|--------|
| `TDX_HAS_DISPLAY` | OLED 显示屏 | 0 |
| `TDX_HAS_VIBRATE` | 振动马达 | 0 |
| `TDX_HAS_CHARGE_IC` | 充电 IC | 0 |
| `TDX_HAS_LED` | LED 指示灯 | 0 |
| `TDX_HAS_RGB_LED` | RGB LED | 0 |
| `TDX_HAS_WIFI` | WiFi 模组 | 0 |
| `TDX_HAS_SPI` | SPI 通信 | 0 |
| `TDX_HAS_SPP` | BT SPP | 0 |
| `TDX_HAS_FILE_STORAGE` | 文件存储 | 0 |
| `TDX_HAS_OFFLINE_RECORD` | 离线录音 | 0 |
| `TDX_HAS_ALGORITHM` | 音频算法 | 0 |
| `TDX_HAS_MIC` | 麦克风 | 0 |
| `TDX_HAS_QR_CODE` | 二维码显示 | 0 |
| `TDX_HAS_TWS` | TWS 双耳 | 0 |
| `TDX_HAS_CHARGE_CASE` | 充电盒 | 0 |
| `TDX_HAS_TRANSLATE` | AI 翻译 | 0 |
| `TDX_HAS_DUT_MODE` | 产测模式 | 0 |
| `TDX_HAS_LOG_MODULE` | 日志模块 | 0 |
| `TDX_HAS_EVENT_MANAGER` | 事件管理器 | 0 |
| `TDX_HAS_DEVICE_PAIRING` | 设备配对 | 0 |

---

## 8. 日志系统

### 8.1 基本使用

```c
#define TDX_LOG_TAG "TDX.MY_MOD"
#include "tdx_log.h"

TDX_LOGE("error: %d", err_code);
TDX_LOGW("warning message");
TDX_LOGI("info: connected, MTU=%d", mtu);
TDX_LOGD("debug data: %02x", byte);
TDX_LOGV("verbose trace");
TDX_LOG_HEX(payload, len);
TDX_LOG_FRAME(frame, frame_len);
```

底层通过 `tdx_hal_log_write()` 输出，由 HAL 实现决定输出到串口、RTT 或其他。

### 8.2 全局配置（tdx_log_cfg.h）

| 宏 | 说明 | 默认值 |
|----|------|--------|
| `TDX_LOG_ENABLE` | 总开关，注释掉则所有日志编译为空 | 启用 |
| `TDX_LOG_STATIC_LVL` | 全局编译期日志级别上限 (0–5) | 5 (Verbose) |
| `TDX_LOG_LINE_BUF_SIZE` | 单行缓冲区大小 | 192 |
| `TDX_LOG_HEXDUMP_ENABLE` | Hexdump 功能 | 启用 |
| `TDX_LOG_FRAME_DUMP_ENABLE` | 协议帧美化打印 | 启用 |
| `TDX_LOG_ISR_ENABLE` | ISR 安全延迟日志 | 启用 |
| `TDX_LOG_ASYNC_ENABLE` | 异步缓冲输出 | 启用 |
| `TDX_LOG_FLASH_ENABLE` | Flash 离线存储 | 启用 |

### 8.3 按模块控制日志

每个 SDK 模块可独立控制编译期日志级别。在 `tdx_log_cfg.h` 中修改对应
`TDX_LOG_MOD_LVL_*` 宏即可：

```c
/* 完全关闭 OTA 模块的日志 */
#define TDX_LOG_MOD_LVL_OTA         -1

/* BLE 模块只保留 Error 和 Assert */
#define TDX_LOG_MOD_LVL_BLE         1

/* WiFi 模块只到 Info 级别 */
#define TDX_LOG_MOD_LVL_WIFI        3

/* 其他模块继承全局 TDX_LOG_STATIC_LVL */
```

可用的模块级别宏：

| 宏 | 对应模块 | 关联源文件 |
|----|---------|-----------|
| `TDX_LOG_MOD_LVL_BLE` | BLE 服务端 | `tdx_ble_server.c` |
| `TDX_LOG_MOD_LVL_BLE_CLI` | BLE 客户端 | `tdx_ble_client.c` |
| `TDX_LOG_MOD_LVL_PROTO` | 协议编解码 | `tdx_protocol_server.c` |
| `TDX_LOG_MOD_LVL_CORE` | 核心/传输 | `tdx_common.c`, `tdx_transport_mgr.c`, `tdx_queue.c` |
| `TDX_LOG_MOD_LVL_OTA` | OTA 升级 | `tdx_ota.c` |
| `TDX_LOG_MOD_LVL_REC` | 录音 | `tdx_record.c` |
| `TDX_LOG_MOD_LVL_FILE` | 文件传输 | `tdx_file.c` |
| `TDX_LOG_MOD_LVL_CRYPT` | 加密 | `tdx_encryption.c` |
| `TDX_LOG_MOD_LVL_SPI` | SPI 传输 | `tdx_spi.c` |
| `TDX_LOG_MOD_LVL_SPP` | BT SPP | `tdx_spp.c` |
| `TDX_LOG_MOD_LVL_WIFI` | WiFi | `tdx_wifi*.c` |
| `TDX_LOG_MOD_LVL_UTIL` | 工具/RTC | `tdx_rtc.c`, `tdx_util.c` |
| `TDX_LOG_MOD_LVL_APP` | 应用层 | `port/app/*.c` |
| `TDX_LOG_MOD_LVL_BSP` | BSP 外设 | `port/bsp/**/*.c` |
| `TDX_LOG_MOD_LVL_HAL` | HAL 驱动 | `port/platform/**/*.c` |

**实现机制**：每个 .c 文件在 `#include "tdx_log.h"` 之前定义 `TDX_LOG_LOCAL_LEVEL`：

```c
#define TDX_LOG_TAG         "TDX.MY_MOD"
#define TDX_LOG_LOCAL_LEVEL TDX_LOG_MOD_LVL_xxx
#include "tdx_log.h"
```

级别值说明：

| 值 | 含义 |
|----|------|
| -1 | 完全禁用该模块所有日志 |
| 0 | 仅 Assert |
| 1 | Assert + Error |
| 2 | + Warn |
| 3 | + Info（生产推荐） |
| 4 | + Debug |
| 5 | + Verbose（全量，默认） |

### 8.4 运行时控制

```c
tdx_log_set_global_level(TDX_LOG_LVL_INFO);           // 设置运行时全局级别
tdx_log_set_tag_level("TDX.BLE", TDX_LOG_LVL_ERROR);  // 按 tag 过滤
tdx_log_set_channels(TDX_LOG_CH_UART | TDX_LOG_CH_BLE); // 输出通道
tdx_log_set_output_enabled(0);                          // 全局静默
```

---

## 9. 传输层多通道路由

SDK 支持多传输通道并发，不同通道用于不同用途：

```c
tdx_transport_bind_channel(TDX_CHANNEL_CTRL, TDX_TRANSPORT_BLE_SERVER);
tdx_transport_bind_channel(TDX_CHANNEL_DATA, TDX_TRANSPORT_SOCKET);
tdx_transport_bind_channel(TDX_CHANNEL_OTA,  TDX_TRANSPORT_BLE_SERVER);

tdx_transport_send_on(TDX_CHANNEL_DATA, audio_data, audio_len);
tdx_transport_send_on(TDX_CHANNEL_CTRL, cmd_data, cmd_len);
```

---

## 10. SDK 版本

### 10.1 查询版本

```c
#include "tdx_version.h"

const char *ver = tdx_sdk_get_version();     // "1.0.0"
uint32_t hex = tdx_sdk_get_version_hex();    // 0x010000
```

### 10.2 条件编译

```c
#if TDX_SDK_VERSION_HEX >= 0x010100
  // 使用 SDK 1.1.0+ 引入的新特性
#endif
```

### 10.3 目录命名

SDK 目录固定为 `tidex_aicap/`（不含版本号），版本升级无需修改 Makefile 路径。

---

## 11. 错误码

SDK 使用 `tdx_err_t` (int32_t) 错误码，按模块分段：

| 范围 | 模块 | 代表错误码 |
|------|------|-----------|
| `0` | 成功 (`TDX_OK`) | — |
| `-0x0001 ~ -0x00FF` | 通用错误 | — |
| `-0x0100 ~ -0x01FF` | 传输层 | `TDX_ERR_TRANSPORT_DISCONNECTED`, `_SEND_FAIL`, `_MTU_EXCEEDED` |
| `-0x0200 ~ -0x02FF` | 文件系统 | `TDX_ERR_FS_DISK_FULL`, `_NOT_FOUND`, `_IO_ERROR` |
| `-0x0300 ~ -0x03FF` | 认证 | `TDX_ERR_AUTH_INVALID_KEY`, `_NOT_BOUND` |
| `-0x0400 ~ -0x04FF` | 协议 | `TDX_ERR_PROTOCOL_PARSE`, `_CRC` |
| `-0x0500 ~ -0x05FF` | OTA | `TDX_ERR_OTA_VERIFY_FAIL`, `_SIZE_EXCEEDED` |
| `-0x0600 ~ -0x06FF` | Socket | `TDX_ERR_SOCKET_CONNECT_FAIL`, `_CLOSED`, `_DNS_FAIL` |
| `-0x0700 ~ -0x07FF` | BLE Client | `TDX_ERR_BLE_CLIENT_SCAN_FAIL`, `_CONN_FAIL` |

详见 `sdk/include/hal/tdx_hal_defs.h`。

---

## 12. 常见问题

### Q: 能 include SDK 库内部的头文件吗？

**不能。** 只能使用 `sdk/include/` 下的公共 API 头文件。`sdk/src/` 下的头文件
是 SDK 内部实现，不对外暴露。

### Q: 如何只启用部分功能模块？

通过 `TDX_HAS_*` 宏控制。在 `board.h` 中将不需要的功能设为 `0`。

### Q: 编译报 undefined reference 怎么办？

检查：
1. `sdk/lib/<chip>/libtdx_sdk.a` 是否已链接
2. `sdk/include/` 和 `sdk/include/hal/` 是否在 include 路径中
3. `port/platform/<chip>/` 是否在 include 路径中（使 `tdx_hal_config.h` 覆盖默认模板）
4. 所有 `TDX_HAL_HAVE_*` 为 1 的 HAL 接口是否都已实现

### Q: `tdx_platform.h` 和 `tdx_hal_*.h` 什么关系？

`tdx_platform.h` 是 facade，内部 include 全部 `tdx_hal_*.h`。已有代码
`#include "tdx_platform.h"` 无需修改。新代码建议直接 include 需要的
`tdx_hal_<domain>.h`，减少编译依赖。

### Q: SDK 是否依赖 mbedtls 等外部加密库？

**不依赖。** SDK 内部所有加密操作均使用自包含实现（SHA256、MD5），无外部库依赖。

### Q: 多个产品共用同一套源码如何区分？

在 Makefile 中通过 `-include` 不同的 `board.h` 注入不同的 `TDX_PRODUCT_*` 宏。
`product_select.h` 会自动路由到正确的产品定义。
