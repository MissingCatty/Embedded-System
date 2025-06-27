# 1.调度器开启过程分析

## 1.1 vTaskStartScheduler函数

前面的所有例程中我们都是在 main 函数中先创建一个开始任务 `start_task`，后面紧接着调用函数 `vTaskStartScheduler()`。这个函数的功能就是开启任务调度器的，这个函数在文件 `tasks.c` 中有定义，缩减后的函数代码如下：

```c
void vTaskStartScheduler(void)
{
    BaseType_t xReturn;
    xReturn = xTaskCreate(prvIdleTask, "IDLE", configMINIMAL_STACK_SIZE, (void *)NULL, (tskIDLE_PRIORITY | portPRIVILEGE_BIT), &xIdleTaskHandle);
#if (configUSE_TIMERS == 1) // 使用软件定时器使能
    {
        if (xReturn == pdPASS)
        {
            xReturn = xTimerCreateTimerTask();
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif                     /* configUSE_TIMERS */
    if (xReturn == pdPASS) // 空闲任务和定时器任务创建成功。
    {
        /*
        	关闭中断，在“调度器未正式启动”前，如果中断提前发生，而 ISR 中用了 RTOS API，
        	此时调度器内部还没准备好，会导致调度器数据结构破坏崩溃！
        	一旦调度器成功切入第一个任务，RTOS 会自动恢复中断使能
        */
        portDISABLE_INTERRUPTS();
        
		#if (configUSE_NEWLIB_REENTRANT == 1) // 使能 NEWLIB
        {
            _impure_ptr = &(pxCurrentTCB->xNewLib_reent);
        }
		#endif
        
        xNextTaskUnblockTime = portMAX_DELAY;
        xSchedulerRunning    = pdTRUE;	// 设置为 pdTRUE，表示调度器开始运行
        xTickCount           = (TickType_t)0U;
        
        portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();
        
        // 调用函数 xPortStartScheduler()来初始化跟调度器启动有关的硬件
        // 比如滴答定时器、FPU 单元和 PendSV 中断等等。
        if (xPortStartScheduler() != pdFALSE)
        {
            // 如果调度器启动成功的话就不会运行到这里，函数不会有返回值的
        } else
        {
            // 不会运行到这里，除非调用函数 xTaskEndScheduler()。
        }
    } else
    {
        // 程序运行到这里只能说明一点，那就是系统内核没有启动成功，导致的原因是在创建
        // 空闲任务或者定时器任务的时候没有足够的内存。
        configASSERT(xReturn != errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY);
    }
    // 防止编译器报错，比如宏 INCLUDE_xTaskGetIdleTaskHandle 定义为 0 的话编译器就会提
    // 示 xIdleTaskHandle 未使用。
    (void)xIdleTaskHandle;
}
```

> 定时器任务（Timer Service Task）是在 FreeRTOS 中专门用来处理 **软件定时器** 的一个后台任务。
>
> 软件定时器的最常见用途就是：**在指定时间之后自动执行一个回调函数（callback）**，不需要你自己管理定时、计数、判断等逻辑。
>
> 如果你有**多个周期性任务**，使用 **多个软件定时器**，你可以为每个功能创建一个独立的周期性软件定时器：
>
> ```c
> TimerHandle_t xHeartbeatTimer;
> TimerHandle_t xSensorTimer;
> TimerHandle_t xUploadTimer;
> 
> void vHeartbeatCallback(TimerHandle_t xTimer) {
>     // 切换 LED
> }
> 
> void vSensorReadCallback(TimerHandle_t xTimer) {
>     // 读取传感器数据
> }
> 
> void vUploadCallback(TimerHandle_t xTimer) {
>     // 上传数据到服务器
> }
> 
> void app_init() {
>     xHeartbeatTimer = xTimerCreate("Heartbeat", pdMS_TO_TICKS(500), pdTRUE, NULL, vHeartbeatCallback);
>     xSensorTimer = xTimerCreate("Sensor", pdMS_TO_TICKS(1000), pdTRUE, NULL, vSensorReadCallback);
>     xUploadTimer = xTimerCreate("Upload", pdMS_TO_TICKS(10000), pdTRUE, NULL, vUploadCallback);
> 
>     xTimerStart(xHeartbeatTimer, 0);
>     xTimerStart(xSensorTimer, 0);
>     xTimerStart(xUploadTimer, 0);
> }
> ```
>
> ---
>
> ##### `xTimerCreateTimerTask`和`xTimerCreate`
>
> - **`xTimerCreate`**：**用户**创建软件定时器用的 API。
> - **`xTimerCreateTimerTask`**：FreeRTOS 内部函数（**不推荐应用层调用**），创建定时器任务。
>
> ---
>
> ##### 为什么要在`vTaskStartScheduler`调用`xTimerCreateTimerTask`？
>
> 因为创建的所有软件定时器的回调函数都**需要一个后台任务来执行**，这个任务就是 **Timer Service Task**。
>
> 等于说`xTimerCreate`只是创建一个定时器，并绑定一个执行函数（callback），`Timer Service Task`将执行函数包裹在一个任务里运行，符合RTOS的基本思想。

## 1.2 xPortStartScheduler函数

FreeRTOS 系统时钟是由滴答定时器来提供的，而且任务切换也会用到 PendSV 中断，这些硬件的初始化由函数 `xPortStartScheduler()`来完成，缩减后的函数代码如下：

```c
BaseType_t xPortStartScheduler(void)
{
    /******************************************************************/
    /****************此处省略一大堆的条件编译代码**********************/
    /*****************************************************************/
    // 设置 PendSV 的中断优先级，为最低优先级。
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    // 设置滴答定时器的中断优先级，为最低优先级。
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
    // 调用函数 vPortSetupTimerInterrupt()来设置滴答定时器的定时周期，并且使能滴答定时器的中断
    vPortSetupTimerInterrupt();
    // 初始化临界区嵌套计数器。
    uxCriticalNesting = 0;
    // 调用函数 prvEnableVFP()使能 FPU。
    prvEnableVFP();
    // 设置寄存器 FPCCR 的 bit31 和 bit30 都为 1，这样 S0~S15 和 FPSCR 寄存器在异常入口和退出时的壮态自动保存和恢复。
    *(portFPCCR) |= portASPEN_AND_LSPEN_BITS;
    prvStartFirstTask();

    // 代码正常执行的话是不会到这里的！
    return 0;
}
```

## 1.3 prvEnableVFP函数

在函数 `xPortStartScheduler()`中会通过调用 `prvEnableVFP()`来使能 FPU，这个函数是汇编形式的，在文件 `port.c` 中有定义，函数如下：

```asm
__asm void prvEnableVFP( void )
{
    PRESERVE8
    
    ldr.w r0, =0xE000ED88		;将立即数 0x800 加载到寄存器R0
    ldr r1, [r0]		        ;从 R0 读取CPACR寄存器的地址赋给 R1
    orr r1, r1, #( 0xf << 20 )	;R1=R1|(0xf<<20)
    str r1, [r0]				;R1 中的值写入 R0 保存的地址中
    bx r14
    nop
}
```

- **寄存器 CPACR 可以使能或禁止 FPU**，此寄存器的地址为 0XE000ED88（具体参考《权威指南》“第 13 章 浮点运算” 13.2.3 章节），此行代码将地址 0XE000ED88 保存在寄存器 R0 中。
- 读取 R0 中保存的存储地址处的数据，也就是 **CPACR 寄存器的值**，并将结果**保存在R1 寄存器中**。
- R1 中的值与(0xf<<20)进行按位或运算，也就是 R1=R1|0X00F00000。此时 **R1 所保存的值的 bit20~bit23 就都为 1 了**，将这个值写入寄存器 CPACR 中就可开启 FPU。

## 1.4 prvStartFirstTask函数

函数 `prvStartFirstTask()`用于启动第一个任务，这是一个汇编函数，函数源码如下：

```asm
__asm void prvStartFirstTask( void )
{
	PRESERVE8
    ldr r0, =0xE000ED08 ;R0=0XE000ED08 (1)
    ldr r0, [r0] 		;取 R0 所保存的地址处的值赋给 R0 (2)
    ldr r0, [r0]		;获取 MSP 初始值 (3)
    msr msp, r0			;复位 MSP (4)
    cpsie I				;使能中断(清除 PRIMASK) (5)
    cpsie f				;使能中断(清除 FAULTMASK) (6)
    dsb					;数据同步屏障 (7)
    isb					;指令同步屏障 (8)
    svc 0				;触发 SVC 中断(异常) (9)
    nop
    nop
}
```

- `(1)`：将 0XE000ED08 保存在寄存器 R0 中 ，一般来说向量表应该是从起始地址(0X00000000)开始存储的，不过，**有些应用可能需要在运行时修改或重定义向量表**， Cortex-M 处理器为此提供了一个叫做**向量表重定位**的特性。向量表重定位特性提供了一个名为**向量表偏移寄存器(VTOR)**的可编程寄存器。VTOR 寄存器的地址就是 0XE000ED08，通过这个寄存器可以重新定义向量表，比如在 STM32F407 的 ST 官方库中会通过函数` SystemInit()`来设置 VTOR 寄存器，代码如下：

  ```c
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; //VTOR=0x08000000+0X00
  ```

  通过上面一行代码就将向量表开始地址重新定义到了 0X08000000，**向量表的起始地址**存储的就是 **MSP （堆栈指针）初始值**。  

- `(2)`：读取 R0 中存储的地址处的数据并将其保存在 R0 寄存器，也就是读取寄存器 VTOR 中的值，并将其保存在 R0 寄存器中。这一行代码执行完就以后 R0 的值应该为 0X08000000。

- `(3)`：读取地址 0X08000000处（中断向量表起始地址）存储的数据（MSP值，中断向量表起始地址中保存的是MSP值），并将其保存在 R0 寄存器中 

(1)、 (2)、(3)这三步起始就是为了获取 MSP 的初始值而已！  

- `(5,6)`：使能中断 （关于这两个指令的详细内容请参考《权威指南》的“第 4 章 架构”的第 4.2.3 小节）
- `(7,8)`：数据同步和指令同步屏障（这两个指令的详细内容请参考《权威指南》的“第 5章 指令集”的 5.6.13 小节）
- `(9)`：调用 SVC 指令触发 SVC 中断（请求管理调用），SVC 和 PendSV 异常对于OS 的设计来说非常重要。**SVC 异常由 SVC 指令触发**（关于 SVC 的详细内容请参考《权威指南》的“第 10 章 OS 支持特性”的 10.3 小节）在 FreeRTOS 中仅仅使用 SVC 异常来启动第一个任务，**后面的程序中就再也用不到 SVC 了**。（在任务调度开启函数`vTaskStartScheduler`中频闭了中断，在SVC中断中开启了中断）

## 1.5 SVC中断服务函数

在函数 `prvStartFirstTask()`中通过调用 SVC 指令触发了 SVC 中断，**第一个任务的启动就是在 SVC 中断服务函数中完成的**，SVC 中断服务函数应该为 `SVC_Handler()`，但是`FreeRTOSConfig.h` 中通过#define 的方式重新定义为了 `xPortPendSVHandler()`，如下：

```c
#define xPortPendSVHandler PendSV_Handler
```

函数 `vPortSVCHandler()`在文件 port.c 中定义，这个函数也是用汇编写的，函数源码如下：  

```asm
__asm void vPortSVCHandler( void )
{
	PRESERVE8
    ldr r3, =pxCurrentTCB 		;R3=pxCurrentTCB 的地址 (1)
    ldr r1, [r3] 				;取 R3 所保存的地址处的值赋给 R1 (2)
    ldr r0, [r1] 				;取 R1 所保存的地址处的值赋给 R0 (3)
    ldmia r0!, {r4-r11, r14} 	;出栈，R4~R11 和 R14 (4)
    msr psp, r0 				;进程栈指针 PSP 设置为任务的堆栈 (5)
    isb 						;指令同步屏障
    mov r0, #0 					;R0=0 (6)
    msr basepri, r0 			;寄存器basepri=0，开启中断 (7)
    bx r14 						;(8)
}
```

- `(1)`：R3中保存的是`pxCurrentTCB`（指针的地址，注意不是指针的值）
- `(2)`：将`pxCurrentTCB`指针的值（当前TCB的地址）保存到R1
- `(3)`：将当前TCB结构体的首地址中的值保存到R0（就是当前任务控制块TCB第一个字段，任务堆栈的栈顶指针`pxTopOfStack`所指向的位置）

所以前三步就是获取`pxCurrentTCB`所指向的任务控制块TCB中保存的**任务堆栈的栈顶指针**（**任务现场都保存在任务的任务堆栈中**，所以需要获取栈顶指针来恢复这些寄存器值）。

- `(4)`：多寄存器加载指令，其作用是从内存中连续加载多个寄存器的值。

  ```asm
  LDR 	R0, 	=0X800
  LDMIA 	R0!, 	{R2~R4}
  ```

  - 第一行代码将R0的内容设置为0x800
  - 第二行表示：依次从R0的内容（0x800）对应的内存地址读取数据到R2,R3,R4。详细来说，读取[0x800]到R2，接下来地址自增（32位ARM一个数据占4个字节，按字节编址）读取[0x804]到R3，最后读取[0x808]到R4。**在读取的过程中R0会自增**，每放入一个数据，就自增0x004，所以读取完三个数据后R0是**0x80C**。

  所以，原函数中

  ```asm
  ldr r0, [r1]
  ldmia r0!, {r4-r11, r14}
  ```

  R0中存放的是task的堆栈栈顶指针，依次出栈9个32位数据，放入R4-R11，以及R14。所以恢复的是这9个寄存器的值，那剩下的寄存器（**R0~R3**，**R12**，**PC**，**xPSR**）为什么没有恢复？这是因为**这些寄存器会在退出中断的时候 MCU 自动出栈(恢复)的**，而 **R4~R11 需要由用户手动出栈**，如果使用FPU 的话还要考虑到 FPU 寄存器。

## 1.6 空闲任务

在`vTaskStartScheduler()`中，会创建一个名为“**IDLE**”的任务，这个任务叫做空闲任务。系统中其他的任务由于各种原因不能运行的时候空闲任务就在运行。

空闲任务不仅仅是为了满足任务调度器启动以后至少有一个任务运行而创建的，空闲任务中还会去做一些其他的事情，如下：

1. 判断系统是否有任务删除，如果有的话就**在空闲任务中释放被删除任务的任务堆栈和任务控制块的内存**。
2. 运行**用户设置的空闲任务钩子函数**。
3. **判断是否开启低功耗 tickless 模式**，如果开启的话还需要做相应的处理

**空闲任务的任务优先级是最低的**，为 0，任务函数为 `prvIdleTask()`，有关空闲任务的详细内容我们后面会有专门的章节讲解，这里大家只要知道有这个任务就行了。

# 2.任务创建过程分析

## 2.1 xTaskCreate函数

```c
BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char *const pcName, const uint16_t usStackDepth, void *const pvParameters, UBaseType_t uxPriority, TaskHandle_t *const pxCreatedTask)
{
    /* 1.创建TCB块指针 */
    TCB_t     *pxNewTCB;
    BaseType_t xReturn; // 返回值
    /********************************************************************/
    /***************使用条件编译的向上增长堆栈相关代码省略***************/
    /********************************************************************/
    /* 2. 创建任务栈指针，并动态申请内存 */
    StackType_t *pxStack; 
    pxStack = (StackType_t *)pvPortMalloc((((size_t)usStackDepth) * sizeof(StackType_t)));
    // 如果栈空间申请成功
    if (pxStack != NULL)
    {
        /* 3. 为TCB申请内存空间 */
        pxNewTCB = (TCB_t *)pvPortMalloc(sizeof(TCB_t));
        /* 4. 如果申请成功，就把创建的栈交给TCB管理 */
        if (pxNewTCB != NULL)
        {
            pxNewTCB->pxStack = pxStack;
        } else
        {
            // 否则释放栈空间
            vPortFree(pxStack);
        }
    } else
    {
        // 释放TCB指针
        pxNewTCB = NULL;
    }
    if (pxNewTCB != NULL)
    {
#if (tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0)
        {
            // 标记该TCB和其中的栈都是以何种方式分配的（静态/动态）
            pxNewTCB->ucStaticallyAllocated = tskDYNAMICALLY_ALLOCATED_STACK_AND_TCB;
            /*
                #define tskDYNAMICALLY_ALLOCATED_STACK_AND_TCB    ( ( uint8_t ) 0 )
                #define tskSTATICALLY_ALLOCATED_STACK_ONLY        ( ( uint8_t ) 1 )
                #define tskSTATICALLY_ALLOCATED_STACK_AND_TCB     ( ( uint8_t ) 2 )
            */
        }
#endif                     
        /* 5. 对TCB中各个字段进行初始化 */
        prvInitialiseNewTask(pxTaskCode, pcName, (uint32_t)usStackDepth, pvParameters, uxPriority, pxCreatedTask, pxNewTCB, NULL);
        /* 6. 将该TCB添加到就绪列表中 */
        prvAddNewTaskToReadyList(pxNewTCB);

        xReturn = pdPASS;
    } else
    {
        xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }
    return xReturn;
}
```

## 2.2 prvInitialiseNewTask函数

用于完成对TCB各字段的初始化 ，参数如下：

- `pxTaskCode`：任务函数的入口函数
- `pcName`：任务的名称
- `ulStackDepth`：任务堆栈的大小
- `pvParameters`：传递给任务函数的参数
- `uxPriority`：任务的优先级
- `pxCreatedTask`：任务句柄，一个指向`TaskHandle_t`类型的指针
- `pxNewTCB`：`TCB`指针
- `xRegions`：一个指向 `MemoryRegion_t` 类型数组的指针，用于定义任务的内存保护区域。

```c
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode, const char *const pcName, const uint32_t ulStackDepth, void *const pvParameters, UBaseType_t uxPriority, TaskHandle_t *const pxCreatedTask, TCB_t *pxNewTCB, const MemoryRegion_t *const xRegions)
{
    StackType_t *pxTopOfStack; // 创建栈顶指针
    UBaseType_t  x;
    /*
    	1. configCHECK_FOR_STACK_OVERFLOW：启用栈溢出检测
    	2. configUSE_TRACE_FACILITY：启用追踪功能，提供对 RTOS 内部运行状态的可视化和分析支持。
    		- 跟踪任务状态变化（就绪、运行、挂起等）
    		- 与 FreeRTOS Tracealyzer 等工具配合使用
    		- 导出调度信息、运行时间等指标
    	3. INCLUDE_uxTaskGetStackHighWaterMark：
    		是否包含 uxTaskGetStackHighWaterMark() 函数，它用于获取任务栈的最小剩余量（即“水位线”）。
    */
#if ((configCHECK_FOR_STACK_OVERFLOW > 1) || (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxTaskGetStackHighWaterMark == 1))
    {
        /*
			1. 如果使能了堆栈溢出检测功能或者追踪功能的话就使用一个定值 tskSTACK_FILL_BYTE 来填充任务堆栈，这个值为 0xa5U。
        */
        (void)memset(pxNewTCB->pxStack, (int)tskSTACK_FILL_BYTE, (size_t)ulStackDepth * sizeof(StackType_t));
    }
#endif
    /* 2. 获取栈顶指针（高地址） */
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
    /*
    	下面这行代码的目的是对齐地址
    	i. portPOINTER_SIZE_TYPE：uint32_t
    	ii. portBYTE_ALIGNMENT_MASK
            #if portBYTE_ALIGNMENT == 32
            #define portBYTE_ALIGNMENT_MASK    ( 0x001f )
            #elif portBYTE_ALIGNMENT == 16
                #define portBYTE_ALIGNMENT_MASK    ( 0x000f )
            #elif portBYTE_ALIGNMENT == 8（默认）
                #define portBYTE_ALIGNMENT_MASK    ( 0x0007 )
            #elif portBYTE_ALIGNMENT == 4
                #define portBYTE_ALIGNMENT_MASK    ( 0x0003 )
            #elif portBYTE_ALIGNMENT == 2
                #define portBYTE_ALIGNMENT_MASK    ( 0x0001 )
            #elif portBYTE_ALIGNMENT == 1
                #define portBYTE_ALIGNMENT_MASK    ( 0x0000 )
            #else
                #error "Invalid portBYTE_ALIGNMENT definition"
            #endif
		iii. 对齐操作见注释
    */
    /* 3. 对齐栈顶指针地址 */
    pxTopOfStack = (StackType_t *)(((portPOINTER_SIZE_TYPE)pxTopOfStack) & (~((portPOINTER_SIZE_TYPE)portBYTE_ALIGNMENT_MASK)));
    
    /* 4.任务名赋值 */
    for (x = (UBaseType_t)0; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x];
        // 读到任务名字符串中的\0，则退出
        if (pcName[x] == 0x00)
        {
            break;
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    // 把任务名数组中的最后一个变为\0，以免溢出
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';
    
    /*
    	5. 判断任务优先级是否合法，如果设置的任务优先级大于 configMAX_PRIORITIES，则
		将优先级修改为 configMAX_PRIORITIES-1。
		
		为什么要限制优先级数量？见注释
    */
    if (uxPriority >= (UBaseType_t)configMAX_PRIORITIES)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
    
    /* 6. 设置优先级 */
    pxNewTCB->uxPriority = uxPriority;
    
    /* 7. 如果开启了互斥信号量功能 */
#if (configUSE_MUTEXES == 1)
    {
        pxNewTCB->uxBasePriority = uxPriority; // 见注释3.
        pxNewTCB->uxMutexesHeld  = 0; // 表示该任务当前持有的互斥信号量数量
    }
#endif
    
    /* 8. 初始化xStateListItem，该列表项代表了当前任务在哪个状态链表中（就绪列表、阻塞列表、挂起列表） */
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    /* 9. 见注释4 */
    vListInitialiseItem(&(pxNewTCB->xEventListItem));

    /* 10.设置xStateListItem的“所有者”指针，用于在任务调度和管理过程中，通过链表项快速找到它属于哪个任务（TCB） */
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);
    /* 11.为 xEventListItem 设置一个用于排序的值，它与任务优先级有关。当多个任务等待一个信号量或队列时，FreeRTOS 会按照它们的 xItemValue（即这个设置的值）来进行链表排序 */
    listSET_LIST_ITEM_VALUE(&(pxNewTCB->xEventListItem), (TickType_t)configMAX_PRIORITIES - (TickType_t)uxPriority);
    /* 12.设置xEventListItem的所有者 */
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xEventListItem), pxNewTCB);
    
    /* 13.对TCB可选字段初始化 */
#if (portCRITICAL_NESTING_IN_TCB == 1) // 使能临界区嵌套
    {
        /* 
        	- 如果启用了该配置，每个任务都会有一个独立的临界区嵌套计数器。
        	- 这样可以支持任务之间切换时正确保存和恢复临界区状态，用于中断屏蔽控制。
        */
        pxNewTCB->uxCriticalNesting = (UBaseType_t)0U;
    }
#endif
#if (configUSE_APPLICATION_TASK_TAG == 1) // 使能任务标签功能
    {
        /* 
        	- 启用后，开发者可以给每个任务绑定一个“任务标签”（void* 类型）
        	- 可以用它做一些自定义功能，比如调试、任务监视、扩展 trace 数据等。
        */
        pxNewTCB->pxTaskTag = NULL;
    }
#endif
#if (configGENERATE_RUN_TIME_STATS == 1) // 使能时间统计功能
    {
        /*
        	- 启用后，会记录该任务累计运行的 CPU 时间（单位由你的定时器频率决定）。
        	- 配合 vTaskGetRunTimeStats() 可以查看各任务的 CPU 使用百分比，用于性能分析和负载监测。
        */
        pxNewTCB->ulRunTimeCounter = 0UL;
    }
#endif
#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0)
    {
        for (x = 0; x < (UBaseType_t)configNUM_THREAD_LOCAL_STORAGE_POINTERS;
             x++)
        {
            pxNewTCB->pvThreadLocalStoragePointers[x] = NULL; // 初始化线程本地存储指针
        }
    }
#endif
#if (configUSE_TASK_NOTIFICATIONS == 1) // 使能任务通知功能
    {
        pxNewTCB->ulNotifiedValue = 0;
        pxNewTCB->ucNotifyState   = taskNOT_WAITING_NOTIFICATION;
    }
#endif
#if (configUSE_NEWLIB_REENTRANT == 1) // 使能 NEWLIB
    {
        _REENT_INIT_PTR((&(pxNewTCB->xNewLib_reent)));
    }
#endif
#if (INCLUDE_xTaskAbortDelay == 1) // 使能函数 xTaskAbortDelay()
    {
        pxNewTCB->ucDelayAborted = pdFALSE;
    }
#endif
    
    /* 14.初始化任务栈，并设置任务栈顶指针 pxTopOfStack，为任务切换做好准备。 */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);
    
    /* 15.生成任务句柄，返回给参数 pxCreatedTask，从这里可以看出任务句柄其实就是指向任务控制块的指针 */
    if ((void *)pxCreatedTask != NULL)
    {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}
```

> ##### 1.栈顶指针对齐
>
> 何为对齐？**数据在内存中的<u>地址</u>必须满足某种特定的规则**。
>
> 比如：
>
> - 4 字节对齐：地址必须是 **4 的倍数**，即 `0x00`, `0x04`, `0x08`, `0x0C`…
> - 8 字节对齐：地址必须是 **8 的倍数**，即 `0x00`, `0x08`, `0x10`, `0x18`…
>
> 为什么要对齐？
>
> - 大多数 CPU 访问对齐地址会更快。例如 32 位系统访问 4 字节对齐的地址时，可以一条指令完成；否则可能需要两次访问或出错。
> - 有些架构（比如 STM32 的 Cortex-M3）虽然能处理非对齐访问，但某些数据类型（如 `uint32_t`）如果访问未对齐地址，会报错或进入 `HardFault`。
>
> 例如：
>
> - 假设 `pxTopOfStack = 0x20000007`
> - 如果对齐要求是 8 字节（`portBYTE_ALIGNMENT = 8`），则`portBYTE_ALIGNMENT_MASK = 0x0007`
> - `0x20000007 & ~0x0007 = 0x20000000` —— **这是最近的、满足8字节对齐的地址**
>
> 所以，**对齐操作可能会让实际可用的栈空间比你申请的少一点点**
>
> ##### 2.为什么要限制优先级数量？
>
> 因为每个优先级都需要一个优先级队列来维护，如果设置最大优先级为20，则要20个队列来管理任务，在资源首先的系统中不合适。
>
> ###### 3.开启互斥信号量为什么要保存初始优先级？
>
> 这涉及到优先级反转，何为优先级反转？
>
> 举个简单的例子，假设有三个任务：
>
> - **任务A**：低优先级，正在运行并持有一个互斥锁（mutex）。
> - **任务B**：中等优先级，周期性运行。
> - **任务C**：高优先级，想要获取任务A持有的互斥锁。
>
> **正常期望：**
>
> 任务C应该抢占其他任务，迅速运行。
>
> **实际发生：**
>
> 1. 任务A持有mutex，还没释放；
> 2. 任务C尝试获取mutex，但被阻塞（等待任务A释放）；
> 3. 中优先级的任务B继续运行，占用CPU；
> 4. 任务A迟迟无法得到CPU执行机会，自然无法释放mutex；
> 5. 所以高优先级任务C一直卡住，被“反转”成“优先级最低的”。
>
> 这种就是优先级反转：低优先级任务间接“压制”了高优先级任务的执行。
>
> 为了解决“优先级反转”，FreeRTOS采用“**优先级继承**”的方法：当某个低优先级的任务A持有互斥锁时，当高优先级的任务C等待其互斥锁时，会临时提高A的优先级至C，确保：
>
> - 如果有优先级比C低却比A原始优先级高的任务B在执行，A不会被B阻塞得不到执行，等待A迅速执行完毕并释放互斥锁时，C就能够得到执行。
> - 如果有优先级比C高的任务D在执行，那么A和C都会被D阻塞，符合正常的逻辑。
>
> **所以，有如果使用了互斥信号量功能，就会涉及优先级继承，所以要把一个任务的原始优先级保存下来**。
>
> ###### 4. 事件等待列表项`xEventListItem`
>
> 当任务等待某个事件（如信号量、队列、事件组等）时，操作系统用 `xEventListItem` 把该任务挂到“事件等待列表”中。
>
> 例如，任务 A 想要从一个队列中接收数据，但队列是空的：
>
> 此时 FreeRTOS 会做两件事：
>
> 1. 把任务 A 的 `xEventListItem` 插入到 `xQueue` 的等待链表中（例如 `xTasksWaitingToReceive`）；
> 2. 挂起任务 A（任务状态变为阻塞）；
>
> 一旦其他任务往队列中发送数据，FreeRTOS 会：
>
> - 从 `xTasksWaitingToReceive` 中找出一个任务；
> - 根据 `xEventListItem` 找到它属于哪个任务；
> - 把它唤醒，放入就绪队列中去运行。

## 2.3 pxPortInitialiseStack函数

堆栈是用来在进行上下文切换的时候保存现场的，一般在新创建好一个堆栈以后会对其先进行初始化处理。

在初始化堆栈时，这些恢复的数据其实并没有实际意义，只是起到一个模板的作用，显示每次挂起任务时应该保存哪些寄存器的值。

寄存器包括：

- `(1)`：寄存器 `xPSR`值为 `portINITIAL_XPSR`，其值为 `0x01000000`。`xPSR `是 `Cortex-M4` 的一个内核寄存器，叫做程序状态寄存器， 0x01000000 表示这个寄存器的 bit24 为 1， 表示处于 `Thumb`状态，即使用的 Thumb 指令（**Thumb 指令集**是 ARM 处理器架构提供的一种**压缩指令格式**，主要目标是**减小代码体积**，提高在嵌入式系统中的效率）。
- `(2)`：寄存器 PC 设置为程序计数器 PC 的初始值，`pxCode` 是任务函数的地址，实际上，它的目的是告诉 CPU：**任务从哪个地址开始执行。**
- `(3)`：寄存器 LR 设置为**任务函数返回地址**，如果任务函数运行完了（没有 return 值），就会跳到 `prvTaskExitError`。
- `(4)`：跳过 4 个寄存器， R12， R3， R2， R1，这四个寄存器不初始化
- `(5)`：设置 **R0 的初始值**，即任务函数的参数，一般情况下，函数调用会将 R0~R3 作为输入参数， R0 也可用作返回结果，如果返回值为 64 位，则 R1 也会用于返回结果  
- `(6)`：保存 `EXC_RETURN`值， 用于退出 SVC 或 PendSV 中断的时候处理器应该处于什么壮态。  
- `(7)`：跳过 8 个寄存器， R11、 R10、 R8、 R7、 R6、 R5、 R4

```c
StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters)
{
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_XPSR; // (1)
    pxTopOfStack--;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK; // (2)
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)prvTaskExitError; // (3)
    pxTopOfStack -= 5; // (4)
    *pxTopOfStack = (StackType_t)pvParameters; // (5)
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_EXEC_RETURN; // (6)
    pxTopOfStack -= 8;
    return pxTopOfStack;
}
```

![image-20250507115530414](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/11-57-22-4c3dca3822e0d2688b230e659f576c19-image-20250507115530414-4793b6.png)

## 2.4 添加任务到就绪队列

### 2.4.1 任务管理有关列表

任务创建完成以后就会被添加到就绪列表中， FreeRTOS 使用不同的列表表示任务的不同状态，在文件 `tasks.c` 中就定义了多个列表来完成不同的功能，这些列表如下：

```c
/* 就绪队列 */
PRIVILEGED_DATA static List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
/* 延时队列 */
PRIVILEGED_DATA static List_t xDelayedTaskList1;
PRIVILEGED_DATA static List_t xDelayedTaskList2;
PRIVILEGED_DATA static List_t * volatile pxDelayedTaskList;
PRIVILEGED_DATA static List_t * volatile pxOverflowDelayedTaskList;
/* 待就绪任务队列 */
PRIVILEGED_DATA static List_t xPendingReadyList;

#if ( INCLUDE_vTaskDelete == 1 )
/* 待就绪任务列队列 */
    PRIVILEGED_DATA static List_t xTasksWaitingTermination;
/* 待就绪任务列队列 */
    PRIVILEGED_DATA static volatile UBaseType_t uxDeletedTasksWaitingCleanUp = ( UBaseType_t ) 0U;
#endif

#if ( INCLUDE_vTaskSuspend == 1 )
    PRIVILEGED_DATA static List_t xSuspendedTaskList;
#endif
```

- `pxReadyTasksLists`：就绪队列列表，长度为`configMAX_PRIORITIES`（最大优先级），每个元素是一个`List_t`（链表），所以说每个优先级都是一个优先级队列

- `xDelayedTaskList1`，`xDelayedTaskList2`：延迟任务链表，当任务调用 `vTaskDelay()` 等函数时，会被放入延迟队列，等待时间到达后再恢复到就绪状态

  > ###### 为什么要设置两个链表？
  >
  > 放入延时队列的函数需要等待被唤醒，当某个任务被放入延时队列时，就会被计算好唤醒的时间。在FreeRTOS中，所有的时间都是被一个tick计数器来表示的。任务待唤醒时间为`task_wake_time`，当前tick为`current_tick_count`。
  >
  > 系统每计一个tick，都会检查每个任务：
  >
  > ```c
  > if (task_wake_time <= current_tick_count)
  > {
  >     // 时间到了，把任务从延时队列移除
  >     // 放入就绪队列中等待执行
  > }
  > ```
  >
  > 然而，tick是一个32位无符号整数，假设当前的`current_tick_count`为`0xFFFFFFFF`，现在来了一个延时8个tick的任务到延时队列，则它的`task_wake_time`为`0x00000008`，产生溢出，此时如果直接判断`if (task_wake_time <= current_tick_count)`，就为True，就从延时队列移除，显然不对，所以就把`task_wake_time`溢出的任务放到`pxOverflowDelayedTaskList`里，没溢出的放到`pxDelayedTaskList`里。
  >
  > 当`current_tick_count`产生溢出时，`pxDelayedTaskList`和`pxOverflowDelayedTaskList`会交换，因为当前tick溢出变为`0x00000000`时，原本溢出的变正常了。**FreeRTOS移除延时任务的链表仅为`pxDelayedTaskList`，`pxOverflowDelayedTaskList`需要等待tick溢出。**

- `pxDelayedTaskList`，`pxOverflowDelayedTaskList`：链表指针，指向 **延迟任务列表**和**溢出任务列表**

  > 初始时
  >
  > ```c
  > pxDelayedTaskList = &xDelayedTaskList1;
  > pxOverflowDelayedTaskList = &xDelayedTaskList2;
  > ```
  >
  > `current_tick_count`溢出时，交换，这样设置的好处是不需要判断指针指向的是哪个链表。
  >
  > ```c
  > List_t *temp = pxDelayedTaskList;
  > pxDelayedTaskList = pxOverflowDelayedTaskList;
  > pxOverflowDelayedTaskList = temp;
  > ```

### 2.4.2 添加新创建的任务到就绪列表

主要需要做两件事：

- 修改系统中的任务总数
- 判断系统中是否有正在执行的任务：
  - 如果没有，则新任务直接准备执行
  - 如果有，检查调度器是否开启，如果没开启就手动切换任务
- 为新的任务控制块分配编号
- 添加新的任务至任务列表
- 如果任务调度器已开启，

```c
static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB)
{
    /*================== 进入临界区 ==================*/
    taskENTER_CRITICAL();
    {
        /* 1. 系统中任务总数加一 */
        uxCurrentNumberOfTasks++;
        
        /* 2. 当前没有任务在运行 */
        if (pxCurrentTCB == NULL)
        {
            /* 3. 把新创建的TCB给当前正在执行的TCB */
            pxCurrentTCB = pxNewTCB;
            
            /* 4. 当前任务总数为1（本来系统中没有任务） */
            if (uxCurrentNumberOfTasks == (UBaseType_t)1)
            {
                /* 5. 初始化任务列表 */
                prvInitialiseTaskLists();
            } else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        } else
        {
            /* 6. 如果调度器没有运行，需要手动处理任务切换 */
            /* 调度器正在运行时，任务可以动态切换；未运行，任务仅仅被添加到列表但不立即调度 */
            if (xSchedulerRunning == pdFALSE)
            {
            	/* 7. 如果新加入的任务比当前正在执行的任务优先级高 */
                if (pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
                {
            		/* 8. 新任务立即切换为当前任务 */
                    pxCurrentTCB = pxNewTCB;
                } else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        
        /* 9. 为任务控制块分配唯一编号 */
        uxTaskNumber++;
        
/* 10. 如果启用了FreeRTOS的追踪功能 */
#if (configUSE_TRACE_FACILITY == 1)
        {
            /* 11. 将全局任务编号赋值给新添加的任务 */
            pxNewTCB->uxTCBNumber = uxTaskNumber;
        }
#endif
        /* 12. 向就绪列表中添加新的TCB */
        prvAddTaskToReadyList(pxNewTCB);
    }
    taskEXIT_CRITICAL();
    /*================== 退出临界区 ==================*/
    
    /* 13. 如果调度器已启动 */
    if (xSchedulerRunning != pdFALSE)
    {
        /* 14. 新任务优先级比正在运行的任务优先级高 */
        if (pxCurrentTCB->uxPriority < pxNewTCB->uxPriority)
        {
            /* 15. 如果启用了抢占式任务调度，则当前任务立即让出，新任务执行 */
            taskYIELD_IF_USING_PREEMPTION();
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}
```

> `prvAddTaskToReadyList`是一个宏，
>
> ```c
> /*
>     1. 用于在任务状态变为就绪（Ready）时记录调试或跟踪信息
>     2. 记录任务的优先级到全局优先级跟踪变量，确保调度器知道当前有哪些优先级的任务处于就绪状态。
>     3. 在对应的优先级队列中，插入一个列表项
>     4. 在任务成功添加到就绪列表后记录调试或跟踪信息
> */
> #define prvAddTaskToReadyList(pxTCB)                                                       \
>     traceMOVED_TASK_TO_READY_STATE(pxTCB);                                                 \
>     taskRECORD_READY_PRIORITY((pxTCB)->uxPriority);                                        \
>     vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]), &((pxTCB)->xStateListItem)); \
>     tracePOST_MOVED_TASK_TO_READY_STATE(pxTCB)
> ```

# 3.任务删除过程分析

主要干了如下的事情：

- 从就绪列表、事件列表中移除任务
- 处理任务删除后的清理工作
- 在必要时触发任务切换

```c
void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    TCB_t *pxTCB;
    taskENTER_CRITICAL();
    {
        /* 1. 从任务句柄（TCB指针）中获取该任务的TCB */
        pxTCB = prvGetTCBFromHandle(xTaskToDelete);
        
        /* 2. 把TCB对应的列表项从列表中移除，并且判断删除之后列表中列表项个数是否为0 */
        // （uxListRemove函数会自动获取pxTCB->xStateListItem属于哪个列表）
        if (uxListRemove(&(pxTCB->xStateListItem)) == (UBaseType_t)0)
        {
            /* 3. 清除 uxReadyPriorities 中的对应位，告诉调度器“这个优先级已经没有就绪任务了” */
            // （uxReadyPriorities是一个位图，每一位标志着当前优先级是否有就绪任务）
            taskRESET_READY_PRIORITY(pxTCB->uxPriority);
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
        
        /* 4.如果任务pxTCB当前还挂在某个事件列表上（比如信号量、队列、事件组的等待列表），就把它从列表里移除 */
        // FreeRTOS 中很多内核对象（例如信号量、队列、事件组）都维护一个等待链表，叫做事件列表（Event List）。
        // 这个列表存放的是当前正在等待这个对象的任务。
        if (listLIST_ITEM_CONTAINER(&(pxTCB->xEventListItem)) != NULL)
        {
            (void)uxListRemove(&(pxTCB->xEventListItem));
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
        
        /* 5. 任务编号加一，每次创建/删除任务都会自增一次，用于跟踪系统中任务变化历史 */
        uxTaskNumber++;
        
        /* 6. 如果当前执行的任务就是要删除的任务 */
        if (pxTCB == pxCurrentTCB)
        {
            // 将其加入等待终止队列
            vListInsertEnd(&xTasksWaitingTermination, &(pxTCB->xStateListItem));
            // 有一个任务等着清理资源
            ++uxDeletedTasksWaitingCleanUp;
            // 提供一个平台相关的删除钩子回调（可自定义）
            portPRE_TASK_DELETE_HOOK(pxTCB, &xYieldPending);
        } else
        /* 7. 如果不是正在执行的任务，就直接删除 */
        {
            // 当前任务数减一
            --uxCurrentNumberOfTasks;
            // 删除TCB
            prvDeleteTCB(pxTCB);
            // 重置下一个 unblock 时间（如果这个任务曾在延时列表）
            prvResetNextTaskUnblockTime();
        }
        // 统一记录日志用途
        traceTASK_DELETE(pxTCB);
    }
    taskEXIT_CRITICAL();
    
    // 如果删除的是正在运行的任务那么就需要强制进行一次任务切换。
    if (xSchedulerRunning != pdFALSE)
    {
        if (pxTCB == pxCurrentTCB)
        {
            configASSERT(uxSchedulerSuspended == 0);
            portYIELD_WITHIN_API();
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
}
```

# 4.任务挂起过程分析

主要干了如下几件事情：

- 找到当前需要挂起的任务的TCB
- 

```c
void vTaskSuspend(TaskHandle_t xTaskToSuspend)
{
    TCB_t *pxTCB;
    
    // 对任务队列的操作需要进入临界区
    taskENTER_CRITICAL();
    {
        /* 1. 获取任务句柄对应的TCB */
        pxTCB = prvGetTCBFromHandle(xTaskToSuspend);
        
        /* 2. 将任务从就绪或者延时列表中删除，并放入挂起列表 */
        // uxListRemove函数会自动获取待删除列表项所属的列表，xStateListItem所属可能的列表包含：就绪列表、延时列表、挂起列表
        // 返回值为零，说明删除后该列表中没有列表项
        if (uxListRemove(&(pxTCB->xStateListItem)) == (UBaseType_t)0)
        {
            /* 3. 如果删除之后，其所处列表（就绪列表/延时列表）中元素为零，则重置该任务的优先级位图 */
            // 即使该任务原来处于的队列不是就绪队列，也可以执行重置就绪队列的优先级重置操作，因为taskRESET_READY_PRIORITY内部会自动检查优先级队列是否为空
            taskRESET_READY_PRIORITY(pxTCB->uxPriority);
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
        
        /* 4. 判断是否当前TCB的xEventListItem处于一个事件等待队列中（当前任务是否在等待某个事件） */
        // #define listLIST_ITEM_CONTAINER( pxListItem )            ( ( pxListItem )->pxContainer )
        if (listLIST_ITEM_CONTAINER(&(pxTCB->xEventListItem)) != NULL)
        {
            /* 5. 如果当前任务确实在等待某个事件，则将其从事件等待列表中删除 */
            (void)uxListRemove(&(pxTCB->xEventListItem));
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
        /* 6. 将当前TCB的xStateListItem放入挂起队列 */
        vListInsertEnd(&xSuspendedTaskList, &(pxTCB->xStateListItem));
    }
    taskEXIT_CRITICAL();
    
    if (xSchedulerRunning != pdFALSE)
    {
        taskENTER_CRITICAL();
        {
            /* 7. 如果调度器正在运行，则重新计算一下还要多长时间执行下一个任务，也就是下一个任务的解锁时间。防止有任务的解锁时间参考了刚刚被挂起的那个任务。（即计算：当前系统中所有阻塞任务中，最先应该被唤醒的那个时间点 */
            prvResetNextTaskUnblockTime();
        }
        taskEXIT_CRITICAL();
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
    
    /* 8. 如果当前任务是正在运行的任务 */
    if (pxTCB == pxCurrentTCB)
    {
        if (xSchedulerRunning != pdFALSE)
        {
            /* 9. 确保调度器没被挂起，否则不能安全切换任务 */
            configASSERT(uxSchedulerSuspended == 0);
            
            /* 10. 调用 portYIELD_WITHIN_API()，触发任务切换 */
            portYIELD_WITHIN_API();
        } else
		// 如果任务调度器没在运行，则需要手动执行任务切换
        {
            if (listCURRENT_LIST_LENGTH(&xSuspendedTaskList) == uxCurrentNumberOfTasks)
            {
                /* 11. 如果所有任务都被挂起了，说明系统中已经没有可运行的任务了，此时将 pxCurrentTCB 设为 NULL */
                pxCurrentTCB = NULL;
            } else
            {
                /* 12. 进行一次手动上下文切换，让别的任务成为当前任务 */
                vTaskSwitchContext();
            }
        }
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}
```

# 5.任务恢复过程

任务恢复函数有两个 `vTaskResume()`和 `xTaskResumeFromISR()`，一个是用在任务中的，一个是用在中断中的，但是基本的处理过程都是一样的，我们就以函数 `vTaskResume()`为例来讲解一下任务恢复详细过程。  

```c
void vTaskResume(TaskHandle_t xTaskToResume)
{
    /* 1. 强制转为TCB指针（虽然TaskHandle_t本质也是指向TCB的指针） */
    TCB_t *const pxTCB = (TCB_t *)xTaskToResume;
    
    /* 2. 确保待恢复任务句柄不为空 */
    configASSERT(xTaskToResume);

    /* 3. 如果待恢复任务存在且当前正在执行的任务不是待恢复任务 */
    if ((pxTCB != NULL) && (pxTCB != pxCurrentTCB))
    {
        taskENTER_CRITICAL();
        {
            /* 4. 判断任务是否处于挂起状态 */
            if (prvTaskIsTaskSuspended(pxTCB) != pdFALSE)
            {
                traceTASK_RESUME(pxTCB);
                
                /* 5. 将当前任务从其所属的队列删除 */
                (void)uxListRemove(&(pxTCB->xStateListItem));
                /* 6. 将当前任务放到就绪队列中 */
                prvAddTaskToReadyList(pxTCB);
                /* 7. 待恢复任务的优先级比当前执行的任务优先级大 */
                if (pxTCB->uxPriority >= pxCurrentTCB->uxPriority)
                {
                    /* 8. 执行任务切换 */
                    taskYIELD_IF_USING_PREEMPTION();
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                } else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            taskEXIT_CRITICAL();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
}
```



# 0.【附】其他函数

## 0.1 taskRESET_READY_PRIORITY函数

**功能**：清除就绪队列中某个优先级对应的位，即将该优先级标记为“没有就绪任务”。

```c
#define taskRESET_READY_PRIORITY( uxPriority )                                                     \
{                                                                                                  \
	if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 ) \
	{                                                                                              \
		portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );                        \
	}                                                                                              \
}
```

上述函数的执行逻辑为：

- 根据参数`uxPriority`所指定的优先级列表中元素是否为零
- `uxTopReadyPriority`是一个全局变量，**记录当前所有就绪队列中优先级最高的优先级**

## 0.2 portRESET_READY_PRIORITY函数

**功能**：重置 就绪队列优先级位图中**指定的优先级位**为0

```c
#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )     ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )
```

> **注意**：使用硬件方法的时候 uxTopReadyPriority 就不代表处于就绪态的最高优先级了，而是使用每个 bit 代表一个优先级， bit0 代表优先级 0， bit31 就代表优先级 31，当某个优先级有就绪任务的话就将其对应的 bit 置 1。  

