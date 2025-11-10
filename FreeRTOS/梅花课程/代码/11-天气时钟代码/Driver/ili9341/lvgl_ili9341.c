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
