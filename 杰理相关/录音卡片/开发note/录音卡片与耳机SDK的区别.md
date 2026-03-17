# 录音卡片与耳机SDK的区别

录音卡片使用的是耳机的SDK，应该是配置上来区分两者的区别。

## 可视化工具的配置差异

- TWS配置中的配对码不一样。公版是0XFFFF，录音卡片是0x1230.
- 录音卡片打开了串口产测，公版没有。

## 代码上配置的差别

**701公版（TWS耳机）和driver（录音卡片）使用的是同一套核心SDK，产品功能的区分完全通过代码中的配置（宏定义和JSON配置文件）来实现。**

以下是主要的配置差异点：

### 1. SDK预处理器定义差异（c_cpp_properties.json）

- **701公版**启用了多项TWS/音频增强功能：
  - `EXPORT_PLATFORM_ANC_ENABLE=1`（主动降噪）
  - `EXPORT_LE_AUDIO_SUPPORT=1`（LE音频）
  - `EXPORT_PLATFORM_AUDIO_SMART_VOICE_ENABLE=1`（离线语音）
  - `EXPORT_SOMATOSENSORY_ENABLE=1`（头部姿态检测）
- **driver**则可能包含`CONFIG_OS_AFFINITY_ENABLE=0`等配置，且包含lwip网络相关路径。

### 2. 核心硬件配置差异（sdk_config.h）

| 配置项                          | 701公版 (TWS耳机) | driver (录音卡片) | 说明              |
| :------------------------------ | :---------------- | :---------------- | :---------------- |
| `TCFG_JLSTREAM_EFFICIENT_MODE`  | 0（定义）         | **未定义**        | TWS音频流高效模式 |
| `TCFG_SDX_CAN_OPERATE_MMC_CARD` | 0                 | 0（但存在定义）   | SD卡支持MMC卡     |
| `TCFG_ANC_BOX_ENABLE`           | 0                 | **1**             | ANC串口调试/产测  |
| `TCFG_AUDIO_DUT_ENABLE`         | 0                 | **1**             | 音频/通话产测     |
| 音频解码器                      | 启用SBC、MSBC等   | 启用**OPUS**等    | 编解码器选择不同  |
| `TCFG_AUDIO_ADC_ENABLE`         | 启用3路ADC配置    | **未启用**ADC配置 | 麦克风输入配置    |

### 3. JSON配置文件差异

- **音频配置.json**：
  - 701公版：配置项为`PA_ISEL0`、`PA_ISEL1`（双路PA电流档位）
  - driver：合并为**`电流档位`**，选项简化（LEVEL0-LEVEL6）
- **蓝牙配置.json**：
  - 701公版：SBC比特池=43，包含**彩屏仓JL_SBOX**选项和说明
  - driver：SBC比特池=**38**，移除彩屏仓选项，增加**“支持连接两路RCSP”**、**“三方协议轻量化”**开关
- **升级配置.json**：
  - 701公版：完整的**“双备份”**组件，支持边升级边播歌、压缩双备份等高级功能
  - driver：简化版，仅保留基本双备份开关

### 4. SDK模块差异

- **701公版**包含`apps/common/ai_audio/`模块（AI音频处理）
- **driver**不包含此模块，但包含更多网络（lwip）相关文件

### 结论

这些差异完全是通过**条件编译（宏定义）**和**配置文件（JSON/sdk_config.h）**控制的，而非不同的SDK代码。这表明：

1. **同一代码库**：两个项目共用相同的SDK源代码
2. **配置驱动**：产品功能通过编译时配置开关决定
3. **模块化设计**：通过宏定义包含/排除特定功能模块
4. **硬件抽象**：同一套代码可适配不同硬件（TWS耳机 vs 录音卡片）

这种设计允许厂家使用同一套SDK快速衍生产品线，只需修改配置即可生成不同功能的固件。

