#ifndef __RINGBUFFER8_H
#define __RINGBUFFER8_H

#include <stdbool.h>
#include <stdint.h>

struct ringbuffer8;
typedef struct ringbuffer8 *ringbuffer8_t;

ringbuffer8_t rb8_new(uint8_t *buff, uint32_t length);
bool          rb8_empty(ringbuffer8_t rb);
bool          rb8_full(ringbuffer8_t rb);
bool          rb8_put(ringbuffer8_t rb, uint8_t data);
bool          rb8_puts(ringbuffer8_t rb, uint8_t *data, uint32_t size);
bool          rb8_get(ringbuffer8_t rb, uint8_t *data);
bool          rb8_gets(ringbuffer8_t rb, uint8_t *data, uint32_t size);
uint32_t      rb8_actual_size(ringbuffer8_t rb);

#endif /* __RINGBUFFER8_H */
