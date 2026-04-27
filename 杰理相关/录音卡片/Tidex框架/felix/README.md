# TideX SDK

TideX 产品线公共协议与功能库，提供与芯片平台无关的 BLE 协议引擎、多通道传输、
文件管理、OTA、加密、日志等核心能力。

## 快速开始

1. 将本目录引入项目（submodule 或拷贝）
2. 实现 `include/tdx_platform.h` 中定义的 HAL 接口
3. 在 `port/` 中编写应用胶水层与驱动适配
4. 配置 Makefile include 路径和源文件列表

详细步骤请阅读 [移植指南](docs/PORTING_GUIDE.md)。

## 目录说明

```
sdk/
├── include/        对外头文件（API + HAL 接口定义）
├── src/            SDK 源码（core / components / utils / vendor）
└── docs/           文档
    ├── PORTING_GUIDE.md    移植指南
    └── API_REFERENCE.md    API 参考
```

## API 入口

```c
#include "tdx_api.h"    // 包含全部公共 API
```

## 版本

v1.0.0 (2026-03-18)
