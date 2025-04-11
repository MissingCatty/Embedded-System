# 1 概念

## 1.1 列表

- 本质：**按照 `xItemValue` 从小到大排序的**、**循环双向链表（circular doubly linked list）**

与列表相关的全部东西都在文件 `list.c` 和 `list.h` 中。在 `list.h` 中定义了一个叫 `List_t` 的结构体，如下：

```c
typedef struct xLIST
{
    listFIRST_LIST_INTEGRITY_CHECK_VALUE
    volatile UBaseType_t uxNumberOfItems;
    ListItem_t * configLIST_VOLATILE pxIndex;
    MiniListItem_t xListEnd;
    listSECOND_LIST_INTEGRITY_CHECK_VALUE
} List_t;
```

- `listFIRST_LIST_INTEGRITY_CHECK_VALUE`和`listSECOND_LIST_INTEGRITY_CHECK_VALUE`：用于**检查列表完整性**，需要将宏 `configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES`设置为 1，设置完成后，这个两个宏被定义为：

  ```c
  #define listFIRST_LIST_INTEGRITY_CHECK_VALUE          TickType_t xListIntegrityValue1;
  #define listSECOND_LIST_INTEGRITY_CHECK_VALUE         TickType_t xListIntegrityValue2;
  ```

  在初始化列表的时候会往这两个变量中写入一个特殊的值。

  在 FreeRTOS 操作链表时检查：

  ```c
  configASSERT( pxItem->ucListItemIntegrityValue1 == 特殊值 );
  configASSERT( pxItem->ucListItemIntegrityValue2 == 特殊值 );
  ```

- `uxNumberOfItems`：用来**记录列表中列表项的数量**。
- `pxIndex`：用来**记录当前列表项索引号**，用于遍历列表。  

- `xListEnd`：列表中最后一个列表项（**链表的尾哨兵节点**），用来表示列表结束，此变量类型为 `MiniListItem_t`，这是一个迷你列表项，关于列表项稍后讲解。

## 1.2 列表项

列表项就是存放在列表中的项目， FreeRTOS 提供了两种列表项： 列表项和迷你列表项。这两个都在文件 `list.h`中有定义，先来看一下列表项，定义如下：

```c
struct xLIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
    void * pvOwner;
    struct xLIST * configLIST_VOLATILE pxContainer;
    listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE
};
typedef struct xLIST_ITEM ListItem_t; 
```

- `xItemValue`：列表项值 ，用于表示列表项的**值、权重或排序依据**。
- `pxNext`：指向下一个列表项。  
- `pxPrevious`：指向前一个列表项。
- `pvOwner`：记录此**列表项归谁拥有**，通常是任务控制块（该列表项代表的哪个任务）。
- `pvContainer`：记录此列表项归哪个列表。

> 在前面讲解任务控制块 `TCB_t`的时候说了在 `TCB_t`中有两个变量 `xStateListItem`和 `xEventListItem`，这两个变量的类型就是 `ListItem_t`，也就是说这两个成员变量都是列表项。
>
> 以 `xStateListItem`为例，当创建一个任务以后 `xStateListItem`的 `pvOwner` 变量就指向这个任务的任务控制块，表示 `xSateListItem`属于此任务。当任务就绪态以后 `xStateListItem`的变量 `pvContainer`就指向就绪列表，表明此列表项在就绪列表中。
>
> 为什么要在`TCB_t`中放一个列表项的数据类型？因为只有在创建TCB时才能让列表项知道该列表项代表哪个任务。

## 1.3 迷你列表项

在文件 list.h 中有定义，如下：

```c
struct xMINI_LIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;
```

迷你列表项只是比列表项少了几个成员变量，那为什么要弄个迷你列表项出来呢？

因为有些情况下我们**不需要列表项这么全的功能**，可能只需要其中的某几个成员变量，如果此时用列表项的话会造成内存浪费！  

# 2 列表和列表项操作

## 2.1 列表初始化

```c
void vListInitialise( List_t * const pxList )
{
    // 开始时列表为空，所以将索引指针指向xListEnd（最后一个列表项，无实际意义，指针指向该列表项代表遍历结束或者列表中无元素）
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );

    // 将List_t中的迷你列表的xListItemIntegrityValue1设置为pdINTEGRITY_CHECK_VALUE
    listSET_FIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE( &( pxList->xListEnd ) );

    // xListEnd 的列表项值初始化为 portMAX_DELAY， portMAX_DELAY 是个宏，在文件 portmacro.h 中有定义。
    // 根据所使用的 MCU 的不同， portMAX_DELAY 值也不相同，可以为 0xffff 或者 0xffffffffUL，本教程中为 0xffffffffUL。
    pxList->xListEnd.xItemValue = portMAX_DELAY;

    // 环形链表，最后一个列表项的下一个列表项为自己，表示列表为空
    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
    // 前一个列表项也为自己
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );

    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;

    // 设置完整性检查的值
    listSET_LIST_INTEGRITY_CHECK_1_VALUE( pxList );
    listSET_LIST_INTEGRITY_CHECK_2_VALUE( pxList );
}
```

## 2.2 列表项的初始化

列表项初始化由函数 `vListInitialiseItem()`，

```c
void vListInitialiseItem( ListItem_t * const pxItem )
{
    // 初始时列表项不属于任何列表
    pxItem->pxContainer = NULL;

    /* Write known values into the list item if
     * configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    listSET_FIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE( pxItem );
    listSET_SECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE( pxItem );
}
```

列表项的成员变量比列表要多，怎么初始化函数就这么短？其他的成员变量什么时候初始化呢？这是因为列表项要根据实际使用情况来初始化

## 2.3 列表项插入

列表项的插入操作通过函数 `vListInsert()`来完成，列表是一个按照，函数原型如下：

```c
void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem )
{
    // 指向列表中列表项的迭代指针
    ListItem_t * pxIterator;
    // 获取要插入的列表项值，要根据这个值来确定列表项要插入的位置。
    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;

    // 检测列表和列表项的完整性
    listTEST_LIST_INTEGRITY( pxList );
    listTEST_LIST_ITEM_INTEGRITY( pxNewListItem );
	
    // 要插入列表项，第一步就是要获取该列表项要插入到什么位置
    // 如果要插入的列表项的值等于 portMAX_DELAY, 也就是说列表项值为最大值，这种情况最好办了，要插入的位置就是列表最末尾了。
    if( xValueOfInsertion == portMAX_DELAY )
    {
        // 如果要插入的列表项的值与xListEnd的值相等，就把其插入到xListEnd之前
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        // 这个 for 循环就是找位置的过程，当找到合适列表项的位置的时候就会跳出。
        // 从尾开始往后找（循环链表），找到第一个itemvalue大于待插入节点的位置
        for( pxIterator = ( ListItem_t * ) &( pxList->xListEnd ); pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext )
        {
            // 什么都不做，空循环
        }
    }

    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;

    // 设置该列表项属于当前列表
    pxNewListItem->pxContainer = pxList;

    ( pxList->uxNumberOfItems )++;
}
```

## 2.4 列表项末尾插入

列表末尾插入列表项的操作通过函数 `vListInsertEnd()`来完成：

```c
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
{
    ListItem_t * const pxIndex = pxList->pxIndex;

    listTEST_LIST_INTEGRITY( pxList );
    listTEST_LIST_ITEM_INTEGRITY( pxNewListItem );

    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;

    mtCOVERAGE_TEST_DELAY();

    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;

    pxNewListItem->pxContainer = pxList;

    ( pxList->uxNumberOfItems )++;
}
```

**注意：这里的末尾并不是指`xListEnd`而是当前游标`pxList->pxIndex`的前一个位置。**

## 2.5 列表项的删除

- 返回：删除列表项以后的**列表剩余列表项数目**  

```c
UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )
{
	// 获取要删除的列表项所属于的列表
    List_t * const pxList = pxItemToRemove->pxContainer;

    // 更改指针
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

    /* Only used during decision coverage testing. */
    mtCOVERAGE_TEST_DELAY();

    // 如果 pxList->pxIndex 恰好指向了被删除的那个节点（pxItemToRemove），那么就必须把它移动到前一个节点，以防遍历时访问到已经被删除的内存。
    if( pxList->pxIndex == pxItemToRemove )
    {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    pxItemToRemove->pxContainer = NULL;
    ( pxList->uxNumberOfItems )--;

    return pxList->uxNumberOfItems;
}
```

## 2.6 列表的遍历

FreeRTOS 提供了一个函数来完成列表的遍历，这个函数是 `listGET_OWNER_OF_NEXT_ENTRY()`，每调用一次这个函数列表的 `pxIndex`变量就会指向下一个列表项，并且返回这个列表项的 `pxOwner`变量值。 这个函数本质上是一个宏，这个宏在文件 `list.h` 中如下定义：

```c
#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList ) \
    { \
    	// 获取列表
        List_t * const pxConstList = ( pxList ); \
        // 将游标指向下一个列表项
        ( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext; \
        // 如果游标指向xListEnd，则忽略直接跳到xListEnd下一个
        if( ( void * ) ( pxConstList )->pxIndex == ( void * ) &( ( pxConstList )->xListEnd ) ) \
        { \
            ( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext; \
        } \
        // 返回游标指向的列表项对应的TCB
        ( pxTCB ) = ( pxConstList )->pxIndex->pvOwner; \
    }
```

# 3 列表项的插入和删除实验  

## 3.1 实验目的  

学习使用 FreeRTOS 列表和列表项相应的操作函数的使用

## 3.2 实验设计  

本实验设计 3 个任务： start_task、 task1_task 和 list_task，这三个任务的任务功能如下： 

- start_task：用来创建其他 2 个任务。 
- task1_task：应用任务 1，控制 LED0 闪烁，用来提示系统正在运行。 
- list_task: 列表和列表项操作任务，调用列表和列表项相关的 API 函数，并且通过串口输出相应的信息来观察这些 API 函数的运行过程。

实验需要用到 key3 按键，用于控制任务的运行。

## 3.3 实验程序

```c
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>
#include <stdio.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define TASK1_TASK_PRIO 2             // 任务优先级
#define TASK1_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t TASK1Task_Handler;       // 任务句柄
void         task1_task(void *p_arg); // 任务函数

#define LIST_TASK_PRIO 3             // 任务优先级
#define LIST_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t LISTTask_Handler;       // 任务句柄
void         list_task(void *p_arg); // 任务函数

extern led_conf_t led0;
extern led_conf_t led1;
extern key_conf_t key3;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    key_init(&key3);
    delay_init();
    usart_init(115200);
    LCD_Init();
    LCD_Display_Dir(1);

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
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)task1_task,
        (const char *)"task1_task",
        (uint16_t)TASK1_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)TASK1_TASK_PRIO,
        (TaskHandle_t *)&TASK1Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)list_task,
        (const char *)"list_task",
        (uint16_t)LIST_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LIST_TASK_PRIO,
        (TaskHandle_t *)&LISTTask_Handler
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void task1_task(void *p_arg)
{
    while (1)
    {
        led_on(&led0);
        vTaskDelay(500); // 延时500ms
        led_off(&led0);
        vTaskDelay(500); // 延时500ms
    }
}

List_t     TestList;  // 测试用列表
ListItem_t ListItem1; // 测试用列表项 1
ListItem_t ListItem2; // 测试用列表项 2
ListItem_t ListItem3; // 测试用列表项 3

void list_task(void *p_arg)
{
    while (1)
    {
        // 第一步：初始化列表和列表项
        vListInitialise(&TestList);
        vListInitialiseItem(&ListItem1);
        vListInitialiseItem(&ListItem2);
        vListInitialiseItem(&ListItem3);
        ListItem1.xItemValue = 40; // ListItem1 列表项值为 40
        ListItem2.xItemValue = 60; // ListItem2 列表项值为 60
        ListItem3.xItemValue = 50; // ListItem3 列表项值为 50

        // 第二步：打印列表和其他列表项的地址
        printf("/**********List and List Item Address***********/\r\n");
        printf("Item Address \r\n");
        printf("TestList %#x \r\n", (int)&TestList);
        printf("TestList->pxIndex %#x \r\n", (int)TestList.pxIndex);
        printf("TestList->xListEnd %#x \r\n", (int)(&TestList.xListEnd));
        printf("ListItem1 %#x \r\n", (int)&ListItem1);
        printf("ListItem2 %#x \r\n", (int)&ListItem2);
        printf("ListItem3 %#x \r\n", (int)&ListItem3);
        printf("/****************end*****************/\r\n");
        printf("Push KEY3 continue!\r\n\r\n\r\n");
        while (key_pushed(&key3) != 1) vTaskDelay(10); // 等待 KEY_UP 键按下

        // 第三步：向列表 TestList 添加列表项 ListItem1，并通过串口打印所有
        // 列表项中成员变量 pxNext 和 pxPrevious 的值，通过这两个值观察列表
        // 项在列表中的连接情况。
        vListInsert(&TestList, &ListItem1); // 插入列表项 ListItem1
        printf("/*********Add ListItem1**********/\r\n");
        printf("Item Address \r\n");
        printf("TestList->xListEnd->pxNext %#x \r\n", (int)(TestList.xListEnd.pxNext));
        printf("ListItem1->pxNext %#x \r\n", (int)(ListItem1.pxNext));
        printf("/**************************/\r\n");
        printf("TestList->xListEnd->pxPrevious %#x \r\n", (int)(TestList.xListEnd.pxPrevious));
        printf("ListItem1->pxPrevious %#x \r\n", (int)(ListItem1.pxPrevious));
        printf("/*****************end****************/\r\n");
        printf("Push KEY3 continue!\r\n\r\n\r\n");
        while (key_pushed(&key3) != 1) vTaskDelay(10); // 等待 KEY_UP 键按下

        // 第四步：向列表 TestList 添加列表项 ListItem2，并通过串口打印所有
        // 列表项中成员变量 pxNext 和 pxPrevious 的值，通过这两个值观察列表
        // 项在列表中的连接情况。
        vListInsert(&TestList, &ListItem2); // 插入列表项 ListItem2
        printf("/*********Add ListItem2**********/\r\n");
        printf("Item Address \r\n");
        printf("TestList->xListEnd->pxNext %#x \r\n", (int)(TestList.xListEnd.pxNext));
        printf("ListItem1->pxNext %#x \r\n", (int)(ListItem1.pxNext));
        printf("ListItem2->pxNext %#x \r\n", (int)(ListItem2.pxNext));
        printf("/*****************************/\r\n");
        printf("TestList->xListEnd->pxPrevious %#x \r\n", (int)(TestList.xListEnd.pxPrevious));
        printf("ListItem1->pxPrevious %#x \r\n", (int)(ListItem1.pxPrevious));
        printf("ListItem2->pxPrevious %#x \r\n", (int)(ListItem2.pxPrevious));
        printf("/****************end*****************/\r\n");
        printf("Push KEY3 continue!\r\n\r\n\r\n");
        while (key_pushed(&key3) != 1) vTaskDelay(10); // 等待 KEY_UP 键按下

        // 第五步：向列表 TestList 添加列表项 ListItem3，并通过串口打印所有
        // 列表项中成员变量 pxNext 和 pxPrevious 的值，通过这两个值观察列表
        // 项在列表中的连接情况。
        vListInsert(&TestList, &ListItem3); // 插入列表项 ListItem3
        printf("/*********Add ListItem3**********/\r\n");
        printf("Item Address \r\n");
        printf("TestList->xListEnd->pxNext %#x \r\n", (int)(TestList.xListEnd.pxNext));
        printf("ListItem1->pxNext %#x \r\n", (int)(ListItem1.pxNext));
        printf("ListItem3->pxNext %#x \r\n", (int)(ListItem3.pxNext));
        printf("ListItem2->pxNext %#x \r\n", (int)(ListItem2.pxNext));
        printf("/*****************************/\r\n");
        printf("TestList->xListEnd->pxPrevious %#x \r\n", (int)(TestList.xListEnd.pxPrevious));
        printf("ListItem1->pxPrevious %#x \r\n", (int)(ListItem1.pxPrevious));
        printf("ListItem3->pxPrevious %#x \r\n", (int)(ListItem3.pxPrevious));
        printf("ListItem2->pxPrevious %#x \r\n", (int)(ListItem2.pxPrevious));
        printf("/****************end*****************/\r\n");
        printf("Push KEY3 continue!\r\n\r\n\r\n");
        while (key_pushed(&key3) != 1) vTaskDelay(10); // 等待 KEY_UP 键按下

        // 第六步：删除 ListItem2，并通过串口打印所有列表项中成员变量 pxNext 和
        // pxPrevious 的值，通过这两个值观察列表项在列表中的连接情况。
        uxListRemove(&ListItem2); // 删除 ListItem2
        printf("/**********Delete ListItem2*********/\r\n");
        printf("Item Address \r\n");
        printf("TestList->xListEnd->pxNext %#x \r\n", (int)(TestList.xListEnd.pxNext));
        printf("ListItem1->pxNext %#x \r\n", (int)(ListItem1.pxNext));
        printf("ListItem3->pxNext %#x \r\n", (int)(ListItem3.pxNext));
        printf("/*****************************/\r\n");
        printf("TestList->xListEnd->pxPrevious %#x \r\n", (int)(TestList.xListEnd.pxPrevious));
        printf("ListItem1->pxPrevious %#x \r\n", (int)(ListItem1.pxPrevious));
        printf("ListItem3->pxPrevious %#x \r\n", (int)(ListItem3.pxPrevious));
        printf("/****************end*****************/\r\n");
        printf("Push KEY3 continue!\r\n\r\n\r\n");
        while (key_pushed(&key3) != 1) vTaskDelay(10); // 等待 KEY_UP 键按下

        // 第七步：删除 ListItem2， 并通过串口打印所有列表项中成员变量 pxNext 和
        // pxPrevious 的值，通过这两个值观察列表项在列表中的连接情况。
        TestList.pxIndex = TestList.pxIndex->pxNext; // pxIndex 向后移一项，
        // 这样 pxIndex 就会指向 ListItem1。
        vListInsertEnd(&TestList, &ListItem2); // 列表末尾添加列表项 ListItem2
        printf("/******InsertEnd ListItem2*******/\r\n");
        printf("Item Address \r\n");
        printf("TestList->pxIndex %#x \r\n", (int)TestList.pxIndex);
        printf("TestList->xListEnd->pxNext %#x \r\n", (int)(TestList.xListEnd.pxNext));
        printf("ListItem2->pxNext %#x \r\n", (int)(ListItem2.pxNext));
        printf("ListItem1->pxNext %#x \r\n", (int)(ListItem1.pxNext));
        printf("ListItem3->pxNext %#x \r\n", (int)(ListItem3.pxNext));
        printf("/*****************************/\r\n");
        printf("TestList->xListEnd->pxPrevious %#x \r\n", (int)(TestList.xListEnd.pxPrevious));
        printf("ListItem2->pxPrevious %#x \r\n", (int)(ListItem2.pxPrevious));
        printf("ListItem1->pxPrevious %#x \r\n", (int)(ListItem1.pxPrevious));
        printf("ListItem3->pxPrevious %#x \r\n", (int)(ListItem3.pxPrevious));
        printf("/****************end*****************/\r\n");

        while (1)
        {
            led_on(&led1);
            vTaskDelay(1000); // 延时 1s，也就是 1000 个时钟节拍
            led_off(&led1);
            vTaskDelay(1000); // 延时 1s，也就是 1000 个时钟节拍
        }
    }
}

```

