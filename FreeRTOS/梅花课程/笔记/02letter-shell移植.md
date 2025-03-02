# 函数指针类型的定义方法

使用`typedef`+`函数指针`的方式定义一个函数指针类型：

```c
typedef int (*uart_rx_callback_t)(uint8_t data);
```

代码中定义了一个形参为`uint8_t`类型，返回值为`void`类型的函数指针类型`uart_rx_callback_t`。

之后可以修饰某个变量时，表示定义了一个参数为`uint8_t`，输出为`void`类型的函数指针：

```c
uart_rx_callback_t func;
```

此时变量`func`就变为了一个函数指针，之后就可以如下使用：

```c
int a = func(uint8_t);
```

# letter-shell使用

![image-20250302163050258](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20250302163050258.png)

移植成功后，在串口调试助手窗口中弹出如上图所示的画面。

在窗口中输入命令时，每输入一个字符，都会通过串口传给单片机，所以要在程序内实现一个`shell_read`函数来按照字符读取接收到的字符进入缓冲区`shellBuffer`，所以read函数的实现逻辑如下：

```c
signed short _shell_read_implement(char *data, unsigned short len)
{
    if (rx_data)
    {
        *data   = rx_data;
        rx_data = 0;
        return 1;
    }
    return 0;
}
```

假如某一时刻单片机通过中断接收到一个字符存放到`rx_data`，由于有效字符一般都大于0，所以规定当`rx_data=0`时代表rx_data的数据无效，即没有接收到有效字符。

letter-shell如果运行在单片机的while循环里，则会一直调用`_shell_read_implement`函数取出读取到的字符到缓冲区`shellBuffer`，所以要规定什么时候数据有效，什么时候数据无效，即当`rx_data=0`时无效，故每次取出数据后，要将`rx_data`归零，防止取重复数据。

还有一种方法，就是不使用中断，直接在`_shell_read_implement`里调用`USART_Receive`函数，读取USART的RDR寄存器的值，程序实现如下：

```c
signed short _shell_read_implement(char *data, unsigned short len)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE)!=RESET)
    {
        *data   = USART_ReiceiveData(USART1);
        return 1;
    }
    return 0;
}
```

**总结**：**切记！！！**如果使用中断（详细来说是`RXNE`中断），就不可以在`_shell_read_implement`函数里直接调用`USART_ReiceiveData`，因为在中断中会清除标志位，导致`_shell_read_implement`进不了`if`条件。

```c
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        rx_data = USART_ReceiveData(USART1);
        // usart_send(rx_data); // 回传数据，用于调试
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); // 清除标志位
    }
}
```

还需要注意的是，由于letter-shell的原理是在串口助手中敲一个字符，就立马传给单片机接收，所以在终端中不可以写回传逻辑，否则会出现如下情况：

![image-20250302165032954](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20250302165032954.png)