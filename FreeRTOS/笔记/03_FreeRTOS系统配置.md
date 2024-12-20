在实际使用 FreeRTOS 的时候我们时常需要根据自己需求来配置 FreeRTOS，而且不同架构的 MCU 在使用的时候配置也不同。FreeRTOS 的系统配置文件为 `FreeRTOSConfig.h`，在此配置文件中可以完成 FreeRTOS 的裁剪和配置，这是非常重要的一个文件。

系统配置通过 **在`FreeRTOSConfig.h`中使用`#define`宏定义实现** 。在官方的例程中，每个工程都有一个该文件。

# 1  ”INCLUDE_“开始的宏

- 作用：使能/失能FreeRTOS中相应的API函数，**用于配置OS中可选的API函数**。

例如：`INCLUDE_vTaskPrioritySet`设置为0时，表示不能使用函数`vTaskPrioritySet`。

其实就是条件编译，只有当该宏置为1时，才编译该函数：

```c
#if (INCLUDE_vTaskPrioritySet==1)
void vTaskPrioritySet()
{
    ...
}
#endif
```

这就是FreeRTOS用来**裁剪和配置系统的方式**，即**通过宏定义来确定源文件中的哪些部分需要被编译**。

| 宏名                                  | 描述                                                         |
| ------------------------------------- | ------------------------------------------------------------ |
| `INCLUDE_xSemaphoreGetMutexHolder`    | 用于获取持有互斥量的任务句柄。                               |
| `INCLUDE_xTaskAbortDelay`             | 用于中断任务的延迟。                                         |
| `INCLUDE_vTaskDelay`                  | 用于使任务延迟指定的时间。                                   |
| `INCLUDE_vTaskDelayUntil`             | 用于使任务按指定时间间隔延迟。                               |
| `INCLUDE_vTaskDelete`                 | 用于删除指定任务。                                           |
| `INCLUDE_xTaskGetCurrentTaskHandle`   | 用于获取当前任务的句柄。                                     |
| `INCLUDE_xTaskGetHandle`              | 用于获取任务句柄。                                           |
| `INCLUDE_xTaskGetIdleTaskHandle`      | 用于获取空闲任务的句柄。                                     |
| `INCLUDE_xTaskGetSchedulerState`      | 用于获取调度器的当前状态。                                   |
| `INCLUDE_uxTaskGetStackHighWaterMark` | 用于获取任务栈的最小剩余空间（高水位标记）。                 |
| `INCLUDE_uxTaskPriorityGet`           | 用于获取任务的优先级。                                       |
| `INCLUDE_vTaskPrioritySet`            | 用于设置任务的优先级。                                       |
| `INCLUDE_xTaskResumeFromISR`          | 用于从 ISR 中恢复一个被挂起的任务。                          |
| `INCLUDE_eTaskGetState`               | 用于获取任务的当前状态。                                     |
| `INCLUDE_vTaskSuspend`                | 用于挂起任务，恢复任务，检查任务是否挂起，以及从 ISR 恢复任务。 |
| `INCLUDE_xTimerPendFunctionCall`      | 用于挂起函数调用并在定时器回调中执行。                       |

# 2 ”config“开始的宏

和以“INCLUDE_”开始的宏一样，都是用来完成OS的剪裁和配置的。

| 宏名                                   | 描述                                                         |
| -------------------------------------- | ------------------------------------------------------------ |
| `configUSE_PREEMPTION`                 | 启用/禁用任务抢占，设置为 `1` 启用抢占调度，设置为 `0` 禁用抢占调度。 |
| `configUSE_IDLE_HOOK`                  | 启用/禁用空闲任务挂钩。设置为 `1` 启用空闲任务挂钩，设置为 `0` 禁用。 |
| `configUSE_TICK_HOOK`                  | 启用/禁用定时器滴答挂钩。设置为 `1` 启用，设置为 `0` 禁用。  |
| `configMAX_PRIORITIES`                 | 设置系统中最大优先级数目。                                   |
| `configMINIMAL_STACK_SIZE`             | 设置任务栈的最小大小（单位：字）。                           |
| `configTOTAL_HEAP_SIZE`                | 设置 FreeRTOS 堆内存的总大小（单位：字节）。                 |
| `configMAX_TASK_NAME_LEN`              | 设置任务名的最大长度。                                       |
| `configUSE_TIMERS`                     | 启用/禁用定时器功能。设置为 `1` 启用定时器，设置为 `0` 禁用。 |
| `configUSE_TRACE_FACILITY`             | 启用/禁用 Trace 功能（任务执行跟踪）。设置为 `1` 启用，设置为 `0` 禁用。 |
| `configUSE_MUTEXES`                    | 启用/禁用互斥量。设置为 `1` 启用，设置为 `0` 禁用。          |
| `configUSE_COUNTING_SEMAPHORES`        | 启用/禁用计数信号量。设置为 `1` 启用，设置为 `0` 禁用。      |
| `configUSE_QUEUE_SETS`                 | 启用/禁用队列集合功能。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configUSE_NEWLIB_REENTRANT`           | 启用/禁用 Newlib 的可重入支持。设置为 `1` 启用，设置为 `0` 禁用。 |
| `configUSE_TASK_NOTIFICATIONS`         | 启用/禁用任务通知功能。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configUSE_CO_ROUTINES`                | 启用/禁用协程功能。设置为 `1` 启用，设置为 `0` 禁用。        |
| `configUSE_QUEUE_REGISTRY`             | 启用/禁用队列注册功能。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configQUEUE_REGISTRY_SIZE`            | 设置队列注册表的大小（即可以注册的队列数）。                 |
| `configMAX_CO_ROUTINE_PRIORITIES`      | 设置协程的最大优先级数目。                                   |
| `configUSE_STATS_FORMATTING_FUNCTIONS` | 启用/禁用统计信息格式化功能。设置为 `1` 启用，设置为 `0` 禁用。 |
| `configTICK_RATE_HZ`                   | 设置时钟滴答率，通常为 1000（表示 1 毫秒滴答）。             |
| `configKERNEL_INTERRUPT_PRIORITY`      | 设置内核中断的优先级（对 ARM Cortex-M 系列微控制器尤为重要）。 |
| `configMAX_TASK_NAME_LEN`              | 设置任务名的最大长度。                                       |
| `configUSE_CO_ROUTINE_PRIORITIES`      | 启用/禁用协程优先级功能。设置为 `1` 启用，设置为 `0` 禁用。  |
| `configUSE_TIME_SLICING`               | 启用/禁用时间切片功能。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configSUPPORT_DYNAMIC_ALLOCATION`     | 启用/禁用动态内存分配。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configSUPPORT_STATIC_ALLOCATION`      | 启用/禁用静态内存分配。设置为 `1` 启用，设置为 `0` 禁用。    |
| `configASSERT`                         | 启用/禁用断言机制。设置为 `1` 启用，设置为 `0` 禁用。        |