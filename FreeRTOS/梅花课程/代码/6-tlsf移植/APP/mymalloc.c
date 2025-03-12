#include "mytlsf.h"
#include "stddef.h" // size_t

/*
    该文件中的函数使用前需要初始化tlsf！！！
*/

void *malloc(size_t size)
{
    return mytlsf_malloc(size);
}

void free(void *ptr)
{
    mytlsf_free(ptr);
}
