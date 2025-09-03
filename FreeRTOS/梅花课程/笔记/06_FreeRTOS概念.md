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

---

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

---

**xQueueSendToFront()**

这个函数与 xQueueSend() 类似，但它将项目发送到队列的**头部**（实现后进先出 LIFO），通常用于发送紧急或高优先级消息。

**函数原型、参数和返回值** 与 xQueueSend() 完全相同。

### 2.4.2 从中断中发送

**警告：绝对不能在ISR中调用标准的 xQueueSend() 或 xQueueSendToFront() 函数！** 因为这些函数可能会导致任务上下文切换，而在ISR中这样做是禁止的。必须使用它们对应的 `...FromISR()` 版本。

> 猜测：因为中断服务函数在遇到阻塞时不应该等待，应该尽快退出，所以处理逻辑可能不同

ISR版本的函数有以下特点：

- **非阻塞**：ISR不能被阻塞。如果队列已满，函数会立即返回失败。
- **上下文切换处理**：它们有一个额外的参数，用于处理可能需要的上下文切换。

---

**xQueueSendFromISR()**

```C
BaseType_t xQueueSendFromISR(
    QueueHandle_t xQueue,
    const void *pvItemToQueue,
    BaseType_t *pxHigherPriorityTaskWoken
);
```

**参数:** 

- **`xQueue`**: 目标队列的句柄。 
- **`pvItemToQueue`**: 一个指向要发送的数据的指针。
- **`pxHigherPriorityTaskWoken`**: 这是一个非常重要的参数。它是一个指向 `BaseType_t` 类型变量的指针。在调用此函数**之前**，必须将该变量初始化为 `pdFALSE`。**如果发送数据到队列导致一个正在等待该队列的任务被解除阻塞，并且这个任务的优先级高于当前正在运行的任务，那么 `xQueueSendFromISR()` 函数会将这个变量设置为 `pdTRUE`**。ISR代码需要在退出前检查此变量的值。如果为 `pdTRUE`，则必须手动请求一次上下文切换。

**返回值:** 

- **`pdPASS`**: 数据成功发送。
- **`errQUEUE_FULL`**: 队列已满，发送失败。

---

**xQueueSendToFrontFromISR()**

这是 `xQueueSendToFront()` 的ISR安全版本，用于将项目发送到队列的**头部**。

### 2.4.3 队列发送示例

**从任务中发送**

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

**从中断服务函数中发送**

```c
QueueHandle_t xRxQueue;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 假设 huart 接收到了一个字节 'received_char'
    extern uint8_t received_char; 

    BaseType_t xHigherPriorityTaskWoken;

    // 在调用API前必须初始化为 pdFALSE
    xHigherPriorityTaskWoken = pdFALSE;

    // 从ISR中向队列发送接收到的字符
    xQueueSendFromISR(xRxQueue, &received_char, &xHigherPriorityTaskWoken);

    // 如果 xHigherPriorityTaskWoken 被设置为 pdTRUE，说明一个更高优先级的任务
    // 已经准备就绪，我们应该在中断退出时立即进行上下文切换。
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

## 2.5 队列接收函数

### 2.5.1 从任务中接收

在任务中，你可以使用阻塞式API。如果队列为空，任务可以选择挂起（阻塞），等待数据到达。

---

**xQueueReceive()**

这是最常用、最标准的队列接收函数。它从队列的**头部**（Front）获取一个项目，遵循先进先出（FIFO）原则。

```c
BaseType_t xQueueReceive(
    QueueHandle_t xQueue,
    void *pvBuffer,
    TickType_t xTicksToWait
);
```

**参数:** 

- **`xQueue`**: 要从中接收数据的队列句柄。
- **`pvBuffer`**: 一个指向缓冲区的指针，用于存放从队列中接收到的数据。这个缓冲区的大小必须至少等于创建队列时定义的项目大小（`uxItemSize`）。
- **`xTicksToWait`**: 如果队列为空，任务将进入阻塞状态等待的最大时间（以系统节拍 Ticks 为单位）。
  - 设置为 `0`：如果队列为空，函数将立即返回，不会等待。
  - 设置为 `portMAX_DELAY`：如果队列为空，任务将无限期等待，直到有数据可接收。这是最常见的用法。
  - 设置为其他正值：任务将等待指定的节拍数。 

**返回值:**

- `pdPASS`**: 成功从队列中接收到一个项目，数据已被复制到 `pvBuffer` 中。**
- `errQUEUE_EMPTY`: 队列为空，并且在 `xTicksToWait` 指定的等待时间内，没有数据到达。 

### 2.5.2 从中断服务函数中接收

**警告：与发送函数一样，绝对不能在ISR中调用标准的 xQueueReceive() 函数！** 必须使用其对应的 ...FromISR() 版本。ISR版本的函数是非阻塞的。

---

**xQueueReceiveFromISR()**

这是 xQueueReceive() 的ISR安全版本。

```c
BaseType_t xQueueReceiveFromISR(
    QueueHandle_t xQueue,
    void *pvBuffer,
    BaseType_t *pxHigherPriorityTaskWoken
);
```

**参数:** 

- **`xQueue`**: 队列句柄。
- **`pvBuffer`**: 用于存放接收数据的缓冲区指针。
-  **`pxHigherPriorityTaskWoken`**: 与 `xQueueSendFromISR()` 中的同名参数作用类似。
  - 在调用此函数**之前**，必须将该指针指向的变量初始化为 `pdFALSE`。
  - 如果从队列接收数据（通常是为了清空队列以腾出空间），导致一个因队列满而阻塞等待发送的任务被唤醒，并且该任务的优先级高于当前运行的任务，那么此函数会将该变量设置为 `pdTRUE`。
  - ISR代码需要在退出前检查此变量的值。如果为 `pdTRUE`，则必须手动请求一次上下文切换。

**返回值:**

- `pdPASS`**: 成功接收数据。**
- `errQUEUE_EMPTY`: 队列为空，接收失败。

### 2.5.3 队列接收示例

```c
QueueHandle_t xCmdQueue;

void Some_ISR(void)
{
    char cReceivedCmd;
    BaseType_t xHigherPriorityTaskWoken;

    // 初始化
    xHigherPriorityTaskWoken = pdFALSE;

    // 尝试从队列接收一个命令。
    // 注意：这里没有等待时间，因为 ISR 不能阻塞。
    if (xQueueReceiveFromISR(xCmdQueue, &cReceivedCmd, &xHigherPriorityTaskWoken) == pdPASS)
    {
        // 成功从队列中获取了一个命令，并将其从队列中移除
        // 可以在ISR中快速处理这个命令...
    }

    // 如果需要，请求上下文切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

### 2.5.4 查看队列头部数据（不移除）

有时你可能想知道队列头部有什么数据，但又不想把它从队列中移除。这时可以使用 "Peek" 函数。

---

**xQueuePeek()**

在任务上下文中查看（窥视）队列头部的项目，但**不**将其从队列中删除。

- **函数原型、参数和返回值** 与 xQueueReceive() 完全相同。
- **核心区别**: 即使返回 pdPASS，该项目仍然保留在队列的头部，下次调用 xQueueReceive() 或 xQueuePeek() 仍然会获取到同样的项目。

---

**xQueuePeekFromISR()**

xQueuePeek() 的ISR安全版本。其函数原型和使用方式与 xQueueReceiveFromISR() 类似，但它不会从队列中移除项目。

# 3.信号量

## 3.1 信号量的概念

想象一个停车场，它有固定数量的停车位。

- **停车场** 就是 **信号量**。
- **停车位的总数** 就是信号量的 **计数值**。
- 一辆车想**进入**停车场，就必须先看看入口的显示屏上“可用车位”是否大于0。如果大于0，它就开进去，同时显示屏上的数字减1。这个过程叫做 **获取 (Take)** 信号量。
- 如果“可用车位”为0，车就必须在入口外排队**等待**，直到有车出来。这个等待过程就是 **任务阻塞**。
- 一辆车**离开**停车场，显示屏上的“可用车位”数字加1。这个过程叫做 **释放 (Give)** 信号量。这时，排队等待的车就可以开进去一辆。

通过这个“停车场”机制，我们有效地管理了有限的停车位资源。在RTOS中，信号量就是用来管理有限的系统资源的。

## 3.2 FreeRTOS中的信号量类型

FreeRTOS提供了多种类型的信号量，它们虽然都基于相同的底层机制，但用途和行为有明显区别。

1. **计数信号量 (Counting Semaphores)**
2. **二进制信号量 (Binary Semaphores)**
3. **互斥量 (Mutexes)**
4. **递归互斥量 (Recursive Mutexes)**

互斥量和递归互斥量在FreeRTOS中是基于信号量机制实现的特殊变体。

## 3.3 计数信号量 (Counting Semaphore)

这和我们的停车场比喻完全一样，是最通用的信号量类型。

- **用途**：**资源管理**。用于控制对一个或多个相同资源的访问。例如，你有3个可用的网络连接池，你可以创建一个最大计数值和初始计数值都为3的计数信号量。任何任务想使用网络连接，都必须先成功 "Take" 信号量。
- **工作方式**：
  - 在创建时，你需要指定一个**最大计数值**和一个**初始计数值**。
  - 任务调用 xSemaphoreTake() 来获取资源，成功则信号量计数值减1。如果计数值为0，任务会阻塞（等待）。
  - 任务调用 xSemaphoreGive() 来释放资源，信号量计数值加1。
- **关键API**:
  - `xSemaphoreCreateCounting( UBaseType_t uxMaxCount, UBaseType_t uxInitialCount )`: 创建一个计数信号量。
  - `xSemaphoreTake( SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait )`: 获取信号量。
  - `xSemaphoreGive( SemaphoreHandle_t xSemaphore )`: 释放信号量。

## 3.4 二进制信号量 (Binary Semaphore)

你可以把它看作是一个**最大计数值为1的计数信号量**，或者一个只能容纳一个项目的队列。它只有两种状态：满（可用）或空（不可用）。

- **用途**：**同步**。它不用于管理资源，而是用于发信号，通常用于一个任务或一个中断通知另一个任务“某个事件已经发生，你可以开始工作了”。
- **工作方式**：
  - 信号量通常被创建时是“空”的。
  - 等待事件的任务调用 xSemaphoreTake()，由于信号量是空的，任务会立即阻塞。
  - 当事件发生时（例如，在另一个任务或ISR中），调用 xSemaphoreGive()。
  - Give 操作使信号量变为“满”，这会唤醒正在阻塞等待的任务，该任务随后成功 Take 信号量（使其再次变为空），然后继续执行。
- **关键API**:
  - `xSemaphoreCreateBinary()`: 创建一个二进制信号量。
  - `xSemaphoreTake()` 和 `xSemaphoreGive()`: 与计数信号量使用相同的获取和释放函数。
  - `xSemaphoreGiveFromISR()`: **非常重要**，用于在中断服务程序中安全地释放信号量，以唤醒一个任务。

**示例：** 一个ADC转换完成中断，需要通知一个数据处理任务。

1. 数据处理任务在其循环开始处调用 xSemaphoreTake() 并阻塞。
2. 当ADC转换完成，触发中断，ISR被执行。
3. ISR中调用 xSemaphoreGiveFromISR()。
4. 数据处理任务被唤醒，从 xSemaphoreTake() 返回，开始处理ADC数据。

## 3.5 互斥量 (Mutex - Mutual Exclusion)

互斥量是用于**保护共享资源**的特殊二进制信号量。共享资源是指可能被多个任务同时访问和修改的全局变量、外设等。如果不加保护，就会导致数据损坏（即**竞态条件 Race Condition**）。

- **用途**：**互斥访问**。确保在任何时刻，只有一个任务可以访问被保护的资源。
- **与二进制信号量的关键区别**：
  1. **所有权 (Ownership)**：<u>一个任务获取了Mutex，那么**只有这个任务**才能释放它。你不能在一个任务中获取Mutex，然后在另一个任务中释放它。而二进制信号量没有这个限制。</u>**关键区别！！！**
  2. **优先级继承 (Priority Inheritance)**：这是Mutex最重要的特性。它可以有效解决“优先级反转”问题。
     - **优先级反转问题**：一个低优先级任务L占有了一个Mutex。此时一个高优先级任务H也需要这个Mutex，于是H被阻塞。如果这时来了一个中等优先级任务M（它不需要这个Mutex），M会抢占L的CPU时间，导致L迟迟无法运行到释放Mutex的地方，从而使高优先级的H也一直被阻塞。结果就是，中优先级的任务反而先于高优先级的任务运行。
     - **优先级继承解决方案**：当高优先级的H因为等待Mutex而阻塞时，FreeRTOS会自动将持有该Mutex的低优先级任务L的优先级**临时提升**到与H相同。这样，中等优先级的M就无法抢占L，L能尽快运行并释放Mutex。一旦L释放了Mutex，它的优先级会恢复到原来的水平，H则立即获得Mutex并开始运行。
- **关键API**:
  - `xSemaphoreCreateMutex()`: 创建一个互斥量。
  - `xSemaphoreTake() `和 `xSemaphoreGive()`: 使用相同的API。

## 3.6 递归互斥量 (Recursive Mutex)

这是Mutex的一个变体。它允许**同一个任务**多次获取同一个递归互斥量。

- **用途**：当一个拥有Mutex的任务需要调用另一个函数，而这个函数也需要获取**同一个**Mutex时使用。如果使用标准Mutex，第二次获取会因为任务自己已经持有了该锁而导致死锁。
- **工作方式**：
  - Mutex内部会维护一个“获取计数”。任务每成功 Take 一次，计数加1。
  - 任务必须 Give 同样多次，直到计数变回0，这个Mutex才会被真正释放，供其他任务使用。
- **关键API**:
  - `xSemaphoreCreateRecursiveMutex()`: 创建递归互斥量。
  - `xSemaphoreTakeRecursive()`: 获取递归互斥量。
  - `xSemaphoreGiveRecursive()`: 释放递归互斥量。

# 4.优先级反转

优先级反转指的是这样一种**不期望的情况**：一个**高优先级**的任务，因为需要等待一个被**低优先级**任务持有的资源，而被迫长时间阻塞。与此同时，一个或多个**中等优先级**的任务却能够运行，从而导致系统的实际执行顺序与设计者期望的优先级顺序完全颠倒。

简单来说：**高优先级的任务反而要等待中等优先级的任务执行完毕，才能最终得到执行。**

解决方案：**优先级继承 (Priority Inheritance)**

# 5.优先级继承

这是解决优先级反转最常用的方法。FreeRTOS的**互斥量（Mutex）** 内置了此机制。

**工作原理：**
当一个高优先级任务因等待一个由低优先级任务持有的互斥量而阻塞时，**系统会自动、临时地将这个低优先级任务的优先级提升到与那个高优先级任务相同的水平。**

**当你的目的是为了保护共享资源时，请使用互斥量 (Mutex)，而不是二进制信号量。**

- xSemaphoreCreateMutex(): 创建的互斥量**内置了优先级继承机制**。
- xSemaphoreCreateBinary(): 创建的二进制信号量**没有**优先级继承机制，如果用它来做资源锁，就可能发生优先级反转。

> **互斥信号量内置优先级继承**
>
> #### 1. 当调用 xSemaphoreTake(myMutex, ...) 时：
>
> - **内核首先检查**：myMutex 是否可用？
>   - **如果可用**：内核将 myMutex 的所有者记录为当前任务，然后函数直接返回 pdPASS。一切正常。
>   - **如果不可用**：myMutex 正被另一个任务（我们称之为Task-L）持有。
>     - **步骤1**：内核将当前任务（我们称之为Task-H）放入 myMutex 的等待列表中，并阻塞Task-H。
>     - **步骤2（“内置机制”的核心）**：内核会比较等待者Task-H的优先级和持有者Task-L的优先级。
>     - **步骤3**：如果发现 Prio(Task-H) > Prio(Task-L)，内核会**立即、自动地**将Task-L的当前优先级提升到和Task-H一样。
>     - 这个优先级提升操作是内核调度器的一部分，对你来说是完全透明的。
>
> #### 2. 当持有者Task-L调用 xSemaphoreGive(myMutex) 时：
>
> - **内核首先检查**：当前任务（Task-L）是否是myMutex的合法所有者？（这是互斥量“所有权”的体现）。
>   - **如果是**：
>     - **步骤1**：内核会检查Task-L的优先级是否被临时提升过。
>     - **步骤2（“内置机制”的核心）**：如果优先级被提升过，内核会**立即、自动地**将Task-L的优先级恢复到它原来的基础优先级。这个过程也叫“disinheritance”（解除继承）。
>     - **步骤3**：内核查看 myMutex 的等待列表，唤醒其中优先级最高的等待任务（也就是Task-H）。
>     - **步骤4**：myMutex 的所有权现在转移给了Task-H。
>     - **步骤5**：由于一个更高优先级的任务Task-H现在进入了就绪态，调度器可能会立即进行一次上下文切换，让Task-H运行。

# 6.队列实验

FreeRTOS 队列通信实验：生产者-消费者模型

**硬件与软件需求**

- **硬件**：任何支持FreeRTOS的开发板（例如：ESP32, STM32, Raspberry Pi Pico等）。
- **软件**：配置好FreeRTOS的开发环境（例如：PlatformIO, STM32CubeIDE, Arduino IDE with FreeRTOS Kernel等）。
- **外设**：一个可用的串口，用于连接到电脑的串口监视器以查看输出。

**实验设计**

1. **定义数据结构**：我们将定义一个结构体 `SensorData_t`来模拟传感器数据，其中包含传感器ID、一个数值和发送时的时间戳。
2. **创建全局队列句柄**：定义一个全局的 `QueueHandle_t` 变量，用于在两个任务之间共享。
3. **生产者任务 (vSenderTask)**：
   - 以固定的时间间隔（例如每2秒）模拟一次传感器读数。
   - 将生成的传感器数据填充到一个 SensorData_t 结构体变量中。
   - 通过 xQueueSend() 将这个结构体发送到队列中。
4. **消费者任务 (vReceiverTask)**：
   - 在一个无限循环中，调用 `xQueueReceive()` 并设置 portMAX_DELAY 来等待队列中的数据。
   - 当队列为空时，此任务将自动进入**阻塞**状态，不消耗任何CPU。
   - 一旦接收到数据，任务将解除阻塞，并将接收到的结构体内容通过串口打印出来。

