# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This workspace is focused on RTC (Real-Time Clock) module development for the **JieLi AC701N TWS Earphone chip** (`701公版SDK`). The two root-level files (`rdx_rtc.c`, `rdx_rtc.h`) are the primary subjects of analysis and development — they are intended to be placed at `SDK/apps/earphone/third_part/tuya/` within the SDK.

## Build System

The SDK uses a GNU Make-based build with JieLi's custom clang toolchain for the `pi32v2` architecture.

**Toolchain location (Windows):** `C:/JL/pi32/bin/clang.exe`

**Build commands (run from `701公版SDK/SDK/`):**
```bash
make         # Full build
make clean   # Clean build artifacts
```

VS Code tasks are configured in `701公版SDK/SDK/.vscode/tasks.json` (tasks: `all`, `clean`).

Build output goes to `701公版SDK/output/`. Flash via `701公版SDK/SDK/download.bat`.

## Architecture

### RTC Module (`rdx_rtc.c` / `rdx_rtc.h`)

Self-contained RTC abstraction that provides Unix-timestamp-based time management. Two compile-time implementations controlled by `#if 1` / `#else` at `rdx_rtc.c:331`:

- **Mode 1 (active, lines 331–569):** Software RTC. Stores a base timestamp + system tick offset (`jiffies_msec()`). Persists to NVM via `syscfg_write/read(VM_RDX_RTC_INIT_VALUE, ...)` every 3 minutes using `sys_timer_add`.

- **Mode 2 (inactive, lines 571–701):** Hardware RTC. Uses the chip's hardware RTC peripheral (`asm/rtc.h`) via `rtc_init()`, `read_sys_time()`, `write_sys_time()`. Clock source: `CLK_SEL_LRC`.

**Key dependencies from SDK:**
- `app_main.h` — application entry/common includes
- `syscfg_id.h` — NVM slot ID (`VM_RDX_RTC_INIT_VALUE`)
- `asm/rtc.h` — hardware RTC driver
- `jiffies_msec()`, `jiffies_msec2offset()` — system tick API
- `sys_timer_add()`, `sys_timer_del()` — timer API
- `y_printf`, `g_printf`, `b_printf` — SDK logging macros

**Public API summary:**
| Function | Description |
|---|---|
| `rdx_rtc_init()` | Initialize RTC; reads NVM or sets default `2025-04-01 16:50:06`; starts persistence timer |
| `rdx_rtc_set_timestamp(time_t)` | Set current UTC timestamp; persists to NVM |
| `rdx_rtc_get()` | Get current UTC timestamp (base + elapsed ticks) |
| `rdx_rtc_utc_string_to_timestamp(str)` | Parse `"YYYY-MM-DD HH:MM:SS"` → `time_t` |
| `rdx_rtc_timestamp_to_datetime(time_t)` | `time_t` → `DateTime` struct |
| `rdx_rtc_datetime_to_timestamp(DateTime)` | `DateTime` → `time_t` |
| `rdx_rtc_timestamp_to_utc_string(time_t, buf)` | `time_t` → `"YYYY-MM-DD HH:MM:SS"` string |
| `rdx_rtc_test()` | Debug print of current time fields |

**Known issue:** `rdx_rtc_get_string()` (line 384) returns a pointer to a local stack buffer — this is a use-after-return bug. `rdx_rtc_get_string_date()` and `rdx_rtc_get_string_time()` use `static` buffers instead.

### SDK Structure (`701公版SDK/SDK/`)

| Directory | Purpose |
|---|---|
| `apps/earphone/` | Top-level earphone application |
| `apps/common/` | Shared modules: clock_manager, bt_common, ui, music, ai_audio, etc. |
| `audio/` | Audio framework, codecs (AAC/SBC/mSBC), ANC, effects, smart voice |
| `cpu/br28/` | BR28/AC701N chip-specific drivers and HAL |
| `interface/` | Abstraction layer: btstack, driver, media, system, net, update |
| `build/` | Makefile fragments and build configuration |

Configuration for a project lives in `701公版SDK/src/` as JSON files (features, buttons, board, power, Bluetooth, audio, upgrade).

## Switching RTC Implementation

To switch between software RTC (Mode 1) and hardware RTC (Mode 2), change the `#if 1` on `rdx_rtc.c:331` to `#if 0`. When using hardware RTC (Mode 2), ensure the hardware clock source (`CLK_SEL_LRC` vs `CLK_SEL_RCO`) is appropriate for the board schematic.
