# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a TWS (True Wireless Stereo) earphone development project based on AC710N chip platform from JieLi Technology. The project combines hardware configuration, firmware SDK, and audio processing workflows.

## Project Structure

```
710_earbox/
├── project.jlproj          # Main project configuration file
├── src/                    # Configuration files for different modules
│   ├── 按键配置.json       # Key/button configuration
│   ├── 板级配置.json       # Board-level configuration  
│   ├── 电源配置.json       # Power management configuration
│   ├── 功能配置.json       # Feature configuration
│   ├── 蓝牙配置.json       # Bluetooth configuration
│   ├── 升级配置.json       # Update/OTA configuration
│   ├── 音频配置.json       # Audio configuration
│   ├── 提示音.tone         # Prompt tone files
│   └── 音频流程/           # Audio flow definitions
│       ├── 系统模式.x6flow
│       ├── 媒体.x6flow
│       ├── 蓝牙通话.x6flow
│       └── [other flow files]
└── SDK/                    # JieLi SDK and firmware code
    ├── apps/earphone/      # Main earphone application
    ├── audio/              # Audio processing modules
    ├── build/              # Build system
    ├── cpu/                # CPU-specific code
    └── tools/              # Development tools
```

## Build System

### Primary Build Commands

**Windows Environment:**
```bash
# Navigate to SDK directory
cd 710_earbox/SDK

# Build project (default target)
make

# Clean build artifacts  
make clean

# Verbose build output
make VERBOSE=1

# Download firmware after build
make download
```

**Linux Environment:**
- Requires JieLi toolchain installed at `/opt/jieli/`
- Same make commands as Windows

### Build Requirements

- **Windows**: JieLi toolchain at `C:/JL/pi32/bin/`
- **Linux**: JieLi toolchain at `/opt/jieli/pi32v2/bin/`
- LLVM/Clang-based compilation toolchain
- Target architecture: PI32V2 (R3 CPU core)

### Build Artifacts

- Main firmware: `cpu/br56/tools/sdk.elf`
- Object files: `objs/` directory
- Configuration binaries generated from JSON configs

## Configuration System

The project uses a JSON-based configuration system where each module has its dedicated configuration file:

- **按键配置.json**: Defines button mappings and LED behaviors
- **板级配置.json**: Hardware board configuration (GPIO, peripherals)
- **电源配置.json**: Power management settings
- **蓝牙配置.json**: Bluetooth stack configuration
- **音频配置.json**: Audio codec and processing parameters

Configuration files are processed during build and converted to binary formats for firmware.

## Audio Flow System

The project uses `.x6flow` files to define audio processing pipelines for different operational modes:

- **系统模式**: System mode audio processing
- **媒体**: Media playback processing
- **蓝牙通话**: Bluetooth call audio processing
- **USB Audio**: USB audio mode processing
- **LE_Audio**: Low Energy Audio processing

These flow definitions control how audio is routed and processed through the DSP pipeline.

## Development Workflow

1. Modify configuration JSON files in `src/` directory
2. Update audio flows in `src/音频流程/` if needed
3. Modify application code in `SDK/apps/earphone/`
4. Build using `make` command
5. Flash firmware using provided download tools

## Key Technical Specifications

- **Target Chip**: AC710N (BR56 core)
- **SDK Version**: 3.0.0
- **Patch Level**: patch_05
- **Architecture**: PI32V2 with R3 CPU
- **Build System**: LLVM/Clang with custom LTO wrapper
- **Configuration Format**: JSON with binary conversion

## File Encoding

The project uses mixed encoding:
- Configuration files: UTF-8
- Some build scripts may require GBK encoding conversion on Windows

## Development Tools Integration

- CodeBlocks project file: `AC710N.cbp`
- VSCode configuration available in `.vscode/`
- Custom build system with Makefile-based automation