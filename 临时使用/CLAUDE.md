# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This project uses a two-stage Make build system for Jieli AC710N (BR56) TWS earphone firmware:

### Build Commands
```bash
# Windows - Build and flash firmware
make

# Build with verbose output
make VERBOSE=1

# Clean build artifacts
make clean

# Flash with specific formats
cpu\br56\tools\download.bat format_flash
cpu\br56\tools\download.bat format_vm
cpu\br56\tools\download.bat download
```

### Build Configuration
- **Customer Configuration**: Set via `customer_path.conf` (currently: KEKJ_K73)
- **Product Configuration**: Defined in `customer/product.conf`
- **IC Model**: Configured via `_IC_Model` (0=AC7106, 1=AC7103D4, 2=AC7103D8)
- **Tool Paths**:
  - Windows: `C:/JL/pi32/bin`
  - Linux: `/opt/jieli/pi32v2/bin`

### Build Output
- **Main Executable**: `cpu/br56/tools/sdk.elf`
- **Object Files**: `objs/` directory
- **Map File**: `cpu/br56/tools/sdk.map`

## Architecture Overview

### Core Application Structure
```
apps/earphone/
├── app_main.c              # Main application entry point
├── mode/bt/                # Bluetooth mode implementation
│   ├── earphone.c          # Main earphone logic
│   ├── a2dp_play.c         # A2DP audio streaming
│   ├── phone_call.c        # Call handling
│   └── tws_*.c             # TWS (True Wireless Stereo) implementation
├── audio/                  # Audio processing
│   ├── vol_sync.c          # Volume synchronization
│   └── scene_switch.c      # Audio scene management
├── battery/                # Power management
├── ble/                    # BLE functionality
└── board/br56/             # Hardware abstraction
```

### TWS Earphone System
This is a **True Wireless Stereo (TWS) earphone** implementation with:
- **Master/Slave Architecture**: One earphone acts as master, connecting to phone and slave earphone
- **Smart Charging Case**: BLE communication with colorful display case (智能彩屏仓)
- **Multi-Protocol Support**: RCSP, Tuya, Google Fast Pair, and custom protocols
- **Audio Processing**: AEC, ANC, EQ, spatial audio effects

### Smart Charging Case Communication (彩屏仓)
Located in `apps/common/third_party_profile/jl_earbox/`:

**Architecture**:
```
External Integration → sbox_user_app.c (API Layer)
                    ↓
BLE Protocol → sbox_protocol.c (Communication Layer)
            ↓
Core Engine → sbox_core_code.c (Protocol Engine)
           ↓
Platform → sbox_core_config.c (Adaptation Layer)
        ↓
UART → sbox_uart_app.c (Serial Communication)
```

**Key Integration Points**:
1. `app_main.c:386-387` - Initialize: `sbox_app_init()`
2. `multi_protocol_main.c:460-462` - Protocol stack: `sbox_demo_all_init()`
3. `vol_sync.c:117` - Status sync: `sbox_cb_func.sbox_sync_volume_info()`
4. `app_testbox.c:534` - UART handling: `user_app_chargestore_data_deal()`

**Communication Flow**:
- **Phase 1**: UART handshake (MAC address exchange)
- **Phase 2**: BLE data communication (AES encrypted)
- **Features**: Volume sync, battery status, music control, EQ settings

## Configuration System

### Multi-Customer Support
- **Customer Path**: `CUSTOMER_PATH=KEKJ_K73` in `customer_path.conf`
- **Customer Config**: `customer/KEKJ_K73/customer.conf`
- **Build Flags**: Customer-specific `CFLAGS_EX` definitions

### Key Configuration Files
- `apps/earphone/include/app_config.h` - Main application configuration
- `apps/earphone/board/br56/board_config.h` - Hardware configuration
- `customer/product.conf` - Product feature configuration
- `apps/common/third_party_profile/jl_earbox/sbox_core_config.h` - Smart case configuration

### Protocol Selection
Third-party protocols use bitmask system:
```c
#define JL_SBOX_EN               (1 << 31)  // Smart charging case
#define RCSP_MODE_EN             (1 << 0)   // Jieli RCSP protocol
#define TUYA_DEMO_EN             (1 << 3)   // Tuya IoT protocol
// Selected via THIRD_PARTY_PROTOCOLS_SEL
```

## Important Notes

### Conditional Compilation
- Smart case protocol (`JL_SBOX_EN`) **cannot coexist** with other third-party protocols
- Only enabled when `APP_ONLINE_DEBUG=1` and no other protocols selected
- Requires `TCFG_THIRD_PARTY_PROTOCOLS_ENABLE=1`

### TWS Synchronization
- Master earphone handles phone connection and slave coordination
- Audio, volume, and state changes synchronized between left/right earphones
- Smart case receives status updates from master earphone

### Hardware Variants
- Multiple IC models supported (AC7106, AC7103D4, AC7103D8)
- Customer-specific hardware configurations in board files
- Audio codec and power management vary by customer requirements

### Development Features
- Debug output via UART/SPP
- Online tools for AEC, ANC, EQ tuning
- Test modes for factory calibration
- OTA update support