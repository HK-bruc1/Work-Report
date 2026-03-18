# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**ESP-Hosted-MCU** enables any MCU to use an Espressif ESP chip as a wireless co-processor (Wi-Fi + Bluetooth). The host MCU communicates with the co-processor via SPI, SDIO, or UART using RPC commands serialized as Protocol Buffers. Current version: **2.12.3** (requires ESP-IDF ≥ 5.3).

## Build Commands

This is an ESP-IDF component. Build targets are either the slave firmware or one of the host examples:

```bash
# Build slave co-processor firmware
cd slave
idf.py set-target esp32c6          # or esp32, esp32s3, esp32c3, etc.
idf.py menuconfig                  # configure transport, features
idf.py build
idf.py -p <PORT> flash monitor

# Build a host example
cd examples/<example_name>
idf.py build
idf.py -p <PORT> flash monitor
```

**CI build tool**: `idf-build-apps` — configured in `.idf_build_apps.toml`, runs matrix builds across targets and transport configs via `.gitlab/ci/`.

## Pre-commit Checks

Run before committing (`.pre-commit-config.yaml`):

```bash
pre-commit run --all-files
```

Checks enforced:
- `tools/check_fw_versions.py` — host/co-processor versions match `idf_component.yml`
- `tools/check_rpc_calls.py` — RPC call IDs in protobuf match `docs/implemented_rpcs.md`
- `tools/check_changelog.py` — `CHANGELOG.md` is updated
- `tools/check_weak_functions.py` — weak function stubs are correct
- `codespell` — spell check (config in `.codespellrc`)
- Copyright header validation

CI also compiles with `-Werror -Werror=deprecated-declarations` and strict prototype checking.

## Architecture

### System Design

```
Host MCU                            Co-processor (ESP32 series)
  Application                         Wi-Fi / BT Stack
  esp_wifi_*() API  ──RPC over──►    Slave firmware (slave/)
  esp-hosted (host/)   SPI/SDIO/UART
```

All Wi-Fi API calls on the host side are thin wrappers (weak functions in `host/api/src/esp_wifi_weak.c`) that serialize the call via RPC, send it over the transport to the slave, and return the response.

### Key Directories

| Path | Role |
|---|---|
| `host/` | Host-side driver (transport + RPC + API layers) |
| `slave/` | Co-processor firmware (ESP-IDF app) |
| `common/` | Shared: proto definitions, protobuf-c submodule, transport headers |
| `examples/` | 17 standalone ESP-IDF example apps |
| `docs/` | Architecture docs, transport guides, RPC list, troubleshooting |
| `tools/` | Python validation scripts used by pre-commit and CI |

### Host Driver Layers (`host/`)

1. **API layer** (`host/api/`) — Public surface: `esp_hosted_api.c`, weak Wi-Fi functions, OTA, power save, transport config.
2. **RPC layer** (`host/drivers/rpc/`) — `rpc_core.c`, `rpc_req.c`, `rpc_rsp.c`, `rpc_evt.c`. Serializes commands via protobuf (`common/proto/esp_hosted_rpc.proto`) and dispatches over transport.
3. **Transport layer** (`host/drivers/transport/`) — Four drivers: `spi/spi_drv.c`, `spi_hd/spi_hd_drv.c`, `sdio/sdio_drv.c`, `uart/uart_drv.c`. Selected at build time via Kconfig.
4. **Port layer** (`host/port/esp/freertos/`) — OS abstraction (task creation, queues, mutexes) and platform-specific transport init.
5. **Support drivers** — `bt/` (HCI VHCI), `mempool/`, `power_save/`, `serial/`, `virtual_serial_if/`.

### RPC Message Flow

```
host app → esp_wifi_*() weak fn → rpc_req.c serializes to protobuf
         → transport driver (SPI/SDIO/UART) → slave receives
         → slave executes real esp_wifi_*() → sends protobuf response back
         → rpc_rsp.c deserializes → return value to app
```

Events (e.g., IP_EVENT, WIFI_EVENT) flow in the reverse direction via `rpc_evt.c`.

### Slave Firmware (`slave/main/`)

Key files: `esp_hosted_coprocessor.c` (main init), `slave_wifi_std.c`, `slave_bt.c`, transport API files (`sdio_slave_api.c`, `spi_slave_api.c`, `uart_slave_api.c`). Features are conditionally compiled via Kconfig (enterprise Wi-Fi, network split, light sleep, GPIO expander, OTA).

### Adding a New RPC Call

1. Add message definitions to `common/proto/esp_hosted_rpc.proto`.
2. Add RPC ID and handler in slave (`slave/main/`).
3. Add weak function stub in `host/api/src/esp_wifi_weak.c`.
4. Add RPC request wrapper in `host/drivers/rpc/wrap/`.
5. Update `docs/implemented_rpcs.md` (required by `check_rpc_calls.py`).

## Configuration

`Kconfig` (88 KB) controls all build-time options:
- `ESP_HOSTED_CP_TARGET` — co-processor chip (ESP32, C2–C6, C61, H2, H4)
- Transport selection (SDIO / SPI full-duplex / SPI half-duplex / UART)
- Feature flags: Wi-Fi enterprise, network split, host power save, DPP, GPIO expander, light sleep
- GPIO pin assignments for each transport

Pre-built `sdkconfig.*` files in `slave/` cover common target + transport combinations used by CI.
