#include "stm32f4xx.h"
#include "main.h"
#include "lvgl.h"

/* Frame buffers
 * Static or global buffer(s). The second buffer is optional
 * TODO: Adjust color format and choose buffer size. DISPLAY_WIDTH * 10 is one suggestion. */
#define DISPLAY_WIDTH 480
#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /* will be 2 for RGB565 */
#define BUFF_SIZE (DISPLAY_WIDTH * 10 * BYTES_PER_PIXEL)
static uint8_t buf_1[BUFF_SIZE];
static uint8_t buf_2[BUFF_SIZE];

int main(void)
{
	while(1);
}
