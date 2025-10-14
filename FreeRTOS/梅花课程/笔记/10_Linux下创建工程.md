本章主要涉及在linux中搭建一个stm32f4的开发环境，因为bootloader的开发需要在linux系统上进行。

同时之后用到的ESP32-C3的开发也是需要在linux中做的。

## 插件安装

1. remote ssh
2. wsl
3. EditorConfig for VS Code

## 创建目录和必要文件

安装editorconfig之后需要在右侧项目根目录里右击创建一个.editorconfig文件，该文件规定了文件被保存后自动格式化的一些配置。

![image-20251012091423539](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/09-14-23-dbb3935c0d82841ae9aca04af38f9159-image-20251012091423539-be8435.png)

下载最新版的stm32f4标准外设库，准备向项目中添加文件。

```
STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Libraries\CMSIS\Include\
	arm_common_tables.h
	arm_const_structs.h
	arm_math.h
	core_cm4.h
	core_cmFunc.h
	core_cmInstr.h
	core_cmSimd.h
	
STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Libraries\CMSIS\Device\ST\STM32F4xx\Include\
	stm32f4xx.h
	system_stm32f4xx.h

STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Project\STM32F4xx_StdPeriph_Templates\
	stm32f4xx_conf.h
	stm32f4xx_it.c
	stm32f4xx_it.h
	system_stm32f4xx.c
	
STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Libraries\CMSIS\Device\ST\STM32F4xx\Source\Templates\gcc_ride7\
	startup_stm32f40xx.s
	
STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Libraries\STM32F4xx_StdPeriph_Driver\inc\*
STM32F4xx_DSP_StdPeriph_Lib_V1.9.0\Libraries\STM32F4xx_StdPeriph_Driver\src\*
```

添加完成后如下

![image-20251012095900760](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/09-59-00-80449cb128ce2ccebbda8cad69122844-image-20251012095900760-ee6e4b.png)

## 添加vscode目录索引

在顶部搜索框输入`> c/c++`打开`c_cpp_properties.json`文件，编辑后变成如下样式：

```json
{
    "configurations": [
        {
            "name": "stm32f4_boot",
            "includePath": [
                "boot",
                "platform/cmsis/core",
                "platform/cmsis/device",
                "platform/driver/inc"
            ],
            "defines": [
                
            ]
        }
    ],
    "version": 4
}
```

## 添加链接脚本(.ld)文件

在platform目录里添加一个`stm32f407vetx_flash.ld`文件，内容如下：

```c
/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
    RAM (xrw)    : ORIGIN = 0x20000000, LENGTH = 128K
    CCMRAM (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
    FLASH (rx)   : ORIGIN = 0x8000000, LENGTH = 32K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  _siccmram = LOADADDR(.ccmram);

  /* CCM-RAM section
  *
  * IMPORTANT NOTE!
  * If initialized variables will be placed in this section,
  * the startup code needs to be modified to copy the init-values.
  */
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;       /* create a global symbol at ccmram start */
    *(.ccmram)
    *(.ccmram*)

    . = ALIGN(4);
    _eccmram = .;       /* create a global symbol at ccmram end */
  } >CCMRAM AT> FLASH


  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM



  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
```

这里解释一下ld文件的作用，如果把整个编译过程比作盖房子：

- **编译器 (Compiler)**：像是把钢筋、水泥、砖块等原材料（你的`.c`源代码）制作成标准化的预制板和墙壁（`.o`目标文件）。
- **链接器 (Linker)**：像是**建筑施工队**，负责把所有的预制板和墙壁组装成一栋完整的建筑（`.elf`/`.axf`可执行文件）。

那么，**`.ld` 文件就是这支施工队的“建筑蓝图”**。它以一种精确的脚本语言，**指挥链接器如何将所有的代码和数据，精确地放置到STM32芯片的内存地图（Flash、SRAM）中**。

---

### **`.ld` 文件的四大核心任务**

#### 1. 🗺️ 定义内存布局 (Define Memory Layout)

脚本首先会告诉链接器，我们手头的这块STM32芯片“长什么样”，它有哪些存储区域，分别在哪里，有多大。

**示例代码 (简化版):**

```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}
```

- `(rx)` 和 `(rwx)` 分别代表了该区域的权限：`r`=读, `w`=写, `x`=执行

#### 2. 🏠 指定段的存放位置 (Specify Section Placement)

这是链接器脚本最核心的功能。它会详细规定编译后产生的各个“段”（Section），应该被放置到哪个内存区域。

示例代码 (简化版):

```c
SECTIONS
{
  /* 中断向量表必须放在Flash的最开头 */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* 保留中断向量表 */
    . = ALIGN(4);
  } >FLASH

  /* .text段 (所有程序代码) 放入Flash */
  .text :
  {
    *(.text*)
  } >FLASH

  /* .data段 (已初始化变量) */
  .data :
  {
    _sdata = .; /* 定义一个符号_sdata，指向.data段的起始地址 */
    *(.data*)
    _edata = .;
  } >SRAM AT> FLASH /* 运行时在SRAM，但其初始值镜像存放在FLASH */

  /* .bss段 (未初始化变量) 放入SRAM */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } >SRAM
}
```

- `>FLASH` 和 `>SRAM` 就是指令，告诉链接器把这些段放到之前`MEMORY`命令定义的区域中。
- **关键点**：`.data`段的处理。`>SRAM AT> FLASH` 意思是，这个段**运行时**的地址在SRAM中，但它需要一个**加载地址**在FLASH中。这完美地解释了为什么**启动代码需要从Flash把数据复制到SRAM**。

```c
/* 中断向量表 */
.isr_vector :
{
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* 保留中断向量表 */
        . = ALIGN(4);
} >FLASH
```

> **`.isr_vector :`**
>
> - 语法：`output_section_name :`
> - 解释：定义一个名为 `.isr_vector` 的 **输出段**。所有后续花括号 `{}` 内的内容都将合并到这个输出段中。
>
> **`. = ALIGN(4);`**
>
> - 语法：`. = expression;`
> - 解释：
>   - **`.`**：这是一个特殊的符号，称为**定位计数器 (Location Counter)**。它表示当前在输出段中的地址。
>   - **`ALIGN(4)`**：这是一个函数，它返回下一个是4的倍数的地址。
>   - 整条语句的意思是：“将当前的地址（定位计数器）推进到下一个4字节对齐的位置”。在ARM架构中，指令和向量表条目通常需要4字节对齐，这是一个保证硬件能正确读取的操作。
>
> **`KEEP(\*(.isr_vector))`**
>
> - 语法：`KEEP(input_section_spec)`
> - 解释：
>   - `*`：是一个通配符，表示“来自于所有输入的`.o`目标文件”。
>   - `(.isr_vector)`：表示输入文件中名为 `.isr_vector` 的段。
>   - `*(.isr_vector)` 的意思是：“将所有输入文件中的 `.isr_vector` 段内容集合到这里”。
>   - **`KEEP(...)`**：这是一个非常重要的指令。链接器为了优化空间，可能会丢弃它认为“未使用”的代码或数据段（死代码消除）。`KEEP` 强制告诉链接器：“**无论你觉得这个段有没有被引用，都必须保留它，不准丢弃！**”。中断向量表是硬件直接访问的，没有任何软件代码会“调用”它，所以必须用`KEEP`来保护。
>
> **`>FLASH`**
>
> - 语法：`> memory_region`
> - 解释：将整个 `.isr_vector` 输出段放置到 `MEMORY` 命令中定义的名为 `FLASH` 的内存区域中。

```c
/* 程序代码 */
.text :
 {
   *(.text*)
 } >FLASH
```

> **`.text :`**
>
> - 解释：定义一个名为 `.text` 的输出段。
>
> **`\*(.text\*)`**
>
> - 解释：这里的 `*` 是通配符。`(.text*)` 意味着匹配所有输入文件中以 `.text` 开头的段，例如 `.text`、`.text.main`、`.text.foo` 等。这会将所有函数编译后的机器码都收集到这个 `.text` 输出段中。
>
> **`>FLASH`**
>
> - 解释：将整个 `.text` 输出段（即所有代码）放置到 `FLASH` 内存区域中。

```c
/* 已初始化变量 */
.data :
 {
   _sdata = .;
   *(.data*)
   _edata = .;
 } >SRAM AT> FLASH
```

> **`.data :`**
>
> - 解释：定义一个名为 `.data` 的输出段。
>
> **`_sdata = .;`**
>
> - 语法：`symbol_name = .;`
> - 解释：定义一个名为 `_sdata` 的新符号，并将其值设置为当前定位计数器 `.` 的值。此时，`.` 的值是 `.data` 段在SRAM中的起始地址。这个符号可以被启动代码引用，以知道从哪里开始存放变量。
>
> **`\*(.data\*)`**
>
> - 解释：收集所有输入文件中以 `.data` 开头的段（即所有已初始化的全局变量和静态变量）。
>
> **`_edata = .;`**
>
> - 解释：定义一个名为 `_edata` 的新符号，其值为当前定位计数器 `.` 的值，也就是 `.data` 段在SRAM中的结束地址。
>
> **`>SRAM AT> FLASH`**
>
> - 语法：`> vma_region AT> lma_region`
> - 解释：这是链接器脚本中最关键的语法之一。
>   - **`>SRAM`**: 指定了该段的**运行时地址 (VMA - Virtual Memory Address)**。也就是说，当程序跑起来的时候，这些变量位于 `SRAM` 中。
>   - **`AT> FLASH`**: 指定了该段的**加载地址 (LMA - Load Memory Address)**。也就是说，在生成的 `.bin` 文件中，这些变量的初始值是存放在 `FLASH` 里的。
>   - 启动代码的任务就是根据 LMA (`AT> FLASH`) 在Flash中找到这些数据，然后把它们复制到 VMA (`>SRAM`) 指定的SRAM位置。

```c
/* 未初始化变量 */
.bss :
 {
   _sbss = .;
   *(.bss*)
   *(COMMON)
   _ebss = .;
 } >SRAM
```

> **`.bss :`**
>
> - 解释：定义一个名为 `.bss` 的输出段。
>
> **`_sbss = .;` 和 `_ebss = .;`**
>
> - 解释：同理，定义了两个符号 `_sbss` 和 `_ebss`，分别标记了 `.bss` 段在SRAM中的起始和结束地址。启动代码会使用这两个地址，将这块SRAM区域清零。
>
> **`\*(.bss\*)`**
>
> - 解释：收集所有输入文件中以 `.bss` 开头的段。
>
> **`\*(COMMON)`**
>
> - 解释：`COMMON` 是一种特殊的未初始化数据段。C语言中允许多个文件定义同名的未初始化全局变量，链接器会将它们合并到 `COMMON` 空间。这条指令就是把这些数据也包含进来。
>
> **`>SRAM`**
>
> - 解释：将 `.bss` 段放置到 `SRAM` 内存区域。注意，这里**没有 `AT>`**，因为 `.bss` 段没有初始值需要从Flash加载，它只是在运行时占据一块需要被清零的SRAM空间。

---

**链接器所需的输入文件是什么？**

主要包括两大类：

- 目标文件 (Object Files, `.o` 或 `.obj` 后缀)
  - 在你的STM32项目中，通常会有很多个 `.c` 文件（例如 `main.c`, `usart.c`, `gpio.c` 等）和 `.s` 文件（汇编启动文件）。**编译器（Compiler）会对每一个** `.c` 文件进行独立的编译，将其从C语言翻译成机器码，并生成一个与之对应的 `.o` 目标文件。
  - 这些 `.o` 文件就是链接器的主要输入。它们是半成品，包含了代码和数据，但地址都还没有确定。
- 库文件 (Library Files, `.a` 或 `.lib` 后缀)
  - 库文件本质上就是一堆**预先编译好的`.o`文件的压缩包**。
  - 你在代码中调用了 `printf` 函数，但你并没有写 `printf` 的源代码。这个函数的实现在C标准库 (`.a` 文件)中。链接器会自动从库文件中找到 `printf` 对应的那个 `.o` 文件，并将其作为输入。STM32的HAL库、标准外设库等也是以这种形式提供的。

---

#### 3.✍️ 定义重要符号 (Define Important Symbols)

链接器脚本可以在链接过程中创建一些非常有用的地址符号（你可以理解为“标签”），这些符号可以在C代码或汇编代码中通过`extern`来引用。

- 从上面的例子中，我们看到了 `_sdata`, `_edata`, `_sbss`, `_ebss` 等符号。
- **它们有什么用？** 启动代码 (`startup_stm32f40xx.s`) 需要知道 `.data` 段在Flash中的起始地址、在SRAM中的起始和结束地址，才能正确地进行复制。它也需要知道 `.bss` 段在SRAM中的起始和结束地址，才能正确地进行清零。这些信息就是通过链接器定义的这些符号来传递的。

#### 4. 🚀 指定程序入口点 (Specify the Entry Point)

脚本会明确告诉链接器，程序的“第一行代码”是哪一个函数。对于STM32来说，这永远是`Reset_Handler`。

```c
ENTRY(Reset_Handler)
```

## 指定编译器路径

由于项目使用的是gcc编译器，而不是集成在Keil MDK中的ARMcc或Armclang。

[Downloads | GNU Arm Embedded Toolchain Downloads – Arm Developer](https://developer.arm.com/downloads/-/gnu-rm)

在wsl中输入`uname -a`查看linux系统的架构，

```bash
(base) zyc@Win-20250418BYV:~$ uname -a
Linux Win-20250418BYV 5.15.167.4-microsoft-standard-WSL2 #1 SMP Tue Nov 5 00:21:55 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
```

所以选择`x86_64`架构的版本：

![image-20251013111415649](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F10%2Fa1174076a9af44facd0b9dc1e7c003b1.png)

下载完成后，将压缩包放到工程的根目录里。

之后，在工程的根目录里新建一个目录叫`tools/toolchain/`，解压刚刚下载的文件到该路径：

```bash
tar xjvf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 -C tools/toolchain/
```

解压后工程目录如下：

![image-20251013113154148](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F10%2Fb33836e507a0f64ebe670f40354b2561.png)

接下来需要配置C/C++的编译器路径，

在上方搜索框输入`> c/c++`打开UI配置，设置编译器路径，这里的UI配置的修改会同步到`c_cpp_properties.json`文件中。

![image-20251013170716352](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F10%2F01971106afcd9aa041076555d834f4c6.png)

接着继续改动以下几个地方：

- C 标准：C99
- C++ 标准：C++11
- IntelliSense 模式：linux-gcc-arm
- 浏览: 将符号限制为包含的标头：勾上

最后完整的`.vscode/c_cpp_properties.json`为：

```json
{
    "configurations": [
        {
            "name": "stm32f4",
            "includePath": [
                "boot",
                "platform/cmsis/core",
                "platform/cmsis/device",
                "platform/driver/inc"
            ],
            "defines": [],
            "compilerPath": "/home/zyc/Projects/stm32f4/tools/toolchain/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc",
            "cStandard": "c99",
            "cppStandard": "c++11",
            "intelliSenseMode": "linux-gcc-arm",
            "browse": {
                "limitSymbolsToIncludedHeaders": true
            }
        }
    ],
    "version": 4
}
```

## Makefile文件编写

为了方便起见，直接使用STM32CubeMX生成Makefile，

![image-20251014135239837](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F10%2F51037a2b58a6f65ad97f1f1eab37b485.png)

此时会在项目文件夹下发现，已经生成了一个Makefile文件：

![image-20251014135343542](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F10%2Fe8ac4583bd3d52d404ef142dab0f0172.png)

直接将这个Makfile复制到我们的wsl路径中，并对其进行修改，需要修改如下几个地方：

```makefile
# 项目名称
TARGET = stm32f4

# 源文件所在目录
C_DIRS = \
boot \
platform/cmsis/device \
platform/driver/src

# C sources
C_SOURCES = $(wildcard $(addsuffix /*.c, $(C_DIRS)))
```

> 解释一下这一行代码
>
> ```makefile
> C_SOURCES = $(wildcard $(addsuffix /*.c, $(C_DIRS)))
> ```
>
> - `$(C_DIRS)`：变量引用，变量本身应该在Makefile的前面某处被定义，它包含了一个或多个存放C源代码的目录（文件夹）路径。
> - `$(addsuffix /*.c, $(C_DIRS))`：它会将第一个参数（这里是 `/*.c`）作为后缀，添加到第二个参数（一个列表，这里是 `$(C_DIRS)` 的内容）中的每一项后面。
>   - 这里提一下makefile函数的使用格式：`$(function argument1, argument2, ...)`
> - `$(wildcard ...)`：它接收一个或多个包含通配符（如 `*`）的模式作为参数，然后去文件系统中**搜索所有匹配该模式的、真实存在的文件**，并返回这些文件的完整路径列表。
>
> 如果需要测试一下获取到的`C_SOURCES`变量是否正确，可以使用打印函数`@echo`在文件结尾加入：
>
> ```
> test:
> 	@echo $(C_SOURCES)
> ```
>
> 以查看该变量的内容。



