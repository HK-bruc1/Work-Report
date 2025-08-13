# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the DHF AC710N-V300P03 SDK - a firmware development kit for Bluetooth earphone/headset devices based on the BR56 CPU architecture. The project supports various customer products including TWS (True Wireless Stereo) earphones with features like ANC (Active Noise Cancellation), spatial audio, and multi-protocol communication.

## Key Architecture Components

### CPU and Hardware Platform
- **Target CPU**: BR56 (pi32v2 architecture)
- **Build System**: Custom Makefile-based with clang toolchain
- **Flash Management**: NOR flash with virtual FAT filesystem
- **Audio DSP**: Hardware-accelerated audio processing with CVP (Communication Voice Processing)

### Application Structure
- **Main Application**: `apps/earphone/` - Primary TWS earphone application
- **Common Libraries**: `apps/common/` - Shared functionality across products
- **Audio Framework**: `audio/` - Comprehensive audio processing pipeline
- **Customer Configs**: `customer/` - Product-specific configurations

### Core Subsystems
1. **Bluetooth Stack** (`interface/btstack/`):
   - Classic Bluetooth and BLE support
   - TWS synchronization and data transfer
   - Third-party protocols (RCSP, Google Fast Pair, etc.)

2. **Audio Processing** (`audio/`):
   - CVP for call processing (AEC, noise suppression)
   - Effects engine (EQ, reverb, spatial audio)
   - Codec support (SBC, AAC, LC3, LDAC)
   - ANC processing

3. **Device Management** (`apps/common/dev_manager/`):
   - Power management and charging
   - Key input handling (touch, physical buttons)
   - LED/UI control
   - Sensor integration (IMU for spatial audio)

## Build and Development Commands

### Build Commands
```bash
# Standard build and download to device
make

# Verbose build output
make VERBOSE=1

# Clean build artifacts
make clean
```

### Project Configuration
The build system uses a three-tier configuration:
1. `customer_path.conf` - Selects active customer configuration
2. `customer/product.conf` - Product-level settings
3. `customer/[CUSTOMER]/customer.conf` - Customer-specific settings

### Customer Management
```bash
# Copy customer configuration to SDK
customer_copy.bat

# Switch between customer configurations by editing customer_path.conf
```

## Development Workflow

### Adding New Features
1. Check existing implementations in `apps/common/` for reusable components
2. Follow the event-driven architecture pattern used throughout the codebase
3. Add configuration options to the customer config system
4. Use the audio framework nodes for audio processing features

### Audio Development
- Audio processing uses a node-based pipeline architecture
- Effects are modular and configurable via customer settings
- CVP parameters are tunable through online debug tools
- Spatial audio requires IMU sensor integration

### Customer Customization
Each customer product has:
- Hardware configuration (GPIO, peripherals)
- Audio tuning parameters (EQ, ANC coefficients)
- Key mapping and LED patterns
- Bluetooth and pairing behavior
- Tone/prompt audio files

### Testing and Validation
- Use testbox mode for production testing
- Audio DUT (Device Under Test) tools for acoustic validation
- Online debug capabilities for real-time parameter tuning
- RSSI and RF testing utilities

## Code Organization Patterns

### Module Structure
- Header files define interfaces in `include/` directories
- Implementation follows component-based architecture
- Configuration is separated from functional code
- Platform-specific code is isolated in `cpu/br56/`

### Memory Management
- Overlay system for code optimization
- Careful RAM usage due to embedded constraints
- Streaming buffers for audio processing
- VM (Virtual Memory) system for persistent storage

### Event System
- Central event manager coordinates subsystems
- Key events trigger mode changes and audio routing
- Battery and charging events manage power states
- Bluetooth events handle connection management

## Important File Locations

- **Main Entry**: `apps/earphone/app_main.c`
- **Build Config**: `Makefile`, `customer_path.conf`
- **Audio Config**: Customer-specific audio configuration files
- **Interface Headers**: `interface/` directory contains all API definitions
- **Customer Assets**: `customer/[NAME]/src/` contains audio files and JSON configs
- **Output Firmware**: `cpu/br56/tools/` contains generated firmware files
