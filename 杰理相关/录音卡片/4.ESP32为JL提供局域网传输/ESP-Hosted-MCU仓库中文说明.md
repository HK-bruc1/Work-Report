# ESP-Hosted-MCU 仓库中文说明

## 1. 这个仓库是做什么的

`esp-hosted-mcu` 是乐鑫提供的一套开源方案，用来把一颗 ESP 芯片当作“无线通信协处理器”使用。

可以把它理解成下面这个结构：

- 一颗主控 MCU 负责你的业务逻辑
- 另一颗 ESP 芯片负责 Wi‑Fi、蓝牙，以及部分扩展能力
- 主控和 ESP 之间通过 `SPI`、`SDIO` 或 `UART` 通信
- 主控像调用本地接口一样去调用远端 ESP 的能力

这个仓库的核心目标，是让“没有 Wi‑Fi/蓝牙能力的主控”，也能借助一颗 ESP 芯片快速获得无线连接能力。

## 2. 它解决了什么问题

很多 MCU 自身没有无线功能，或者无线能力不够完整。传统做法是：

- 自己移植完整无线协议栈
- 自己设计主控和无线模组之间的通信协议
- 自己处理固件升级、事件同步、接口兼容等问题

`esp-hosted-mcu` 把这些基础工作封装好了。开发者只需要：

- 选一颗主控作为 Host
- 选一颗 ESP 作为 Co-processor / Slave
- 按文档接好总线
- 编译并烧录 host 与 slave 固件

这样主控就能使用 ESP 侧提供的 Wi‑Fi / 蓝牙能力。

## 3. 仓库里的核心概念

### 3.1 Host MCU

Host 就是主控。它可以是：

- 另一颗 ESP
- 非 ESP 的 MCU

Host 负责运行你的应用逻辑，并通过远程调用方式使用 ESP 的功能。

### 3.2 Hosted Co-Processor / Slave

Slave 是作为协处理器工作的 ESP 芯片。它负责：

- Wi‑Fi
- 蓝牙
- 部分扩展功能

从文档看，常见支持对象包括 `ESP32`、`ESP32-C2`、`ESP32-C3`、`ESP32-C5`、`ESP32-C6/C61`、`ESP32-S2`、`ESP32-S3`，但不同芯片支持的传输方式不完全一样。

### 3.3 RPC 远程调用

这个方案不是简单地“串口透传数据”，而是走一套 RPC 思路：

- Host 发起请求
- 请求通过 `SPI / SDIO / UART` 传给 Slave
- Slave 在 ESP 侧执行对应功能
- 结果再返回给 Host

文档中提到，RPC 控制消息使用 `protobuf-c` 编码；而数据包本身不会都走序列化，以减少开销。

## 4. 这个仓库的主要能力

根据 `docs/features.md`，当前重点能力包括：

- Wi‑Fi Station
- SoftAP
- SoftAP + Station
- 蓝牙相关 Hosted HCI 支持
- Network Split
- Host Power Save
- GPIO Expander
- iTWT
- Wi‑Fi Enterprise
- Wi‑Fi Easy Connect（DPP）

所以它不只是“让主控联网”，还提供了一些面向量产和低功耗的扩展能力。

## 5. 支持哪些通信方式

仓库文档重点介绍了三类主从通信链路：

- `SPI`
- `SDIO`
- `UART`

它们各自适合的场景不同。

### 5.1 SPI

`SPI Full Duplex` 是文档里非常推荐的入门方案，特点是：

- 容易上手
- 适合跳线测试
- 对很多 MCU 都友好
- 适合先把系统跑通

如果你只是想先验证“主控能不能通过 ESP 上网/控蓝牙”，SPI 往往是最省事的。

### 5.2 SDIO

`SDIO` 是更偏高性能的方案，特点是：

- 吞吐更高
- 更适合正式设计
- 对硬件要求更严格
- 对布线、上拉、电气条件更敏感

文档明确提到 SDIO 对信号完整性要求高，很多情况下更适合 PCB，不太适合随便拉很长的杜邦线。

### 5.3 UART

`UART` 是最容易理解、接线最简单的一种方式，但文档也明确提醒：

- 速度较低
- 不适合高吞吐网络场景

如果你只是做低速控制、简单联网、功能验证，它依然是有价值的。

## 6. 仓库整体结构可以怎么理解

从文档内容看，这个仓库大致可以按下面方式理解：

- `README.md`
  - 总览整个方案、架构、依赖、支持的传输方式
- `slave/`
  - 协处理器固件示例，也就是跑在 ESP 从机上的固件
- `docs/`
  - 各种详细文档，包括 SPI、SDIO、UART、性能、排障、P4 开发板说明等
- `examples/`
  - Host 侧示例工程，例如 OTA、蓝牙、网络功能等
- `common/`
  - 公共协议与通用代码，例如 protobuf 相关内容

可以把它看成：

- `docs` 负责教你怎么搭系统
- `slave` 负责提供从机固件模板
- `examples` 负责展示 host 侧怎么调用

## 7. 这个仓库不是一个“现成 app”，而是一套方案

这是阅读完文档后最容易误解的一点。

这个仓库不是：

- 一个单独可直接运行的桌面程序
- 一个只需要点一下就能用的成品固件包
- 一个只面向某一块开发板的小 demo

它更像是：

- 一套主控 + 协处理器通信框架
- 一套基于 ESP-IDF 的组件化能力
- 一套参考实现和示例集合

如果你的项目需要“主控负责业务，ESP 负责无线”，这个仓库就是用来搭这套架构的。

## 8. 它依赖什么

根 README 里提到几个关键依赖：

- `ESP-IDF`
- `esp_wifi_remote`
- `protobuf-c`

可以简单理解为：

- `ESP-IDF` 提供乐鑫芯片开发框架
- `esp_wifi_remote` 提供一层远程 Wi‑Fi API 接口
- `protobuf-c` 用于 RPC 消息序列化

也就是说，这个仓库并不是“完全独立自成体系”的，它是建立在 ESP-IDF 生态上的。

## 9. 典型工作流程是什么

按文档的思路，典型流程是：

1. 选 Host 和 Slave 芯片
2. 确定主从之间的传输方式
3. 根据文档接线
4. 生成并配置 `slave` 工程
5. 编译并烧录 slave 固件
6. 选择或编译 host 示例工程
7. 把 host 配置成对应的 transport 和 GPIO
8. 烧录 host，并观察主从是否成功建立连接

也就是说，它本质上是一个“双端系统”：

- 一端是 Host
- 一端是 Slave

只烧一边通常是不够的。

## 10. 文档里强调的几个实践建议

阅读几份核心 MD 后，可以提炼出这些很重要的建议：

- 如果只是快速验证，优先考虑 `SPI Full Duplex`
- 如果追求更高性能，可以考虑 `SDIO`
- 如果硬件简单、速度要求不高，可以用 `UART`
- Host 和 Slave 两边的 transport 配置必须一致
- GPIO 配置必须和实际接线一致
- SDIO 对硬件质量要求明显更高，通常要注意上拉、电平和线长
- 这个方案推荐 Host 与 Slave 使用匹配的 ESP-Hosted 版本

这些建议基本贯穿了 `README.md`、`slave/README.md`、`docs/sdio.md`、`docs/spi_full_duplex.md` 和 `docs/uart.md`。

## 11. 哪些场景适合用它

这套方案比较适合下面几类项目：

- 主控不是 ESP，但想快速获得 Wi‑Fi / 蓝牙能力
- 主控已经承担很多业务，希望把无线通信独立到另一颗芯片
- 需要保留主控体系，同时复用乐鑫成熟无线能力
- 想先用 ESP 做 Host 示例验证，再逐步移植到自家 MCU

尤其对“已有主控平台，只缺无线能力”的项目，这个仓库价值比较大。

## 12. 一句话总结

`esp-hosted-mcu` 是一套让 ESP 芯片充当无线协处理器的双端框架：Host 负责应用，ESP Slave 负责无线，通过 SPI/SDIO/UART 通信，把 Wi‑Fi、蓝牙和部分扩展能力提供给主控使用。

## 13. 本说明参考的主要文档

- `README.md`
- `slave/README.md`
- `docs/features.md`
- `docs/sdio.md`
- `docs/spi_full_duplex.md`
- `docs/uart.md`

## 14. 编译与烧录（Slave：ESP32-C5）

本节说明如何使用 **VS Code + ESP-IDF 插件**编译和烧录 ESP-Hosted-MCU 的 Slave 固件。当前方案的 Slave 芯片为 **ESP32-C5**。

---

### 14.1 ESP-IDF 是什么

**ESP-IDF**（IoT Development Framework）是乐鑫官方为 ESP32 系列芯片提供的**开发框架/SDK**，本身不是 IDE。

| 对比项 | IDE（如 Keil） | ESP-IDF |
|---|---|---|
| 本质 | 集成开发**环境** | 开发**框架 / SDK** |
| 包含内容 | 编辑器 + 编译器 + 调试器 + 工程管理 | 库函数 + 构建系统 + 工具链 |
| 使用方式 | 打开软件直接用 | 需要搭配编辑器使用 |
| STM32 类比 | Keil MDK | HAL 库 + arm-gcc + Makefile |

ESP-IDF 具体包含：

- **工具链**：ESP32-C5 基于 RISC-V 架构，对应 `riscv32-esp-elf-gcc` 交叉编译器
  - 在windows上编译RISC架构的固件

- **构建系统**：CMake + Ninja，由 `idf.py` 统一封装
- **组件库**：Wi‑Fi、蓝牙、FreeRTOS、外设驱动、网络协议栈等
- **工具集**：`esptool.py`（烧录）、`idf.py`（构建入口）、组件管理器

**安装了 ESP-IDF，就同时拥有了编译 ESP 芯片所需的全套工具和库，无需单独安装编译器。**

对应到开发体验：Keil 的一体化操作，对应到 ESP32 就是 **VS Code + ESP-IDF 插件**——插件将 `idf.py` 的命令行操作封装成状态栏按钮和命令面板，点击即可完成编译、烧录、监视。

> ESP-Hosted-MCU 要求 **ESP-IDF v5.3 或更高版本**。

---

### 14.2 环境准备

#### 安装 ESP-IDF

**Windows：** 从乐鑫官网下载 ESP-IDF Windows 安装包，按向导完成安装。安装时勾选 **ESP-IDF v5.3 或更新版本**，安装包会自动配置工具链路径和环境变量。

安装 ESP-IDF 必备工具最简易的方式是下载一个 ESP-IDF 工具[安装器](https://dl.espressif.com/dl/esp-idf/?idf=4.4)。

- 挂着代理下会快一点。成功后会出选两个快捷方式，一个cmd，一个powershell。

#### 安装 VS Code ESP-IDF 插件

1. 打开 VS Code，进入扩展面板（`Ctrl+Shift+X`）
2. 搜索 **ESP-IDF**，安装 Espressif 官方插件
3. 首次安装后，VS Code 会提示「Configure ESP-IDF Extension」，选择之前已安装的 IDF 路径（**Use existing installation**），指向安装目录即可

安装完成后，VS Code 底部状态栏会出现 ESP-IDF 的快捷操作区域：

```
[串口选择]  [目标芯片]  [⚙ menuconfig]  [🔨 Build]  [⚡ Flash]  [🖥 Monitor]  [🔥 Build Flash Monitor]
```

---

### 14.3 获取 Slave 工程

克隆本仓库（包含子模块）：

```bash
git clone --recurse-submodules --depth 1 https://github.com/espressif/esp-hosted-mcu.git
```

克隆完成后，在 VS Code 中打开 `slave/` 子目录：

**File → Open Folder → 选择 `esp-hosted-mcu/slave`**

VS Code ESP-IDF 插件会自动识别该目录为 ESP-IDF 工程。

---

### 14.4 Slave 固件编译与烧录（ESP32-C5 + SPI Full-Duplex）

#### 第一步：设置目标芯片

点击状态栏中的芯片名称区域，在弹出的列表中选择 **`esp32c5`**。

> 等效命令行：`idf.py set-target esp32c5`
>
> 此步骤告知构建系统切换到 ESP32-C5 的 RISC-V 工具链，必须在编译前完成。

#### 第二步：配置传输方式（menuconfig）

点击状态栏的 **⚙ SDK Config** 按钮，或通过命令面板（`Ctrl+Shift+P`）执行：

```
ESP-IDF: SDK Configuration editor (menuconfig)
```

VS Code 会打开图形化配置界面（Web UI）。在搜索框输入 `transport` 快速定位，或手动按以下路径导航：

```
Example Configuration
└── Bus Config in between Host and Co-processor
    └── Transport layer
        └── [选择] SPI Full-Duplex
```

默认值为 SDIO，需改为 **SPI Full-Duplex**。

同时确认以下 SPI 参数（初次调试可保持默认，联调时再按实际接线调整）：

| 参数 | 说明 |
|---|---|
| MOSI / MISO / SCLK / CS GPIO | SPI 四线的引脚编号 |
| Handshake GPIO | Slave 通知 Host 可以发起事务的引脚 |
| Data Ready GPIO | Slave 通知 Host 有数据待取的引脚 |
| Reset GPIO | Host 控制 Slave 复位的引脚 |
| SPI 时钟频率 | 初次调试建议设为 **5 MHz**，稳定后再提高 |

配置完成后点击右上角 **Save** 保存。

#### 第三步：编译

点击状态栏的 **🔨 Build** 按钮，或命令面板执行：

```
ESP-IDF: Build your project
```

> 等效命令行：`idf.py build`

编译输出会显示在 VS Code 终端面板。编译成功后，固件文件位于：

```
slave/build/network_adapter.bin
```

该文件也是后续 OTA 升级时使用的固件包。

#### 第四步：选择串口

通过 USB 串口线连接 ESP32-C5 开发板，点击状态栏左侧的串口选择区域，从下拉列表中选择对应端口（Windows 为 `COMx`，Linux / macOS 为 `/dev/ttyUSBx` 或 `/dev/cu.usbserialx`）。

#### 第五步：烧录并监视

点击状态栏的 **🔥 Build Flash Monitor** 按钮一键完成编译、烧录和串口监视，或分步点击 **⚡ Flash** → **🖥 Monitor**。

> 等效命令行：`idf.py -p COM5 flash monitor`

烧录完成后串口监视器自动启动。正常启动日志末尾会显示 SPI 初始化完成并等待 Host 连接的信息，说明 Slave 固件运行正常。

---

### 14.3 Host 固件说明

本方案的 Host 是蓝牙音频芯片，**不是 ESP 芯片**，因此 Host 侧无法使用 ESP-IDF 进行编译。Host 侧需要在蓝牙音频芯片的开发环境中，按照第 16 节的要求自行实现以下内容：

- SPI 主机驱动
- Handshake / Data Ready / Reset 信号处理
- ESP-Hosted 帧头解析
- RPC 请求与响应收发

Host 侧的配置参数（SPI GPIO、时钟频率、传输模式等）必须与 Slave 的 `menuconfig` 配置保持完全一致。

---

### 14.4 OTA 升级说明

OTA（Over-The-Air）用于在系统运行期间远程更新 Slave 固件，**不是首次上线的推荐方式**。

第一次烧录建议直接使用串口（即上文第五步的方式）。待系统稳定后，如需支持后续固件升级，可参考 `examples/host_performs_slave_ota/` 示例，该示例支持三种 OTA 来源：LittleFS、Host 分区、HTTPS。

以 Host 分区方式为例，需将编译好的固件文件复制到指定位置：

```bash
cp slave/build/network_adapter.bin \
   examples/host_performs_slave_ota/components/ota_partition/slave_fw_bin/
```

---

### 14.5 常见问题

| 现象 | 可能原因 |
|---|---|
| Host 与 Slave 无法建立连接 | 两侧 transport 类型不一致，或 GPIO 配置与实际接线不符 |
| 日志出现 `drop pkt` | SPI 时钟过高，或杜邦线过长导致信号完整性差，建议先降低时钟频率 |
| 协议交互异常 | Host 与 Slave 使用的 ESP-Hosted 版本不一致，建议保持同版本 |
| SDIO 无法启动 | SDIO 对硬件要求严格，需检查外部上拉电阻、线长和电气条件 |

---

### 14.6 分阶段推进建议

由于 Host 是非 ESP 芯片，Slave 与 Host 两侧可以独立推进，无需同时就绪：

1. **阶段一（Slave 先行）**：按上文步骤编译烧录 ESP32-C5 slave 固件，通过串口日志确认固件正常启动，SPI 处于等待状态。此阶段完全不依赖 Host。

2. **阶段二（Host 独立开发）**：在蓝牙音频芯片侧独立实现 SPI 主机驱动及 ESP-Hosted 协议层，参考第 16 节。

3. **阶段三（联合调试）**：接上 SPI 线，由蓝牙音频芯片主动发起 RPC 请求，驱动 ESP32-C5 开启热点，验证手机能否搜到并连接。

> **注意**：ESP-Hosted slave 固件不会自动开热点，必须由 Host 通过 SPI 发送 RPC 命令触发。单独给 Slave 上电时，手机看不到热点是正常的。

## 15. 专门说明：ESP32-C5 作为无线协处理器，使用 SPI 与非 ESP Host 通信

这一节专门对应你们当前的方案：

- 主控：一颗非 `ESP` 的蓝牙音频芯片
- 无线协处理器：`ESP32-C5`
- 主从链路：`SPI`
- 角色分工：蓝牙音频芯片是 Host，`ESP32-C5` 是 Slave

这里的 `ESP32-C5` 不是主控，而是跑 `ESP-Hosted` slave 固件，负责：

- Wi‑Fi
- 蓝牙
- 与 Host 之间的数据与控制交互

### 15.1 先明确目标

你们现在要编译和烧录的重点，是 `ESP32-C5` 这边的 slave 固件。

也就是说，要在 `ESP32-C5` 上烧一个 `esp_hosted:slave` 工程，并把 transport 配成 `SPI Full-Duplex`。

对应文档思路主要来自：

- `slave/README.md`
- `docs/spi_full_duplex.md`

### 15.2 为什么这里建议优先用 SPI Full-Duplex

对“非 ESP Host + `ESP32-C5` Slave”这个组合来说，SPI-FD 是很合适的起步方式，因为：

- 文档把它视为最容易 bring-up 的方案
- 更适合跳线验证
- 比 SDIO 对硬件条件宽松
- 更适合非 ESP 主控先验证主从链路是否正常

所以在没有特别高吞吐要求的前提下，优先把 SPI-FD 跑通是合理的。

### 15.3 ESP32-C5 Slave 的编译步骤

#### 第一步：创建 slave 工程

在 ESP-IDF 环境里执行：

```bash
idf.py create-project-from-example "espressif/esp_hosted:slave"
cd slave
```

#### 第二步：设置目标芯片为 ESP32-C5

```bash
idf.py set-target esp32c5
```

这一步很关键，因为你现在烧的是 `ESP32-C5`，不是 `ESP32`。

#### 第三步：进入 menuconfig，把 transport 改成 SPI

```bash
idf.py menuconfig
```

进入下面这个配置路径：

```text
Example Configuration
  -> Bus Config in between Host and Co-processor
    -> Transport layer
```

把 transport 选成：

```text
SPI Full-Duplex
```

然后继续检查 SPI 相关配置项。文档中提到通常需要确认这些内容：

- `MOSI`
- `MISO`
- `SCLK`
- `CS`
- `Handshake`
- `Data Ready`
- `Reset`
- SPI 模式
- SPI 时钟频率
- 是否启用校验

这里最重要的不是“某个默认值一定对”，而是：

- `ESP32-C5` slave 的配置
- `ESP32` host 的配置
- 你的真实接线

这三者必须完全一致。

### 15.4 建议的接线理解

根据 `docs/spi_full_duplex.md`，SPI-FD 除了标准 SPI 线之外，还要额外信号：

- `MOSI`
- `MISO`
- `SCLK`
- `CS`
- `Handshake`
- `Data Ready`
- `Reset`
- `GND`

也就是说，它不是只有四根 SPI 线，还需要：

- Host 拉一根 `Reset` 去控制 C5 复位
- C5 给 Host 提供 `Handshake`
- C5 给 Host 提供 `Data Ready`

文档强调这些额外引脚在 SPI-FD 模式下是必须的。

### 15.5 ESP32-C5 Slave 的编译命令

配置完成后，直接编译：

```bash
idf.py build
```

编译成功后，重点关注生成出来的固件文件。通常会在类似下面的位置：

```text
slave/build/network_adapter.bin
```

如果以后你们要做 OTA，这个文件也会被继续用到。

### 15.6 ESP32-C5 Slave 的烧录命令

通过串口烧录 `ESP32-C5`：

```bash
idf.py -p <C5串口号> flash monitor
```

例如：

```bash
idf.py -p COM6 flash monitor
```

如果只是先烧录、不急着看日志，也可以：

```bash
idf.py -p COM6 flash
```

### 15.7 Host 侧不是 ESP 工程，这一点要特别注意

虽然这一节重点讲 `ESP32-C5` 怎么编译烧录，但实际联调时一定要记住：

- 只有 C5 配成 `SPI Full-Duplex` 还不够
- 你的蓝牙音频 Host 芯片也必须实现与之匹配的 SPI Host 通信逻辑

也就是说，你们不能直接照搬仓库里“ESP 作为 Host”的示例去烧到 Host 芯片，而是要在 Host 侧自己完成或移植这些能力：

- SPI 主机驱动
- `Handshake` 输入处理
- `Data Ready` 输入处理
- `Reset` 输出控制
- ESP-Hosted host 侧协议适配
- RPC 请求与响应交互

而且 Host 侧必须和 C5 对齐这些内容：

- SPI 模式
- SPI 引脚定义
- `Handshake` 引脚
- `Data Ready` 引脚
- `Reset` 引脚
- SPI 时钟频率
- 校验设置

如果任意一项不一致，就很容易出现连不上、初始化失败、掉包等问题。

### 15.8 这个场景下最推荐的调试顺序

建议按下面顺序做：

1. 先只管把 `ESP32-C5` slave 编译通过并烧进去
2. 再确认非 ESP Host 侧的 SPI 主机逻辑已经按 `SPI Full-Duplex` 约束实现
3. 再一根一根核对 SPI 与额外 GPIO 接线
4. 首次联调时把 SPI 时钟设低一点
5. 跑通后再逐步提高频率

这是因为 `docs/spi_full_duplex.md` 和 `docs/troubleshooting.md` 都在强调：

- 首次 bring-up 时，先求稳定
- 不要一开始就把频率拉太高
- 如果出现 `drop pkt`，通常先怀疑信号完整性或时钟过高

### 15.9 常见故障表现

在“非 ESP Host + `ESP32-C5` Slave”的 SPI 场景下，最容易遇到这些问题：

#### 1. Host 侧日志提示连不上 Slave

这通常优先检查：

- transport 是否两边都选了 `SPI`
- GPIO 配置是否一致
- 实际接线是否一致
- `Reset / Handshake / Data Ready` 是否接对
- Host 侧是否真的按 ESP-Hosted SPI 时序在驱动总线

#### 2. SPI 有通信，但频繁掉包

如果日志里有类似 `drop pkt` 的报错，优先检查：

- SPI 时钟是否太高
- 杜邦线是否太长
- 接地是否稳定
- 是否启用了 checksum

文档里甚至建议一开始先用更低频率验证，再逐步往上调。

#### 3. Host 侧并没有真正完成协议适配

因为你们的 Host 不是 ESP，所以很多问题不一定出在 C5 固件上，而可能出在：

- SPI 事务长度处理
- 中断处理时机
- `Handshake / Data Ready` 逻辑
- 包头解析
- RPC 收发流程

也就是说，C5 烧进去只是第一步，Host 侧协议适配同样是主工作量。

#### 4. Slave 和 Host 代码版本或协议实现不一致

文档明确建议主从尽量使用同一版本的 `ESP-Hosted` 思路和实现，否则可能协议兼容性出问题。

### 15.10 一个可以直接照抄的最小流程

如果只看 `ESP32-C5` 这一端，最小流程可以简化为：

```bash
idf.py create-project-from-example "espressif/esp_hosted:slave"
cd slave
idf.py set-target esp32c5
idf.py menuconfig
```

然后在菜单中把 transport 设成：

```text
Example Configuration
  -> Bus Config in between Host and Co-processor
    -> Transport layer
      -> SPI Full-Duplex
```

接着编译和烧录：

```bash
idf.py build
idf.py -p <C5串口号> flash monitor
```

### 15.11 这一节的核心结论

对你们这个方案来说，`ESP32-C5` 的角色就是：

- `Slave`
- `无线协处理器`
- `通过 SPI 与非 ESP Host 通信`

它的编译烧录关键点只有三件事：

1. `set-target` 要选 `esp32c5`
2. `menuconfig` 里 transport 要选 `SPI Full-Duplex`
3. Host 侧 SPI 时序、GPIO 和协议适配必须与 C5 完全对齐

## 16. 非 ESP Host 需要实现哪些能力，才能和 ESP32-C5 跑通

这一节专门回答一个实际问题：

既然你们的 Host 是蓝牙音频芯片，不是 ESP，那么 Host 侧最少要自己做哪些东西，才能和 `ESP32-C5` 这个 slave 建立可工作的 ESP-Hosted 链路。

先说结论：至少要补齐三层能力：

- 硬件引脚层
- SPI 事务层
- ESP-Hosted 协议层

如果只把 SPI 总线打通，而没有把额外控制信号和包处理逻辑做完整，通常是跑不起来的。

### 16.1 第一层：硬件引脚层

根据 `docs/spi_full_duplex.md`，SPI Full-Duplex 模式下，Host 侧至少要具备这些信号：

- `MOSI`
- `MISO`
- `SCLK`
- `CS`
- `Reset`
- `Handshake`
- `Data Ready`
- `GND`

在你们这个架构里，方向关系可以这样理解：

- Host 输出：
  - `SCLK`
  - `CS`
  - `MOSI`
  - `Reset`
- Host 输入：
  - `MISO`
  - `Handshake`
  - `Data Ready`

这里最容易漏掉的是后面三个控制信号里的两个：

- `Handshake`
- `Data Ready`

这两个不是“可选优化”，文档语义里它们在 SPI-FD 下是必需的。

### 16.2 第二层：复位控制能力

Host 必须能控制 `ESP32-C5` 的 `EN` 或 `RST`。

作用是：

- Host 启动时主动复位 slave
- 让主从状态重新同步
- 异常时可重新拉起链路

如果 Host 侧完全没有这根控制线，很多初始化和异常恢复会很难处理。

所以在你们的 Host 驱动设计中，至少要有一个 `reset_slave()` 之类的动作。

### 16.3 第三层：GPIO 中断或轮询能力

文档里提到：

- `Handshake`
- `Data Ready`

在 Host 侧通常按中断来处理。

也就是说，非 ESP Host 至少需要做到其中一种：

1. 更推荐：GPIO 中断方式
2. 退一步：高频轮询方式

推荐中断方式的原因很简单：

- Host 可以在 slave 准备好事务时及时响应
- 更符合文档中的工作方式
- 事务时序更稳定

如果 Host 平台中断支持比较弱，也不是完全不能做，但轮询实现会更容易遇到时序和效率问题。

### 16.4 第四层：SPI 主机事务能力

Host 侧必须支持完整的 SPI 主机事务，而不是只支持简单的“发几个字节命令”。

文档里 SPI-FD 的关键点是：

- 一次事务里同时收和发
- Host 与 slave 同时交换数据
- 每次事务都有 TX buffer 和 RX buffer

而且文档里提到一个很关键的约束：

- 事务缓冲区按最大 `1600` 字节准备
- Host 最多可在一个事务中收发 `1600` 字节

这意味着你们的 Host SPI 驱动最好支持：

- 全双工事务
- 指定固定事务长度
- 收发同时进行
- 连续事务调度

如果 Host 的 SPI 外设或驱动模型只适合短控制命令，而不适合这种固定长度的数据交换，就要在驱动层补适配。

### 16.5 第五层：遵守 Handshake / Data Ready 时序

这是最核心的一层。

根据 `docs/spi_full_duplex.md`，你可以把 Host 侧逻辑理解成：

1. slave 先准备好 SPI 事务
2. slave 拉高 `Handshake`，表示“可以开始事务了”
3. 如果 slave 还有数据要发给 Host，会同时拉高 `Data Ready`
4. Host 在确认条件满足后发起 SPI 事务
5. 事务结束后，slave 拉低相关信号

对应到 Host 实现上，最少要做到：

- Host 不能在 `Handshake` 无效时乱发事务
- Host 要能判断“当前是否该发起一次 SPI 交换”
- Host 有待发送数据时，可以在 slave ready 后送出
- Host 无待发送数据，但 `Data Ready` 拉高时，也要能主动把 slave 的数据读出来

可以把这个判断简化成一句话：

- `Handshake` 表示“slave 已准备好一次事务”
- `Data Ready` 表示“这次事务里 slave 有有效数据要给你”

### 16.6 第六层：固定包长事务 + 包头解析

根 README 说明，ESP-Hosted 每帧前面都有固定格式的包头。

至少会涉及这些头字段：

- `if_type`
- `if_num`
- `flags`
- `len`
- `offset`
- `checksum`
- `seq_num`

这意味着 Host 侧做完 SPI 收发之后，不能把收到的 1600 字节原样丢掉，而要进一步：

1. 解析包头
2. 识别 payload 的真实长度
3. 判断这是控制帧、数据帧、HCI 帧还是测试帧
4. 再把数据交给上层处理

文档里列出的接口类型包括：

- `ESP_STA_IF`
- `ESP_AP_IF`
- `ESP_SERIAL_IF`
- `ESP_HCI_IF`
- `ESP_PRIV_IF`
- `ESP_TEST_IF`

对你们来说，最关键的通常是：

- `ESP_SERIAL_IF`
  - RPC 控制消息
- `ESP_STA_IF` / `ESP_AP_IF`
  - 网络数据
- `ESP_HCI_IF`
  - 蓝牙相关数据

### 16.7 第七层：RPC 请求/响应机制

ESP-Hosted 不是简单 SPI 数据透传，它有明确的 RPC 请求/响应机制。

`docs/implemented_rpcs.md` 里列出了很多已实现命令，比如：

- `WifiInit`
- `WifiStart`
- `WifiConnect`
- `WifiScanStart`
- `WifiGetMode`
- `GetCoprocessorFwVersion`
- `CustomRpc`

这意味着 Host 侧至少要有一套机制去完成：

1. 组装 RPC 请求
2. 发给 `ESP32-C5`
3. 等待响应
4. 把响应结果映射成 Host 自己的接口

如果你们最终只想用一小部分功能，也不代表 Host 可以完全不做 RPC 层。至少基础初始化、状态获取、功能控制都要经过它。

### 16.8 第八层：异步事件接收机制

除了请求-响应，slave 还会主动给 Host 发事件。

文档列出的事件包括：

- `ESPInit`
- `Heartbeat`
- `StaScanDone`
- `StaConnected`
- `StaDisconnected`
- 其他 Wi‑Fi / DPP / 自定义事件

所以 Host 侧不能只会“我发一条命令，等一个返回”，还要能：

- 在后台接收异步上报
- 把事件分发到 Host 自己的软件框架

如果没有这一层，很多 Wi‑Fi 状态变化将没法正常被上层感知。

### 16.9 第九层：最小 bring-up 建议

如果你们现在是第一次做非 ESP Host 适配，建议不要一上来就做全功能，而是按最小闭环推进：

1. 先把 `ESP32-C5` slave 烧好，SPI-FD 配好
2. Host 先只实现：
   - `Reset`
   - `Handshake`
   - `Data Ready`
   - 基础 SPI 1600 字节事务
3. 再实现包头解析
4. 再先打通最少几个 RPC：
   - 读取版本
   - 初始化
   - 启动 Wi‑Fi
5. 再处理异步事件
6. 最后再接入完整业务流

这样做的好处是，问题定位会清楚很多。

### 16.10 第十层：最值得先验证的点

如果你们现在要做排期或拆任务，我建议优先验证这几个最关键点：

#### 1. Host 的 SPI 外设能否稳定跑全双工固定长度事务

这是底座。

#### 2. Host 能否可靠处理中断或轮询 `Handshake / Data Ready`

这是时序基础。

#### 3. Host 是否能正确解析 ESP-Hosted 帧头

这是协议入口。

#### 4. Host 是否能完成至少一个最简单 RPC 闭环

例如：

- 发送一个简单请求
- 收到对应响应
- 确认整个收发和解析通路完整

#### 5. Host 是否能接收 slave 主动事件

这是系统从“能通”走向“能用”的关键一步。

### 16.11 对你们项目最现实的理解

对于“蓝牙音频芯片 Host + `ESP32-C5` 无线协处理器”这个项目来说：

- `ESP32-C5` 编译烧录并不复杂
- 真正的核心工作量更可能在 Host 侧移植

也就是说，这个仓库能直接提供给你们的主要是：

- slave 固件
- SPI-FD 工作方式
- 包头定义
- RPC 命令集合
- 参考时序

而你们需要自己完成的，主要是：

- 非 ESP Host 的底层驱动
- Host 侧协议接入
- Host 软件框架与 ESP-Hosted 的桥接

### 16.12 这一节的核心结论

非 ESP Host 如果想和 `ESP32-C5` 跑通 ESP-Hosted SPI，最少要自己实现：

1. `Reset / Handshake / Data Ready` 这些 GPIO 控制与采样
2. 全双工 SPI 固定长度事务
3. ESP-Hosted 包头解析
4. RPC 请求/响应收发
5. 异步事件接收与分发

少了其中任意一个环节，通常都很难真正把系统跑通。

## 17. 针对你们当前场景的理解：蓝牙音频芯片通过 SPI 连接 ESP32-C5，用热点传录音文件

这一节专门对应你们现在的目标场景：

- 主控是一颗蓝牙音频芯片
- 主控通过 `SPI` 与 `ESP32-C5` 通信
- `ESP32-C5` 作为无线协处理器
- 主要目标不是普通联网，而是利用 `ESP32-C5` 开热点
- 让手机、电脑或平板连上这个热点后，在局域网里拿到录音文件

这个场景和“普通把 ESP 当联网模组”相比，更像是一个“局域网文件传输设备”。

### 17.1 这个场景下，ESP32-C5 扮演什么角色

在你们这个系统里，`ESP32-C5` 最准确的角色不是：

- 纯透明 SPI 数据转发器
- 只搬运 Wi‑Fi 信号的中继

而是：

- `Wi‑Fi 无线协处理器`
- `SoftAP 提供者`
- `局域网通信入口`

也就是说，`ESP32-C5` 这边会负责：

- 打开热点
- 允许手机或电脑连接
- 参与网络数据收发
- 承担 Wi‑Fi 相关控制和协议能力

所以它不是只做“搬运”，而是实实在在在跑 Wi‑Fi 相关功能。

### 17.2 蓝牙音频芯片 Host 扮演什么角色

你们的蓝牙音频芯片更像是业务主控，主要负责：

- 录音采集
- 音频文件缓存或存储
- 文件管理
- 触发传输
- 告诉 `ESP32-C5` 要传哪些数据

也就是说，录音业务本身在 Host 上，Wi‑Fi 接入能力在 `ESP32-C5` 上。

### 17.3 这个场景下，数据是怎么流动的

可以把整个链路理解成下面这样：

1. 蓝牙音频芯片先产生录音文件
2. 录音文件保存在 Host 一侧
3. Host 通过 `SPI` 与 `ESP32-C5` 交互
4. `ESP32-C5` 打开热点，允许终端连接
5. 终端通过局域网访问文件传输服务
6. 文件数据在 Host 和 `ESP32-C5` 之间通过 `SPI` 交换
7. `ESP32-C5` 再通过 Wi‑Fi 发给客户端

所以它本质上是一个“双通道系统”：

- 业务数据通道：Host <-> `ESP32-C5` 走 `SPI`
- 无线数据通道：`ESP32-C5` <-> 手机/电脑 走 `Wi‑Fi`

### 17.4 “ESP32-C5 还跑不跑网络协议”这个问题，在你们场景里的答案

答案是：`要跑，而且是关键角色。`

在你们这个方案里，`ESP32-C5` 至少会承担：

- 热点管理
- 无线链路管理
- 网络收发
- 一部分网络协议处理

因此它不是一个纯透传芯片。

更贴切的理解应该是：

- Host 负责“录音文件是什么”
- `ESP32-C5` 负责“怎么通过 Wi‑Fi 让别人拿到这个文件”

### 17.5 这个场景下有两种常见架构

对“通过热点传录音文件”这个目标，通常可以有两种实现思路。

#### 架构 A：文件服务主要放在 ESP32-C5

这种方式下：

- `ESP32-C5` 开热点
- `ESP32-C5` 运行一个简单的文件传输服务
- 比如 HTTP 下载服务、简单 TCP 传输服务，或者自定义文件服务
- Host 通过 SPI 把录音数据喂给 `ESP32-C5`
- 客户端直接连热点访问 `ESP32-C5`

它的优点是：

- 客户端访问路径清晰
- Wi‑Fi 和文件服务都集中在 `ESP32-C5`
- Host 侧网络协议压力较小

它的代价是：

- `ESP32-C5` 侧要承担更多上层服务逻辑
- Host 和 `ESP32-C5` 之间要定义好文件读写接口

#### 架构 B：Host 负责更多业务协议，ESP32-C5 更偏网络协处理器

这种方式下：

- `ESP32-C5` 主要负责提供 Wi‑Fi 联网能力
- Host 侧主导文件传输流程
- `ESP32-C5` 更像“把 Host 的网络数据送到空口”

它的优点是：

- 业务控制权更多留在 Host
- 如果 Host 已经有现成文件协议框架，可能更容易复用

它的代价是：

- 非 ESP Host 的协议适配工作量更大
- Host 侧需要承担更多网络相关逻辑

### 17.6 就你们这个项目而言，更推荐哪种思路

如果你们当前的核心目标是：

- 尽快把“录音文件通过热点传出去”跑通
- 而不是做一个非常复杂的统一网络协议框架

那我更推荐优先考虑：

- `架构 A：文件服务主要放在 ESP32-C5`

原因很直接：

- `ESP32-C5` 天然更适合承担热点和局域网服务
- Host 是蓝牙音频芯片，不是 ESP，网络协议开发成本通常更高
- 把热点访问入口、文件下载入口放在 `ESP32-C5`，系统边界更清晰

简单说就是：

- Host 负责“提供录音内容”
- `ESP32-C5` 负责“把录音内容通过热点服务出去”

这通常是工程实现上更稳妥的分工。

### 17.7 一个很实用的系统分工方式

如果按推荐方式做，可以这样拆：

#### Host 侧负责

- 录音开始/停止
- 文件分段存储
- 文件索引管理
- 给 `ESP32-C5` 提供：
  - 文件列表
  - 文件大小
  - 文件内容读取接口

#### ESP32-C5 侧负责

- 创建 `SoftAP`
- 管理局域网连接
- 提供文件访问接口
- 把客户端请求转换成对 Host 的读取请求
- 再通过 Wi‑Fi 回传给客户端

### 17.8 对客户端来说，会看到什么

如果按这个方案实现，用户侧可能就是这样的体验：

1. 手机或电脑连接 `ESP32-C5` 开出来的热点
2. 在浏览器打开一个本地地址
3. 看到录音文件列表
4. 点击下载或播放

这类交互对于调试和产品化都比较友好。

### 17.9 这个场景下最值得优先验证的能力

如果你们要做阶段性验证，我建议按下面顺序验证：

1. `ESP32-C5` 能稳定作为 SPI slave 跑起来
2. 非 ESP Host 能和 `ESP32-C5` 打通 SPI 基础通信
3. `ESP32-C5` 能成功开 `SoftAP`
4. 客户端能连接热点
5. Host 能通过 SPI 传递一小段测试数据给 `ESP32-C5`
6. 客户端能通过局域网拿到这段测试数据
7. 再替换成真正录音文件

这个顺序能把问题拆得很清楚。

### 17.10 对这个场景的一句话总结

在你们的方案里，`ESP32-C5` 不是单纯数据中转，而是“负责热点和局域网传输的无线协处理器”；蓝牙音频芯片负责录音业务，双方通过 `SPI` 配合，把录音文件通过 `ESP32-C5` 的热点传给局域网客户端。

## 18. 这个场景下建议的最小实现方案

这一节不再只讲概念，而是给一个更适合落地的“最小可运行方案”。

目标很明确：

- 蓝牙音频芯片负责录音
- `ESP32-C5` 负责开热点
- 手机或电脑连上热点后，能下载录音文件
- 先追求“能跑通”，再追求“更优雅、更高性能”

### 18.1 推荐的总体方案

对你们这个项目，我建议优先采用下面这个分工：

- Host 侧：
  - 管理录音文件
  - 提供文件元数据
  - 提供文件内容读取能力
- `ESP32-C5` 侧：
  - 运行 `SoftAP`
  - 提供一个简单的文件下载服务
  - 通过 SPI 从 Host 取文件内容
  - 回传给局域网客户端

这个方案的关键词可以概括成：

- `Host 管文件`
- `C5 管网络`

### 18.2 为什么推荐这种方案

因为这个方案对你们最现实：

- Host 不是 ESP，网络栈开发成本更高
- `ESP32-C5` 更适合承担热点和局域网服务
- 系统边界更清楚
- 调试路径更短

如果反过来让 Host 承担更多网络协议组织工作，非 ESP 侧的开发成本通常会明显上升。

### 18.3 最小功能目标

建议第一版只实现下面这几个最小能力：

1. `ESP32-C5` 开一个固定热点
2. 客户端能连接热点
3. `ESP32-C5` 提供一个最简单的下载接口
4. Host 能通过 SPI 向 `ESP32-C5` 提供一个测试文件
5. 客户端能成功下载这个测试文件

只要这五步跑通，后续再扩展多文件、分页列表、断点续传都容易得多。

### 18.4 第一版最推荐的访问方式

我建议第一版优先用：

- `HTTP`

而不是一上来就做复杂私有协议。

原因很简单：

- 手机、电脑直接浏览器可测
- 调试成本最低
- 方便抓包
- 便于快速验证链路

所以第一版可以做成这样：

- `ESP32-C5` 开热点，例如 `Recorder_AP`
- `ESP32-C5` 启一个简单 HTTP 服务
- 提供一个下载 URL，例如：
  - `/recordings`
  - `/recordings/latest`
  - `/recordings/<id>`

### 18.5 最小接口建议

为了让 Host 和 `ESP32-C5` 的配合尽可能简单，第一版建议只定义几类最小接口。

#### Host -> ESP32-C5 侧需要支持的逻辑

从 `ESP32-C5` 的视角看，Host 最少要能回答这些问题：

1. 当前有没有可导出的录音文件
2. 最新录音文件的名字是什么
3. 文件总长度是多少
4. 从某个偏移开始，能读出一段数据吗

也就是说，Host 最小只要能提供：

- `get_latest_file_info()`
- `read_file(offset, length)`

就足够支撑第一版下载。

#### ESP32-C5 -> 客户端 侧建议暴露的 HTTP 接口

第一版甚至可以只做一个接口：

- `GET /latest`

功能就是：

- 获取最新录音文件
- 直接下载二进制内容

如果你们想略微友好一点，再加一个：

- `GET /info`

返回：

- 文件名
- 文件大小
- 时间戳

这样浏览器或 App 侧会更容易做。

### 18.6 推荐的数据流设计

最小方案下，推荐数据流如下：

1. 客户端连上 `ESP32-C5` 热点
2. 客户端请求 `GET /latest`
3. `ESP32-C5` 收到请求
4. `ESP32-C5` 通过 SPI 向 Host 请求文件信息
5. Host 返回：
   - 文件名
   - 文件大小
6. `ESP32-C5` 开始向 Host 分块读取文件内容
7. Host 每次通过 SPI 返回一段数据
8. `ESP32-C5` 边收边通过 HTTP 发送给客户端

这个模式的优点是：

- Host 不需要一次性把整个文件推给 `ESP32-C5`
- `ESP32-C5` 不需要完整缓存整个文件
- 更适合录音文件这种可能较大的内容

### 18.7 推荐按“分块读取”实现

第一版就建议按分块来做，而不是整文件一次搬运。

推荐原因：

- 内存压力更小
- 更容易适配大文件
- 网络发送和 SPI 读取更容易流水化

比如可以按固定块大小读取：

- `1 KB`
- `2 KB`
- `4 KB`

具体值取决于：

- Host SPI 吞吐
- `ESP32-C5` 可用 RAM
- HTTP 发送缓冲策略

第一版建议从保守值开始，不要一开始就把块做太大。

### 18.8 最小 SPI 业务接口建议

如果你们要自定义 Host 与 `ESP32-C5` 之间的上层业务协议，我建议第一版只设计下面几种命令。

#### 1. 获取文件信息

Host 返回：

- 文件是否存在
- 文件名
- 文件长度
- 时间戳

#### 2. 读取文件块

输入参数：

- 文件标识
- 偏移
- 长度

输出内容：

- 实际读取长度
- 数据块内容

#### 3. 获取状态

比如：

- 是否正在录音
- 文件是否可读
- 是否忙碌

这样就够支撑第一版 HTTP 下载。

### 18.9 第一版先不要急着做的功能

为了尽快跑通，建议先不要一上来就做这些：

- 多文件复杂列表
- 文件删除
- 在线播放
- 断点续传
- 上传认证
- App 私有协议
- 热点和蓝牙复杂联动

这些都可以等最小下载闭环跑通后再加。

### 18.10 推荐的阶段划分

建议按四个阶段推进。

#### 阶段 1：链路打通

目标：

- `ESP32-C5` slave 跑起来
- 非 ESP Host 能稳定用 SPI 通信
- `Reset / Handshake / Data Ready` 正常

验收标准：

- 能完成最简单的 Host-C5 请求/响应

#### 阶段 2：热点打通

目标：

- `ESP32-C5` 能开热点
- 手机/电脑能连上

验收标准：

- 客户端稳定拿到 IP

#### 阶段 3：测试数据下载

目标：

- `ESP32-C5` HTTP 服务起来
- Host 提供一段固定测试数据
- 客户端能下载成功

验收标准：

- 浏览器访问 `GET /latest` 能拿到完整测试文件

#### 阶段 4：接入真实录音文件

目标：

- Host 改成真实录音文件读取
- 支持较大文件的分块读取和下载

验收标准：

- 客户端能稳定下载真实录音文件

### 18.11 最值得优先验证的技术风险

如果你们要尽快判断项目风险，我建议最先验证这几个点：

#### 1. 非 ESP Host 的 SPI 模型是否适合这种持续分块交互

这是整个方案的底层前提。

#### 2. Host 和 `ESP32-C5` 之间的持续分块读取吞吐是否够

如果录音文件比较大，吞吐会直接影响下载体验。

#### 3. `ESP32-C5` 在热点 + HTTP 服务 + SPI 取数同时运行时是否稳定

这是系统稳定性的关键点。

#### 4. Host 侧文件系统或存储读出能力是否容易按偏移分块读取

如果 Host 文件读接口很别扭，整体方案复杂度会明显上升。

### 18.12 一个建议的最小用户体验

如果第一版偏产品验证，我建议把体验做得尽量简单：

1. 设备上电后，`ESP32-C5` 自动开热点
2. 用户连接热点
3. 浏览器打开固定地址，例如：
   - `http://192.168.4.1/latest`
4. 直接下载最新录音文件

这个流程对内部联调、演示和问题复现都很友好。

### 18.13 第二版再考虑什么

当第一版闭环跑通后，再考虑这些增强项会更合适：

- 录音文件列表页
- 多文件下载
- 文件大小和时间显示
- 简单网页 UI
- 文件校验
- 下载进度
- 边录边传
- 客户端认证

### 18.14 这一节的核心结论

对你们当前场景，最合适的最小实现方案通常是：

- 蓝牙音频芯片保存录音文件
- `ESP32-C5` 开热点并提供简单 HTTP 下载服务
- `ESP32-C5` 通过 SPI 分块向 Host 取录音数据
- 客户端通过局域网直接从 `ESP32-C5` 下载文件

这样能最快把”录音文件通过热点传输”这个核心目标跑通。

## 19. 和以前用 AT 指令固件有什么不一样

### 19.1 先回顾”AT 指令固件”是怎么工作的

你之前用 STM32 控制 ESP32 的方式，是这套经典模式：

1. 从乐鑫官网下载一份**预编译好的 AT 固件**（`.bin` 文件）
2. 用烧录工具直接把它写进 ESP32，不需要自己编译
3. STM32 通过 **UART 串口**给 ESP32 发**文本命令**，例如：
   - `AT+CWJAP=”MyWiFi”,”password”` — 连 Wi‑Fi
   - `AT+MQTTCONN=...` — 连 MQTT 服务器
4. ESP32 执行命令，回复 `OK` 或数据

这种方式的本质是：**ESP32 是一个黑盒命令解释器，主控只需要发字符串。**

---

### 19.2 这个仓库和 AT 固件有什么根本区别

**核心区别只有一句话：**

> AT 固件 = 主控发文本命令，ESP 解释执行。
> ESP-Hosted-MCU = 主控调用 API 函数，这些调用被序列化成二进制包通过 SPI/SDIO 发给 ESP 执行。

展开来说，两者在每一个环节上都不同：

| 对比维度 | AT 固件方式 | ESP-Hosted-MCU 方式 |
|---|---|---|
| 固件来源 | 官网下载预编译包，直接烧录 | 从这个 git 仓库编译，自己烧录 |
| 主控与 ESP 的交互协议 | 文本 AT 命令（`AT+CWJAP=...`） | 二进制 RPC（protobuf 编码） |
| 通信总线 | 通常只用 UART | SPI / SDIO / UART 均可 |
| 主控侧 API | 发字符串、解析回显 | 调用标准 ESP-IDF Wi‑Fi 函数 |
| 可定制程度 | AT 命令集固定，无法扩展 | 开源，可以增加自定义 RPC 命令 |
| 事件处理 | 轮询回显或 URC 字符串 | 标准事件回调（结构化数据） |
| 吞吐能力 | UART 上限（通常 1 Mbps 以下） | SPI/SDIO 可达 20~79 Mbps |

---

### 19.3 烧录完之后，主控还是发 AT 指令吗

**不是。**

这是最容易误解的地方。

烧录 ESP-Hosted-MCU slave 固件之后，ESP32/C5/C6 不再是一个 AT 指令解释器。它变成了一个 **RPC 服务端**。

主控和它的交互方式完全变了：

- **AT 固件：** STM32 → UART → `AT+CWJAP=”ssid”,”pass”\r\n` → ESP32 回 `OK`
- **ESP-Hosted-MCU：** Host → SPI → `[protobuf 编码的 WifiConnect 请求包]` → ESP32 执行并回 `[protobuf 编码的响应包]`

主控侧写的不是字符串拼接逻辑，而是调用 API 函数，例如：

```c
esp_wifi_connect();        // 这个函数调用会被 ESP-Hosted 打包成 RPC 发给 ESP
```

这个函数在 Host 端是一个”弱函数”实现（`esp_wifi_weak.c`），内部把调用序列化成 protobuf，通过 SPI 发出去，再把响应反序列化回来返回给你。

对应用代码来说，感觉像是在本地调用 Wi‑Fi API，但实际上背后是 RPC。

---

### 19.4 自由度更高在哪里

比 AT 固件方式自由度确实高很多，主要体现在三点：

#### 1. API 更完整

AT 命令集是一个有限的文本接口，乐鑫实现了哪些就只有哪些。

ESP-Hosted-MCU 暴露的是完整的 **ESP-IDF Wi‑Fi API**，包括：

- 完整的扫描接口（获取结构化 AP 列表，而不是解析字符串）
- 细粒度的连接控制（可以设置 BSSID、信道、协议等）
- 所有标准 Wi‑Fi 事件（`WIFI_EVENT_STA_CONNECTED`、`IP_EVENT_STA_GOT_IP` 等以结构体形式到达）

#### 2. 可以扩展自定义命令

AT 固件不能加新命令（除非用乐鑫定制版，但那是另一套开发流程）。

ESP-Hosted-MCU 是开源的，你可以在 `.proto` 文件里加新的 RPC 消息类型，在 slave 侧加对应处理逻辑，Host 侧加对应调用——相当于给这套框架打桩加功能。

#### 3. 传输带宽更高

AT 固件通常跑在 UART 上，实测吞吐通常只有几百 kbps 到 1 Mbps 量级。

ESP-Hosted-MCU 走 SPI 可以达到 20+ Mbps，走 SDIO 可以到 50~79 Mbps，适合真正的数据传输场景。

---

### 19.5 那 AT 固件还有没有价值

有。两种方式不是谁替代谁，而是适合不同场景：

| 场景 | 更适合哪种方式 |
|---|---|
| 快速验证联网功能，Host 侧只需要简单脚本 | AT 固件 |
| Host 是资源极为有限的单片机，不想移植驱动 | AT 固件 |
| 只需要 MQTT / HTTP 联网，功能固定 | AT 固件 |
| Host 需要完整 Wi‑Fi 事件、扫描结果等结构化数据 | ESP-Hosted-MCU |
| 需要 SPI/SDIO 高带宽传输（如传文件、传音频流） | ESP-Hosted-MCU |
| Host 不是 ESP，但想复用 ESP-IDF Wi‑Fi API | ESP-Hosted-MCU |
| 需要自定义扩展命令 | ESP-Hosted-MCU |

你们现在的场景——蓝牙音频芯片通过 SPI 连接 ESP32-C5 传录音文件——显然属于后者。AT 固件走 UART 的带宽完全不够用，而且 AT 命令集也没有”Host 主动传数据给 ESP，再由 ESP 通过热点发出去”的能力。

---

### 19.6 一句话总结这两种方式的本质差异

- **AT 固件：** ESP32 是一个受主控文本命令驱动的”网络模组”，主控发什么命令，ESP 就干什么，主控不需要了解 Wi‑Fi 内部细节。

- **ESP-Hosted-MCU：** ESP32/C5/C6 是一个 Wi‑Fi/蓝牙协处理器，主控通过 SPI/SDIO 调用完整的 Wi‑Fi API，拥有更高的控制权、更丰富的事件信息和更高的传输带宽，但主控侧需要移植对应的 Host 驱动。
