FreeRTOS 的中断配置是一个很重要的内容，任务的切换和管理就是靠中断来实现的。

# 1 Cortex-M中断

Cortex-M 内核的 MCU 提供了一个用于中断管理的嵌套向量中断控制器(NVIC)。Cotex-M3 的 NVIC 最多支持 240 个 IRQ(中断请求)、1 个不可屏蔽中断(NMI)、1 个 Systick(滴答定时器)定时器中断和多个系统异常。

Cortex-M 处理器有多个用于管理中断和异常的可编程寄存器，这些寄存器大多数都在NVIC 和系统控制块(SCB)中，CMSIS 将这些寄存器定义为结构体。以 STM32F103 为例，打开**core_cm3.h**，有两个结构体，NVIC_Type 和 SCB_Type。这些寄存器在移植RTOS时是不需要关心的，详细的需查看《Cortex-M权威指南》。

需要重点关系的是三个中断屏蔽寄存器：`PRIMASK`, `FAULTMASK`, `BASEPRI`

## 1.1 优先级分组

当多个中断来临的时候处理器应该响应哪一个中断是由中断的优先级来决定的，高优先级的中断(优先级编号小)肯定是首先得到响应，而且高优先级的中断可以抢占低优先级的中断，这个就是**中断嵌套**。Cortex-M 处理器的有些中断是具有固定的优先级的，比如复位、NMI、HardFault，这些中断的优先级都是负数，优先级也是最高的。

Cortex-M 处理器有**三个固定优先级**和 **256 个可编程的优先级（8位）**，最多有 128 个抢占等级，但是实际的优先级数量是由芯片厂商来决定的。**绝大多数的芯片都会精简设计**的，比如 STM32 就只有 16 级优先级。在设计芯片的时候会裁掉表达优先级的几个低端有效位，以减少优先级数，所以不管用多少位来表达优先级，都是 MSB 对齐的，例如下面这张图就是一个最多8优先级的芯片：

![image-20241221121536078](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241221121536078.png)

---

**为什么最多是128个抢占等级？**

Cortex-M处理器把256个优先级分为高低两个部分（如上图的白黄两部分），分别是抢占优先级和响应优先级（响应优先级也称亚优先级），高位的部分（左边）对应抢占优先级，低位部分（右边）对应响应优先级。

STM32中的`AIRCR`寄存器有个位段为`PRIGROUP-优先级组`。优先级分组就是确定优先级高低两部分的长度，其值为分隔的位置（插板位置），例如`PRIGROUP=1`，表示在`Bit1`左边插板，主优先级为`[7:2]`，亚优先级为`[1:0]`，所有的对应关系如下：

| 分组位置 | 主优先级的位段 | 亚优先级的位段 |
| -------- | -------------- | -------------- |
| 0 (默认) | [7:1]          | [0:0]          |
| 1        | [7:2]          | [1:0]          |
| 2        | [7:3]          | [2:0]          |
| 3        | [7:4]          | [3:0]          |
| 4        | [7:5]          | [4:0]          |
| 5        | [7:6]          | [5:0]          |
| 6        | [7:7]          | [6:0]          |
| 7        | 无             | [7:0]          |

- 主优先级：决定是否允许一个中断打断另一个中断的执行。
- 亚优先级：在多个中断具有相同主优先级时，亚优先级决定它们的执行顺序。

---

STM32 仅仅使用了8位中的4位，所以最多有5种优先级分组方法，这五个分组在`misc.h`中定义。

```c
#define NVIC_PriorityGroup_0 ((uint32_t)0x700) /*!< 0 bits for pre-emption priority
 4 bits for subpriority */
#define NVIC_PriorityGroup_1 ((uint32_t)0x600) /*!< 1 bits for pre-emption priority
 3 bits for subpriority */
#define NVIC_PriorityGroup_2 ((uint32_t)0x500) /*!< 2 bits for pre-emption priority
 2 bits for subpriority */
#define NVIC_PriorityGroup_3 ((uint32_t)0x400) /*!< 3 bits for pre-emption priority
 1 bits for subpriority */
#define NVIC_PriorityGroup_4 ((uint32_t)0x300) /*!< 4 bits for pre-emption priority
 0 bits for subpriority */
```

在移植RTOS时，我们需要配置的是`NVIC_PriorityGroup_4`，因为**FreeRTOS的中断配置没有处理亚优先级这种情况，所以只能配置为组4，直接就16个优先级**，使用起来也简单。

## 1.2 优先级设置

Cortex-M3最多支持240个外部中断，每个**外部中断**（**GPIO引脚触发的中断**）都有一个对应的优先级寄存器，每个寄存器占8位。

| 名称    | 类型 | 地址        | 复位值   | 描述                |
| ------- | ---- | ----------- | -------- | ------------------- |
| PRI_0   | R/W  | 0xE000_E400 | 0 (8 位) | 外中断#0 的优先级   |
| PRI_1   | R/W  | 0xE000_E401 | 0 (8 位) | 外中断#1 的优先级   |
| PRI_2   | R/W  | 0xE000_E402 | 0 (8 位) | 外中断#2 的优先级   |
| PRI_3   | R/W  | 0xE000_E403 | 0 (8 位) | 外中断#3 的优先级   |
| ...     | ...  | ...         | ...      | ...                 |
| PRI_239 | R/W  | 0xE000_E4EF | 0 (8 位) | 外中断#239 的优先级 |

| 名称   | 类型 | 地址        | 复位值 | 描述                    |
| ------ | ---- | ----------- | ------ | ----------------------- |
| PRI_4  |      | 0xE000_ED18 |        | 存储管理 fault 的优先级 |
| PRI_5  |      | 0xE000_ED19 |        | 总线 fault 的优先级     |
| PRI_6  |      | 0xE000_ED1A |        | 用法 fault 的优先级     |
| -      |      | 0xE000_ED1B |        | -                       |
| -      |      | 0xE000_ED1C |        | -                       |
| -      |      | 0xE000_ED1D |        | -                       |
| -      |      | 0xE000_ED1E |        | -                       |
| PRI_11 |      | 0xE000_ED1F |        | SVC 优先级              |
| PRI_12 |      | 0xE000_ED20 |        |                         |
| -      |      | 0xE000_ED21 |        | -                       |
| PRI_14 |      | 0xE000_ED22 |        | PendSV 的优先级         |
| PRI_15 |      | 0xE000_ED23 |        | SysTick 的优先级        |

FreeRTOS在管理这些中断地址时，会将4个连续的寄存器拼接成一个32位的寄存器，所`0xE000_ED20~0xE000_ED23` 这四个寄存器就可以拼接成一个地址为 `0xE000_ED20` 的 32 位寄存器。这一点很重要！因为 **FreeRTOS 在设置 PendSV 和 SysTick 的中断优先级的时候都是直接操作的地址 `0xE000_ED20`**。

## 1.3 用于中断频闭的特殊寄存器

### 1.3.1 PRIMASK寄存器

在许多应用中，需要**暂时屏蔽所有的中断以执行一些对时序要求严格的任务**，这个时候就可以使用 PRIMASK 寄存器，**PRIMASK 用于禁止除 NMI（Non-Maskable Interrupt，不可屏蔽中断） 和 HardFalut（硬件异常） 外的所有异常和中断**。

UCOS 中的**临界区代码代码保护就是通过开关中断实现**的，而开关中断就是直接操作 PRIMASK寄存器的，所以在 UCOS 中关闭中断的时候时关闭了除复位、NMI 和 HardFault 以外的所有中断。

### 1.3.2 FAULTMASK寄存器

FAULTMASK 比 PRIMASK 更狠，它可以连 HardFault 都屏蔽掉，使用方法和 PRIMASK 类似，FAULTMASK 会在退出时自动清零。

### 1.3.3 BASEPRI寄存器

PRIMASK 和 FAULTMASK 寄存器太粗暴了，直接关闭除复位、NMI 和 HardFault 以外的其他所有中断，但是在有些场合需要对中断屏蔽进行更细腻的控制，比如只屏蔽优先级低于某一个阈值的中断。那么这个作为阈值的优先级值存储在哪里呢？

**FreeRTOS 的开关阈值中断就是操作 BASEPRI 寄存器来实现的**！BASEPRI寄存器用于关闭低于BASEPRI中的值的中断，高于这个值的中断就不会被关闭！

如果**向BASEPRI写0的话就会停止屏蔽中断**。

# 2 FreeRTOS中断配置宏

## 2.1 configPRIO_BITS

优先级位数

此宏用来设置 MCU 使用几位优先级，STM32 使用的是 4 位，因此此宏为 4！

## 2.2 configLIBRARY_LOWEST_INTERRUPT_PRIORITY

最低中断优先级

此宏是用来设置最低优先级，前面说了，STM32 优先级使用了 4 位，而且 STM32 配置的使用组 4，也就是 4 位都是抢占优先级。因此优先级数就是 16 个，最低优先级那就是 15，所以此宏就是 15。

## 2.3 configKERNEL_INTERRUPT_PRIORITY

内核中断优先级

```c
#define configKERNEL_INTERRUPT_PRIORITY \\
	(configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
```

为什么要移`8 - configPRIO_BITS`位？

因为之前说过，优先级位数是高`configPRIO_BITS`位，在使用时需要对优先级进行高位对齐。

- 作用：用来设置 **PendSV** 和**滴答定时器**的中断优先级，`port.c`中有如下定义：

  ```c
  #define portNVIC_PENDSV_PRI (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 16UL)
  
  #define portNVIC_SYSTICK_PRI (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 24UL)
  ```

  为什么要左移16位和24位？因为1.2中说过，FreeRTOS 在设置 PendSV 和 SysTick 的中断优先级的时候都是直接操作的地址 `0xE000_ED20`（四个8位寄存器拼起来的32位寄存器），一次写入32位的数据。

  | 0xE000_ED23<br />高`8 - configPRIO_BITS`位有效 | 0xE000_ED22<br />高`8 - configPRIO_BITS`位有效 | 0xE000_ED21<br />高`8 - configPRIO_BITS`位有效 | 0xE000_ED20<br />高`8 - configPRIO_BITS`位有效 |
  | :--------------------------------------------: | :--------------------------------------------: | :--------------------------------------------: | :--------------------------------------------: |
  |                    SysTick                     |                     PendSV                     |                       -                        |                       -                        |


---

**PendSV 和 SysTick 中断是什么？**

FreeRTOS 的任务调度方法主要依赖于以下几个关键要素：

- **优先级调度**：任务优先级决定了任务的执行顺序。
- **抢占式调度**：默认情况下，任务调度是抢占式的，高优先级任务可以抢占低优先级任务。
- **时间片轮转**：对于相同优先级的任务，FreeRTOS 使用时间片轮转来公平分配 CPU 时间。

任务切换的两种情况：

- 时间片用完
- 有更高优先级的中断

`SysTick`是一个系统定时器，用于生成定时中断。每次定时中断发生，会让OS做两件事：

- 检查当前任务时间片是否用完
- 检查是否有更高优先级的任务等待执行

当`SysTick`发现任务需要切换时，OS会决定是否调用`PendSV`进行任务切换。

---

**SysTick和PendSV优先级设置**

在`port.c`文件中的`xPortStartScheduler()`函数中设置

```c
BaseType_t xPortStartScheduler( void )
{
	...
        
    /* Make PendSV and SysTick the lowest priority interrupts. */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;	// 设置PENDSV优先级

    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI; // 设置SYSTICK优先级
    
    ...
}
```

上述代码直接向`portNVIC_SHPR3_REG`中写入数据，`portNVIC_SHPR3_REG` 是个宏，在文件 `port.c` 中由定义如下：

```C
#define portNVIC_SHPR3_REG	( *( ( volatile uint32_t * ) 0xe000ed20 ) )
```

可以看出，其实`portNVIC_SHPR3_REG`就是指示了`0xe000ed20`中的内容。同时也可以看出在 FreeRTOS中 **PendSV 和 SysTick 的中断优先级都是最低的**！

---

**为什么PendSV 和 SysTick中断优先级最低？**

**SysTick**：操作系统的心跳节拍本质上讲，就是一个保证操作系统正常运行的节拍而已，就像人的心跳一样，有的人60次/分，有的人70次/分，有的人80次/分，没有唯一答案，你肯定也不会想着要让自己的心跳整齐划一的一直是66次/分。有个大概正常稳定的范围就可以了。嵌入式OS的时钟节拍，压根就不是精准的，也不需要太准，大概准就可以了，因为只要有周期性的节拍，就能保证周期性的调度。**软件定时器也不是特别准的，一般用于【短时间】、对时间要求不严苛的场景。如果非要对时间要求的特别准，还是需要用硬件定时器实现的。**
**PendSV**：一个专门用于 **任务上下文切换** 的中断，它的作用是执行任务的切换操作。当任务需要切换时，`PendSV` 会被触发来保存当前任务的上下文（即寄存器状态），并恢复下一个任务的上下文，完成任务的切换。既然操作系统也是程序，那么**操作系统的优先级肯定不能超过外部中断的优先级，因为外部中断一般是硬件中断，优先级都需要很高。**

---

## **2.4 configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY**

最大中断优先级

BASEPRI 寄存器说的那个阈值优先级，这个大家可以自由设置，这里我设置为了 5。也就是高于 5 的优先级不归 FreeRTOS 管理！

## 2.5 configMAX_SYSCALL_INTERRUPT_PRIORITY

此宏是 `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` 左移 4 位而来的，原因和宏 `configKERNEL_INTERRUPT_PRIORITY` 一样。

例如：`configMAX_SYSCALL_INTERRUPT_PRIORITY==5`，`configKERNEL_INTERRUPT_PRIORITY==15`

![image-20241221200816066](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241221200816066.png)

# 3 FreeRTOS开关中断

开关中断包含两个`portmacro.h`函数`portENABLE_INTERRUPTS ()`和`portDISABLE_INTERRUPTS()`，定义如下：

```c
#define portDISABLE_INTERRUPTS()	vPortRaiseBASEPRI()	// 将BASEPRI寄存器设置为OS可管理的最大优先级，即关中断
#define portENABLE_INTERRUPTS()		vPortSetBASEPRI(0)	// 将BASEPRI寄存器置0，即开中断
```

```c
static portFORCE_INLINE void vPortRaiseBASEPRI( void )
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

    __asm
    {
        /* Set BASEPRI to the max syscall priority to effect a critical
         * section. */
/* *INDENT-OFF* */
        msr basepri, ulNewBASEPRI	// 将RTOS可管理的最大优先级放到BASEPRI寄存器
        dsb
        isb
/* *INDENT-ON* */
    }
}
```

```c
// 向BASEPRI寄存器写入一个值，
static portFORCE_INLINE void vPortSetBASEPRI( uint32_t ulBASEPRI )
{
    __asm
    {
        /* Barrier instructions are not used as this function is only used to
         * lower the BASEPRI value. */
/* *INDENT-OFF* */
        msr basepri, ulBASEPRI
/* *INDENT-ON* */
    }
}
```

# 4 临界段

临界段代码也叫做临界区，是指那些必须完整运行，不能被打断的代码段，比如**有的外设的初始化需要严格的时序，初始化过程中不能被打断**。

FreeRTOS在进入临界段代码的时候需要关闭中断，当处理完临界段代码以后再打开中断。

## 4.1 任务级临界段代码保护

`taskENTER_CRITICAL()`和 `taskEXIT_CRITICAL()`是任务级的临界代码保护，一个是进入临界段，一个是退出临界段，这两个函数是成对使用的，这函数的定义如下：

```c
#define taskENTER_CRITICAL()	portENTER_CRITICAL()
#define taskEXIT_CRITICAL()		portEXIT_CRITICAL()
```

而 `portENTER_CRITICAL()`和 `portEXIT_CRITICAL()`也是宏定义，在文件 `portmacro.h` 中有定义，如下：

```c
#define portENTER_CRITICAL()	vPortEnterCritical()
#define portEXIT_CRITICAL()		vPortExitCritical()
```

函数 `vPortEnterCritical()`和 `vPortExitCritical()`在文件 `port.c` 中，函数如下：

```C
void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    /* This is not the interrupt safe version of the enter critical function so
     * assert() if it is being called from an interrupt context.  Only API
     * functions that end in "FromISR" can be used in an interrupt.  Only assert if
     * the critical nesting count is 1 to protect against recursive calls if the
     * assert function also uses a critical section. */
    if( uxCriticalNesting == 1 )
    {
        configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
    }
}

void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;

    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
```

可以看出在进入函数 `vPortEnterCritical()`以后会首先关闭中断，然后给变量 `uxCriticalNesting`加一，`uxCriticalNesting` 是个全局变量，用来记录临界段嵌套次数的。

函数 `vPortExitCritical()`是退出临界段调用的，函数每次将 `uxCriticalNesting` 减一，只有当 `uxCriticalNesting` 为 0 的时候才会调用函数 `portENABLE_INTERRUPTS()`使能中断。这样保证了在有多个临界段代码的时候不会因为某一个临界段代码的退出而打乱其他临界段的保护，**只有所有的临界段代码都退出以后才会使能中断**！

---

**任务级临界代码保护使用方法**

```c
void taskcritical_test(void)
{
    while(1)
    {
        taskENTER_CRITICAL();	// 进入临界区
        total_num+=0.01f;
        printf("total_num 的值为: %.4f\r\n",total_num);
        taskEXIT_CRITICAL();	// 退出临界区
        vTaskDelay(1000);
    }
}
```

**注意**：**临界区代码一定要精简**，因为进入临界区会关闭中断，这样会导致优先级低于 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 的中断得不到及时的响应。

## 4.2 中断级临界段代码保护

函数 `taskENTER_CRITICAL_FROM_ISR()`和 `taskEXIT_CRITICAL_FROM_ISR()`中断级别临界段代码保护，是**用在中断服务程序中的**。而且这个中断的优先级一定要低于`configMAX_SYSCALL_INTERRUPT_PRIORITY`！原因前面已经说了（FreeRTOS只管低于该阈值的中断）。

这两个函数在文件` task.h`中有如下定义：

```c
#define taskENTER_CRITICAL_FROM_ISR()	portSET_INTERRUPT_MASK_FROM_ISR()
#define taskEXIT_CRITICAL_FROM_ISR(x)	portCLEAR_INTERRUPT_MASK_FROM_ISR(x)
```

接着找`portSET_INTERRUPT_MASK_FROM_ISR()`和`portCLEAR_INTERRUPT_MASK_FROM_ISR()`，这两个在文件 `portmacro.h` 中有如下定义：

```c
#define portSET_INTERRUPT_MASK_FROM_ISR()		ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	vPortSetBASEPRI(x)
```

函数 `ulPortRaiseBASEPRI()`在文件 `portmacro.h` 中定义的，如下：

```c
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI( void )
{
    uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

    __asm
    {
        /* Set BASEPRI to the max syscall priority to effect a critical
         * section. */
/* *INDENT-OFF* */
        mrs ulReturn, basepri
        msr basepri, ulNewBASEPRI
        dsb
        isb
/* *INDENT-ON* */
    }

    return ulReturn;
}
```

该函数执行了三步：

- 先读出 `BASEPRI` 的值，保存在 `ulReturn` 中
- 将 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 写入到寄存器 `BASEPRI` 中
- 返回 `ulReturn`，退出临界区代码保护的时候要使用到此值

---

**中断级临界代码保护使用方法**

```c
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
    {
        status_value=taskENTER_CRITICAL_FROM_ISR();
        total_num+=1;
        printf("float_num 的值为: %d\r\n",total_num);
        taskEXIT_CRITICAL_FROM_ISR(status_value);·
    }
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update); //清除中断标志位
}
```

# 5 容易陷入的误区

要区分“RTOS任务切换”和“CPU中断”的使用场景。

RTOS任务切换：完全靠软件实现，需要靠CPU定时检查任务的完成情况和任务队列，通过软件的方式进行任务管理。

FreeRTOS与中断有关的就是两个`SYSTICK`和`Pendv`。

前者定时触发中断，通知系统查看任务时间片和是否有高优先级任务以决定是否要切换任务，后者进行任务的切换操作。

# 6 FreeRTOS 中断测试实验

## 6.1 实验目的

上面我们讲了在FreeRTOS中优先级低于`configMAX_SYSCALL_INTERRUPT_PRIORITY`的中断会被屏蔽掉，高于的就不会，那么本节我们就写个简单的例程测试一下。使用两个定时器，一个优先级为 4，一个优先级为 5，两个定时器每隔 1s 通过串口输出一串字符串。然后在某个任务中关闭中断一段时间，查看两个定时器的输出情况。

## 6.2 实验设计

本实验设计了两个任务 `start_task()`和`interrupt_task()`, 这两个任务的任务功能如下：

- `start_task()`：创建另外一个任务。
- `interrupt_task()`：中断测试任务，任务中会调用FreeRTOS的关中断函数`portDISABLE_INTERRUPTS()`来将中断关闭一段时间。

## 6.3 实验过程

```c
#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  256                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define INTERRUPT_TASK_PRIO 2             // 任务优先级
#define INTERRUPT_STK_SIZE  256           // 任务堆栈大小
TaskHandle_t INTERRUPTTask_Handler;       // 任务句柄
void         interrupt_task(void *p_arg); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    delay_init();
    usart_init(115200);
    timer_init();

    xTaskCreate(
        (TaskFunction_t)start_task,        // 任务函数
        (const char *)"start_task",        // 任务名称
        (uint16_t)START_STK_SIZE,          // 任务堆栈大小
        (void *)NULL,                      // 传递给任务函数的参数
        (UBaseType_t)START_TASK_PRIO,      // 任务优先级
        (TaskHandle_t *)&StartTask_Handler // 任务句柄
    );
    vTaskStartScheduler(); // 启动任务调度
}

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); // 进入临界区

    // 注册中断测试任务
    xTaskCreate(
        (TaskFunction_t)interrupt_task,
        (const char *)"interrupt_task",
        (uint16_t)INTERRUPT_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)INTERRUPT_TASK_PRIO,
        (TaskHandle_t *)&INTERRUPTTask_Handler
    );

    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void interrupt_task(void *p_arg)
{
    while (1)
    {
        // 中断关闭五秒
        portDISABLE_INTERRUPTS();
        delay_s(5);
        // 中断开启五秒
        portENABLE_INTERRUPTS();
        delay_s(5);
    }
}
```

```c
void _timer_it_cmd(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}
```

