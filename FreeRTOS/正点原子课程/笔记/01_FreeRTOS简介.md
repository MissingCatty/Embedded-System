# 1.RTOS简介

RTOS 全称是 **Real Time Operating System**，例如：UCOS, FreeRTOS, RTX, RT-thread

操作系统允许多个任务同时运行，实际上，一个处理器核心在某一时刻只能运行一个任务。操作系统中任务调度器的责任就是决定在某一时刻究竟运行哪个任务，任务调度在各个任务之间的切换非常快，这就给人们造成了同一时刻有多个任务同时运行的错觉。

操作系统的分类方式可以由任务调度器的工作方式决定，比如有的操作系统给每个任务分配同样的运行时间，时间到了就轮到下一个任务，Unix 操作系统就是这样的。

实时环境中要求操作系统必须对某一个事件做出实时的响应，因此系统**任务调度器的行为必须是可预测的**。像 FreeRTOS 这种传统的 **RTOS 类操作系统是由用户给每个任务分配一个任务优先级**，任务调度器就可以根据此优先级来决定下一刻应该运行哪个任务。

# 2.FreeRTOS的优势

学习RTOS，UCOS是首选，因为中文资料多，但是收费

1. 做产品是要考虑成本的，所以免费RTOS是好的选择
2. 许多其他半导体厂商的产品的SDK包使用FreeRTOS作为操作系统，尤其是WIFI、蓝牙这些带协议栈的芯片和模块
3. 许多软件厂商也使用FreeRTOS作为本公司软件的操作系统，其历程基于FreeRTOS编写
4. 使用简单，FreeRTOS的文件数量相比于UCOS少很多
5. 文档相对齐全，官网（www.freertos.org）上都能找到对应的源码和文档
6. 社会占有量高

# 3.FreeRTOS特点

1. 内核支持**抢占式**，**合作式**和**时间片调度**
2. 提供了一个用于**低功耗的Tickless 模式**
3. 系统的组件在创建时可以选择**动态或者静态的 RAM**，比如任务、消息队列、信号量、软件定时器等等
4. 已经在超过 30 种架构的芯片上进行了移植
5. FreeRTOS 系统简单、小巧、易用，通常情况下内核占用 **4~9kB**的空间
6. 高可移植性，代码主要 C 语言编写
7. 支持实时任务和协同程序
8. 任务与任务、任务与中断之间可以使用任务通知、消息队列、二值信号量、数值型信号量、递归互斥信号量和互斥信号量进行通信和同步。
9. 创新的事件组
10. 具有优先级继承特性的互斥信号量
11. 高效的软件定时器
12. 强大的跟踪执行功能
13. 堆栈溢出检测功能
14. 任务数量不限
15. 任务优先级不限

# 4.FreeRTOS资料

进入官网www.freertos.org

查看官方已经移植好的支持的设备[Supported Devices - FreeRTOS™](https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/00-Supported-devices)，如果已经支持，说明我们可以直接拿过来稍作修改在自己的板子上用

官方手册[FreeRTOS Documentation - FreeRTOS™](https://www.freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book)

Cortex-M架构参考书：《The Definitive Guide to ARM Cortex-M3 and Cortex-M4 Processors, 3rd Edition》，中文版《ARM Cortex-M3 与 Cortex-M4 权威指南(第三版)》

# 5.FreeRTOS源码预览

从官网下载好包含源码的zip文件，解压得到如下路径

![image-20241219151913281](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219151913281.png)

重点在于`FreeRTOS`和`FreeRTOS-Plus`两个文件夹，这两个文件夹下为FreeRTOS源码，区别就是Plus版比普通版功能多一点。

## 5.1 FreeRTOS文件夹

![image-20241219152231718](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219152231718.png)

- Demo文件夹：FreeRTOS相关例程，该文件夹中提供了包含针对不同MCU的历程，从中可以找到`CORTEX_STM32F103_Keil`是关于STM32F103系列的历程
- License文件夹：许可信息
- Source文件夹：FreeRTOS源码

## 5.2 FreeRTOS源码文件夹

![image-20241219153251375](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219153251375.png)

`include`文件夹下是头文件，`portable`文件夹下是源码，打开`portable`文件夹：

![image-20241219153757854](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219153757854.png)

该文件夹下，包含了多个**编译器**和**硬件平台**的子文件夹

|   文件夹    |                             内容                             |
| :---------: | :----------------------------------------------------------: |
|  ARMCLang   |              ARM 的 Clang **编译器**的移植代码               |
|   ARMv8M    |         ARMv8-M **架构**（如 Cortex-M33）的移植代码          |
|     BCC     |                Borland C **编译器**的移植支持                |
|     CCS     | TI Code Composer Studio (CCS) **编译器**的移植支持，通常用于德州仪器（TI）的处理器 |
| CodeWarrior |      NXP（前 Freescale）的 CodeWarrior **编译器**的支持      |
|   Common    |                   共享代码或通用的移植实现                   |
|     GCC     | GNU **编译器**（GCC）的支持代码，广泛用于多个平台（如 **ARM Cortex** 系列） |
|     IAR     |         IAR Embedded Workbench **编译器**的移植代码          |
|  **Keil**   |  ARM 的 Keil MDK **编译器**支持，常用于 Cortex-M 系列处理器  |
| **MemMang** |                       内存管理相关代码                       |
|   MikroC    |           MikroElektronika MikroC **编译器**的支持           |
|    MPLAB    |   Microchip MPLAB X IDE **编译器**（如 XC16、XC32）的支持    |
| MSVC-MingW  | 微软 Visual Studio 和 MinGW **编译器**的支持，通常用于在 Windows 上的模拟和测试 |
|   oWatcom   |                  Open Watcom **编译器**支持                  |
|  Paradigm   |                 Paradigm C++ **编译器**支持                  |
|   Renesas   | 针对瑞萨（Renesas）**处理器的移植**支持，通常用于其特定编译器和架构 |
|   Rowley    |              Rowley CrossWorks **编译器**的支持              |
|  **RVDS**   | ARM RealView Development Suite 的支持（已被 ARM Compiler 替代） |
|    SDCC     | Small Device C Compiler (SDCC) 的支持，常用于小型 8 位和 16 位处理器 |
|   Softune   |          富士通（Fujitsu）**处理器**相关的支持代码           |
|   Tasking   |                 TASKING **编译器**的移植支持                 |
| ThirdParty  |               第三方工具链或平台的相关支持代码               |
|    WizC     |                针对 WizC **编译器**的移植支持                |

---

**`portable`文件夹**

Keil 文件夹里面的东西肯定是必须的，但是我们打开Keil文件夹以后里面只有一个文件：`See-also-the-RVDS-directory.txt`，意思就是参考 RVDS文件夹里面的东西

RVDS 文件夹针对不同的架构的 MCU 做了详细的分类，STM32F103 就参考 ARM_CM3

![image-20241219160127844](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219160127844.png)

其中包含两个文件`port.c`和`portmacro.h`，这两个文件就是移植的时候需要的

## 5.3 FreeRTOS-Plus源码文件夹

打开`FreeRTOS-Plus/Source`查看源码

![image-20241219160732037](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241219160732037.png)

可以看出，FreeRTOS-Plus 中的源码其实并不是 FreeRTOS 系统的源码，而是**在 FreeRTOS系统上另外增加的一些功能代码**，比如 CLI、FAT、Trace 等等。就系统本身而言，和 FreeRTOS里面的一模一样的，所以我们如果只是学习 FreeRTOS 这个系统的话，FreeRTOS-Plus 就没必要看了。



