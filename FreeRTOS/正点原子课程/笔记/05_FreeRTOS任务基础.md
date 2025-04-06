# 1 前后台系统和多任务系统

没接触RTOS之前，编写的单片机程序都是在一个while循环中不断执行的，外加中断服务。其中，while循环中的程序段所执行的任务被称为**后台任务**，中断服务程序被称为**前台任务**。

- **前台**：处理实时性要求高的任务，通常由**中断服务程序**实现，响应外部事件（如按键、传感器信号）。
- **后台**：处理非实时任务，通过**主循环**轮询执行，如数据处理、状态更新等。

前后台系统的优点是代码简单，资源占用率低。

FreeRTOS是一种抢占式的多任务系统，高优先级的任务可以打断低优先级的任务。

同时，**<u>中断服务程序（ISR）的优先级通常高于所有任务的优先级</u>**，以确保硬件事件（如定时器、外设中断等）能够被及时响应，从而满足系统的实时性需求。

![image-20250405091856012](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/09-19-03-f7bb07c334fea003fa5041698e5a1758-image-20250405091856012-8b5a22.png)

# 2 FreeRTOS任务

RTOS中的每个任务都有自己的运行环境，不依赖于系统中其他的任务。

任何一个时间点只能有一个任务运行，具体运行哪个任务是由RTOS调度器来决定的，**RTOS调度器因此就会重复的开启、关闭每个任务**。RTOS调度器的职责是确保当一个任务开始执行的时候其上下文环境(寄存器值，堆栈内容等)和任务上一次退出的时候相同。为了做到这一点，每个任务都必须有个堆栈，**当任务切换的时候将上下文环境保存在堆栈中**，这样当任务再次执行的时候就可以从堆栈中取出上下文环境，任务恢复运行。因此，每个任务都拥有堆栈导致了RAM使用量增大。

# 3 任务状态

FreeRTOS中的任务永远处于下面几个状态中的某一个：

**运行态**

当一个任务正在运行时，那么就说这个任务处于运行态，处于运行态的任务就是当前正在使用处理器的任务。

**就绪态**

处于就绪态的任务是那些已经准备就绪(这些任务没有被阻塞或者挂起)，可以运行的任务，但是处于就绪态的任务还没有运行，因为有一个同优先级或者更高优先级的任务正在运行。

**阻塞态**

如果一个任务当前正在等待某个外部事件的话就说它处于阻塞态，比如说如果某个任务调用了函数`vTaskDelay`的话就会进入阻塞态，直到延时周期完成。任务在等待队列、信号量、事件组、通知或互斥信号量的时候也会进入阻塞态。**任务进入阻塞态会有一个超时时间**，当超过这个超时时间任务就会退出阻塞态，即使所等待的事件还没有来临。

**挂起态**

像阻塞态一样，任务进入挂起态以后也**不能被调度器调用进入运行态**，但是**进入挂起态的任务没有超时时间**。任务进入和退出挂起态通过调用函数`vTaskSuspend`和`xTaskResume`。

# 4 任务优先级

每个任务都可以分配一个从`0~(configMAX_PRIORITIES-1)`的优先级，`configMAX_PRIORITIES`在文件`FreeRTOSConfig.h`中有定义，前面我们讲解FreeRTOS系统配置的时候已经讲过了。

如果所使用的硬件平台支持类似计算前导零（**CLZ：快速找出一个二进制数中最左侧为 1 的位置的指令**）这样的指令，并且宏`configUSE_PORT_OPTIMISED_TASK_SELECTION`也设置为了1，那么宏`configMAX_PRIORITIES`不能超过32！也就是优先级不能超过32级。其他情况下宏`configMAX_PRIORITIES`可以为任意值，但是考虑到RAM的消耗，宏`configMAX_PRIORITIES`最好设置为一个满足应用的最小值。

> 当你设置：
>
> ```c
> #define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
> ```
>
> 代表系统**使用位图进行调度优化**，即用一个 32 位整数来作为就绪任务的“优先级位图”，每一位代表一个优先级是否有就绪任务。即用一个 32 位整数来标记每个优先级的任务是否就绪，最多只能表示 32 个不同的优先级。**使用CLZ指令，就能够快速找到32位优先级位图中最左侧的1，即优先级最高的任务。**
>
> 如果你 **不使用** 优化选择算法（即 `configUSE_PORT_OPTIMISED_TASK_SELECTION = 0`）， 那么 FreeRTOS 会用一种更通用的方式来选择就绪任务（比如线性查找）。这种方式没有 32 的限制，可以设置更大的优先级数。

**优先级数字越低表示任务的优先级越低**， 0 的优先级最低， configMAX_PRIORITIES-1 的优先级最高。空闲任务的优先级最低，为 0。  

FreeRTOS 调度器确保**处于就绪态的且具有最高优先级的任务才会运行**。

当宏 `configUSE_TIME_SLICING`定义为 1 的时 ，表示相同优先级的多个任务之间**使用时间片轮转** 的调度方式，此时处于就绪态的优先级相同的任务就会使用时间片轮转调度器获取运行时间。

# 5 任务实现

在使用 FreeRTOS 的过程中，我们要使用函数 `xTaskCreate`或 `xTaskCreateStatic`来创建任务，  

```c
xTaskCreate(TaskFunction_t pxTaskCode,
            const char * const pcName,
            const configSTACK_DEPTH_TYPE usStackDepth,
            void * const pvParameters,
            UBaseType_t uxPriority,
            TaskHandle_t * const pxCreatedTask)
```

其中，任务的具体实现是写在一个函数里的，并将该函数传递给第一个参数`pxTaskCode`。

任务函数的基本逻辑为：

```c
void vATaskFunction(void *pvParameters)
{
	while(1)
    {
        ...
    }
    vTaskDelete(NULL)
}
```

**需要注意几个要点**：

- 任务函数的返回类型一定要为 void 类型 ， 而且任务的参数也是 void 指针类型的

- 任务的具体执行过程是一个大循环  

- 任务函数一般不允许跳出循环，如果一定要跳出循环的话在跳出循环以后一定要调用函数 `vTaskDelete(NULL)`删除此任务！  

- 不能从任务函数中返回或者退出，从任务函数中返回或退出的话就会调用`configASSERT()`，前提是你定义了`configASSERT()`。如果一定要从任务函数中退出的话那一定要调用函数`vTaskDelete(NULL)`来删除此任务。如果任务函数返回，且未删除自身，调度器会尝试继续执行该任务，但由于任务函数已终止，会导致未定义行为

  > 当 FreeRTOS 内核检测到非法操作（如：任务函数意外返回但未调用 `vTaskDelete`）触发 `configASSERT`，可以在 FreeRTOS 配置文件 `FreeRTOSConfig.h` 中定义该宏。
  >
  > ```c
  > #define configASSERT(x)  if (!(x)) vAssertCalled(__FILE__, __LINE__)
  > 
  > void vAssertCalled(const char *file, int line) {
  >   printf("Assert failed: %s, line %d\n", file, line);
  >   while(1);
  > }
  > ```

# 6 任务控制块

FreeRTOS 的每个任务都有一些属性需要存储， FreeRTOS 把这些属性集合到一起用一个结构体来表示， 这个结构体叫做任务控制块： TCB_t。在使用函数 xTaskCreate()创建任务的时候就会自动的给每个任务分配一个任务控制块。此结构体在文件 tasks.c 中有定义，如下：  

```c
typedef struct tskTaskControlBlock
{
    volatile StackType_t *pxTopOfStack; // 任务堆栈栈顶
#if (portUSING_MPU_WRAPPERS == 1)
    xMPU_SETTINGSxMPUSettings; // MPU 相关设置
#endif
    ListItem_t   xStateListItem;                      // 状态列表项
    ListItem_t   xEventListItem;                      // 事件列表项
    UBaseType_t  uxPriority;                          // 任务优先级
    StackType_t *pxStack;                             // 任务堆栈起始地址
    char         pcTaskName[configMAX_TASK_NAME_LEN]; // 任务名字
#if (portSTACK_GROWTH > 0)
    StackType_t *pxEndOfStack; // 任务堆栈栈底
#endif
#if (portCRITICAL_NESTING_IN_TCB == 1)
    UBaseType_t uxCriticalNesting; // 临界区嵌套深度
#endif
#if (configUSE_TRACE_FACILITY == 1) // trace 或到 debug 的时候用到
    UBaseType_t uxTCBNumber;
    UBaseType_t uxTaskNumber;
#endif
#if (configUSE_MUTEXES == 1)
    UBaseType_t uxBasePriority; // 任务基础优先级,优先级反转的时候用到
    UBaseType_t uxMutexesHeld;  // 任务获取到的互斥信号量个数
#endif
#if (configUSE_APPLICATION_TASK_TAG == 1)
    TaskHookFunction_t pxTaskTag;
#endif
#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0) // 与本地存储有关
    void
        *pvThreadLocalStoragePointers[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
#endif
#if (configGENERATE_RUN_TIME_STATS == 1)
    uint32_t ulRunTimeCounter; // 用来记录任务运行总时间
#endif
#if (configUSE_NEWLIB_REENTRANT == 1)
    struct _reent xNewLib_reent; // 定义一个 newlib 结构体变量
#endif
#if (configUSE_TASK_NOTIFICATIONS == 1) // 任务通知相关变量
    volatile uint32_t ulNotifiedValue;  // 任务通知值
    volatile uint8_t  ucNotifyState;    // 任务通知状态
#endif
#if (tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0)
    // 用来标记任务是动态创建的还是静态创建的，如果是静态创建的此变量就为 pdTURE，
    // 如果是动态创建的就为 pdFALSE
    uint8_t ucStaticallyAllocated;
#endif
#if (INCLUDE_xTaskAbortDelay == 1)
    uint8_t ucDelayAborted;
#endif
} tskTCB;
// 新版本的 FreeRTOS 任务控制块重命名为 TCB_t，但是本质上还是 tskTCB，主要是为了兼容
// 旧版本的应用。
typedef tskTCB TCB_t;
```

# 7 任务堆栈

FreeRTOS 之所以能正确的恢复一个任务的运行就是因为有任务堆栈在保驾护航，任务调度器在进行任务切换的时候会将当前任务的现场(CPU 寄存器值等)保存在此任务的任务堆栈中，等到此任务下次运行的时候就会先用堆栈中保存的值来恢复现场，恢复现场以后任务就会接着从上次中断的地方开始运行。  

创建任务的时候需要给任务指定堆栈，如果使用的函数 `xTaskCreate()`创建任务(动态方法)的话那么**任务堆栈就会由函数 `xTaskCreate()`自动创建**。如果使用函数 `xTaskCreateStatic()`创建任务(静态方法)的话就**需要程序员自行定义任务堆栈**，然后堆栈首地址作为函数的参数 `puxStackBuffer`传递给函数，如下：  

```c
TaskHandle_t xTaskCreateStatic( TaskFunction_t pxTaskCode,
                               const char * const pcName,
                               const uint32_t ulStackDepth,
                               void * const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t * const puxStackBuffer, // 任务堆栈，需要用户定义
                               StaticTask_t * const pxTaskBuffer ) PRIVILEGED_FUNCTION;
```

**堆栈大小**

任务堆栈中存储的数据类型为 `StackType_t`， `StackType_t` 本质上是 `uint32_t`，在 `portmacro.h` 中有定义。

即：`StackType_t `类型的变量为 **4 个字节**，那么任务的实际堆栈大小就应该是我们所定义的 4 倍。  