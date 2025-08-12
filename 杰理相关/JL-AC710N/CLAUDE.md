# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a TWS (True Wireless Stereo) earphone firmware project built on the Jieli BR56 chipset. The project contains SDK code, customer configurations, and build tools for developing TWS earphones with advanced audio processing, ANC (Active Noise Cancellation), and Bluetooth connectivity.

## Repository Structure

```
DHF-AC710N-V300P03/
├── SDK/                    # Core SDK containing firmware source
│   ├── apps/               # Application layer code
│   │   ├── earphone/       # Main earphone application
│   │   └── common/         # Common application utilities
│   ├── audio/              # Audio processing modules
│   │   ├── anc/            # Active Noise Cancellation
│   │   ├── CVP/            # Call Voice Processing
│   │   ├── effect/         # Audio effects
│   │   └── interface/      # Audio interfaces
│   ├── customer/           # Customer-specific configurations
│   │   ├── GK158_Left/     # Left earphone config
│   │   ├── GK158_Right/    # Right earphone config
│   │   └── product.conf    # Product configuration
│   ├── cpu/br56/           # BR56 chip-specific code
│   └── tools/              # Build and utility tools
├── src/                    # Configuration files (JSON/flow files)
├── output/                 # Build output directory
└── project.jlproj         # Project file
```

## Build System

### Primary Build Commands

**Main build command:**
```bash
# Windows (from SDK directory)
product_compile_image.bat

# Alternative make commands
make all           # Build the project
make clean         # Clean build artifacts
make VERBOSE=1     # Build with detailed output
```

**VSCode Integration:**
- Uses `.vscode/tasks.json` with default build task
- Build task runs `product_compile_image.bat` on Windows or `make all -j 16` on Linux

### Build Configuration

The build system uses a hierarchical configuration approach:

1. **customer_path.conf** - Defines current customer configuration (e.g., `CUSTOMER_PATH=GK158_Left`)
2. **customer/product.conf** - Product-level configuration and compiler flags
3. **customer/{CUSTOMER}/customer.conf** - Customer-specific settings including IC model selection

**Key configuration variables:**
- `_IC_Model`: IC selection (0=AC7106A8, 1=AC7103D4)
- `_DAC_PA_EN`: Manual power amplifier control
- `_LOW_POWER_WARN_TIME`: Low battery warning timing

### Build Process

1. **Pre-build**: `genFileList.c` is preprocessed to generate `fileList.mk` with dynamic file lists
2. **Main build**: `make -f build/Makefile.mk` compiles the project
3. **Post-processing**: Runs download scripts for firmware packaging

### Output Files
- **Primary output**: `cpu/br56/tools/sdk.elf`
- **Customer binaries**: Generated in `output/` directory as `.fw` and `.ufw` files
- **Configuration files**: `cfg_tool.bin`, `stream.bin`, `tone_en.cfg`

## Development Workflow

### Customer Configuration Selection

To switch between different customer configurations:
1. Edit `customer_path.conf` to set `CUSTOMER_PATH`
2. Each customer directory contains:
   - `customer.conf` - Build configuration
   - `sdk_config.c/.h` - SDK feature configuration
   - Binary files (ANC coefficients, audio streams, etc.)

### Common Development Tasks

**Build for specific customer:**
```bash
# Edit customer_path.conf to set desired customer
# Then run build
product_compile_image.bat
```

**Clean build:**
```bash
make clean
make all
```

**Audio configuration:**
- Audio flow files in `src/音频流程/*.x6flow`
- Configuration files in `src/*.json`

## Architecture Overview

### Message-Driven Architecture

The firmware uses a sophisticated message dispatching system with multiple message sources:
- `MSG_FROM_KEY` - Button/key events
- `MSG_FROM_TWS` - TWS peer communication
- `MSG_FROM_BT_STACK` - Bluetooth stack events
- `MSG_FROM_BATTERY` - Battery/charging events
- `MSG_FROM_AUDIO` - Audio system events

### Key Components

**Main Application Loop** (`SDK/apps/earphone/app_main.c:633`):
```c
while (1) {
    app_set_current_mode(mode);
    switch (mode->name) {
    case APP_MODE_BT:
        mode = app_enter_bt_mode(g_mode_switch_arg);
        break;
    // Other modes...
    }
}
```

**TWS Synchronization**: Automatic state synchronization between left/right earphones including:
- Battery levels
- Audio controls
- ANC settings  
- Button presses

### Task Architecture
- **app_core** (Priority 1): Main application control
- **btstack** (Priority 3): Bluetooth protocol stack
- **jlstream** (Priority 3): Audio stream processing
- **aec** (Priority 2): Audio echo cancellation
- **anc** (Priority 3): Active noise cancellation

## Key Files and Locations

**Core Application:**
- `SDK/apps/earphone/app_main.c` - Main application entry point and mode switching
- `SDK/apps/earphone/mode/bt/earphone.c` - Bluetooth mode implementation

**Message System:**
- `SDK/apps/earphone/include/app_msg.h` - Message type definitions
- `SDK/apps/earphone/message/` - Message adapters and handlers

**Audio Processing:**
- `SDK/audio/` - Audio effects, ANC, CVP implementations
- `SDK/audio/framework/` - Audio processing framework

**Configuration:**
- `SDK/customer/product.conf` - Product build configuration
- `SDK/customer/{CUSTOMER}/customer.conf` - Customer-specific settings

## Build Environment Setup

**Windows Requirements:**
- Jieli toolchain at `C:/JL/pi32/bin/`
- System environment includes required build tools

**Linux Requirements:**  
- Jieli toolchain at `/opt/jieli/pi32v2/bin/`
- Ensure `ulimit -n` is sufficient (>8096) for linking

The build system automatically detects the platform and configures appropriate tool paths and commands.