# 1. 准备工作

要移植 FreeRTOS，肯定需要一个基础工程，基础工程越简单越好。

源码的移植主要是在`FreeRTOSv202212.01\FreeRTOS\Source`下：

![image-20241220121650073](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220121650073.png)

需要移植的内容包含：

- 所有`.c`文件
- `portable`文件夹中的`keil`，`MemMang`和`RVDS`
- `include`文件夹及其中所有内容

# 2. 开始移植

---

**添加文件**

- 新建STM32基础工程（略过）
- 将1中提到的三部分文件复制到工程根目录内，`portable`中只保留`keil`，`MemMang`和`RVDS`
  - 修改`include`文件夹为`FreeRTOS_Inc`
  - 修改`portable`文件夹为`FreeRTOS_Port`
  - 创建`FreeRTOS_Core`文件夹，并将复制来的`.c`文件移动进去

![image-20241220123704225](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220123704225.png)

---

**加入Keil工程**

将上面添加好的文件加入到Keil的工程里，包括两步：新建分组并添加文件，添加头文件路径

**新建分组并添加文件**

- 在Keil里创建`FreeRTOS_CORE`和`FreeRTOS_PORTABLE`分组

- 将`~/FreeRTOS_Core/`和`~/FreeRTOS_Port/`路径下的所有文件分别加入到分组里

  - `~/FreeRTOS_Core/`下所有`.c`文件

  - `~/FreeRTOS_Port/MemMang/`下的`heap_4.c`

    `heap_4.c` 是 `MemMang`文件夹中的，前面说了 `MemMang` 是跟内存管理相关的，里面有 5 个 `.c` 文件：`heap_1.c`、`heap_2.c`、`heap_3.c`、`heap_4.c` 和 `heap_5.c`。这 5 个 `.c` 文件是五种不同的内存管理方法，都可以用来作为FreeRTOS 的内存管理文件，只是它们的实现原理不同，各有利弊。

  - `~/FreeRTOS_Port/RVDS/ARM_CM3`中的 `port.c` 文件
  
    - 如果是F4xx系列开发板，就添加`~/FreeRTOS_Port/RVDS/ARM_CM4F`中的文件
  

![image-20241220125338554](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220125338554.png)

**添加头文件路径**

![image-20241220125555754](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220125555754.png)

---

**添加`FreeRTOSConfig.h`文件**

- 在官方例程文件夹`FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_STM32F103_Keil/`里找
  - 如果是F4xx系列开发板，就在`FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/`

- 复制到`~/FreeRTOS_Inc/`文件夹里

---

**编译测试**

**问题一**

首次编译，会报错提示`SystemCoreClock`没有定义：

![image-20250316134344307](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/13-43-44-ce5471cf835de08dd5f42b9405c54cdb-image-20250316134344307-37a03e.png)

这是因为在`FreeRTOSConfig.h`文件里，`SystemCoreClock`被用于宏定义

```c
/* Ensure stdint is only used by the compiler, and not the assembler. */
#ifdef __ICCARM__
	#include <stdint.h>
	extern uint32_t SystemCoreClock;
#endif

...
#define configCPU_CLOCK_HZ				( SystemCoreClock )
...
```

而`SystemCoreClock`是在哪定义的呢？是在`system_stm32f4xx.h`文件中被定义为：

```c
#if defined(STM32F40_41xxx)
  uint32_t SystemCoreClock = 168000000;
#endif /* STM32F40_41xxx */
```

所以在使用该变量时要把它引用进来，但由于上面使用`#ifdef __ICCARM__`预编译条件判断是否要引入`SystemCoreClock`，而`__ICCARM__`又没有被定义，所以下面这段代码不起作用：

```c
#ifdef __ICCARM__
	#include <stdint.h>
	extern uint32_t SystemCoreClock;
#endif
```

因此，要将预编译条件改成：

```c
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
	#include <stdint.h>
	extern uint32_t SystemCoreClock;
#endif
```

- **`__ICCARM__`**：表示使用 **IAR 编译器**。
- **`__CC_ARM`**：表示使用 **ARM Compiler 5/6（Keil）**。
- **`__GNUC__`**：表示使用 **GCC 编译器**（如 STM32CubeIDE、Makefile 等）。

这段代码的目的是根据不同的编译器选择对应的 FreeRTOS 移植层实现。

**问题二**

上面问题解决后，还会遇到如下问题：

![image-20250316135551402](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/13-55-51-9e6b02b6cbf3e2e7d886449283867900-image-20250316135551402-994d23.png)

这是因为`SVC_Handler,PendSV_Handler,SysTick_Handler`被重复定义了。

在`FreeRTOSConfig.h`文件里，定义了如下三个宏：

```c
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
```

又发现`vPortSVCHandler,xPortPendSVHandler,xPortSysTickHandler`在`port.c`中被定义，等于说在这个文件里`SVC_Handler,PendSV_Handler,SysTick_Handler`被定义了一遍，在`stm32f4xx_it.h`中同样被定义了一遍，重复定义报错。因此只需要注释掉一处的定义即可，由于`stm32f4xx_it.h`中的函数体内是空的，就将该文件中的三个函数定义全部注释掉。

**问题三**

上面问题解决后，编译又会遇到下面的问题：

![image-20250316140336188](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/14-03-36-c7ddaad3513ffb50e717850cdb03296b-image-20250316140336188-aad0ed.png)

全局搜索发现，四个`hook`函数在`task.c`里被引入，但并没有被定义，因为这四个函数需要用户自己在源文件中实现的

```c
#if ( configUSE_IDLE_HOOK == 1 )
{
    extern void vApplicationIdleHook( void );
    vApplicationIdleHook();
}
#endif /* configUSE_IDLE_HOOK */
```

如果不想执行其中的代码，就将预编译条件的值`configUSE_IDLE_HOOK`改为0，需要在`FreeRTOSConfig.h`里修改：

```c
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				0
#define configUSE_MALLOC_FAILED_HOOK	0
#define configCHECK_FOR_STACK_OVERFLOW	0
```

至此，再编译就能顺利通过了。

# 3. 移植测试

本实验设计四个任务： start_task()、 led0_task ()、 led1_task ()和 float_task()，这四个任务的任务功能如下：

- start_task()：用来创建其他三个任务。
- led0_task ()：控制 LED0的闪烁，提示系统正在运行。
- led1_task ()：控制 LED1的闪烁。
- float_task()：简单的浮点测试任务，用于测试 STM32F4的 FPU是否工作正常。

预计实验现象：LED0和 LED1均匀闪烁，LED0闪烁的频率快，LED1闪烁的频率慢。打开串口调试助手，串口调试助手接收到开发板发送过来的数据，如图：

![image-20250316152459584](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/15-24-59-10b1f02d1115334a3ad33cd64c0dd347-image-20250316152459584-b878e1.png)

在main函数中编写以下函数：

```c
#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define LED0_TASK_PRIO 2             // 任务优先级
#define LED0_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t LED0Task_Handler;       // 任务句柄
void         led0_task(void *p_arg); // 任务函数

#define LED1_TASK_PRIO 3             // 任务优先级
#define LED1_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t LED1Task_Handler;       // 任务句柄
void         led1_task(void *p_arg); // 任务函数

#define FLOAT_TASK_PRIO 4             // 任务优先级
#define FLOAT_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t FloatTask_Handler;       // 任务句柄
void         float_task(void *p_arg); // 任务函数

led_conf_t led0 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_5,
    GPIOE,
    Bit_RESET,
    Bit_SET
};

led_conf_t led1 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_6,
    GPIOE,
    Bit_RESET,
    Bit_SET
};

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    delay_init();
    usart_init(115200);

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
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)led0_task,
        (const char *)"led0_task",
        (uint16_t)LED0_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LED0_TASK_PRIO,
        (TaskHandle_t *)&LED0Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)led1_task,
        (const char *)"led1_task",
        (uint16_t)LED1_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LED1_TASK_PRIO,
        (TaskHandle_t *)&LED1Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)float_task,
        (const char *)"float_task",
        (uint16_t)FLOAT_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)FLOAT_TASK_PRIO,
        (TaskHandle_t *)&FloatTask_Handler
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void led0_task(void *p_arg)
{
    while (1)
    {
        led_on(&led0);
        vTaskDelay(500);
        led_off(&led0);
        vTaskDelay(500);
    }
}

void led1_task(void *p_arg)
{
    while (1)
    {
        led_on(&led1);
        vTaskDelay(1000);
        led_off(&led1);
        vTaskDelay(1000);
    }
}

void float_task(void *p_arg)
{
    static float temp = 0;
    while (1)
    {
        temp += 0.01f;
        char str[64];
        sprintf(str, "temp = %.2f\r\n", temp);
        usart_send_str(str);
        vTaskDelay(1000);
    }
}

```

