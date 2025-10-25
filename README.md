# 1.新建STM32工程

## 1.1 准备工作

- 安装Keil5（默认已完成）
- 准备STM32F10x系列固件库

---

**固件库下载**

- 到以下地址下载固件库

  https://www.st.com/zh/embedded-software/stm32-standard-peripheral-libraries.html

- 打开后点击图片中的F1的蓝色方块

  ![image-20241219184037692](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F5e0cfc3ade42366fcbb2ad4907a8f359.png)

- 选择对应的版本下载，得到一个`en.stsw-stm32054_v3-6-0.zip`压缩文件，解压后得到`STM32F10x_StdPeriph_Lib_V3.6.0`的文件夹，文件夹内容如下：

  ![image-20241219184331761](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F14146c7c90a6dd3e58bfb3bf6e7c26ec.png)

  `Libraries`是固件库源码，`Project`是例程，`Utilities`是测试工具

## 1.2 新建Keil工程

- 打开Keil，工具栏中`Project-New μVision Project`新建工程（选择对应芯片不做详细介绍），得到以下项目目录结构

  ![image-20241219184943772](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F4595347112d2fbc0b89fc02f6957cd80.png)

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

  ![image-20251025155242351](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/25%2F10%2F4ed3df67e4bcc35f193e0c4b4a868178.png)
- 点击“魔术棒”，在`Target`下选择ARM编译器

## 2.修改Keil字体

点击扳手图标

![image-20241219213717069](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F41327efb98d85e043f52919c85cc7462.png)

![image-20250503202539976](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F5683f6e10688439005fe793eb2bd6080.png)

## 3.使用新版MDK，程序下载后无法直接运行

两个地方：

- 魔术棒-Debug-”ST-Link Debugger“右侧的Settings-”Flash Download“-勾选”Reset and Run“

  ![image-20241220133833007](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F1b23f99886cc126d5dbf653284553010.png)

- 魔术棒-Debug-”ST-Link Debuffer“右侧的Settings-”Pack“-把Enable取消勾选

  ![image-20241220133842679](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F22e8ffe884e2ada2d369f3b3864bf907.png)

## 4.使用Keil的示波器软件调试

- 点击“扳手”，修改其中的配置

  ![image-20250205230007730](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2Fb23266ac2dc8df6ecde3ef047a1b41d0.png)

- 进入Debug模式，并开启“逻辑分析仪”

  ![image-20250205230202862](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F05%2F9d1be9af7a5ac9899e64fb7bde0e97a2.png)

- 点击左上角的"Setup"按钮，设置要追踪的引脚

  - 引脚设置语法为`GPIOx_IDR.y`，其中`x`为端口，`y`为引脚

- 设置”Disaplay Type“为“Bit”

- 随后就可以调试了

## 5. 每次安装Keil后记得要破解

以**管理员**身份运行Keil，点击”License Management“ 进行下一步

![](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/25%2F10%2F68ccea7e091e7c5cbc836d108a62f8f9.png)

在这里复制我们自己的 CID

![CID](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/25%2F10%2Ff9687edaacc1da81fad2f7f7512b07b4.png)

然后打开我们的注册机，把我们的 CID 粘贴到对应处 Target

这里选择我们 ” ARM “

![生成](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/25%2F10%2Fb8696b13e9afb4151a0a13996d09d725.png)

回到Keil5 中 将生成的 ID Code 粘贴至”New License ID Code“ 处 并且点击 ”Add LIC“

![ID COde](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/25%2F10%2F783f9c830068086c9a45da51987768ba.png)

”Add LIC“后 就出现如图片中的界面 说明我们完成了注册机的注册
