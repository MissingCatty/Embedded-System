#ifndef LVGL_ILI9341_H
#define LVGL_ILI9341_H

#include "src/misc/lv_types.h"
#include "lvgl.h"

static void lvgl_ili9341_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);

static void lvgl_ili9341_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size);

void lvgl_ili9341_init(void);

#endif // !LVGL_ILI9341_H