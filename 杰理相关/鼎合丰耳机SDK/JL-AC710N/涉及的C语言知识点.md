# 幽灵构建

## 构建举例说明

```bat
# Makefile
all:
	@echo "=== 开始编译 ==="
	gcc -c main.c -o objs/main.o
	gcc -c hello.c -o objs/hello.o
	ld objs/main.o objs/hello.o -o sdk.elf
	@echo "=== 编译完成，调用 post-build 脚本 ==="
	@call post_build.bat sdk
    
    
    
:: post_build.bat（你贴的脚本，假设 %ELFFILE% 已设为 sdk.elf）
@echo off
set ELFFILE=sdk.elf

%OBJDUMP% -D -address-mask=0x1ffffff -print-dbg sdk.elf > sdk.lst
%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .data %ELFFILE% data.bin
%OBJCOPY% -O binary -j .data_code %ELFFILE% data_code.bin
%OBJCOPY% -O binary -j .data_code_z %ELFFILE% data_code_z.bin
%OBJCOPY% -O binary -j .overlay_init %ELFFILE% init.bin
%OBJCOPY% -O binary -j .overlay_aec %ELFFILE% aec.bin
%OBJCOPY% -O binary -j .overlay_aac %ELFFILE% aac.bin

echo 所有 bin 文件生成完毕！
```

### 实验 1：正常构建（成功）

```bat
make clean
make all

输出：
=== 开始编译 ===
=== 编译完成，调用 post-build 脚本 ===
所有 bin 文件生成完毕！

生成文件：
sdk.elf     sdk.lst     text.bin     data.bin     ...
```

### 实验 2：删除 hello.c，再次构建

```bat
del hello.c
make all

make 实际发生了什么？
gcc -c hello.c -o objs/hello.o
→ 错误：hello.c: No such file or directory
→ 链接失败！sdk.elf **没有生成**
但！旧的 sdk.elf 还在！
```

### 关键：post_build.bat 会怎么执行？

| 第一行是否执行         | 结果           |
| ---------------------- | -------------- |
| **执行 `%OBJDUMP%`**   | **幽灵构建！** |
| **不执行（加 `REM`）** | **真失败！**   |

**情况 A：你现在的脚本（未注释 objdump）→ 幽灵构建**

```bat
%OBJDUMP% -D ... sdk.elf > sdk.lst   ← 执行！
→ 旧 sdk.elf 存在且可读
→ objdump 成功，生成 sdk.lst
→ 所有 objcopy 也成功
→ 输出：
所有 bin 文件生成完毕！
你以为构建成功了！但其实 hello.c 没了，固件是旧的！
```

**情况 B：安全做法（加 REM）→ 真失败**

```bat
REM %OBJDUMP% -D ... sdk.elf > sdk.lst   ← 不执行
→ make 失败 → sdk.elf 未生成
→ objcopy 执行：
objcopy: 'sdk.elf': No such file or directory
→ 脚本报错中断
→ 你立刻知道构建失败了！
```

## make clean不会删除elf文件

| 文件       | `make clean` 是否删除 |
| ---------- | --------------------- |
| `objs/*.o` | 是                    |
| `sdk.elf`  | **否**                |
| `*.bin`    | 否                    |
| `*.lst`    | 否                    |

```bat
# 1. 构建成功
make clean && make all
ls sdk.elf    ← 应该存在

# 2. 再运行 clean
make clean
ls sdk.elf    ← 如果还存在 → 证明 clean 不删 elf！
```

