# MAC搭建环境指南

## 问题回顾

在 M4 Mac mini 上安装乐鑫 ESP-IDF 开发环境时，EIM（ESP-IDF Installation Manager）无法识别已安装的 Python 3.12，持续报 "Python 3.9.6 is not supported"。

### 踩坑路径

| 尝试                                | 结果               | 原因                                                         |
| :---------------------------------- | :----------------- | :----------------------------------------------------------- |
| 官网下载 `.dmg` 安装 GUI 版 EIM     | ❌ 检测失败         | `.app` 不读取 `.zshrc`，PATH 里没有 pyenv                    |
| `pyenv` 安装 Python 3.12.3          | 终端正常，EIM 不认 | 同上，GUI 应用不走 shell 初始化                              |
| `brew install python@3.12` 独立安装 | 终端正常，EIM 不认 | GUI 版 EIM 的 PATH 是系统默认的，没把 `/opt/homebrew/bin` 放前面 |
| `brew install --cask eim-gui`       | ❌ 同样检测失败     | 本质还是 `.app`，问题一样                                    |
| `brew install eim`（CLI 版）        | ✅ 成功             | 命令行工具继承终端环境，正确读取 pyenv/Homebrew 的 Python    |

### 关键教训

1. **macOS 的 `.app` 与终端是两个世界**：图形应用不读取 `.zshrc`、`.bash_profile` 等 shell 配置，pyenv 的 PATH 注入对它们无效
2. **PATH 优先级只在终端生效**：即使把 `/opt/homebrew/bin` 放最前面，GUI 应用启动时看到的仍是系统默认 PATH
3. **CLI 工具才是正确选择**：命令行版本 `eim` 继承当前 shell 环境，能正确识别 pyenv 或 Homebrew 管理的 Python
4. **不需要重复安装 Python**：pyenv 的 3.12.3 和 Homebrew 的 3.12 都可以，CLI 版 EIM 都能检测到

### 最终方案

```bash
# 1. 确保 pyenv 或 Homebrew Python 已安装（二选一即可）
pyenv install 3.12.3 && pyenv global 3.12.3
# 或
brew install python@3.12

# 2. 安装 CLI 版 EIM
brew tap espressif/eim
brew install eim

# 3. 直接使用
eim wizard   # 交互式安装 ESP-IDF
```

GUI 版 EIM 对 macOS 用户来说是个陷阱，CLI 版才是正确打开方式。

## 配置EIM

### 第一步：启动安装向导

```bash
eim wizard
```

这会进入交互式界面，引导你选择：

- **语言**：选 `cn`（中文）或 `en`
- **ESP-IDF 版本**：推荐 `5.2`（稳定版）或 `5.3`（较新）
- **目标芯片**：根据你的开发板选择
  - `esp32`（经典款）
  - `esp32s3`（AI 加速，推荐）
  - `esp32c3`（RISC-V，低成本）
- **安装路径**：默认 `~/esp` 即可

### 第二步：安装完成后激活环境

```bash
# 查看已安装版本
eim list

# 激活指定版本（替换为你实际安装的版本号）
eim select 5.2

# 验证
idf.py --version
```

### 第三步：配置 shell 自动激活（可选）

把下面加到 `~/.zshrc`，这样新终端自动加载 ESP-IDF：

```bash
# ESP-IDF 环境（根据实际安装路径调整）
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

使用时手动执行 `get_idf`，避免每个终端都加载（启动会变慢）。

