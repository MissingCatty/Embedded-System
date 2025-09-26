# 1.移植步骤

## 1.1 下载源码

进入lvgl官方github仓库：[lvgl/lvgl: Embedded graphics library to create beautiful UIs for any MCU, MPU and display type.](https://github.com/lvgl/lvgl)

去release中下载最新版的源码并解压。

将源码中的`./lv_conf_template.h`，`./lv_version.h`，`./lvgl.h`，和`./src`目录全部复制到Keil项目目录`Lib/lvgl`中

**注：所有源文件都要加到目录里，但可以挑选部分.c文件进入项目结构中**

![image-20250915161954640](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2025%2F09%2F6328866c1745150ee162df0d7eb641fc.png)

## 1.2 保留移植文件

1. 修改`lv_conf_template.h`为`lv_conf.h`
2. 打开`lv_conf.h`，修改宏为1以启用该配置文件
3. `lv_conf_internal.h`文件中`LV_CONF_INCLUDE_SIMPLE`，在keil项目中添加宏`LV_CONF_INCLUDE_SIMPLE`和`LV_LVGL_H_INCLUDE_SIMPLE`

**注意**：如果使用的是`ARM Compiler 6 (armclang)`，则支持`__has_include`宏的定义，如果使用的是`ARM Compiler 5 (armcc)`则不支持，在编译`lv_font_montserrat_14_aligned.c`文件时会找不到宏，从而触发找不到`lvgl/lvgl.h`错误。所以，需要手动在魔术棒中添加宏`LV_LVGL_H_INCLUDE_SIMPLE`定义。

## 1.3 在Keil中添加.c文件

`Lib/lvgl/src`中添加的源文件包含：

```
./src/*.c
./src/core/*.c
./src/display/*.c
./src/draw/*.c
./src/draw/convert/*.c
./src/draw/sw/*.c
./src/draw/sw/blend/*.c
./src/drivers/display/ili9341/*.c [这是对应的显示屏，按需更换驱动代码]
./src/drivers/display/lcd/*.c
./src/font/*.c
./src/indev/*.c
./src/layouts/*.c
./src/layouts/flex/*.c
./src/layouts/grid/*.c
./src/libs/bin_decoder/*.c
./src/misc/*.c
./src/misc/cache/*.c
./src/misc/cache/instance/*.c
./src/misc/cache/class/*.c
./src/osal/cache/lv_os.c + lv_freertos.c + lv_os_none.c
./src/stdlib/*.c
./src/stdlib/builtin/*.c
./src/stdlib/clib/*.c
./src/themes/*.c
./src/themes/default + mono + simple/*c
./src/tick/*.c
./src/others/observer/*.c
```

`Lib/lvgl/widgets`中添加的源文件包含：

```
./src/widgets/lv_button.c
./src/widgets/lv_arc.c
./src/widgets/lv_bar.c
./src/widgets/lv_buttonmatrix.c
./src/widgets/lv_checkbox.c
./src/widgets/lv_label.c
./src/widgets/lv_led.c
./src/widgets/lv_line.c
./src/widgets/lv_switch.c
./src/widgets/lv_textarea.c
```

**注：选择widgets引入某个控件的时候需要在`lv_conf.h`文件里打开对应的宏，同时不要忘了导入对应的.c文件**

如果还出现`symbol undefined error`，首先看看源文件有没有加进来，再看看是不是有宏定义没开放。

## 1.4 宏定义修改

主要在`lv_conf.h`配置文件里修改对应的宏定义，用于开启或关闭相应的功能。

将涉及到的宏修改如下：

```c
// 系统
#define LV_USE_OS LV_OS_FREERTOS

// 显示屏
#define LV_USE_ILI9341       1

// 组件
#define LV_USE_ANIMIMG    0 // 动画
#define LV_USE_CALENDAR   0 // 日历
#define LV_USE_CANVAS     0
#define LV_USE_CHART      0
#define LV_USE_DROPDOWN   0   /**< Requires: lv_label */
#define LV_USE_IMAGE      0   /**< Requires: lv_label */
#define LV_USE_IMAGEBUTTON     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LIST       0
#define LV_USE_LOTTIE     0  /**< Requires: lv_canvas, thorvg */
#define LV_USE_MENU       0
#define LV_USE_MSGBOX     0
#define LV_USE_ROLLER     0   /**< Requires: lv_label */
#define LV_USE_SCALE      0
#define LV_USE_SLIDER     0   /**< Requires: lv_bar */
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABLE      0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

// 主题
#define LV_USE_THEME_SIMPLE 0
#define LV_USE_THEME_MONO 0

// 布局
#define LV_USE_FLEX 0
#define LV_USE_GRID 0
```

# 2. 驱动编写

主要基于9341屏幕进行编写

lvgl官方9341屏幕使用方式：[ILI9341 LCD Controller driver（ILI9341液晶控制器驱动程序） — LVGL 文档](https://lvgl.100ask.net/9.2/integration/driver/display/ili9341.html)

通过CMake编译器定义或KConfig，在lv_conf.h中启用ILI9341驱动程序支持。

```
#define LV_USE_ILI9341  1
```

在APP项目路径下创建`gui.c`文件，其中您需要实现两个与平台相关的函数：

```c
#include "src/misc/lv_types.h"

/* Send short command to the LCD. This function shall wait until the transaction finishes. */
int32_t my_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
        
}

/* Send large array of pixel data to the LCD. If necessary, this function has to do the byte-swapping. This function can do the transfer in the background. */
int32_t my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
        
}
```

要创建基于ILI9341驱动的显示器，请使用以下函数：

```c
/**
 * Create an LCD display with ILI9341 driver
 * @param hor_res       horizontal resolution
 * @param ver_res       vertical resolution
 * @param flags         default configuration settings (mirror, RGB ordering, etc.)
 * @param send_cmd      platform-dependent function to send a command to the LCD controller (usually uses polling transfer)
 * @param send_color    platform-dependent function to send pixel data to the LCD controller (usually uses DMA transfer: must implement a 'ready' callback)
 * @return              pointer to the created display
 */
lv_display_t *lv_ili9341_create(uint32_t hor_res, uint32_t ver_res, lv_lcd_flag_t flags, lv_ili9341_send_cmd_cb_t send_cmd_cb, lv_ili9341_send_color_cb_t send_color_cb);
```

最后，lvgl关于ili9341屏幕的驱动如下：

```c
 #include "lvgl_ili9341.h"
 #include "FreeRTOS.h"
 #include "task.h"

 #define DISP_HOR_RES  240
 #define DISP_VER_RES  320
 // lvgl官方推荐的屏幕刷新的缓冲区最小大小为屏幕大小*颜色深度/10
 // 较小的缓冲区：(例如 /10 或 /20)更节省 RAM，但需要更频繁地向屏幕发送数据，可能会降低复杂动画的流畅度或导致画面撕裂。
 // 较大的缓冲区： (例如 /4 或 /2)使用更多 RAM，但由于每次发送的数据块更大、次数更少，通常渲染效果会更平滑。
 #define DISP_BUF_SIZE (DISP_HOR_RES * DISP_VER_RES * sizeof(lv_color_t) / 8)

 static lv_color_t    gui_buff[DISP_BUF_SIZE];
 static lv_display_t *ili9341_display;

 // 对硬件LCD的操作本质是对寄存器的写操作，所以首先需要获取寄存器基地址
 #define LCD_BASE ((uint32_t)(0x60000000 | 0x00007FFFE))
 #define LCD      ((LCD_TypeDef *)LCD_BASE)

 // 创建寄存器数据结构体，方便读写
 /*
   在ILI9341中，命令的本质就是控制器内部的寄存器索引
   当你写到 LCD_REG 时，实际上是在告诉 ILI9341： “我要访问寄存器 XX”。

 */
 typedef struct
 {
     uint16_t LCD_REG; // 指定寄存器号
     uint16_t LCD_RAM; // 指定需要写入的数据
 } LCD_TypeDef;

 static inline void ili9341_write_reg(uint16_t reg)
 {
     reg          = reg;
     LCD->LCD_REG = reg;
 }

 static inline void ili9341_write_data(uint16_t data)
 {
     data         = data;
     LCD->LCD_RAM = data;
 }

 static void lvgl_ili9341_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
 {
     // 命令分为单字节和多字节命令，对于多字节命令，要把命令逐字节写入LCD_REG
     // ILI9341 的命令都是 单字节的，所以cmd_size=1，这里这么写是为了兼容
     for (uint16_t i = 0; i < cmd_size; i++)
     {
         ili9341_write_reg(cmd[i]);
     }
     // 命令的参数写到LCD_RAM
     for (uint16_t i = 0; i < param_size; i++)
     {
         ili9341_write_data(param[i]);
     }
 }

 static void lvgl_ili9341_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
 {
     for (uint16_t i = 0; i < cmd_size; i++)
     {
         ili9341_write_reg(cmd[i]);
     }

     for (uint16_t i = 0; i < param_size; i += 2)
     {
         // 一次取出两位的数据
         ili9341_write_data(*(uint16_t *)&param[i]); // 获取第一个数据地址并转为16位指针 param[0] cat param[1]
     }

     lv_display_flush_ready(disp);
 }

 void lvgl_ili9341_init(void)
 {
     lv_init();

     // 这是将LVGL与FreeRTOS绑定的关键
     lv_tick_set_cb(xTaskGetTickCount); // 使用FreeRTOS的系统节拍 xTaskGetTickCount 作为LVGL动画和事件的时间基准。
     lv_delay_set_cb(vTaskDelay);       // 使用FreeRTOS的任务延时 vTaskDelay。这能确保在等待时不空耗CPU，而是让操作系统可以调度其他任务，效率极高。

     /* 创建显示驱动实例
       这一步创建了显示驱动实例。它使用了LVGL为ILI9341提供的便捷函数，该函数会发送一系列标准的初始化指令来点亮屏幕。
       最重要的是，它注册了您自己编写的两个硬件通信函数：ili9341_send_cmd 和 ili9341_send_color 作为回调。
     */
     ili9341_display = lv_ili9341_create(DISP_HOR_RES, DISP_VER_RES, LV_LCD_FLAG_NONE, lvgl_ili9341_send_cmd, lvgl_ili9341_send_color);

     /* 配置LVGL用于刷写屏幕的内存区域
       使用了一个大小为屏幕1/8的静态缓冲区 gui_buff，通过 LV_DISPLAY_RENDER_MODE_PARTIAL 标志，它告诉LVGL采用部分渲染模式，即分块绘制UI。这是在RAM有限的MCU上运行复杂GUI的必备技巧。
     */
     lv_color_t *buf1 = NULL, *buf2 = NULL;
     buf1 = gui_buff;
     // color_buf2 = pvPortMalloc(DISP_BUF_SIZE);

     // 驱动完成数据传输后，要调用 lv_display_flush_ready(disp) 告诉 LVGL： “我已经把数据送到 LCD 了，可以继续下一步了”。
     lv_display_set_buffers(ili9341_display, buf1, buf2, DISP_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
 }

```

在main.c文件中，应该如下编写：

```c
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 4                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t Start_Task_Handler;             // 任务句柄
void         start_task(void *pvParameters); // 任务函数

// 创建LVGL任务GUI_TASK
#define GUI_TASK_PRIO 1             // 任务优先级
#define GUI_STK_SIZE  1024          // 任务堆栈大小
TaskHandle_t GUI_Task_Handler;      // 任务句柄
void         gui_task(void *p_arg); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    usart_init(115200);
    delay_init();

    LCD_Init();

    xTaskCreate(
        (TaskFunction_t)start_task,       // 任务函数
        (const char *)"start_task",       // 任务名称
        (uint16_t)START_STK_SIZE,         // 任务堆栈大小
        (void *)NULL,                     // 传递给任务函数的参数
        (UBaseType_t)START_TASK_PRIO,     // 任务优先级
        (TaskHandle_t *)&GUI_Task_Handler // 任务句柄
    );
    vTaskStartScheduler(); // 启动任务调度
}

void start_task(void *pvParameters)
{
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)gui_task,
        (const char *)"gui_task",
        (uint16_t)GUI_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)GUI_TASK_PRIO,
        (TaskHandle_t *)&GUI_Task_Handler
    );
    vTaskDelete(Start_Task_Handler); // 删除开始任务
}

void gui_task(void *p_arg)
{
    // lvgl驱动初始化
    lvgl_ili9341_init(); // 初始化lvgl

    lv_obj_t *screen = lv_screen_active();                                       // 获取当前为active的屏幕指针，一个屏幕为所有空间根容器，自己没有父容器
    lv_obj_set_style_bg_color(screen, lv_color_make(0xff, 0xff, 0xff), 0); // 将屏幕背景设置为白色
    lv_obj_t *label = lv_label_create(screen);                                   // 在screen对象上创建一个label
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);                                 // 将label对象中心点对齐到屏幕的顶部中间，且无偏移
    lv_label_set_text(label, "Hello World!");
	lv_timer_handler();

    while (1)
    {
    }
}

```

