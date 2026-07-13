# HOGP BLE APP 键值下发最佳实践草案

> 适用范围：T2620 / RDX HOGP Profile v1 / 当前 `rdx_hogp_key_action` 实现。  
> 目标：定义 BLE APP 向固件下发 HOGP 按键配置时，最适合当前代码上限、后续也便于 VM 持久化和扩展的键值表达方式。正式协议要求 APP 向固件提交完整 4 键可配置 keymap：KEY2 到 KEY5。KEY1 保留给 HFP/录音采集指定功能。

## 1. 结论

BLE APP 不应下发 ASCII、Android KeyCode、iOS key enum 或 APP 私有键值。

推荐 BLE APP 直接下发 **标准 HID Keyboard Usage**，并用固定长度的 **Keyboard Action** 表达每个物理键的单键或组合键：

```text
modifier  +  usages[6]
```

其中：

- `modifier` 是 HID Keyboard modifier byte。
- `usages[6]` 是最多 6 个标准 HID Keyboard Usage ID。
- 未使用的 usage byte 必须补 `0x00`。
- 固件生成 HOGP Input Report 时，`reserved` byte 固定填 `0x00`，不由 APP 下发。

这和当前 HOGP Input Report 契约一致：

```text
[modifier, reserved, key1, key2, key3, key4, key5, key6]
```

## 2. 当前固件支持上限

当前代码支持的能力边界如下：

| 能力 | 当前上限 |
|---|---|
| 物理按键数量 | 固件内部当前有 5 个 KEY1 到 KEY5；APP 只配置 KEY2 到 KEY5 |
| 触发事件 | 只建议接入 `CLICK` |
| HOGP action 类型 | 只支持 Keyboard Report |
| Modifier | 1 byte，标准 HID modifier bit |
| 普通按键 usage | 最多 6 个 HID Keyboard Usage |
| Key release | 固件 click 后通过短 timer 自动发送全 0 report |
| Report ID | 不支持，Input Report payload 不带 Report ID |
| Consumer Control | 当前 Profile v1 不支持 |
| Macro / Layer / Hold-tap | 当前不支持 |
| VM 持久化 | 当前未完成，应作为 R5-B 单独实现 |

因此，BLE APP 侧应把每个物理键配置成一个 `Keyboard Action`，而不是一个模糊的“键值”。

## 3. 推荐数据模型

正式协议要求 APP **整表下发**。

也就是说，APP UI 可以只修改 KEY2/KEY3/KEY4/KEY5 中的某一个键，但提交给固件时必须下发 KEY2 到 KEY5 的完整 4 键 keymap。固件不把单键下发作为正式配置事务。

KEY1 不进入 APP keymap。KEY1 后期保留给 HFP/录音采集指定功能；当前 KEY1 三击模式切换也应继续保留为独立正式路径。

### 3.0 字段分层

不要求首版把所有字段都做进去。字段可以分成三层：

| 层级 | 字段 | 是否首版必需 | 说明 |
|---|---|---|---|
| 最小必需 | `modifier + usages[6]` | 必需 | 直接匹配当前固件 HOGP Keyboard Action 能力 |
| 建议保留 | `version` | 强烈建议 | 避免后续协议升级时 APP/固件无法区分格式 |
| 取决于下发方式 | `first_key` / `key_index` | 整表建议保留 `first_key=0x02` | 明确整表覆盖 KEY2 到 KEY5 |
| 可省略 | `usage_count` | 可省略 | 因为未用 usage 已规定补 0，固件可从 0 结尾推导 |
| 可省略 | `trigger` | 当前可省略 | 当前只支持 CLICK，长按/保持属于后续版本 |
| 可省略 | `action_type` | 当前可省略 | 当前只支持 keyboard；disabled 可用全 0 action 表达 |
| VM 专用 | `magic/length/crc` | BLE 下发可省略，VM 持久化建议必须有 | 用于 Flash/VM 数据完整性和回滚 |

如果目标是尽快让 APP 配置 HOGP 键值闭环，首版可以采用精简整表格式。

精简整表格式：

```text
version    : 1 byte   当前 0x01
first_key  : 1 byte   当前固定 0x02，表示从 KEY2 开始
key_count  : 1 byte   当前固定 0x04
keys[4]:
  modifier  : 1 byte
  usages[6] : 6 bytes
```

总长度：

```text
3 + 4 * 7 = 31 bytes
```

单键格式仅可作为调试或临时开发接口，不作为正式 APP 配置协议：

```text
version    : 1 byte
key_index  : 1 byte   2..5
modifier   : 1 byte
usages[6]  : 6 bytes
```

总长度：

```text
9 bytes
```

首版真正不能省的是：**使用 HID Usage 编码、modifier 独立表达、usages 固定 6 字节且未用补 0**。

### 3.1 单个按键配置记录

建议 BLE APP 和固件之间使用如下逻辑字段：

```text
version      : 1 byte   协议版本，当前为 0x01
key_index    : 1 byte   2..5
trigger      : 1 byte   当前只接受 0x01 = CLICK
action_type  : 1 byte   0x00 = DISABLED, 0x01 = KEYBOARD
modifier     : 1 byte   HID Keyboard modifier byte
usage_count  : 1 byte   0..6
usages[6]    : 6 bytes  HID Keyboard Usage ID，未用补 0x00
```

字段说明：

- `key_index` 使用 1-based 编号；正式 APP 配置只允许 KEY2 到 KEY5。
- `trigger` 当前只允许 `CLICK`，其他值保留。
- `action_type=DISABLED` 表示该物理键在 HOGP 模式下不输出按键。
- `action_type=KEYBOARD` 表示输出标准键盘 report。
- `usage_count=0` 时，`usages[6]` 必须全 0。
- `usage_count>0` 时，前 `usage_count` 个 usage 有效，其余必须为 0。

### 3.2 整表配置

正式协议要求 APP 以整表形式提交 KEY2 到 KEY5 的 4 个可配置键，而不是只提交单个 key：

```text
version      : 1 byte
op           : 1 byte   0x01 = APPLY_KEYMAP
first_key    : 1 byte   当前固定为 0x02
key_count    : 1 byte   当前固定为 4
reserved     : 1 byte   固定 0
keys[4]      : 4 个 Keyboard Action 记录，对应 KEY2 到 KEY5
crc          : 可选，建议 VM 持久化格式中必须有
```

要求整表提交的原因：

1. 当前固件 executor 的运行状态是 active keymap，现有 apply 入口是一次替换完整 keymap。
2. APP UI 可以只改 KEY2 到 KEY5 中的一个键，但提交时生成完整 4 键表。
3. KEY2 到 KEY5 的行为不会因为只下发单个 key 而变成未定义。
4. VM 持久化、读回确认、回滚都更简单，可以把整表当作一个配置事务。
5. 后续新增出厂默认表、用户表、临时表时，边界更清楚。

当前固件内部 `rdx_hogp_key_action` 仍按 5 个物理键建模。正式解析 APP 4 键表时，固件应展开为内部 5-entry keymap：

```text
internal KEY1 = reserved/disabled，不由 APP 配置
internal KEY2 = APP keys[0]
internal KEY3 = APP keys[1]
internal KEY4 = APP keys[2]
internal KEY5 = APP keys[3]
```

同时，`rdx_app` 的 HOGP CLICK 路由应排除 KEY1，避免 KEY1 被 HOGP executor 消费，确保后续 HFP/录音采集功能能接管 KEY1。

### 3.3 当前 custom 通道容量判断

当前代码中的 RDX custom 回调结构限制为：

```text
CUSTOM_CMD_MAX_LENGTH   = 30
CUSTOM_VALUE_MAX_LENGTH = 100
```

因此，调试阶段也可以整表下发，但必须采用紧凑格式。

精简整表二进制 payload：

```text
version      1 byte
first_key    1 byte   固定 0x02
key_count    1 byte   固定 0x04
keys[4]      4 * (modifier 1 byte + usages[6] 6 bytes)
```

总长度：

```text
1 + 1 + 1 + 4 * 7 = 31 bytes
```

如果通过字符串 custom 通道传输，建议把这 37 字节编码成连续 hex：

```text
31 bytes -> 62 hex chars
```

62 个 hex 字符可以放进当前 `CUSTOM_VALUE_MAX_LENGTH=100`，比 5 键整表更宽裕。

允许示例：

```text
*APP#custom#hogpmap#010204000400000000000028000000000001060000000000002A0000000000#
```

其中：

```text
hogpmap = cmd
后面的连续 hex = value
01 = version
02 = first_key，表示从 KEY2 开始
04 = key_count
后续 4 组，每组 7 字节：
  modifier + usages[6]
```

不建议使用下面这种每个 byte 都用 `#` 分隔的格式：

```text
*APP#custom#hogpmap#01#05#01#19#00#00#00#00#00#...
```

原因是整表 byte 转成 `#` 分隔文本后会浪费 value 空间，也会增加解析复杂度。

如果后续要加入 `trigger/action_type/usage_count/crc` 等扩展字段，建议切换到二进制协议，或先扩大 custom value buffer；不要在当前 100 字节 value 限制内硬塞冗长文本字段。

## 4. HID Usage 编码要求

APP 必须下发标准 HID Keyboard Usage ID。

不要下发：

- ASCII 码。
- Android KeyCode。
- iOS / macOS key enum。
- Web key code。
- APP 自定义 key id。

示例：

| 用户选择 | APP 应下发 |
|---|---|
| `A` | usage `0x04` |
| `P` | usage `0x13` |
| `V` | usage `0x19` |
| `Enter` | usage `0x28` |
| `Backspace` | usage `0x2A` |
| `Delete Forward` | usage `0x4C` |

例如，十进制 `112` 不能直接作为 HOGP 键值使用，除非协议明确它就是 HID Usage `0x70`。如果 APP 实际想表达字母 `P`，应下发 `0x13`，不是 ASCII `0x70`。

## 5. Modifier 编码要求

`modifier` 使用 HID Keyboard 标准 bit：

| Bit | 值 | 含义 |
|---|---:|---|
| bit0 | `0x01` | Left Ctrl |
| bit1 | `0x02` | Left Shift |
| bit2 | `0x04` | Left Alt |
| bit3 | `0x08` | Left GUI |
| bit4 | `0x10` | Right Ctrl |
| bit5 | `0x20` | Right Shift |
| bit6 | `0x40` | Right Alt |
| bit7 | `0x80` | Right GUI |

组合 modifier 直接按 bit 或起来。

## 6. 示例

### 6.1 KEY2 = A

```text
version     = 01
key_index   = 02
trigger     = 01
action_type = 01
modifier    = 00
usage_count = 01
usages      = 04 00 00 00 00 00
```

固件发送 HOGP report：

```text
00 00 04 00 00 00 00 00
```

### 6.2 KEY2 = Ctrl+V

```text
version     = 01
key_index   = 02
trigger     = 01
action_type = 01
modifier    = 01
usage_count = 01
usages      = 19 00 00 00 00 00
```

固件发送 HOGP report：

```text
01 00 19 00 00 00 00 00
```

### 6.3 KEY2 = Ctrl+Shift+V

```text
version     = 01
key_index   = 02
trigger     = 01
action_type = 01
modifier    = 03
usage_count = 01
usages      = 19 00 00 00 00 00
```

固件发送 HOGP report：

```text
03 00 19 00 00 00 00 00
```

### 6.4 KEY2 = Ctrl+Alt+Delete

```text
version     = 01
key_index   = 02
trigger     = 01
action_type = 01
modifier    = 05
usage_count = 01
usages      = 4C 00 00 00 00 00
```

固件发送 HOGP report：

```text
05 00 4C 00 00 00 00 00
```

### 6.5 KEY2 disabled

```text
version     = 01
key_index   = 02
trigger     = 01
action_type = 00
modifier    = 00
usage_count = 00
usages      = 00 00 00 00 00 00
```

固件不应发送业务 key-down report。实现上可以保持为全 0 action。

## 7. 如果继续使用 custom 字符串通道

如果当前阶段仍通过 RDX custom 字符串下发，建议使用固定字段、十六进制、无歧义的格式。

调试阶段可以保留单键配置示例：

```text
*APP#custom#hogpkey#01#01#01#01#00#01#19#00#00#00#00#00#
```

字段解释：

```text
hogpkey
version     = 01
key_index   = 02
trigger     = 01
action_type = 01
modifier    = 00
usage_count = 01
usages      = 19 00 00 00 00 00
```

不建议继续使用类似下面的格式：

```text
*APP#custom#setkey1#S#112#0#
```

原因：

- `S` 的含义不明确。
- `112` 的编码体系不明确。
- 缺少 version。
- 缺少 action type。
- 缺少 usage_count。
- 不支持完整 6 usage 表达。
- 后续扩展和持久化困难。

正式 APP 配置不应只发送单键命令。正式提交时，APP 必须发送完整 4 键表，也就是 KEY2 到 KEY5。单键命令最多用于 bring-up、日志验证或临时调试。

## 8. 固件侧校验规则

固件收到 APP keymap 后，应先完整校验，再 apply。

建议校验规则：

1. `version == 0x01`。
2. `first_key == 0x02`。
3. `key_count == 4`。
4. `key_index` 必须在 `2..5`。
5. `trigger` 当前必须为 `CLICK`。
6. `action_type` 当前只接受 `DISABLED` 或 `KEYBOARD`。
7. `usage_count <= 6`。
8. `usage_count=0` 时 usages 必须全 0。
9. `usage_count<N` 时，`usages[N..5]` 必须全 0。
10. `action_type=DISABLED` 时 modifier 必须为 0，usages 必须全 0。
11. 不允许 APP 下发 Report ID。
12. 不允许 APP 下发 reserved byte；固件固定补 0。
13. APP 不允许配置 KEY1。

通过校验后，固件转换成当前 RAM keymap：

```text
rdx_hogp_key_action_keymap_t
  version
  key_count
  keys[5]:
    modifiers
    usages[6]
```

然后调用：

```text
rdx_hogp_key_action_keymap_apply()
```

## 9. 持久化建议

持久化必须保存完整 4 键可配置 keymap，也就是 KEY2 到 KEY5，而不是保存单个 key 的零散命令。

推荐 VM blob：

```text
magic        : 4 bytes  "HKM1"
version      : 1 byte
first_key    : 1 byte   固定 0x02
key_count    : 1 byte   固定 4
record_len   : 1 byte
flags        : 1 byte
keys[5]      : 固定长度 records
crc16/crc32  : 建议必须有
```

启动加载流程：

```text
1. 读取 VM blob
2. 校验 magic/version/length/crc
3. 校验每个 key record
4. 转换为 RAM active keymap
5. 调用 rdx_hogp_key_action_keymap_apply()
6. 失败则加载出厂默认表或 disabled 表
```

持久化边界：

- VM 格式不要直接等同当前 RAM struct。
- VM 格式必须有独立 `magic/version/length/crc`。
- 测试 keymap 不应作为用户配置写入 VM。
- APP 写入成功后应支持读回确认。
- 写入失败时应继续使用旧 keymap，避免半配置状态。

## 10. 当前明确不支持的内容

以下内容不进入当前 HOGP Profile v1 键值下发协议：

1. Consumer Control / 多媒体键。
2. Report ID。
3. Mouse / pointer usage。
4. Macro delay。
5. Layer 状态机。
6. Hold-tap。
7. 长按、保持、抬起分别配置。
8. OS 专用键值编码。
9. APP 私有键值直通。

如后续需要这些能力，应通过 `action_type` 增加 Profile v2 或专项协议，不应塞进当前 Keyboard Action。

## 11. 推荐落地顺序

1. BLE APP 侧把 UI 选择统一转换为 HID Keyboard Usage。
2. BLE APP 侧按整表生成 4 个 Keyboard Action，对应 KEY2 到 KEY5；即使用户只改了一个键，也提交完整 4 键表。
3. 固件侧新增 custom 命令解析或二进制配置解析。
4. 固件侧校验后转换为 RAM keymap 并 apply。
5. 硬件验证 HOGP 模式下 KEY2 到 KEY5 输出是否符合 APP 配置。
6. 单独验证 KEY1 CLICK 不被 HOGP keymap 消费，后续可接入 HFP/录音采集指定功能。
7. 再增加 VM blob 持久化、读回和失败回滚。
8. 最后关闭或隔离 `RDX_HOGP_KEY_ACTION_TEST_ENABLE` 测试键表。

## 12. 最终建议

当前最佳实践是：

```text
APP 下发标准 HID Keyboard Action 整表：
4 个 APP 可配置键：KEY2 到 KEY5，每键 1 个 CLICK keyboard action，
每个 action = modifier byte + usages[6]。
```

这正好匹配当前固件支持上限，也最容易做 RAM apply、VM 持久化、读回确认和后续 Profile v2 扩展。

正式协议不采用单键替换作为配置事务。

KEY1 不进入 APP keymap，保留给 HFP/录音采集指定功能。
