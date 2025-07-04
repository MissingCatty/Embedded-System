# 1 任务切换概述

RTOS 系统的核心是任务管理，而任务管理的核心是任务切换，**任务切换决定了任务的执行顺序**，**任务切换效率的高低也决定了一款系统的性能**， 尤其是对于实时操作系统。  

RTOS依赖`PendSV`来进行任务切换，`PendSV`：

- Cortex-M 架构下的一个**可编程系统异常**；
- **优先级最低**（通常设置为最低），以便让其他中断先执行完；
- **不会被自动触发**，需要显式“挂起”它

在 FreeRTOS 中，**任务上下文切换是通过触发 PendSV 中断来实现的**。

```c
#define portYIELD()            portYIELD_WITHIN_API()
#define portYIELD_WITHIN_API() SCB->ICSR = SCB_ICSR_PENDSVSET_Msk
```

这个宏做了这件事：

```c
SCB->ICSR |= (1 << 28); // 设置 PENDSVSET 位，触发 PendSV 中断
```

当你设置了 PendSV 挂起位后，**CPU 会在当前中断或指令执行完后跳转到 PendSV_Handler**。

在 FreeRTOS 的移植代码中（`port.c`），你会看到类似这样的：

```c
void PendSV_Handler(void)
{
    // 保存当前任务上下文
    // 选择下一个要运行的任务（通过 vTaskSwitchContext）
    // 恢复新任务上下文
}
```

不同 Cortex-M 移植版本略有差异，但核心步骤如下：

| 步骤 | 描述                                       |
| ---- | ------------------------------------------ |
| ①    | 当前任务保存上下文（寄存器、SP）           |
| ②    | 调用 `vTaskSwitchContext()` 选择下一个任务 |
| ③    | 加载新任务的上下文                         |
| ④    | 设置 `PSP`，恢复状态后返回新任务           |

相关的中断有：

| 名称      | 作用                                                         |
| --------- | ------------------------------------------------------------ |
| `SVC`     | 当一个运行在用户模式的任务需要操作系统提供服务（如创建任务、发送信号量、延迟、请求内存分配等）时，它**不能直接调用内核函数**。而是需要通过执行一条特定的汇编指令（**`SVC` 指令**）来**主动触发一个 SVC 异常**。 |
| `SysTick` | 周期性产生时钟中断，用于时间片轮转                           |
| `PendSV`  | 任务切换的实际执行者                                         |

上下文切换被触发的场合可以是：  

- 执行一个系统调用  
- 系统滴答定时器(SysTick)中断

上下文切换需要`SysTick`和`PendSV`之间配合完成，每次触发`SysTick`，系统都需要问：是否需要进行上下文切换？

当需要进行切换时，才通过系统手动触发`PendSV`中断。

实时操作系统，一个重要的原则就是：**任务上下文的切换，不应该对中断请求IRQ造成延时**。也就是说，如果IRQ正在处理，此时来了一个`Systick`并且需要进行上下文切换（触发`PendSV`异常），如果上下文切换过程中断了IRQ的处理过程，那么这是不被允许的，属于系统的设计错误。

![image-20250604083752987](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F06%2Fc42112d11b63bf3a2dac82094ed39722.png)

为了解决这个问题，需要将`PendSV`异常对上下文切换的请求推迟到其他IRQ处理都完成之后，也就是**将`PendSV`的优先级设置为最低**。

![image-20250604085031642](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F06%2Fe66678931fabdaf00e6fbb19e609b3ba.png)

# 2 任务切换的场合

之前讲到，**可能**产生上下文切换（也就是任务切换）的场合有两种：

- 执行一个系统调用
- 系统滴答定时器(SysTick)中断

## 2.1 执行系统调用

**系统调用（System Call）** 是用户任务请求内核服务（如任务管理、资源同步等）的核心机制，主要通过 **SVC（Supervisor Call）异常**实现。

**系统调用的实现机制**

1. **SVC 异常触发**：用户任务通过执行 `SVC` 指令（例如 `svc #0`）主动触发异常，强制 CPU 从**非特权模式（用户态）** 切换到**特权模式**
   - **调用号传递**：`SVC` 指令中的立即数（如 `#0`）用于标识具体的系统调用类型（如任务创建、信号量操作等）
   - **硬件自动行为**：触发后，CPU 自动保存现场（PSR、PC、LR 等寄存器到任务栈），并跳转到 `SVC_Handler` 异常处理函数
2. **SVC中断处理**：`SVC_Handler` 函数在特权模式下执行以下操作：
   - **解析调用号**：从栈帧中提取 `SVC` 指令的立即数，确定请求的服务类型
   - **获取参数**：通过寄存器（R0-R3）传递用户任务的参数（如任务优先级、堆栈大小等）
   - **执行内核函数**：根据调用号跳转到对应的服务函数（如 `xTaskCreate`、`xQueueSend` 等）
3. **返回用户模式**：服务执行完成后，内核将结果写入寄存器（如 R0 存储返回值），并通过 `BX LR` 指令退出异常。CPU 自动恢复现场并切换回用户模式，继续执行用户任务的下一条指令。

---

**哪些系统调用会导致上下文切换？**

|    系统调用类型    |               典型函数                |  是否切换  |
| :----------------: | :-----------------------------------: | :--------: |
|    主动放弃 CPU    |     `vTaskDelay()`, `taskYIELD()`     |   必然 ✅   |
| 唤醒更高优先级任务 |  `xSemaphoreGive()`, `xQueueSend()`   | 条件触发 ⚡ |
|  自身阻塞等待资源  | `xSemaphoreTake()`, `xQueueReceive()` |   必然 ✅   |
|  改变优先级/状态   | `vTaskPrioritySet()`, `vTaskResume()` | 条件触发 ⚡ |
|      创建任务      |            `xTaskCreate()`            | 条件触发 ⚡ |
|      删除自身      |          `vTaskDelete(NULL)`          |   必然 ✅   |

## 2.2 SysTick中断  

FreeRTOS 中滴答定时器`SysTick`中断服务函数中也会进行任务切换，滴答定时器中断服务函数如下：

```c
void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) // 调度器正在运行
    {
        xPortSysTickHandler();
    }
}
```

在滴答定时器中断服务函数中调用了 FreeRTOS 的 API 函数 `xPortSysTickHandler()`，此函数源码如下：  

```c
void xPortSysTickHandler(void)
{
    /* 1. 关闭中断，提升中断屏蔽优先级 */
    vPortRaiseBASEPRI();
    {
        /* 2. 增加系统时钟计数器 */
        if (xTaskIncrementTick() != pdFALSE)
        {
            /* 3. 挂起PendSV，触发PendSV异常，进行任务切换 */
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
    }
    /* 4. 打开中断，提升中断屏蔽优先级 */
    vPortClearBASEPRIFromISR();
}
```

# 3 PendSV中断服务函数

PendSV 中断服务函数本应该为 PendSV_Handler()，但是 FreeRTOS 使用#define 重定义了，如下：

```c
#define xPortPendSVHandler PendSV_Handler
```

```asm
__asm void xPortPendSVHandler(void)
{
    extern uxCriticalNesting;
    extern pxCurrentTCB;
    extern vTaskSwitchContext;

    PRESERVE8

    // ---- 保存当前任务上下文 ----
    mrs     r0, psp                        ; (1) 读取进程栈指针（PSP）到 r0
    isb                                    ;     指令同步屏障，确保前一条完成

    ldr     r3, =pxCurrentTCB              ; (2) 加载 pxCurrentTCB 的地址
    ldr     r2, [r3]                       ; (3) 获取当前任务的 TCB 地址

    tst     r14, #0x10                     ; (4) 检查是否使用浮点扩展
    it      eq                             ; (5) 如果使用，则进入下面的指令
    vstmdbeq r0!, {s16-s31}                ; (6) 保存浮点寄存器 s16-s31 到堆栈

    stmdb   r0!, {r4-r11, r14}             ; (7) 保存核心寄存器 r4-r11 和 lr 到堆栈，r0自动递减，指向栈顶
    str     r0, [r2]                       ; (8) 更新 TCB 中保存的当前任务的堆栈指针（PSP）

    // ---- 调用任务调度器切换任务 ----
    stmdb   sp!, {r3}                      ; (9) 保存 r3（pxCurrentTCB 地址）到 MSP
    mov     r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY ; (10) 设置最大系统调用中断优先级
    msr     basepri, r0                    ; (11) 设置中断屏蔽，设置最大调用优先级
    dsb                                    ;     数据同步屏障
    isb
    bl      vTaskSwitchContext             ; (12) 调用调度函数，更新 pxCurrentTCB

    mov     r0, #0                         ; (13) 清除中断屏蔽
    msr     basepri, r0                    ; (14)
    ldmia   sp!, {r3}                      ; (15) 恢复 r3（pxCurrentTCB 地址）

    // ---- 恢复新任务上下文 ----
    ldr     r1, [r3]                       ; (16) 获取新任务的 TCB 地址
    ldr     r0, [r1]                       ; (17) 获取新任务的堆栈指针（PSP）

    ldmia   r0!, {r4-r11, r14}             ; (18) 恢复 r4-r11 和 lr（EXC_RETURN）

    tst     r14, #0x10                     ; (19) 检查是否使用浮点扩展
    it      eq                             ; (20)
    vldmiaeq r0!, {s16-s31}                ; (21) 如果使用了浮点，恢复 s16-s31

    msr     psp, r0                        ; (22) 设置 PSP 为新任务堆栈
    isb                                    ;     同步
    bx      r14                            ; (23) 跳转到r14指向的地址，返回到新任务，完成切换
}

```

- `(8)`：更新TCB的栈顶指针
- `(9),(15)`：保存和恢复`pxCurrentTCB`指针的地址，由于任务调度器执行后，任务被切换，`pxCurrentTCB`的值被改变，但其所属的地址不变。调度完成后，直接访问该指针的地址，就能找到其指向的地址。
- `(10)`：`configMAX_SYSCALL_INTERRUPT_PRIORITY`指的是**允许调用 FreeRTOS API 的最高中断优先级**，`(11)`把所有允许调用FreeRTOS API的中断都给屏蔽了，也就是**确保了在调度器执行期间任务切换的过程不会被打断**。 

# 4 查找下一个要运行的任务

在 PendSV 中断服务程序中有调用函数 `vTaskSwitchContext()`来获取下一个要运行的任务，也就是查找已经就绪了的优先级最高的任务：

```c
void vTaskSwitchContext(void)
{
    if (uxSchedulerSuspended != (UBaseType_t)pdFALSE)
    {
        xYieldPending = pdTRUE;
    } else
    {
        xYieldPending = pdFALSE;
        traceTASK_SWITCHED_OUT();
        taskCHECK_FOR_STACK_OVERFLOW();
        taskSELECT_HIGHEST_PRIORITY_TASK();
        traceTASK_SWITCHED_IN();
    }
}
```
