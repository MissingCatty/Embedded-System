# 1 任务创建和删除

| 函数                      | 描述                                                      |
| ------------------------- | --------------------------------------------------------- |
| `xTaskCreate()`           | 使用动态的方法创建一个任务。                              |
| `xTaskCreateStatic()`     | 使用静态的方法创建一个任务。                              |
| `xTaskCreateRestricted()` | 创建一个使用MPU进行限制的任务，相关内存使用动态内存分配。 |
| `vTaskDelete()`           | 删除一个任务。                                            |

## 1.1 xTaxkCreate

创建一个任务 ，任务需要开辟的RAM空间包括：

- 与任务有关的状态信息 (任务控制块)
- 任务堆栈

所需的 RAM 自动的从 **FreeRTOS 的堆**中分配，因此**必须提供内存管理文件**，默认我们使用heap_4.c 这个内存管理文件，且宏 `configSUPPORT_DYNAMIC_ALLOCATION`必须为 1。

新创建的任务**默认为就绪态**，如果当前没有比它更高优先级的任务运行那么此任务就会立即进入运行态开始运行。在任务调度器启动前或启动后，都可以创建任务。  

```c
BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                       const char * const pcName,
                       const configSTACK_DEPTH_TYPE usStackDepth,
                       void * const pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t * const pxCreatedTask )
```

|      参数名       |       类型       |                        描述                         |                           注意事项                           |
| :---------------: | :--------------: | :-------------------------------------------------: | :----------------------------------------------------------: |
|  **pxTaskCode**   | `TaskFunction_t` |       任务函数指针，指向任务的具体实现代码。        | 函数需为`void foo(void *pvParameters)`格式，且包含无限循环或自我删除逻辑。 |
|    **pcName**     |  `const char *`  |        任务名称（字符串），用于调试和追踪。         | 长度不可超过`configMAX_TASK_NAME_LEN`（默认16字符），超长部分会被截断。 |
| **usStackDepth**  |    `uint16_t`    |            任务堆栈深度（以字为单位）。             | 实际堆栈大小=usStackDepth×4字节（32位系统）。空闲任务默认使用`configMINIMAL_STACK_SIZE`。 |
| **pvParameters**  |     `void *`     |             传递给任务函数的参数指针。              |     可传递结构体地址，需确保参数生命周期覆盖任务运行期。     |
|  **uxPriority**   |  `UBaseType_t`   | 任务优先级（0最低，`configMAX_PRIORITIES-1`最高）。 |        优先级数值越大等级越高，需避免优先级反转问题。        |
| **pxCreatedTask** | `TaskHandle_t *` |       输出参数，用于保存创建成功的任务句柄。        | 句柄可用于后续操作（如删除任务、修改优先级）。若不需要可传`NULL`。 |

|                 **返回值**                  |   **含义**   |                  **原因分析**                   |
| :-----------------------------------------: | :----------: | :---------------------------------------------: |
|                **`pdPASS`**                 | 任务创建成功 |    系统成功分配了任务控制块(TCB)和堆栈内存。    |
| **`errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`** | 任务创建失败 | 堆内存不足，无法分配任务所需资源（TCB或堆栈）。 |

## 1.2 xTaskCreateStatic

此函数和 `xTaskCreate()`的功能相同，也是用来创建任务的，但是使用此函数创建的任务**所需的 RAM 需要用户来提供**。如果要使用此函数的话需要将宏 `configSUPPORT_STATIC_ALLOCATION`定义为 1。

```c
TaskHandle_t xTaskCreateStatic( TaskFunction_t pxTaskCode,
                               const char * const pcName,
                               const uint32_t ulStackDepth,
                               void * const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t * const puxStackBuffer,
                               StaticTask_t * const pxTaskBuffer )
```

|       参数名       |       类型       |                      关键说明                       |                           技术要点                           |
| :----------------: | :--------------: | :-------------------------------------------------: | :----------------------------------------------------------: |
|   **pxTaskCode**   | `TaskFunction_t` |                  任务入口函数指针                   | 必须为`void func(void *)`格式，且包含永久循环或自杀逻辑（`vTaskDelete(NULL)`） |
|     **pcName**     |  `const char *`  |               任务名称（ASCII字符串）               | 长度受限于`configMAX_TASK_NAME_LEN`（默认16字节），超长部分自动截断 |
|  **usStackDepth**  |    `uint16_t`    |          堆栈深度（以`StackType_t`为单位）          | 实际内存=深度×4字节（32位系统），需通过`uxTaskGetStackHighWaterMark()`验证余量 |
|  **pvParameters**  |     `void *`     |                    任务参数指针                     |      可传递结构体地址，需确保参数生命周期≥任务生命周期       |
|   **uxPriority**   |  `UBaseType_t`   | 优先级（0=空闲任务，`configMAX_PRIORITIES-1`=最高） |     优先级数值越大等级越高，建议保留2-3级给系统关键任务      |
| **puxStackBuffer** | `StackType_t *`  |               静态分配的堆栈数组指针                | 数组需定义为`static StackType_t xStack[深度]`，且内存对齐需满足架构要求 |
|  **pxTaskBuffer**  | `StaticTask_t *` |              静态分配的任务控制块指针               |   需定义为`static StaticTask_t xTaskBuffer`，不可重复使用    |

|       **返回结果**       |              **输入条件**              |                       **技术原理**                        |
| :----------------------: | :------------------------------------: | :-------------------------------------------------------: |
|       任务创建失败       | **puxStackBuffer或pxTaskBuffer为NULL** | 静态创建必须预分配堆栈和TCB内存，NULL指针违反内存安全原则 |
| 返回任务句柄（创建成功） |       **传入有效的静态内存指针**       |   系统验证内存有效性后，初始化任务控制块并加入调度队列    |

## 1.3 xTaskCreateRestricted

此函数也是用来创建任务的，只不过此函数要求所使用的 MCU 有 MPU(内存保护单元)，用此函数创建的任务会受到 MPU 的保护。其他的功能和函数 `xTaxkCreate()`一样。

```c
BaseType_t xTaskCreateRestricted(const TaskParameters_t * const pxTaskDefinition,
                                 TaskHandle_t * pxCreatedTask)
```

| 参数名             | 类型                | 描述                                                         |
| ------------------ | ------------------- | ------------------------------------------------------------ |
| `pxTaskDefinition` | `TaskParameters_t*` | 指向任务参数结构体，包含任务函数、堆栈大小、优先级等定义（定义于`task.h`） |
| `pxCreatedTask`    | `TaskHandle_t*`     | 输出参数，用于返回创建成功的任务句柄                         |

|  **返回值**  |   **含义**   |                     **原因分析**                      |
| :----------: | :----------: | :---------------------------------------------------: |
| **`pdPASS`** | 任务创建成功 |       系统成功分配了任务控制块(TCB)和堆栈内存。       |
|  **其他值**  | 任务创建失败 | 任务未创建成功， 很有可能是因为 FreeRTOS 的堆太小了。 |

## 1.4 vTaskDelete

删除一个用函数 xTaskCreate()或者 xTaskCreateStatic()创建的任务，被删除了的任务不再存在，也就是说再也不会进入运行态。

任务被删除以后就不能再使用此任务的句柄！如果此任务是使用动态方法创建的，也就是使用函数 `xTaskCreate()`创建的，那么在此任务被删除以后此**任务申请的堆栈和控制块内存会在空闲任务中被释放掉**，因此当调用函数 `vTaskDelete()`删除任务以后**必须给空闲任务一定的运行时间**。 

只有那些由**内核<u>分配给任务的内存</u>才会在任务被删除以后自动的释放掉**，**用户分配给任务的内存需要用户自行释放掉**。比如某个任务中用户调用函数 `pvPortMalloc()`分配了 500 字节的内存，那么在此任务被删除以后用户也必须调用函数 `vPortFree()`将这 500 字节的内存释放掉，否则会导致内存泄露。

```c
vTaskDelete( TaskHandle_t xTaskToDelete )
```

无返回值。

# 2 任务创建和删除实验(动态方法) 

## 2.1 实验目的

学习 `xTaskCreate()`和 `vTaskDelete()`这两个函数的使用。

## 2.2 实验设计

本实验设计三个任务： start_task、 task1_task 和 task2_task ，这三个任务的任务功能如下：

- start_task：用来创建其他两个任务。
- task1_task ：当此任务运行 5 秒以后就会调用函数 `vTaskDelete()`删除任务task2_task，此任务也会控制 LED0 的闪烁，并且周期性的刷新 LCD 指定区域的背景颜色（LED0闪烁和LCD刷新周期都是1秒，等于说程序运行5秒后删除task2）。
- task2_task ： 此任务普通的应用任务，此任务也会控制 LED1 的闪烁，并且周期性的刷新 LCD 指定区域的背景颜色。

## 2.3 实验程序

```c
#define TASK1_TASK_PRIO 2
#define TASK2_TASK_PRIO 2 // 相同优先级

void task1_task(void *p_arg)
{
    u16 sx = 20, sy = 20, width = 40;
    u16 ex = sx + width - 1, ey = sy + width - 1;
    u8 count = 0, task2_deleted = 0;
    while (1)
    {
        LCD_Fill(sx, sy, ex, ey, colors[count % 3]);
        led_on(&led0);
        vTaskDelay(500);
        led_off(&led0);
        vTaskDelay(500);
        count++;
        if (count==5 && !task2_deleted)
        {
            vTaskDelete(TASK2Task_Handler); // 删除任务
			task2_deleted = 1;
        }
    }
}

void task2_task(void *p_arg)
{
    u16 sx = 200, sy = 20, width = 40;
    u16 ex = sx + width - 1, ey = sy + width - 1;
    u8 count = 0;
    while (1)
    {
		LCD_Fill(sx, sy, ex, ey, colors[count % 3]);
        led_on(&led1);
        vTaskDelay(500);
        led_off(&led1);
        vTaskDelay(500);
        count++;
    }
}

```

# 3 任务创建和删除实验(静态方法) 

## 3.1 实验目的

使用函数 `xTaskCreateStatic()`来创建任务  

## 3.2 实验设计

与第2部分一致

## 3.3 实验程序

首先需要打开静态内存分配功能：

```c
#define configSUPPORT_STATIC_ALLOCATION 1
```

编译后，会报错：

![image-20250406100124068](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/10-01-31-450ce3cbcd40d5cb8a6de73b303c0927-image-20250406100124068-6644f1.png)

发现在`tasks.c`文件中，`vTaskStartScheduler`函数中调用了`vApplicationGetIdleTaskMemory`函数，但该函数并没有实现：

```c
void vTaskStartScheduler( void )
{
    BaseType_t xReturn;

    /* Add the idle task at the lowest priority. */
    #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
    {
        StaticTask_t * pxIdleTaskTCBBuffer = NULL;
        StackType_t * pxIdleTaskStackBuffer = NULL;
        uint32_t ulIdleTaskStackSize;

        /* The Idle task is created using user provided RAM - obtain the
         * address of the RAM then create the idle task. */
        vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer, &pxIdleTaskStackBuffer, &ulIdleTaskStackSize );
        xIdleTaskHandle = xTaskCreateStatic( prvIdleTask,
                                             configIDLE_TASK_NAME,
                                             ulIdleTaskStackSize,
                                             ( void * ) NULL,
                                             portPRIVILEGE_BIT,
                                             pxIdleTaskStackBuffer,
                                             pxIdleTaskTCBBuffer );

        if( xIdleTaskHandle != NULL )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }
    }
    #else{
    	...
	}
}
```

---

`vApplicationGetIdleTaskMemory`函数在`task.h`中有声明：

```c
#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
/**
 * task.h
 * @code{c}
 * void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
 * @endcode
 *
 * This function is used to provide a statically allocated block of memory to FreeRTOS to hold the Idle Task TCB.  This function is required when
 * configSUPPORT_STATIC_ALLOCATION is set.  For more information see this URI: https://www.FreeRTOS.org/a00110.html#configSUPPORT_STATIC_ALLOCATION
 *
 * @param ppxIdleTaskTCBBuffer A handle to a statically allocated TCB buffer
 * @param ppxIdleTaskStackBuffer A handle to a statically allocated Stack buffer for the idle task
 * @param pulIdleTaskStackSize A pointer to the number of elements that will fit in the allocated stack buffer
 */
    void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                        StackType_t ** ppxIdleTaskStackBuffer,
                                        uint32_t * pulIdleTaskStackSize );
#endif
```

根据注释可知，这个函数是用于给Idle任务分配一个静态内存块的，以存储Idle任务的TCB和任务栈。看这三个参数：

- `ppxIdleTaskTCBBuffer`：存储TCB的内存块地址的指针，`StaticTask_t`是一个TCB结构体类型，规定了TCB中应该包含哪些内容
  - 这里为什么是`StaticTask_t **`而不是`StaticTask_t *`，因为该函数不需要对TCB的内容读取或修改（对TCB内容不关心），只需要获取TCB对应的分配的内存块的地址，所以实际关心的是`StaticTask_t *`。又因为要获取修改后的地址，所以要传一个指针进去，直接传值的话原参数不变，所以对`StaticTask_t *`取指针为`StaticTask_t **`。
- `ppxIdleTaskStackBuffer`：存储栈的内存地址的指针
- `pulIdleTaskStackSize`：栈中能够存储元素的最大个数（非字节数）

`vApplicationGetIdleTaskMemory`实现：

```c
#include "FreeRTOS.h"

static StaticTask_t xIdleTaskTCB;
static StackType_t  xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    * ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    * ppxIdleTaskStackBuffer = xIdleStack;
    * pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
```

---

`vApplicationGetIdleTaskMemory`函数在`timers.h`中有声明：

```c
/**
 * task.h
 * @code{c}
 * void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
 * @endcode
 *
 * This function is used to provide a statically allocated block of memory to FreeRTOS to hold the Timer Task TCB.  This function is required when
 * configSUPPORT_STATIC_ALLOCATION is set.  For more information see this URI: https://www.FreeRTOS.org/a00110.html#configSUPPORT_STATIC_ALLOCATION
 *
 * @param ppxTimerTaskTCBBuffer   A handle to a statically allocated TCB buffer
 * @param ppxTimerTaskStackBuffer A handle to a statically allocated Stack buffer for the idle task
 * @param pulTimerTaskStackSize   A pointer to the number of elements that will fit in the allocated stack buffer
 */
    void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                         StackType_t ** ppxTimerTaskStackBuffer,
                                         uint32_t * pulTimerTaskStackSize );

#endif
```

由注释可知，获取**定时器服务任务**的任务堆栈和任务控制块内存。

> **什么是定时器任务？**
>
> 当你使用 FreeRTOS 的 `xTimerCreate()`、`xTimerStart()` 这些函数时，FreeRTOS 会自动创建一个隐藏的 **Timer Service Task** 来帮你处理定时器的回调函数。这个任务的作用就是：
>
> **管理所有软件定时器的运行时间，并在时间到达时调用相应的回调函数（callback）。**

```c
static StaticTask_t xTimerTaskTCB;
static StackType_t  xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    * ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    * ppxTimerTaskStackBuffer = xTimerStack;
    * pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
```

---

在创建任务时，使用`xTaskCreateStatic`，每个任务除了动态创建的参数外，还需要自定义静态TCB存储区和任务栈存储区：

```c
#define START_TASK_PRIO 1
#define START_STK_SIZE  128
static StaticTask_t StartTaskTCB;                   // 任务控制块
static StackType_t  StartTaskStk[START_STK_SIZE];   // 任务堆栈
TaskHandle_t        StartTask_Handler;
void                start_task(void *pvParameters);
```

然后`xTaskCreateStatic`函数的返回值是任务句柄，和`xTaskCreate`稍有不同：

```c
StartTask_Handler = xTaskCreateStatic(
    (TaskFunction_t)start_task,
    (const char *)"start_task",
    (uint32_t)START_STK_SIZE,
    (void *)NULL,
    (UBaseType_t)START_TASK_PRIO,
    (StackType_t *)&StartTaskStk,
    (StaticTask_t *)&StartTaskTCB
);
```

# 4 任务挂起和恢复API

有时候我们需要暂停某个任务的运行，过一段时间以后在重新运行。这个时候要是使用任务删除和重建的方法的话那么任务中变量保存的值肯定丢失了！FreeRTOS 给我们提供了解决这种问题的方法， 那就是任务挂起和恢复，**当某个任务要停止运行一段时间的话就将这个任务挂起**，当要重新运行这个任务的话就恢复这个任务的运行。

| 函数名称               | 核心功能说明                                                 |
| ---------------------- | ------------------------------------------------------------ |
| `vTaskSuspend()`       | 挂起指定任务（任务进入阻塞状态，不再参与调度）               |
| `vTaskResume()`        | 恢复被挂起的任务（使任务重新进入就绪状态）                   |
| `xTaskResumeFromISR()` | 在中断服务程序(ISR)中恢复任务（带返回值，需检查是否需要上下文切换） |

## 4.1 vTaskSuspend

此函数用于将某个任务设置为挂起态， 进入挂起态的任务永远都不会进入运行态。退出挂起态的唯一方法就是调用任务恢复函数 `vTaskResume()`或 `xTaskResumeFromISR()`。

```c
void vTaskSuspend( TaskHandle_t xTaskToSuspend)
```

- `xTaskToSuspend`：要挂起的任务的任务句柄。也可以通过函数 `xTaskGetHandle()`来**根据任务名字来获取**某个任务的任务句柄。 注意！ 如果**参数为 NULL 的话表示挂起任务自己**。

## 4.2 vTaskResume

将一个任务从挂起态恢复到就绪态， 只有通过函数 `vTaskSuspend()`设置为挂起态的任务才可以使用 `vTaskRexume()`恢复。

```c
void vTaskResume( TaskHandle_t xTaskToResume)
```

## 4.3 xTaskResumeFromISR

此函数是 `vTaskResume()`的中断版本，用于在中断服务函数中恢复一个任务。  

```c
BaseType_t xTaskResumeFromISR( TaskHandle_t xTaskToResume)
```

| 返回值  | 含义                                                         |
| ------- | ------------------------------------------------------------ |
| pdTRUE  | 恢复运行的任务的任务优先级等于或者高于正在运行的任务(被中断打断的任务)， 这意味着在退出中断服务函数以后必须进行一次上下文切换。（**因为被恢复的任务优先级高，恢复后会立刻抢占当前正在执行的任务**） |
| pdFALSE | 恢复运行的任务的任务优先级低于当前正在运行的任务(被中断打断的任务)，这意味着在退出中断服务函数的以后不需要进行上下文切换。 |

# 5 任务挂起和恢复实验  

## 5.1 实验目的

学习使用 FreeRTOS 的任务挂起和恢复相关 API 函数，包括 `vTaskSuspend()`、 `vTaskResume()`和 `xTaskResumeFromISR()`。

## 5.2 实验设计

本实验设计 4 个任务： start_task、 key_task、 task1_task 和 task2_task，这四个任务的任务功能如下：

- start_task：用来创建其他 3 个任务。 
- key_task： 按键服务任务，检测按键的按下结果，根据不同的按键结果执行不同的操作。
- task1_task：应用任务 1。 
- task2_task：应用任务 2。

由于只有两个按键，所以仅用两个按键控制任务1：

- KEY3：此按键为输入模式，用于挂起任务 1 的运行。
- KEY4：此按键为输入模式，用于恢复任务 1 的运行。

## 5.3 实验程序

由于挂起函数`vTaskSuspend()`不能用在中断函数里，所以直接在按键任务中以轮询的方式监测其是否按下：

```c
void key_task(void *p_arg)
{
    while (1)
    {
        // 轮询key3状态
        if (key_pushed(&key3))
        {
            // 延时消抖
            delay_ms(10);
            if (key_pushed(&key3))
            {
                // 挂起task1
                vTaskSuspend(TASK1Task_Handler);
                LCD_Fill(280, 10, 280 + 20, 10 + 20, RED);
            }
        }
    }
}
```

在按键中断函数中恢复任务：

```c
void EXTI0_IRQHandler(void)
{
    // key4中断函数
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        delay_ms(10);
        if (key_pushed(&key4))
        {
            // 恢复task1
            xTaskResumeFromISR(TASK1Task_Handler);
            LCD_Fill(280, 10, 280 + 20, 10 + 20, GREEN);
        }
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除中断标志位
    }
}
```

