#include "mytlsf.h"
#include "stm32f4xx.h"

#define ARENA_SIZE 64 * 1024

uint8_t arena[ARENA_SIZE]; // 内存池
tlsf_t  tlsf = NULL;       // tlsf实例

void mytlsf_init(void)
{
    tlsf = tlsf_create_with_pool(arena, ARENA_SIZE);
}

void *mytlsf_malloc(size_t size)
{
    if (!tlsf)
    {
        return NULL; // 未初始化tlsf，返回NULL
    }
    return tlsf_malloc(tlsf, size);
}

void mytlsf_free(tlsf_t ptr)
{
    if (!tlsf)
    {
        return;
    }
    tlsf_free(tlsf, ptr);
}

void mytlsf_deinit(void)
{
    tlsf_remove_pool(tlsf, tlsf_get_pool(tlsf));
}
