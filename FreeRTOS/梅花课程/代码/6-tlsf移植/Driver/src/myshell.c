#include "myshell.h"
#include "myringbuffer.h"

Shell shell;
char  shellBuffer[512];

/**
 * @brief shell读函数实现（单片机接收usart发送过来的字符），规定对于接收到的字节要保存到data里
 *
 * @param data 用于存放并返回接收到的字符
 * @param len 请求读取的字符数量
 * @return signed 实际读取到的字符数量
 */
signed short _shell_read_implement(char *data, unsigned short len)
{
    if (!rb8_empty(rb))
    {
        rb8_get(rb, (uint8_t *)data);
        return 1;
    }
    return 0;
}

/**
 * @brief shell写函数实现（单片机通过usart发送字符）
 *
 * @param data 需写的字符数据
 * @param len 需要写入的字符数
 * @return signed 实际写入的字符数量
 */
signed short _shell_write_implement(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        usart_send(data[i]);
    }
    return len;
}

void myshell_init(void)
{
    shell.read  = _shell_read_implement;
    shell.write = _shell_write_implement;
    shellInit(&shell, shellBuffer, sizeof(shellBuffer));
}
