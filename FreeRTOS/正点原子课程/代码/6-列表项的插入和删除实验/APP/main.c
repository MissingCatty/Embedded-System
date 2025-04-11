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
