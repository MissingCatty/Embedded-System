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

# letter-shell使用注意事项

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

# letter-shell自定义命令

命令本质上就是一个函数，即输入一段命令，该命令中包含的参数就是对应函数的参数，同样的命令执行的功能就是在函数中编写的功能。

比如，现在要编写一个`helloworld`命令，执行该命令后，会输出一些东西。步骤如下：

- 首先需要定义该命令所对应的函数，假设该函数为`cmd_helloworld`。根据letter-shell官方教程所述，支持两种定义函数的方式：

  - 一种是类main函数的方式，函数形式为`cmd_helloworld(int argc, char *argv[])`，其中`argc`表示命令中的字符串个数，argv是用于保存命令中的字符串的字符串数组。

    ```c
    int cmd_helloworld(int argc, char *argv[])
    {
        shellWriteString(shellGetCurrent(), "This is a test command.\n");
        shellPrint(shellGetCurrent(), "Arguments: %d\n", argc);
        for (int i = 0; i < argc; i++)
        {
            shellPrint(shellGetCurrent(), "Argument %d: %s\n", i, argv[i]);
        }
        return 0;
    }
    ```

    在shell中执行命令后结果如下：

    ```shell
    letter:/$ helloworld -c 123
    
    This is a test command.
    Arguments: 3
    Argument 0: helloworld
    Argument 1: -c
    Argument 2: 123
    ######################split line########################
    letter:/$ helloworld "ni hao"
    
    This is a test command.
    Arguments: 2
    Argument 0: helloworld
    Argument 1: ni hao
    ```

    用这种类main函数的方法定义的函数，需要手动解析命令中的各个参数。

  - 另一种是普通的c函数的方式，函数形式为`cmd_helloworld(int i, double j, char c, string s,...)`。在这种方式下，shell会根据命令中包含指定的参数自动给函数的形参赋值。

    ```c
    int func(int i, char ch, char *str)
    {
        shellPrint(shellGetCurrent(), "input int: %d, char: %c, string: %s\r\n", i, ch, str);
        return 0;
    }
    ```

    在串口助手中输入命令：

    ```shell
    letter:/$ func 12 'b' "hhh"
    
    input int: 12, char: b, string: hhh
    ######################split line########################
    letter:/$ func x -1 4
    
    input int: 536871129, char: , string: ?FFF
    ```

    可见，这种方式定义的命令，其参数必须严格按照函数形参中的数据类型来赋值，否则会出现无法解析的情况。

- 上面已经按照两种方式定义了函数，如果想要在shell中以命令的方式取调用对应的函数，就需要对该函数注册。有两种注册方式：

  - 第一种是直接使用`SHELL_EXPORT_CMD`宏在**定义函数的文件中**注册

    ```c
    // 类main函数
    SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, helloworld, cmd_helloworld, helloworld_test);
    // 普通c函数
    SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN, func, func, test);
    ```

    注册的格式为：

    ```c
    SHELL_EXPORT_CMD(命令属性, 命令名（命令提示符）, 命令绑定函数名, 命令描述)
    ```

  - 第二种是在`shell_cmd_list.c`文件中的命令表中统一注册：

    ```c
    // 在命令表数组中加入注册宏
    extern int cmd_helloworld(int argc, char *argv[]);
    ...
    const ShellCommand shellCommandList[] = 
    {
    	...,
        SHELL_CMD_ITEM(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, helloworld, cmd_helloworld, helloworld_test),
    }
    ```

  注：以上两种注册方法二者选其一即可。