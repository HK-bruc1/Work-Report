# 仓与耳机通过 BLE APP 的绑定逻辑

## 1. 核心结论

当前版本里，仓与耳机通过 BLE APP 的“绑定”本质上不是仓自己主动搜索耳机并完成一套独立判定，而是下面这条数据链路：

1. 仓通过 BLE 读特征向 APP 上报一个固定格式的 95 字节字符串。
2. APP 根据这个字符串中固定位置的字段，判断仓当前是否已经保存了耳机信息。
3. 用户在 APP 中点击绑定后，协议层会把耳机的关键数据回调给仓侧。
4. 仓侧用新的耳机信息覆盖本地 VM 中的记录，并立即重新拼装 BLE 读特征。
5. 冷启动时仓会再次从 VM 中读回这份耳机信息，并重新生成同样格式的 BLE 读特征。

所以，当前“是否已绑定”的核心依据是 BLE 读特征内容，而绑定结果的落点是 VM。

本文只讨论这条仓耳绑定链路，不讨论另一套设备账号 bound/unbound 机制。

## 2. 协议格式

仓侧 BLE 读特征当前上报的固定格式如下：

```text
role,Lmac,Rmac,Cmac,SN,label sn,wifi Mac
```

字段定义如下：

| 字段 | 长度 | 含义 |
|------|------|------|
| role | 1 | 角色标识，L=左耳，R=右耳，C=仓 |
| Lmac | 12 | 左耳 MAC，无冒号 |
| Rmac | 12 | 右耳 MAC，无冒号 |
| Cmac | 12 | 仓 MAC，无冒号 |
| SN | 24 | 客户认证码，耳机和仓一致 |
| label sn | 16 | 盒标 SN |
| wifi Mac | 12 | 仓自身 Wi-Fi MAC，无冒号 |

总长度固定为 95 字节：

```text
1 + 12 + 12 + 12 + 24 + 16 + 12 + 6 个逗号 = 95
```

其中字段位置固定不变。对仓产品来说，当前 role 固定为 C。

补充约定：

1. Rmac 仅用于 1V1 场景，当前实现里由左耳 MAC 派生得到。
2. 当 Wi-Fi 信息尚未就绪时，wifi Mac 使用 `000000000000` 作为合法占位值。

## 3. 当前代码中的字段来源

BLE 读特征由 `rdx_app_earphone_pack_readchardata()` 统一拼装，最终格式为：

```c
"C,%s,%s,%s,%s,%s,%s"
```

各字段的真实来源如下：

| 字段 | 来源 |
|------|------|
| role | 固定写死为 `C` |
| Lmac | `epInfo.ep_mac_str` |
| Rmac | 根据 `epInfo.ep_mac` 计算，最后一个字节加 1 |
| Cmac | 取值优先级为“flash 解析出的仓 BLE MAC -> 本机 BLE MAC 回退”；当前测试设备因 flash 无效，实际就是本机 BLE MAC |
| SN | `epInfo.sn` |
| label sn | 优先使用 `epInfo.label_sn`，否则退回 `rdx_auth_info.fac_sn` |
| wifi Mac | `xxp_uart_get_wifi_AP_info()` 返回的 Wi-Fi MAC；为空时填 `000000000000` |

这里要特别注意两点：

1. 仓侧真正持久化的是耳机信息三元组，不是整条 95 字节字符串。
2. `Cmac` 和 `wifi Mac` 都不是从耳机回调里直接写入 VM 的，而是在仓侧实时拼包时补出来的运行时字段。

## 4. flash 与 VM 的关系

这两套数据在字段长度上看起来很像，但语义并不相同，不能把它们理解成“同一份信息分别存了两遍”。

flash 中解析出来的是仓本机的认证信息，落在 `rdx_auth_info` 里，核心字段是：

1. `AuthKey`，24 字节
2. `ble_mac_hex / ble_mac_str`，仓自己的 BLE MAC
3. `fac_sn`，16 字节工厂盒标

VM 中保存的是绑定后耳机的信息，落在 `EarphoneInfo` 里，核心字段是：

1. `sn`，24 字节
2. `ep_mac / ep_mac_str`，耳机 MAC
3. `label_sn`，16 字节

两者之所以容易混淆，是因为它们都长得像 `24 + 12 + 16` 这一类字段组合，但当前业务语义其实是：

1. flash 负责提供仓侧自己的出厂认证和回退信息。
2. VM 负责保存当前绑定耳机的身份信息。
3. BLE 读特征不是只读 flash，也不是只读 VM，而是把两边数据按协议混合拼装出来。

当前混合规则可以概括为：

1. `Lmac`、`Rmac`、`SN` 主要来自 VM 中的 `EarphoneInfo`。
2. `Cmac` 的来源优先级是“flash 解析出的仓 BLE MAC -> 本机 BLE MAC 回退”；在当前测试设备上，由于 flash 授权区无效，实际取到的是本机 BLE MAC。
3. `label sn` 优先来自 VM 中绑定后的 `label_sn`；如果 VM 没有，再回退 flash 中的 `fac_sn`。
4. `wifi Mac` 来自运行时 Wi-Fi 信息，不在 flash 和 VM 中持久化保存。

所以当前真实关系应该表述为：

```text
flash 保存仓本机认证信息，VM 保存绑定后的耳机信息，BLE 读特征是把这两类数据按固定字段位置拼起来。
```

## 5. VM 中到底保存了什么

仓耳绑定信息保存在 `VM_RDX_CUSTOM_AUTH` 对应的 `EarphoneInfo` 结构中，核心字段只有四项：

```c
typedef struct{
    u8 sn[25];
    u8 ep_mac[6];
    char ep_mac_str[13];
    u8 label_sn[17];
} EarphoneInfo;
```

可以看到，仓侧真正落盘的是：

1. 耳机 SN
2. 左耳 MAC 字符串
3. 左耳 MAC 的 6 字节数组形式
4. label sn

也就是说，当前绑定结果的持久化对象是“耳机身份三元组”，而不是整条 BLE 读特征。

对应的 VM 接口为：

1. `rdx_vm_write_ep_info_intoVM()`：写入 `VM_RDX_CUSTOM_AUTH`
2. `rdx_vm_read_ep_info_fromVM()`：从 `VM_RDX_CUSTOM_AUTH` 读回
3. `rdx_vm_get_ep_info()`：返回 RAM 中缓存的 `epInfo`

## 6. APP 如何判断是否已绑定

当前仓耳绑定场景下，APP 更关心的是 BLE 读特征里这些固定位置的字段是否已经是有效耳机信息，而不是单独读取一个“仓耳绑定状态位”。

可以把它理解成：

1. 如果 `Lmac`、`SN`、`label sn` 这些关键字段已经是有效值，APP 就能识别出仓里已经保存过耳机信息。
2. 如果这些字段是全 0 占位或空值，APP 就会认为当前还没有可用的耳机绑定数据，需要发起绑定。

系统里确实还存在另一套 `bound_state` 广播状态位，但那不是本文要说明的仓耳绑定核心依据。对于当前 BLE APP 这条链路来说，关键仍然是读特征里的固定字段。

## 7. 绑定时序

### 7.1 绑定前

仓启动后，会先尝试读取 VM 中的 `EarphoneInfo`，然后拼装当前 BLE 读特征。

如果 VM 中没有有效耳机信息，那么读特征中和耳机相关的字段会表现为默认占位值。

### 7.2 APP 点击绑定

从当前实际运行日志可以直接看到，APP 触发绑定后，协议层先解析出耳机信息，再回调仓侧入口：

```text
_rdx_protocol_handle_device_pair --> auth_code: ..., ep_mac: ..., case_mac: ...
rdx_app_device_pair_handle --> au_code: ..., mac_str: ..., label_sn: ...
```

这说明当前真实生效的仓侧绑定入口就是：

```c
int rdx_app_device_pair_handle(char* au_code, char* mac_str, char* label_sn)
```

这个函数的行为非常明确：

1. 校验 `au_code`、`mac_str`、`label_sn` 不能为无效占位值。
2. 用这三段数据组装新的 `EarphoneInfo`。
3. 把 `mac_str` 转成 6 字节数组并做字节序翻转。
4. 调用 `rdx_vm_write_ep_info_intoVM()` 覆盖 VM 中旧的耳机信息。
5. 立即调用 `rdx_app_earphone_pack_readchardata()` 重新生成 BLE 读特征。

这里有一个很重要的实现细节：

仓侧持久化的是 `au_code + mac_str + label_sn`，并不会把协议层日志里的 `case_mac` 一起写进 `EarphoneInfo`。`case_mac` 仍然是在仓侧拼包时根据认证信息或本机 BLE MAC 实时得出的。

### 7.3 绑定后的即时效果

只要 `rdx_vm_write_ep_info_intoVM()` 成功，仓就会立刻把新的耳机信息反映到 BLE 读特征里，不需要等重启。

实测日志已经验证这一点：

```text
rdx_app_device_pair_handle --> au_code: ZENCHORDE202507070004928, mac_str: 7878359A1340, label_sn: ZCE1250825004928
=== write earphone info to vm, result: 61
===rdx_app_earphone_pack_readchardata --> readchardata: C,7878359A1340,7978359A1340,CB4ED95EF23C,ZENCHORDE202507070004928,ZCE1250825004928,000000000000
```

这说明绑定当下就已经完成了两件事：

1. VM 覆盖成功
2. BLE 读特征重拼成功

## 8. 冷启动恢复逻辑

仓冷启动时，`rdx_vm_auth_info_init()` 会先执行 `rdx_vm_read_ep_info_fromVM()`，把上次绑定成功后写入的耳机信息重新读回 RAM。

然后流程继续做两件事：

1. 尝试从 flash 读取仓自己的认证信息，用于补充 `Cmac` 和 `fac_sn`
2. 调用 `rdx_app_earphone_pack_readchardata()` 重新生成 BLE 读特征

因此，冷启动后的读特征不是重新走一次绑定，而是“从 VM 恢复耳机信息，再重新拼包”。

实测冷启动日志如下：

```text
=== read ep info from vm, result: 61
=== read ep info from vm success, auth code: ZENCHORDE202507070004928
=== read ep info from vm success, mac: 7878359A1340
=== read ep info from vm success, label sn: ZCE1250825004928
rdx_vm_auth_info_init --> read flash failed, auth info keeps default zero value
===rdx_app_earphone_pack_readchardata --> readchardata: C,7878359A1340,7978359A1340,CB4ED95EF23C,ZENCHORDE202507070004928,ZCE1250825004928,000000000000
```

这条日志能说明三件事：

1. 绑定信息确实已经持久化在 VM 里。
2. 冷启动后 VM 中的数据会被正确读回。
3. 即使 flash 授权区当前无效，仓仍然可以依靠 VM 中的耳机信息重新生成符合协议的 BLE 读特征。

## 9. 为什么之前不满足协议要求

从当前 staged `git diff` 可以直接看出，之前的问题不在于“仓没保存数据”，而在于旧的 BLE 读特征实现根本不满足你们现在这套 95 字节协议。

关键变化有三点：

1. `BLE_READCHAR_INFO_SIZE` 以前在不同编译分支下分别是 39 或 24，当前已经统一改成了 95。
2. 旧版 `rdx_app_earphone_pack_readchardata()` 并不会按 `role,Lmac,Rmac,Cmac,SN,label sn,wifi Mac` 去拼字符串，而是直接把 `p_auth->AuthKey` 拷进 `ble_readchar_info`。
3. 也就是说，旧版读特征本质上只暴露了 flash 里的 `AuthKey`，最多只是“仓本机认证信息的一小部分”，根本没有把协议要求的耳机 MAC、仓 MAC、SN、label sn、wifi Mac 这些字段按固定位置带出来。

旧版可以近似理解成：

```text
BLE read characteristic = flash.AuthKey
```

而你们当前协议要求的是：

```text
BLE read characteristic = role + Lmac + Rmac + Cmac + SN + label sn + wifi Mac
```

这就是之前 BLE APP 侧判断不成立的根因。

再往下看 diff 还能看到另一个关键修正：

1. 现在 `rdx_vm_auth_info_init()` 一上来就先 `rdx_vm_read_ep_info_fromVM()`。
2. 然后才去读 flash 认证信息。
3. 最后再统一调用 `rdx_app_earphone_pack_readchardata()`。

这个顺序非常关键，因为它意味着：

1. 绑定后的耳机信息先从 VM 恢复。
2. flash 只作为仓侧认证和回退数据源参与拼包。
3. 最终读特征才有机会同时满足协议规定的耳机字段和仓字段。

所以更准确的原因总结应该是：

```text
之前不行，不是因为 flash 和 VM 关系不清，而是因为旧版 BLE 读特征几乎只返回 flash 中的 AuthKey，根本不具备协议要求的完整字段集合；后续才改成“VM 中耳机信息 + flash 中仓信息 + 运行时 Wi-Fi 信息”的混合拼包模式。
```

## 10. 为什么 flash 无效也不影响当前绑定链路

当前仓耳绑定链路里，耳机侧关键数据来自 VM 中的 `EarphoneInfo`，不是来自 flash 授权区。

flash 读取失败时，影响的是：

1. `rdx_auth_info` 里与仓认证相关的字段
2. `Cmac` 的优先来源
3. `label sn` 的备用回退来源

但只要 VM 中已经有有效的：

1. `epInfo.sn`
2. `epInfo.ep_mac_str`
3. `epInfo.ep_mac`
4. `epInfo.label_sn`

那么读特征里的 `Lmac`、`Rmac`、`SN`、`label sn` 仍然可以正常恢复。

当前日志里 `Cmac` 仍然有效，是因为仓侧在拼包时会在 flash 无效的情况下退回使用本机 BLE MAC。

## 11. Wi-Fi 字段为什么是 000000000000

`wifi Mac` 不是绑定失败，也不是空字段，它是当前协议里允许存在的占位值。

当前实现会先把 `wifi Mac` 预填成 `000000000000`，只有当 `xxp_uart_get_wifi_AP_info()` 能返回有效 Wi-Fi 信息时，才会把这 12 个字符替换成真实 Wi-Fi MAC。

因此，仓耳绑定链路里看到：

```text
...,ZCE1250825004928,000000000000
```

其含义应解释为：

```text
盒标 SN 已就绪，Wi-Fi 信息暂未就绪
```

这与当前协议要求是一致的。

## 12. 清空与重新绑定

当前 `rdx_app_device_unpair_handle()` 的实现是：

1. 取出当前 `EarphoneInfo`
2. 整体清零
3. 写回 VM
4. 再读回 VM
5. 重新拼 BLE 读特征

也就是说，当前“解绑/清空”行为的本质是把 VM 中保存的耳机身份三元组清空。

清空后的直接表现是：

1. `Lmac`、`Rmac`、`SN`、`label sn` 会回到默认占位值
2. 下一次 APP 重新发起绑定时，仓又会通过 `rdx_app_device_pair_handle()` 用新的耳机信息重新覆盖 VM

因此，当前实现里的“解绑”可以理解为：

```text
清空仓侧已保存的耳机身份记录，等待下一次绑定重新覆盖
```

## 13. 与模拟开关的关系

`RDX_READCHAR_SIMULATE_ENABLE` 只在开机初始化的兜底路径里参与判断，默认值为 0。

它的作用是：

1. 当 flash 信息和 VM 信息都不满足要求时，允许用一套模拟数据临时补齐读特征。
2. 它不参与正常的配对回调写 VM 逻辑。
3. 它也不参与绑定完成后的即时重拼包逻辑。

所以，正常真实设备上的仓耳绑定链路应理解为：

```text
APP 回调耳机信息 -> 仓写 VM -> 仓重拼读特征 -> 冷启动再从 VM 恢复
```

而不是：

```text
依赖模拟开关维持绑定结果
```

## 14. 当前版本已经验证通过的结论

基于代码和实测日志，当前可以确认下面这些结论：

1. BLE 读特征的协议格式已经固定为 `role,Lmac,Rmac,Cmac,SN,label sn,wifi Mac`，总长 95 字节。
2. APP 对仓耳是否已绑定的判断，当前实质上依赖这条 BLE 读特征中的固定字段。
3. 仓侧真正持久化到 VM 的是耳机身份三元组：`SN + 左耳 MAC + label sn`。
4. APP 点击绑定后，协议层会回调到仓侧 `rdx_app_device_pair_handle()`。
5. 仓侧会用新的耳机信息覆盖 VM，并立刻刷新 BLE 读特征。
6. 冷启动后，仓会先从 VM 中读回耳机信息，再重新拼装 BLE 读特征。
7. 当前实测已经证明：绑定后立即读取和冷启动后读取，这两次 read 特征内容是一致的。
8. `wifi Mac = 000000000000` 在当前实现中是“Wi-Fi 信息暂未就绪”的合法占位值。

## 15. 关键代码位置

与这条绑定链路直接相关的代码如下：

1. `SDK/apps/common/third_party_profile/rdx_protocol/rdx_app.c`
   - `rdx_app_earphone_pack_readchardata()`
   - `rdx_app_device_pair_handle()`
   - `rdx_app_device_unpair_handle()`

2. `SDK/apps/common/third_party_profile/rdx_protocol/rdx_vm.c`
   - `rdx_vm_write_ep_info_intoVM()`
   - `rdx_vm_read_ep_info_fromVM()`
   - `rdx_vm_get_ep_info()`
   - `rdx_vm_auth_info_init()`

3. `SDK/apps/common/third_party_profile/rdx_protocol/rdx_vm.h`
   - `EarphoneInfo`

## 16. 最终结论

当前仓与耳机通过 BLE APP 的绑定逻辑，可以用一句话概括：

```text
APP 通过仓的 BLE 读特征判断是否已有耳机信息；当用户点击绑定后，仓侧回调拿到耳机关键数据，覆盖并持久化到 VM，然后立即刷新 BLE 读特征；冷启动时再从 VM 恢复这份信息。
```

这就是当前版本中已经被代码和日志共同证实的仓耳绑定真实实现。