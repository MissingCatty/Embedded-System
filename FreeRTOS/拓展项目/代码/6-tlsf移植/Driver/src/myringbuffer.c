#include "myringbuffer.h"

#define RX_BUF_LEN (uint32_t)512

uint8_t       rx_buff[RX_BUF_LEN];
ringbuffer8_t rb;

void myringbuffer_init(void)
{
    rb = rb8_new(rx_buff, RX_BUF_LEN);
}
