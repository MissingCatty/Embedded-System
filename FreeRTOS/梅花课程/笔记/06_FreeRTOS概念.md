# 1.移植步骤

1. 添加RTOS源码到Keil工程
2. 添加head_4.c到Keil工程
3. 添加port.c到Keil工程
4. 添加头文件路径
5. 添加FreeRTOSConfig.h
6. 修改FreeRTOSConfig.h配置文件，直到工程编译无错误

# 2.队列

队列是一种用于**任务间通信**的数据结构，遵循先进先出（**FIFO**）的原则，可以在：

- 任务和任务之间
- 任务和中断之间

传递消息，可以实现任务接收来自其他任务或中断的，**不固定长度**的消息。

这种机制实现了任务的解耦，一个任务不需要知道另一个任务的状态，只需要通过队列进行数据交换即可。

队列可以被多个任务读，也可以被多个任务写。但**需要防止同一时刻被多个任务访问**（也就是多线程或多进程对共享变量即队列的并发访问）。但好在**freertos的队列是被设计成线程安全**的，因为所有对队列的操作，都会创建临界区。

**核心机制：数据拷贝**

FreeRTOS队列的一个显著特点是，它们**通过数据拷贝的方式来传递信息，而非仅仅传递数据的引用（指针）**。当一个任务向队列发送数据时，操作系统会将数据本身完整地复制到队列的存储空间中。这种方式带来了诸多好处：

- 线程安全和数据独立性： **发送方任务可以立即重用发送数据的变量，而不必担心接收方任务是否已经处理完毕**，因为队列中存的是数据的副本。这也避免了复杂的指针管理和潜在的内存访问冲突问题。
- 中断服务程序（ISR）中的安全使用： 在中断服务程序中也可以安全地向队列发送数据，因为数据被复制后，中断程序可以快速退出，不会长时间占用CPU。
- 简化编程模型： 开发者无需关心数据的生命周期和所有权问题，降低了编程的复杂性。

> 注意，这里说的复制指的是值复制，并不是说不能在队列里存一个指针变量。如果往队列里放的是一个指针，那就是将该指针代表的地址值放到队列里。

**中断不阻塞**

当在中断中读写队列时，如果队列空时读/队列满时写，**不会进行阻塞**。因为中断要满足快进快出，当在中断中**写队列时发现队列满**了，就立即退出，**返回队列空错误**；当在中断中**读队列时发现队列为空**，就**返回队列满错误**。

## 2.1 创建队列

`xQueueCreate()`，这是创建队列最常用的宏。

```c
QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength, UBaseType_t uxItemSize );
```

**参数**

- `uxQueueLength`: 队列中能容纳的最大项目（item）数量。
- `uxItemSize`: 每个队列项目的数据大小（以字节为单位）。

**返回值**

- **成功**: 返回一个非NULL的 QueueHandle_t 类型的句柄，该句柄用于在其他API函数中引用该队列。[[1](https://www.google.com/url?sa=E&q=https%3A%2F%2Fvertexaisearch.cloud.google.com%2Fgrounding-api-redirect%2FAUZIYQEW4VlldPzvO7UPaw8jFMT2CeBX0DeGzlAGG4ZdAVCVrQLa4Q4X4lFFxNKsNDeSBlaNnN6cZC72bBCduTtSmrJ8ywjZHs_MCKhunzeO5pnEcHJsgfIpzNXBZnmBH7-sTy5rd8FcsliLGwPnVI1A_Knhs5ZQ6cXyINNwu3jf7Mgsh17kBuhmkhab_3mC6jZyMA0bIqaXKFRv)]
- **失败**: 如果无法分配创建队列所需的内存，则返回 NULL。

---

**队列的静态创建**

FreeRTOS还提供了 xQueueCreateStatic() 函数，用于在编译时静态分配队列内存，这**适用于不允许动态内存分配的应用程序**。

---

## 2.2 删除队列

`vQueueDelete()` 函数用于删除一个已经创建的队列，并释放为其分配的所有内存。

```c
void vQueueDelete(QueueHandle_t xQueue);
```

**参数**

- `xQueue`: 要删除的队列的句柄。此句柄是通过调用 `xQueueCreate()` 创建的

在FreeRTOS中，当一个任务尝试执行一个无法立即完成的操作时，它可以进入“阻塞”状态。在删除队列之前，应确保没有任务正在阻塞等待该队列上的数据发送或接收。尝试删除一个有任务阻塞的队列可能会导致未定义的行为。

> **什么叫有任务阻塞的队列？**
>
> - **因接收而阻塞**：一个任务调用 xQueueReceive() 尝试从一个空队列中读取数据。如果它指定了一个非零的等待时间（Block Time），任务就会被挂起（进入阻塞状态），并且不会消耗任何CPU时间，直到队列中有数据可读或等待超时。
> - **因发送而阻塞**：一个任务调用 xQueueSend() 尝试向一个满队列中写入数据。同样，如果它指定了等待时间，任务就会被挂起，直到队列中有空间可用或等待超时。

## 2.3 创建和删除队列示例

```c
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// 定义一个队列句柄
QueueHandle_t xMyQueue;

void vATask(void *pvParameters)
{
    // 创建一个队列，可以容纳10个unsigned int类型的数据
    xMyQueue = xQueueCreate(10, sizeof(unsigned int));

    if (xMyQueue == NULL)
    {
        // 队列创建失败，因为内存不足
    }
    else
    {
        // 队列创建成功，可以在这里使用 xQueueSend() 和 xQueueReceive() 等函数
        // ... 任务的其他代码 ...

        // 当不再需要队列时，将其删除
        vQueueDelete(xMyQueue);
    }

    // 任务必须有一个无限循环或者在完成时自我删除
    vTaskDelete(NULL);
}
```

## 2.4 队列发送函数

向队列发送数据有一系列函数，选择哪个函数取决于您是在**任务代码**中还是在**中断服务程序（ISR）**中执行该操作。

所有队列发送函数都遵循一个核心原则：**数据是通过复制的方式存入队列的，而不是通过引用**。

### 2.4.1 从任务中发送

当你在一个正常的任务中运行时，你应该**使用带有阻塞功能的API**。这意味着如果队列已满，任务可以选择等待一段时间，直到队列有可用空间。

**xQueueSend() 或 xQueueSendToBack()**

这是最常用的队列发送函数，它将一个项目发送到队列的**尾部**（实现先进先出 FIFO）。`xQueueSend()`是为了向后兼容而保留的宏，它等同于 `xQueueSendToBack()`。

```c
BaseType_t xQueueSend(
    QueueHandle_t xQueue,
    const void *pvItemToQueue,
    TickType_t xTicksToWait
);
```

**参数:**

- **xQueue**: 目标队列的句柄，由 `xQueueCreate()` 返回。
- **pvItemToQueue**: 一个指向要发送的数据的指针。函数会从这个地址复制 `uxItemSize`（在创建队列时定义的大小）字节的数据到队列中。
- **xTicksToWait**: 如果队列已满，任务将**进入阻塞状态等待的最大时间**（以系统节拍 Ticks 为单位）。[[5](https://www.google.com/url?sa=E&q=https%3A%2F%2Fvertexaisearch.cloud.google.com%2Fgrounding-api-redirect%2FAUZIYQF1MiXH6hSNAKuRC9Gku6Wz2l2X2LjGyLGfjKvrlpgLOlGvWyReXNjLgwhvPYKgnefQp46WeqWkF2-qfDDmwlXDUHgFaub_wpON6ZhPjuqZa5fd8ZQD1mvFHqN4zeOMSO1gY0-ZqSbaBp8rQB0PHFoc6sPkrHhyJqtz1w%3D%3D)]
  - 设置为 0：如果队列已满，函数将立即返回，不会等待。
  - 设置为 `portMAX_DELAY`：如果队列已满，任务将无限期等待，直到队列有可用空间。
  - 设置为其他正值：任务将等待指定的节拍数。

**返回值:**

- **pdPASS**: 数据成功发送到队列。
- **errQUEUE_FULL**: 队列已满，并且在 `xTicksToWait`指定的等待时间内，队列仍未出现可用空间。

### 2.4.2 从中断中发送



### 2.4.3 队列发送示例

```c
QueueHandle_t xMyQueue;

void vSenderTask(void *pvParameters)
{
    long lValueToSend = 100;

    // 假设 xMyQueue 已经在别处创建
    // xMyQueue = xQueueCreate(5, sizeof(long));

    while(1)
    {
        // 发送 lValueToSend 的值到队列。
        // 如果队列满了，最多等待 100 个系统节拍。
        if (xQueueSend(xMyQueue, &lValueToSend, (TickType_t)100) != pdPASS)
        {
            // 在100个节拍内发送失败
            // 可以在这里处理错误，例如打印日志
        }
        
        lValueToSend++;
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒发送一次
    }
}
```

## 2.5 队列接收函数

### 2.5.1 
