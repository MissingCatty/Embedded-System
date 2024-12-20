# 1.新建STM32工程

## 1.1 准备工作

- 安装Keil5（默认已完成）
- 准备STM32F10x系列固件库

---

**固件库下载**

- 到以下地址下载固件库

  https://www.st.com/zh/embedded-software/stm32-standard-peripheral-libraries.html

- 打开后点击图片中的F1的蓝色方块

  ![image-20241219184037692](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219184037692.png)

- 选择对应的版本下载，得到一个`en.stsw-stm32054_v3-6-0.zip`压缩文件，解压后得到`STM32F10x_StdPeriph_Lib_V3.6.0`的文件夹，文件夹内容如下：

  ![image-20241219184331761](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219184331761.png)

  `Libraries`是固件库源码，`Project`是例程，`Utilities`是测试工具

## 1.2 新建Keil工程

- 打开Keil，工具栏中`Project-New μVision Project`新建工程（选择对应芯片不做详细介绍），得到以下项目目录结构

  ![image-20241219184943772](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219184943772.png)

  - `Listings` 文件夹存储了编译器在编译源代码时生成的中间文件，通常是一些汇编代码或其他相关的调试信息，该文件夹通常包括每个源文件的 `.lst` 文件，这些文件包含源代码和编译时生成的汇编代码。
  - `Objects` 文件夹存储的是编译过程中生成的目标文件（`.o` 文件或 `.obj` 文件）。这些文件是源代码经过编译后的二进制文件，但还没有经过链接生成最终的可执行文件。每个源文件会对应一个目标文件，例如 `main.o` 或 `stm32f4xx_hal_msp.o`。

---

**Start文件夹**

- 从固件库源码中拷贝STM32的启动文件，放到工程根目录`~/Start`文件夹下，启动文件存放在固件库目录`STM32F10x_StdPeriph_Lib_V3.6.0\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm`里
- 同样从`STM32F10x_StdPeriph_Lib_V3.6.0\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup`里拷贝三个文件到`~/Start`文件夹下
  - `stm32f10x.h`：外设寄存器描述文件，描述STM32的寄存器及其对应地址
  - `system_stm32f10x.c`
  - `system_stm32f10x.h`：配置系统时钟
- STM32由Cortex-M3内核及片上外设组成，所以需要添加内核寄存器描述文件。将`STM32F10x_StdPeriph_Lib_V3.6.0\Libraries\CMSIS\CM3\CoreSupport`目录里的`core.h`和`core.c`复制到`~/Start`文件夹下
- 在Keil5的工具栏中的“Project-Mange-Project Items”，创建`Start`分组（Group），并把`~/Start`文件夹里的`.c`文件添加到工程里
  - `_md.s`
  - 所有`.c`和`.h`文件
- 点击魔术棒，添加头文件路径

---

**User文件夹**

- 创建User文件夹，在Group里同样添加一个User组

- 右击左侧的User，新建一个`main.c`文件
- 在文件中写入一个死循环用来测试
- 点击构建项目

此时项目已经能够通过寄存器的方式烧写到板子上了，以下是固件库方式还需要添加的几个文件

- `STM32F10x_StdPeriph_Lib_V3.6.0\Project\STM32F10x_StdPeriph_Template`目录下
  - `stm32f10x_conf.h`：用来配置库函数头文件的包含关系，以及用来参数检查的函数定义
  - `stm32f10x_it.c`
  - `stm32f10x_it.h`：存放中断函数
- 点击魔术棒-C/C++添加头文件路径

---

**Library文件夹**

该文件夹用于存放库函数

- 在项目根目录创建文件夹，并添加组
- 将`STM32F10x_StdPeriph_Lib_V3.6.0\Libraries\STM32F10x_StdPeriph_Driver\src`里的`.c`文件全部复制到Library文件夹中
- 将`STM32F10x_StdPeriph_Lib_V3.6.0\Libraries\STM32F10x_StdPeriph_Driver\inc`里的`.h`文件全部复制到Library文件夹中
- 在组里添加所有`.c`文件
- 添加头文件路径

---

- 在魔术棒的C/C++中，添加`USE_STDPERIPH_DRIVER`宏（对应`stm32f10x.h`的8341行）

# 题外话

## 1.手动添加编译器

由于MDK5.37不再支持ARM编译器v5，所以需要手动添加，否则编译时会出现问题

- 将ARMCC5安装到`Keil_v5/Arm`目录下（否则会有授权问题）

- `Manage Project Items-Folders/Extensions-...`中添加ARMCC5的安装根目录
- 点击“魔术棒”，在`Target`下选择ARM编译器

## 2.修改Keil字体

点击扳手图标

![image-20241219213717069](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219213717069.png)

### 3.使用新版MDK，程序下载后无法直接运行

两个地方：

- 魔术棒-Debug-”ST-Link Debuffer“右侧的Settings-”Flash Download“-勾选”Reset and Run“

  ![image-20241220133833007](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220133833007.png)

- 魔术棒-Debug-”ST-Link Debuffer“右侧的Settings-”Pack“-把Enable取消勾选

  ![image-20241220133842679](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220133842679.png)