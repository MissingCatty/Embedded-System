ringbuffer本质上是一个**先进先出**的环形数组。每次从**头部放入数据**，从**尾部取数据**。

每次放入一个元素，head就**向后移一个**；每次取一个元素，tail就**向后移一个**。

```c
typedef struct ringbuffer8 *ringbuffer8_t;

// 缓冲区管理块
struct ringbuffer8
{
    uint32_t tail;
    uint32_t head;
    uint32_t length;

    uint8_t buffer[];
};

// 缓冲区申请
ringbuffer8_t rb8_new(uint8_t *buff, uint32_t length)
{
    ringbuffer8_t rb = (ringbuffer8_t)buff; // 强制转为结构体指针，指向位置不变
    rb->length = length - sizeof(struct ringbuffer8);

    return rb;
}

// 存一个字节数据
bool rb8_put(ringbuffer8_t rb, uint8_t data)
{
    if (next_head(rb) == rb->tail)
        return false;

    rb->buffer[rb->head] = data;
    rb->head = next_head(rb);

    return true;
}

// 取一个字节数据
bool rb8_get(ringbuffer8_t rb, uint8_t *data)
{
    if (rb->head == rb->tail)
        return false;

    *data = rb->buffer[rb->tail];
    rb->tail = next_tail(rb);

    return true;
}
```

![image-20250308091255248](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20250308091255248.png)

**为什么要使用ringbuffer？**

讨论如下的情况，假如在letter-shell命令行连续输入两条指令一起发送，且假设第一条指令都需要执行很长的时间才能执行第二条。

这个过程详细来说就是：按字符逐个接收第一条指令，当收到换行符`\r\n`时，执行第一条指令，此时主程序会花很长时间一直在执行第一条指令，就会阻塞其他代码的执行。

由于发送方发送指令的步骤不会停止，此时就会继续发送第二条指令的内容，

```c
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        rx_data = USART_ReceiveData(USART1);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
```

由于是中断的方式接收指令的内容，每接收一个字节，就将其保存在`rx_data`里，letter-buffer对应的read程序段需要将`rx_data`里的数据**及时**取到定义的指令缓冲区里，但目前主程序是阻塞在第一跳指令对应的功能里的，无法及时将`rx_data`移到缓冲区，等下一次接收中断发生时，新到的字节就会覆盖`rx_data`中未取出的内容，造成第二条指令前几个字节丢失（如果第一条指令执行很长时间，甚至最后只会读到第二个指令中最后一个字节`\n`）

所以，问题的本质就是`rx_data`太小，只能存一个字节，如果后面的字节到了，这个字节还没取就丢失了（即数据接收的太快，但letter-shell使用太慢）。所以可以再开辟一个数组作为缓冲区，但不美观，使用也不方便，就使用一个环形数组统一管理，这样读字节和取字节都很直观清晰。

**如何使用ringbuffer？**

接收中断中接收到的字节放到ringbuffer里，之后letter-shell的读函数从ringbuffer里取。