## 阶段一：最基础的核心规则 (Target, Prerequisite, Command)

`make` 的全部魔力都基于一条规则：

```makefile
目标 (Target): 依赖 (Prerequisites)
<Tab>	命令 (Command)
```

- **目标 (Target):** 你想生成的东西，通常是个文件名，比如 `app.elf` 或 `main.o`。也可以是个“伪目标”（Phony Target），比如 `all`, `clean`，它们不代表真实文件。
- **依赖 (Prerequisites):** 为了生成“目标”，需要先准备好的东西。
- **命令 (Command):** 如何用“依赖”生成“目标”。**注意：命令前面必须是一个真实的 `Tab` 键，不能是空格！**

`make` 的工作逻辑是：

1. 你告诉 `make` 你要生成哪个“目标”（比如 `make all`）。
2. `make` 检查这个“目标”的“依赖”。
3. 如果“依赖”文件不存在，或者“依赖”文件的修改时间比“目标”文件新，`make` 就会执行“命令”来重新生成“目标”。

### 示例 1：最简单的 Makefile

我们先不编译 C 代码，就用 `echo` 打印信息。

```makefile
all:
	echo "Hello, Makefile!"

clean:
	echo "Cleaning up..."
```

- `all` 和 `clean` 都是“目标”。
- 它们都没有“依赖”。
- `echo "..."` 是它们的“命令”。

在命令行里：

- `make` (默认执行第一个目标) -> 会执行 `echo "Hello, Makefile!"`
- `make all` -> 同上
- `make clean` -> 会执行 `echo "Cleaning up..."`

### 示例 2：编译一个 C 文件

假设我们有一个 `main.c`。

```makefile
my_app: main.c
	gcc main.c -o my_app

clean:
	rm -f my_app
```

- **目标:** `my_app`
- **依赖:** `main.c`
- **命令:** `gcc main.c -o my_app`

现在，`make` 的智能之处体现了：

1. 你运行 `make`。`make` 发现目标 `my_app` 依赖 `main.c`。
2. `make` 检查：`my_app` 文件存在吗？
   - **如果不存在：** 执行 `gcc` 命令。
   - **如果存在：** `make` 会比较 `my_app` 和 `main.c` 的最后修改时间。
     - 如果 `main.c` 比 `my_app` 新（意味着你修改了 C 文件），就执行 `gcc` 命令重新编译。
     - 如果 `my_app` 比 `main.c` 新（你没改过 C 文件），`make` 会说 `my_app is up to date.` 并跳过编译。

### 示例2.1：引入 `.PHONY` (伪目标)

**问题：** 如果你的目录下"不巧"有一个也叫 `clean` 的文件怎么办？ 当你运行 `make clean` 时，`make` 发现 `clean` 目标没有依赖，而且 `clean` 文件已经存在，它就会说 `clean is up to date.` 而拒绝执行 `rm` 命令！

**解决：** 使用 `.PHONY` 告诉 `make`，这个目标是“伪”的，它不代表一个真实文件，每次调用都必须执行命令。

```makefile
.PHONY: all clean  # 声明 all 和 clean 是伪目标

all: my_app

my_app: main.c
	gcc main.c -o my_app

clean:
	rm -f my_app
```

你会看到 `.PHONY: FORCE _all all test clean distclean $(BUILD)/$(TARGET)`。这就是在做同样的事情，确保 `all`, `clean` 等目标总是被执行。

## 阶段二：使用变量 (Variables)

你的范例里充满了各种变量 (比如 `PROJECT_NAME`, `C_FLAGS`)。这是为了 **"D.R.Y." (Don't Repeat Yourself)**。

### 示例 3：用变量重构

假设我们 C 项目变复杂了，有 `main.c` 和 `utils.c`。

```makefile
.PHONY: all clean

# 变量定义 (使用 := 推荐，表示立即赋值)
TARGET := my_app
CC     := gcc
RM     := rm -f

# 源文件列表
SRCS := main.c utils.c
# 目标文件列表 (高级用法：变量替换)
# 把 SRCS 变量里所有 .c 结尾的替换为 .o
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

# 链接 (Link)
# 目标 my_app 依赖于 main.o 和 utils.o
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 编译 (Compile)
# 目标 main.o 依赖于 main.c
main.o: main.c
	$(CC) -c main.c -o main.o

# 目标 utils.o 依赖于 utils.c
utils.o: utils.c
	$(CC) -c utils.c -o utils.o

clean:
	$(RM) $(TARGET) $(OBJS)
```

- `$(OBJS)` 会被展开为 `main.o utils.o`。
- `$(TARGET): $(OBJS)` 就等于 `my_app: main.o utils.o`。
- **好处：** 如果你新增一个 `helper.c`，你只需要改 `SRCS := main.c utils.c helper.c` 这一行，`OBJS` 变量会自动更新，`make` 也会自动找到新的编译规则（等等，`helper.o` 规则我们没写啊？别急，阶段三解决）。

## 阶段三：模式规则 (Pattern Rules)

在版本 4 中，我们有两个编译规则，它们长得几乎一样：

```makefile
main.o: main.c
	$(CC) -c main.c -o main.o
utils.o: utils.c
	$(CC) -c utils.c -o utils.o
```

如果我有 100 个 .c 文件，我不想写 100 遍。`make` 提供了“模式规则”来解决这个问题。

`%.o: %.c` 的意思是：**“任何一个 .o 文件，都依赖于同名的 .c 文件”**。

在模式规则中，我们可以使用“自动化变量”：

- `$@`：代表“目标” (比如 `main.o`)
- `$<`：代表“第一个依赖” (比如 `main.c`)
- `$^`：代表“所有依赖” (比如 `main.o utils.o`)

### 示例 4：使用模式规则

```makefile
.PHONY: all clean

TARGET := my_app
CC     := gcc
RM     := rm -f
CFLAGS := -Wall -g # 编译选项也用变量

SRCS := main.c utils.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@  # 使用了自动化变量 $^ 和 $@

# 模式规则：
# 告诉 make 如何从任意 .c 文件生成 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJS)
```

现在，`make` 变得非常聪明：

1. `make` 发现 `all` 依赖 `my_app`。
2. `my_app` 依赖 `main.o` 和 `utils.o`。
3. `make` 查找 `main.o` 的规则。它没找到显式规则，但找到了模式规则 `%.o: %.c`。
4. `make` 应用此规则，发现 `main.o` 依赖 `main.c`，并执行 `$(CC) $(CFLAGS) -c main.c -o main.o`。
5. 对 `utils.o` 同理。
6. 最后，所有 `OBJS` 都准备好了，`make` 执行 `$(TARGET)` 规则，链接它们。

## 阶段四：条件判断与函数 (Conditionals & Functions)

这是 Makefile 变得强大的关键。

### 1. 条件判断 (`ifeq`)

你可能想在“调试(Debug)”模式和“发布(Release)”模式下使用不同的编译参数。

```makefile
.PHONY: all clean

# 如果 DEBUG 变量没被定义过，就给它赋值 y，允许从命令行覆盖 DEBUG 变量
# 比如运行: make DEBUG=n
DEBUG ?= y

TARGET := my_app
CC     := gcc
RM     := rm -f

# 基础编译选项
CFLAGS := -Wall
# 条件判断
ifeq ($(DEBUG), y)
	CFLAGS += -g -O0         # 调试模式: 加调试信息，不优化
else
	CFLAGS += -O2            # 发布模式: 开启优化
endif

SRCS := main.c utils.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJS)
```

### 2. 函数 (Functions)

Makefile 有很多强大的内置函数。

- `$(shell ...)`：执行一个 shell 命令，并返回其输出。
- `$(wildcard ...)`：查找匹配模式的文件。
- `$(addprefix ...)` / `$(addsuffix ...)`：给列表中的每一项添加前缀/后缀。

**示例：自动查找所有 .c 文件**

假设我们把源文件放到了 `src/` 和 `lib/` 目录。

```makefile
...
# 1. 定义源文件目录
SRC_DIRS := src lib

# 2. 查找所有 .c 文件
#    $(wildcard src/*.c lib/*.c)
SRCS := $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

# 3. 把 src/main.c 转换为 obj/src/main.o
#    我们希望所有 .o 文件都放到 obj/ 目录下
BUILD_DIR := obj
OBJS := $(SRCS:.c=.o)           # 得到: src/main.o lib/utils.o
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS)) # 得到: obj/src/main.o obj/lib/utils.o
...
```

## 阶段五：高级魔法 (自动依赖管理)

我们还有一个大问题没解决（示例 4 里留下的）： 如果 `main.c` 包含了 `utils.h`，当我只修改 `utils.h` 而不修改 `main.c` 时，`make` 会傻傻地认为 `main.o` 是最新的，从而不重新编译 `main.c`，导致链接出错！

我们需要告诉 `make`：`main.o` 不仅依赖 `main.c`，还依赖 `utils.h`。

**笨办法：** `main.o: main.c utils.h`

**聪明的办法（自动依赖管理）：** 利用 `gcc` 的一个神奇参数 `-MMD`。它会在编译时，自动分析 `#include`，并生成一个 `.d` (依赖) 文件。编译 `main.c` 时，`gcc -MMD` 会“扫视” `main.c` 里的所有 `#include`，并自动生成一个 `main.d` 文件。

```makefile
.PHONY: all clean

TARGET := my_app
CC     := gcc
RM     := rm -f

# 1. 告诉 GCC 生成 .d 依赖文件
CFLAGS := -Wall -g -MMD -MP

SRCS := main.c utils.c
OBJS := $(SRCS:.c=.o)
# 依赖文件 .d 列表
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

# 2. 在 make 执行时，把所有 .d 文件包含进来
#    -include 的意思是：如果文件存在，就包含；如果不存在，别报错。
#    （第一次编译时 .d 文件肯定不存在）
-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	# 清理时也要删除 .o 和 .d
	$(RM) $(TARGET) $(OBJS) $(DEPS)
```

**工作流程：**

1. **第一次 `make`：**
   - `.d` 文件不存在，`-include` 被跳过。
   - `make` 执行 `%.o: %.c` 规则。
   - `gcc -MMD ... -c main.c -o main.o`。
   - `gcc` 除了生成 `main.o`，还“顺便”生成了一个 `main.d` 文件，内容可能是：`main.o: main.c utils.h`。
2. **修改 `utils.h` 后，第二次 `make`：**
   - `make` 首先执行 `-include $(DEPS)`，它找到了 `main.d` 和 `utils.d`。
   - `make` 读取 `main.d`，得知 `main.o` 依赖 `utils.h`。
   - `make` 检查发现，`utils.h` 的修改时间比 `main.o` 新。
   - `make` 决定重新执行 `%.o: %.c` 规则来生成 `main.o`。
   - 问题完美解决！

## GCC命令简介

| **你想做什么？**              | **使用的 gcc 参数 (Flag)** | **对应你的 Makefile 变量** |
| ----------------------------- | -------------------------- | -------------------------- |
| **【编译】** .c -> .o         | `-c`                       | （模式规则中的命令）       |
| **【链接】** .o -> .elf       | (无, 自动)                 | （链接规则中的命令）       |
| 指定头文件路径                | `-I...`                    | `$(B_INC)`                 |
| 定义 C 语言宏                 | `-D...`                    | `$(B_DEF)`                 |
| 开启调试                      | `-g`                       | `$(C_FLAGS)`               |
| 设置优化等级                  | `-O0`, `-Os`               | `$(C_FLAGS)`               |
| 链接一个库 (如数学库)         | `-lm`                      | `$(LIBS)`                  |
| **【嵌入式】** 指定内存地图   | `-T...`                    | `$(LDSCRIPT)`              |
| **【嵌入式】** 生成 Map 文件  | `-Wl,-Map=...`             | `$(L_FLAGS)`               |
| **【Makefile】** 自动生成依赖 | `-MMD`                     | `$(C_FLAGS)`               |

## 编写Makefile编译自己的项目

项目结构如下：

```python
boot
├── board
│   ├── inc
│   │   ├── arginfo.h
│   │   └── flash_layout.h
│   └── src
│       ├── arginfo.c
│       └── lowlevel.c
├── bootloader.c
├── component
│   ├── component.h
│   ├── crc
│   │   ├── crc32.c
│   │   └── crc32.h
│   ├── easylogger
│   │   ├── inc
│   │   │   ├── elog.h
│   │   │   └── elog_cfg.h
│   │   ├── port
│   │   │   └── elog_port.c
│   │   └── src
│   │       ├── elog.c
│   │       └── elog_utils.c
│   └── ringbuffer
│       ├── ringbuffer8.c
│       └── ringbuffer8.h
├── driver
│   ├── driver.h
│   ├── inc
│   │   ├── button.h
│   │   ├── led.h
│   │   ├── norflash.h
│   │   ├── uart copy.h
│   │   └── uart.h
│   └── src
│       ├── button.c
│       ├── led.c
│       ├── norflash.c
│       └── uart.c
├── main.c
├── main.h
└── utils
    └── utils.c
    
platform
├── cmsis
│   ├── core
│   │   ├── arm_common_tables.h
│   │   ├── arm_const_structs.h
│   │   ├── arm_math.h
│   │   ├── core_cm4.h
│   │   ├── core_cmFunc.h
│   │   ├── core_cmInstr.h
│   │   └── core_cmSimd.h
│   └── device
│       ├── startup_stm32f40xx.s
│       ├── stm32f4xx.h
│       ├── stm32f4xx_conf.h
│       ├── system_stm32f4xx.c
│       └── system_stm32f4xx.h
├── driver
│   ├── inc
│   │   ├── misc.h
│   │   ├── stm32f4xx_adc.h
│   │   ├── stm32f4xx_can.h
│   │   ├── stm32f4xx_cec.h
│   │   ├── stm32f4xx_crc.h
│   │   ├── stm32f4xx_cryp.h
│   │   ├── stm32f4xx_dac.h
│   │   ├── stm32f4xx_dbgmcu.h
│   │   ├── stm32f4xx_dcmi.h
│   │   ├── stm32f4xx_dfsdm.h
│   │   ├── stm32f4xx_dma.h
│   │   ├── stm32f4xx_dma2d.h
│   │   ├── stm32f4xx_dsi.h
│   │   ├── stm32f4xx_exti.h
│   │   ├── stm32f4xx_flash.h
│   │   ├── stm32f4xx_flash_ramfunc.h
│   │   ├── stm32f4xx_fmc.h
│   │   ├── stm32f4xx_fmpi2c.h
│   │   ├── stm32f4xx_fsmc.h
│   │   ├── stm32f4xx_gpio.h
│   │   ├── stm32f4xx_hash.h
│   │   ├── stm32f4xx_i2c.h
│   │   ├── stm32f4xx_iwdg.h
│   │   ├── stm32f4xx_lptim.h
│   │   ├── stm32f4xx_ltdc.h
│   │   ├── stm32f4xx_pwr.h
│   │   ├── stm32f4xx_qspi.h
│   │   ├── stm32f4xx_rcc.h
│   │   ├── stm32f4xx_rng.h
│   │   ├── stm32f4xx_rtc.h
│   │   ├── stm32f4xx_sai.h
│   │   ├── stm32f4xx_sdio.h
│   │   ├── stm32f4xx_spdifrx.h
│   │   ├── stm32f4xx_spi.h
│   │   ├── stm32f4xx_syscfg.h
│   │   ├── stm32f4xx_tim.h
│   │   ├── stm32f4xx_usart.h
│   │   └── stm32f4xx_wwdg.h
│   └── src
│       ├── misc.c
│       ├── stm32f4xx_adc.c
│       ├── stm32f4xx_can.c
│       ├── stm32f4xx_cec.c
│       ├── stm32f4xx_crc.c
│       ├── stm32f4xx_cryp.c
│       ├── stm32f4xx_cryp_aes.c
│       ├── stm32f4xx_cryp_des.c
│       ├── stm32f4xx_cryp_tdes.c
│       ├── stm32f4xx_dac.c
│       ├── stm32f4xx_dbgmcu.c
│       ├── stm32f4xx_dcmi.c
│       ├── stm32f4xx_dfsdm.c
│       ├── stm32f4xx_dma.c
│       ├── stm32f4xx_dma2d.c
│       ├── stm32f4xx_dsi.c
│       ├── stm32f4xx_exti.c
│       ├── stm32f4xx_flash.c
│       ├── stm32f4xx_flash_ramfunc.c
│       ├── stm32f4xx_fmpi2c.c
│       ├── stm32f4xx_fsmc.c
│       ├── stm32f4xx_gpio.c
│       ├── stm32f4xx_hash.c
│       ├── stm32f4xx_hash_md5.c
│       ├── stm32f4xx_hash_sha1.c
│       ├── stm32f4xx_i2c.c
│       ├── stm32f4xx_iwdg.c
│       ├── stm32f4xx_lptim.c
│       ├── stm32f4xx_ltdc.c
│       ├── stm32f4xx_pwr.c
│       ├── stm32f4xx_qspi.c
│       ├── stm32f4xx_rcc.c
│       ├── stm32f4xx_rng.c
│       ├── stm32f4xx_rtc.c
│       ├── stm32f4xx_sai.c
│       ├── stm32f4xx_sdio.c
│       ├── stm32f4xx_spdifrx.c
│       ├── stm32f4xx_spi.c
│       ├── stm32f4xx_syscfg.c
│       ├── stm32f4xx_tim.c
│       ├── stm32f4xx_usart.c
│       └── stm32f4xx_wwdg.c
└── stm32f407vetx_flash.ld
```

编译文件核心的就是批量的将：

- `.c`文件编译成`.o`文件
- `.s`文件编译成`.o`文件
- `.o`文件链接成`elf`或`bin`或`hex`文件

所以，需要指定：

- 所有`.c`文件的路径
- 所有`.s`文件的路径
- 所有`.o`文件的路径

接下来需要为编译、链接命令生成对应的规则，以自动化批量进行

```makefile
# Debug
DEBUG ?= y

# 禁用隐含规则
MAKEFLAGS += -rR

# 项目名称
PROJECT_NAME := stm32f4_boot

# 输出根路径
BUILD_BASE := build

# 目标名
TARGET := $(BUILD_BASE)/$(PROJECT_NAME)

# toolchain
CC := gcc
CROSS_COMPILE ?= tools/toolchain/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-
MKDIR := mkdir -p

# 平台配置
MCU := -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDSCRIPT := platform/stm32f407vetx_flash.ld

# 库文件
LIBS := -lm

# 宏定义
ORIGIN_DEF := STM32F40_41xxx \
		USE_STDPERIPH_DRIVER \
		HSE_VALUE=8000000

ifeq ($(DEBUG), y)
ORIGIN_DEF += DEBUG
endif

###################################################################
#               添加源文件和头文件后只需要修改下方的内容即可             #
###################################################################
# 源文件路径
C_DIRS := boot \
		boot/board \
		boot/component/crc \
		boot/component/ringbuffer \
		boot/driver/src \
		boot/override \
		boot/utils \
		platform/cmsis/device \
		platform/driver/src

ifeq ($(DEBUG), y)
C_DIRS += boot/component/easylogger/port \
		boot/component/easylogger/src
endif

# 头文件路径
H_DIRS := boot \
		boot/board \
		boot/component \
		boot/component/crc \
		boot/component/easylogger/inc \
		boot/component/ringbuffer \
		boot/driver/inc \
		boot/override \
		platform/cmsis/core \
		platform/cmsis/device \
		platform/driver/inc \

# s文件路径
S_DIRS := platform/cmsis/device \

###################################################################
#                           准备文件列表                            #
###################################################################

# 源文件列表
C_PATHS := $(wildcard $(addsuffix /*.c, $(C_DIRS)))

# 头文件列表
H_PATHS := $(wildcard $(addsuffix /*.h, $(H_DIRS)))

# .s文件列表
S_PATHS := $(wildcard $(addsuffix /*.s, $(S_DIRS)))

# .o文件列表
O_PATHS := $(C_PATHS:.c=.o) \
		$(S_PATHS:.s=.o)

# .o文件编译后存放的位置
O_PATHS := $(addprefix $(TARGET)/, $(O_PATHS))

# .d文件列表
D_PATHS := $(C_PATHS:.c=.d)

# .的文件生成后存放的位置
D_PATHS := $(addprefix $(TARGET)/, $(D_PATHS))

###################################################################
#                               参数                              #
###################################################################

# 编译参数
A_FLAGS  = $(MCU) -g
C_FLAGS  = $(MCU) -g
C_FLAGS += -std=gnu11
C_FLAGS += -ffunction-sections -fdata-sections -fno-builtin -fno-strict-aliasing
C_FLAGS += -Wall -Werror

ifeq ($(DEBUG), y)
C_FLAGS += -Wno-comment -Wno-unused-value -Wno-unused-variable -Wno-unused-function
endif
C_FLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

ifeq ($(DEBUG), y)
C_FLAGS += -O0
else
C_FLAGS += -Os
endif

# 链接参数
L_FLAGS  = $(MCU)
L_FLAGS += -T$(LDSCRIPT)
L_FLAGS += -static -specs=nano.specs
L_FLAGS += -Wl,-cref
L_FLAGS += -Wl,-Map=$(TARGET)/$(PROJECT_NAME).map
L_FLAGS += -Wl,--gc-sections
L_FLAGS += -Wl,--start-group
L_FLAGS += $(LIBS)
L_FLAGS += -Wl,--end-group

# define的前面加上-D
D_DEFINE := $(addprefix -D, $(ORIGIN_DEF))

# 头文件路径的前面加上-I
I_HEADER := $(addprefix -I, $(H_DIRS))

FINAL_OBJ_FILE := $(TARGET)/$(PROJECT_NAME)

.PHONY: test all clean $(TARGET)

all: $(TARGET)

# 包含所有.d文件
-include $(D_PATHS)

# 等待所有相关文件编译完成后，需要进行最后一步链接
# 这一步需要几个依赖：
# 	1. 链接文件（.ld文件）
#	2. 所有目标文件（.o文件）
#	3. Makefile文件（目的是Makefile被修改、其他依赖没被修改的情况，也需要编译整个项目）
$(TARGET): $(LDSCRIPT) $(O_PATHS) Makefile
# 	@$(MKDIR) $(TARGET)
#	这里建不建目录都无所谓，因为生成.o文件的规则里已经帮忙建好了
	@echo "  LD    $(FINAL_OBJ_FILE).elf"
	@echo "  OBJ   $(FINAL_OBJ_FILE).hex"
	@echo "  OBJ   $(FINAL_OBJ_FILE).bin"
	@$(CROSS_COMPILE)gcc $(O_PATHS) $(L_FLAGS) -o $(FINAL_OBJ_FILE).elf
	@$(CROSS_COMPILE)objcopy --gap-fill 0x00 -O ihex $(FINAL_OBJ_FILE).elf $(FINAL_OBJ_FILE).hex
	@$(CROSS_COMPILE)objcopy --gap-fill 0x00 -O binary -S $(FINAL_OBJ_FILE).elf $(FINAL_OBJ_FILE).bin
	@$(CROSS_COMPILE)size $(FINAL_OBJ_FILE).elf
	@echo "  Build Finish"


# 如果target是一个文件，则目标名必须是实际的文件路径名
# %是一个通配符，表示任意个字符，包含斜杠
# (TARGET)/%.o表示：boot/stm32f4_boot/.../xxx.o
$(TARGET)/%.o: %.c Makefile
	@echo "  CC    $@" 
	@$(MKDIR) $(dir $@)
	@$(CROSS_COMPILE)gcc -c $(C_FLAGS) $(D_DEFINE) $(I_HEADER) $< -o $@

$(TARGET)/%.o: %.s Makefile
	@echo "  AS    $@"
	@$(MKDIR) $(dir $@)
	@$(CROSS_COMPILE)as -c $(A_FLAGS) $< -o $@

clean:
	@rm -rf $(BUILD_BASE)
	@echo "Build path cleaned up"

test:
	@echo $(H_PATHS)
```

```makefile
# 禁用隐含规则
MAKEFLAGS += -rR

# 项目名称
PROJECT_NAME := stm32f4_boot

# 目标
# 我们采用 Makefile 1 的逻辑：BUILD 是目录, TARGET 是目标名
BUILD_BASE := build
BUILD := $(BUILD_BASE)/$(PROJECT_NAME)
TARGET ?= $(PROJECT_NAME)

# 生成配置
DEBUG ?= y
V ?=

# Toolchain
CROSS_COMPILE ?= tools/toolchain/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-
ECHO := echo
MKDIR := mkdir -p
ifeq ($(V),)
QUITE := @
endif

# 平台配置
MCU := -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDSCRIPT := platform/stm32f407vetx_flash.ld

# 库文件
LIBS := -lm

# 宏定义 (使用 Makefile 1 的变量名 P_DEF)
P_DEF :=
P_DEF += STM32F40_41xxx \
		USE_STDPERIPH_DRIVER \
		HSE_VALUE=8000000
ifeq ($(DEBUG), y)
P_DEF += DEBUG
endif

###################################################################
#                           文件列表                             #
#       (使用 Makefile 1 的 s_inc-y/s_dir-y 变量)                 #
###################################################################

# 头文件路径 (来自 Makefile 2 的 H_DIRS)
s_inc-y = boot \
		boot/board \
		boot/component \
		boot/component/crc \
		boot/component/easylogger/inc \
		boot/component/ringbuffer \
		boot/driver \
		boot/driver/inc \
		boot/override \
		platform/cmsis/core \
		platform/cmsis/device \
		platform/driver/inc

# 源文件路径 (来自 Makefile 2 的 C_DIRS, 但 easylogger 单独处理)
s_dir-y = boot \
		boot/board \
		boot/component/crc \
		boot/component/ringbuffer \
		boot/driver/src \
		boot/override \
		boot/utils \
		platform/cmsis/device \
		platform/driver/src

# s文件路径 (来自 Makefile 2 的 S_DIRS)
S_DIRS := platform/cmsis/device

# 采用 Makefile 1 的条件编译逻辑
ifeq ($(DEBUG), y)
s_dir-y += boot/component/easylogger/port \
		boot/component/easylogger/src
endif

# 采用 Makefile 1 的源文件处理逻辑
s_src-y = $(wildcard $(addsuffix /*.c, $(s_dir-y)))
s_src-y += $(wildcard $(addsuffix /*.s, $(S_DIRS)))

P_INC := $(sort $(s_inc-y))
P_SRC := $(sort $(s_src-y))

###################################################################
#                               参数                              #
###################################################################

# 编译标记 (采用 Makefile 2 的完整版)
A_FLAGS  = $(MCU) -g
C_FLAGS  = $(MCU) -g
C_FLAGS += -std=gnu11
C_FLAGS += -ffunction-sections -fdata-sections -fno-builtin -fno-strict-aliasing
C_FLAGS += -Wall -Werror

ifeq ($(DEBUG), y)
C_FLAGS += -Wno-comment -Wno-unused-value -Wno-unused-variable -Wno-unused-function
endif
C_FLAGS += -MMD -MP -MF"$(@:%.o=%.d)" # 自动依赖

ifeq ($(DEBUG), y)
C_FLAGS += -O0
else
C_FLAGS += -Os
endif

# 链接标记 (采用 Makefile 1 的 L_FLAGS, 它的 .map 路径是对的)
L_FLAGS  = $(MCU)
L_FLAGS += -T$(LDSCRIPT)
L_FLAGS += -static -specs=nano.specs
L_FLAGS += -Wl,-cref
L_FLAGS += -Wl,-Map=$(BUILD)/$(TARGET).map
L_FLAGS += -Wl,--gc-sections
L_FLAGS += -Wl,--start-group
L_FLAGS += $(LIBS)
L_FLAGS += -Wl,--end-group

###################################################################
#                           依赖对象                              #
#       (这部分"引擎"代码, 完全复制自 Makefile 1)                   #
###################################################################

B_INC := $(addprefix -I, $(P_INC))
B_DEF := $(addprefix -D, $(P_DEF))
B_OBJ := $(P_SRC)
B_OBJ := $(B_OBJ:.c=.o)
B_OBJ := $(B_OBJ:.s=.o)
B_OBJ := $(addprefix $(BUILD)/, $(B_OBJ))
B_DEP := $(B_OBJ:%.o=%.d)

LINKERFILE := $(LDSCRIPT)
FROCE_RELY := Makefile

###################################################################
#                               规则                               #
#       (这部分"引擎"代码, 完全复制自 Makefile 1)                   #
###################################################################

.PHONY: all test clean distclean $(BUILD)/$(TARGET)

# 最终链接目标是伪目标 $(BUILD)/$(TARGET)
# 展开为 build/stm32f4_boot/stm32f4_boot
all: $(BUILD)/$(TARGET)

-include $(B_DEP)

# 模式规则：编译 .s
# 例如：build/stm32f4_boot/platform/cmsis/device/startup.o: platform/cmsis/device/startup.s
$(BUILD)/%.o: %.s $(FROCE_RELY)
	$(QUITE)$(ECHO) "  AS    $@"
	$(QUITE)$(MKDIR) $(dir $@)
	$(QUITE)$(CROSS_COMPILE)as -c $(A_FLAGS) $< -o$@

# 模式规则：编译 .c
# 例如：build/stm32f4_boot/boot/main.o: boot/main.c
$(BUILD)/%.o: %.c $(FROCE_RELY)
	$(QUITE)$(ECHO) "  CC    $@"
	$(QUITE)$(MKDIR) $(dir $@)
	$(QUITE)$(CROSS_COMPILE)gcc -c $(C_FLAGS) $(B_DEF) $(B_INC) $< -o$@

# 链接规则
# 目标是伪目标 $(BUILD)/$(TARGET)
# 它依赖所有的 .o 文件
$(BUILD)/$(TARGET): $(LINKERFILE) $(B_OBJ) $(FROCE_RELY)
	$(QUITE)$(MKDIR) $(BUILD)
	$(QUITE)$(ECHO) "  LD    $@.elf"
	$(QUITE)$(ECHO) "  OBJ   $@.hex"
	$(QUITE)$(ECHO) "  OBJ   $@.bin"
	$(QUITE)$(CROSS_COMPILE)gcc $(B_OBJ) $(L_FLAGS) -o$@.elf
	$(QUITE)$(CROSS_COMPILE)objcopy --gap-fill 0x00 -O ihex $@.elf $@.hex
	$(QUITE)$(CROSS_COMPILE)objcopy --gap-fill 0x00 -O binary -S $@.elf $@.bin
	$(QUITE)$(CROSS_COMPILE)size $@.elf
	$(QUITE)$(ECHO) "  BUILD FINISH"

clean:
	$(QUITE)rm -rf $(BUILD)
	$(QUITE)$(ECHO) "clean up"

distclean:
	$(QUITE)rm -rf $(BUILD_BASE)
	$(QUITE)$(ECHO) "distclean up"

test:
	@echo $(BUILD)/$(TARGET)
```

## Makefile注意事项

不要在代码之后注释，最好另起一行：

```makefile
CROSS_COMPILE := tools/toolchain/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi- # 注释
```

上面这个写法就有问题，因为注释符号`#`前面打了一个空格，所以会导致该变量最后会多一个空格。
