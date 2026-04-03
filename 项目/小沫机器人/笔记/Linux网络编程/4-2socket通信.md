# 1.概念

之前学过的进程间通信的方法都是用在**同一主机多个进程间的通信**，但网络上数据的交换更多的是网络中的两个不同主机进程之间的通信。

`socket（套接字）`，就是对网络中**不同主机**上的应用进程之间进行**双向通信的端点的抽象**。

> 一个套接字就是**网络上进程通信的一端**，提供了应用层进程利用网络协议交换数据的机制。从所处的地位来讲，套接字上联应用进程，下联网络协议栈，是应用程序通过网络协议进行通信的接口，是应用程序与网络协议根进行交互的接口

如果**没有套接字**，那一个进程想把数据发出去，就得自己面对一整套底层通信问题。

> #### 1. 先决定发给谁
>
> 进程得自己明确：
>
> - 目标 IP 是谁
> - 目标端口是谁
> - 用 TCP 还是 UDP
>
> 也就是说，得自己处理“寻址”。
>
> #### 2. 如果用 TCP，还得自己建立连接
>
> 如果没有套接字提供的 `connect()`、`accept()` 这些抽象，
>
> 那进程就得自己处理：
>
> - 发 SYN
> - 收 SYN+ACK
> - 回 ACK
> - 维护连接状态
>
> 也就是三次握手和连接状态机都得自己管。
>
> 3. 自己给数据编号
> 4. 自己切分数据
> 5. 自己封装协议头
> 6. 自己处理重传
> 7. 自己处理乱序和去重
> 8. 自己处理流量控制和拥塞控制
> 9. 最后还得想办法交给网卡（这已经非常底层了，普通应用进程根本不适合直接做这些事）

有套接字的时候，进程只需要做这种事：

- 告诉操作系统：我要连谁
- 把数据交给操作系统
- 操作系统负责后面的 TCP/IP 处理和发送

# 2.字节序

- 大端：高位字节存放在低地址部分
- 小端：低位字节存放在低地址部分

假设一个4字节数据`0x12345678`

- 大端中

  ```
  低地址 -> 高地址
  78 56 34 12
  ```

- 小端

  ```
  低地址 -> 高地址
  12 34 56 78
  ```

---

**在网络通信中有什么用呢？**

网络传输时，多字节数据按**网络字节序**（**通常是大端**）发送，也就是高位字节先发。

所以对于大端存储的机器，低地址先发，然后小端存储的机器要转换字节序。

## 2.1 检查当前主机的字节序

使用联合体，因为联合体里的所有数据成员都共享同一块内存。

可以设置两个成员，一个整数另一个是数组

```c
bool isBigEndian(void) {
    union {
        short val;	// 2字节
        char bytes[sizeof(short)];	// 2字节
	}test;
    test.val = 0x0102;
    
    return test.bytes[0] == 0x01;	// 看低地址是不是高位
}
```

## 2.2 字节序转化

```c’
#include <arpa/inet.h>
```

```c
uint16_t htons(uint16_t hostshort);
/*
    - 功能：将主机字节序的 16 位整数转换为网络字节序
    - hostshort：主机字节序的 16 位数据
    - 返回值：
        - 转换后的网络字节序 16 位数据
    - 说明：
        - h：host，主机字节序
        - n：network，网络字节序
        - s：short，16 位
*/
```

```c
uint16_t ntohs(uint16_t netshort);
/*
    - 功能：将网络字节序的 16 位整数转换为主机字节序
    - netshort：网络字节序的 16 位数据
    - 返回值：
        - 转换后的主机字节序 16 位数据
    - 说明：
        - n：network，网络字节序
        - h：host，主机字节序
        - s：short，16 位
*/
```

```c
uint32_t htonl(uint32_t hostlong);
/*
    - 功能：将主机字节序的 32 位整数转换为网络字节序
    - hostlong：主机字节序的 32 位数据
    - 返回值：
        - 转换后的网络字节序 32 位数据
    - 说明：
        - h：host，主机字节序
        - n：network，网络字节序
        - l：long，32 位
*/
```

```c
uint32_t ntohl(uint32_t netlong);
/*
    - 功能：将网络字节序的 32 位整数转换为主机字节序
    - netlong：网络字节序的 32 位数据
    - 返回值：
        - 转换后的主机字节序 32 位数据
    - 说明：
        - n：network，网络字节序
        - h：host，主机字节序
        - l：long，32 位
*/
```

- 注意：
  - `htonl`和`ntohl`一般用于转换IP，IP地址是32位
  - `ntohs`和`htons`一般用于转换端口，端口（0~65535），是16位

**案例1：简单的字节序转换**

```c
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>

int main(void)
{
    printf("%d\n", isBigEndian());
    unsigned short a = 0x0102;
    unsigned short b = htons(a);
    printf("a: %x\n", a);
    printf("b: %x\n", b);
}
```

**案例2：转换IP地址**

```c
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>

int main(void)
{
    unsigned char ip[] = {192, 168, 0, 1};
    int num = *(int *)ip;
    int trans = htonl(num);
    *(int *)ip = trans;
    printf("%d.%d.%d.%d\n", *ip, *(ip + 1), *(ip + 2), *(ip + 3));
}

/*
	输出：1.0.168.192
*/
```

# 3.socket地址

socket地址其实是一个`结构体`，**封装端口号和IP等信息**，后面的socket相关的api中需要使用到这个 socket地址。

> 比如：
>
> - `127.0.0.1:8080`
> - `192.168.1.10:80`
>
> 这就是最常见的 socket 地址。

定义如下：

```c
#include <bits/socket.h> 
struct sockaddr { 
    sa_family_t sa_family; 
    char sa_data[14]; 
};

typedef unsigned short int sa_family_t;
```

- `sa_family`：地址族（address family），告诉系统：**这个地址到底是哪一类地址**

  以下三个最常用：

  |  地址族  |       描述       |
  | :------: | :--------------: |
  | AF_UNIX  | UNIX本地域协议族 |
  | AF_INET  |  TCP/IPv4协议族  |
  | AF_INET6 |  TCP/IPv6协议族  |

- `sa_data`：用于存放 socket 地址值，不同的协议族的地址值具有不同的含义和长度

  `sa_family` 说明这是什么地址类型，告诉系统如何解析`sa_data`

  - `sa_family = AF_INET`：`sa_data` 就会被当成：

    - **端口号**
    - **IPv4 地址**

    不过实际编程一般不用直接操作 `sa_data`，而是用：

    ```c
    struct sockaddr_in {
        sa_family_t    sin_family;   // 地址族
        in_port_t      sin_port;     // 端口号
        struct in_addr sin_addr;     // IPv4地址
        unsigned char  sin_zero[8];  // 填充用，通常memset清零即可
    };
    
    struct in_addr {
        in_addr_t s_addr;  // 32 位无符号整数（网络字节序）
    };
    ```
  
  - `sa_family = AF_INET`：
  
    ```c
    struct sockaddr_in6 {
        sa_family_t     sin6_family;   // 地址族，AF_INET6
        in_port_t       sin6_port;     // 端口号
        uint32_t        sin6_flowinfo; // 流信息
        struct in6_addr sin6_addr;     // IPv6地址
        uint32_t        sin6_scope_id; // 作用域ID
    };
    
    struct in6_addr {
        union {
            uint8_t  __u6_addr8[16];   // 16 个字节
            uint16_t __u6_addr16[8];   // 8 个 16 位短整型
            uint32_t __u6_addr32[4];   // 4 个 32 位整型
        } __in6_u;
    };
    ```
  
  -  `sa_family = AF_UNIX`：那 `sa_data` 就会被当成：
  
    本地 socket 路径的一部分，用于**本机进程间通信**，不走 IP 网络。
  
    对应常用结构体是：
  
    ```c
    struct sockaddr_un {
        sa_family_t sun_family;   // 地址族，AF_UNIX
        char        sun_path[108]; // 路径名，本地文件路径，例如"/tmp/test.sock"
    };
    ```
  
  ![image-20211117213426327](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2026%2F03%2F5c1e1624d9f2680d7a19dd56b19bb1ae.png)

# 4.IP地址转化

我们在程序里为了方便阅读，通常将IP地址写成字符串的形式：

```c
char ip[] = "192.168.1.4";
```

而网络中传输的数据一般是用一个32位（四字节）数据来表示ip地址，所以需要有一个函数来将字符串转为四字节数据：

```c
#include <arpa/inet.h>

int inet_pton(int af, const char *src, void *dst);
/*
    - 功能：将字符串形式的 IP 地址转换为网络字节序的二进制数据
    - af：地址族
        - AF_INET：IPv4
        - AF_INET6：IPv6
    - src：字符串形式的 IP 地址
    - dst：保存转换后结果的内存地址
    - 返回值：
        - 成功转换：1
        - 输入格式非法：0
        - 失败：-1，并设置 errno
    - 说明：
        - p：presentation，表现形式（字符串形式）
        - n：numeric，数值形式（二进制形式）
        - 常用于把 "192.168.1.10" 转成可用于 socket 的地址数据
*/
```

同样的，也需要将数据转为字符串表示：

```c
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
/*
    - 功能：将网络字节序的二进制 IP 地址转换为字符串形式
    - af：地址族
        - AF_INET：IPv4
        - AF_INET6：IPv6
    - src：二进制形式的 IP 地址
    - dst：保存转换后字符串的缓冲区
    - size：dst 缓冲区的大小
    - 返回值：
        - 成功：返回 dst
        - 失败：返回 NULL，并设置 errno
    - 说明：
        - n：numeric，数值形式（二进制形式）
        - p：presentation，表现形式（字符串形式）
        - 常用于把二进制 IP 地址转成 "192.168.1.10" 这种可读字符串
*/
```

# 5.TCP通信流程

![image-20211121104748003](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2026%2F03%2F03b03b54fe6bf274aef2b951926b24d4.png)

---

**服务器端**

1. `socket()`：创建套接字接口

   - 用于监听客户端连接

   - 套接字本质：**文件描述符**

2. `bind()`：将套接字和本地IP和端口绑定

3. `listen()`：设置监听，监听fd开始工作

4. `accept()`：阻塞等待，当有客户端发起连接，解除阻塞，接受客户端的连接，会得到一个`和客户端通信的套接字(fd)`）（这是另外一个套接字）

5. `recv()+send()`：开始通信

6. `recv()`：接收客户端断开连接请求

7. `close()`：关闭连接

---

**客户端**

1. `socket()`：创建套接字接口
2. `connect()`：发起连接
3. `recv()+send()`：收发数据
4. `close()`：关闭连接

# 6.socket函数

## 6.1 socket：创建套接字

```c
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
/*
    - 功能：创建一个网络通信的端点（套接字）
    - domain：协议族
        - AF_INET：IPv4 因特网协议
        - AF_INET6：IPv6 因特网协议
        - AF_UNIX / AF_LOCAL：本地通信（进程间通信）
    - type：通信语义类型
        - SOCK_STREAM：流式传输（面向连接，如 TCP）
        - SOCK_DGRAM：数据报传输（无连接，如 UDP）
    - protocol：具体的协议
        - 通常设置为 0，表示根据 domain 和 type 自动选择默认协议（如 TCP 或 UDP）
    - 返回值：
        - 成功：返回一个文件描述符（fd），用于后续操作
        - 失败：-1，并设置 errno
*/
```

## 6.2 bind：绑定本地协议地址

```c
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
/*
    - 功能：将一个本地地址（IP + 端口）与套接字文件描述符绑定
    - sockfd：通过 socket() 函数创建的文件描述符
    - addr：指向包含本地地址信息的 sockaddr 结构体指针
        - 通常使用 struct sockaddr_in 进行初始化，然后强制类型转换为 struct sockaddr *
    - addrlen：传入的地址结构体的大小（单位：字节），常用 sizeof(struct sockaddr_in)
    - 返回值：
        - 成功：0
        - 失败：-1，并设置 errno
*/
```

## 6.3 listen：将套接字设置为监听状态

```c
#include <sys/types.h>
#include <sys/socket.h>

int listen(int sockfd, int backlog);
/*
    - 功能：将一个主动套接字转换为被动套接字，用于等待客户端的连接请求
    - sockfd：通过 socket() 创建并经过 bind() 绑定后的文件描述符
    - backlog：未完成连接队列和已完成连接队列的总和最大值
        - 现代 Linux 内核中，通常指已完成三次握手、等待 accept 处理的队列长度
        - 如果队列满了，客户端可能会收到 ECONNREFUSED 错误
    - 返回值：
        - 成功：0
        - 失败：-1，并设置 errno
*/
```

内核为每一个监听套接字维护了 **两个队列**，`backlog` 的值直接影响这两个队列的容量：

- **半连接队列 (Incomplete Connection Queue)**：

  - 客户端发送了 SYN，服务器也回复了 SYN+ACK，但还没收到客户端的最后一个 ACK。

  - 此时连接处于 `SYN_RCVD` 状态。

- **全连接队列 (Completed Connection Queue)**：

  - 三次握手已经完全结束，连接处于 `ESTABLISHED` 状态。

  - 这些连接正排队等着你的程序调用 `accept()` 把它们领走。

> 本质上就是为一个套接字指示：
>
> - 哪些客户端还在连接
> - 哪些客户端已经连接完成

## 6.4 accept：从已完成连接队列提取连接

```c
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
/*
    - 功能：从已完成三次握手的连接队列中提取下一个连接，阻塞函数，等待客户端连接进来
    - sockfd：监听套接字（由 socket() 创建，bind() 绑定，listen() 监听后的描述符）
    - addr：-O，记录连接成功的客户端的地址信息（IP 和 端口）
    - addrlen：-O-I，指明 addr 结构体的大小
        - 调用前：传入 addr 结构体的长度
        - 调用后：返回内核实际写入 addr 的字节数
    - 返回值：
        - 成功：返回一个新的文件描述符（已连接套接字），用于与客户端通信
        - 失败：-1，并设置 errno
*/
```

**误区：传入的监听套接字`sockfd`和返回的已连接套接字fd不是一个！**

因为服务器上的一个端口需要：

- 被监听，所以包含**唯一监听套接字**

- 连接客户端，一个本地端口（如 80）可以同时与成千上万个不同的客户端（不同的 IP 或不同的远程端口）建立连接，包含**多个连接套接字**。

  > 每一个具体的 TCP 连接确实是“一对一”的，但对于服务器的**同一个端口**来说，它可以同时维护**成千上万个**这种“一对一”的连接。

## 6.5 connect：建立连接（TCP）或 关联对端（UDP）

```c
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
/*
    - 功能：
        - 对于 TCP：发起三次握手，建立可靠的连接。
        - 对于 UDP：在内核中记录对端的 IP 和 端口，后续收发数据只针对该地址。
    - sockfd：本地创建的套接字文件描述符。
    - addr：服务器的地址信息（IP + 端口）。
    - addrlen：地址结构体的大小。
    - 返回值：
        - 成功：0
        - 失败：-1，并设置 errno
*/
```

## 6.6 send：通过套接字发送数据

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
/*
    - 功能：向 TCP 连接或已 connect 的 UDP 套接字发送数据
    - sockfd：用于传输数据的 socket 文件描述符
    - buf：待发送数据缓冲区
    - len：待发送数据的字节数
    - flags：发送标志，常用：
        - 0：等同于 write()
        - MSG_DONTWAIT：非阻塞发送
        - MSG_NOSIGNAL：在对方关闭连接时，不触发 SIGPIPE 信号
    - 返回值：
        - 成功：返回实际发送的字节数
        - 失败：-1，并设置 errno
*/
```

send的本质不是往网络上发送数据，而是将数据从【用户缓冲区】（你定义的 char buf[]）拷贝到【内核发送缓冲区】（TCP Send Buffer），真正的发送由内核中的TCP/IP协议栈负责。

> 本质就是向内核的内存里写数据

- 当`flags=0`的时候，`send(fd, buf, len, 0)`完全等价于`write(fd, buf, len)`
  - write：适用于任何文件描述符（普通文件、管道、套接字等）
  - send：仅限于套接字文件描述符
- `MSG_DONTWAIT`：如果内核缓冲区满了，则不等待，立刻返回-1并设置错误 `EAGAIN`

- `MSG_NOSIGNAL`：如果对方已经断开了（Broken pipe），普通的 `write` 会导致程序收到 `SIGPIPE` 信号直接崩溃退出。设置这个 flag 后，程序不会死，只会让 `send` 返回错误。

## 6.7 recv：通过套接字接收数据

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
/*
    - 功能：从 TCP 连接或已 connect 的 UDP 套接字接收数据
    - sockfd：用于接收数据的 socket 文件描述符
    - buf：存放接收数据的缓冲区
    - len：搬运最大长度
    - flags：接收标志，常用：
        - 0：等同于 read()，缓冲区有多少拿多少，最多拿len个
        - MSG_PEEK：查看缓冲区数据但不移除（下次 recv 还能读到）
        - MSG_WAITALL：直到读满 len 字节才返回（除非连接出错或断开）
    - 返回值：
        - > 0：实际收到的字节数
        - = 0：表示对方已正常关闭连接（重要！判定 TCP 断线的标准）
        - -1：失败，并设置 errno
*/
```

recv的本质就是从内核缓冲区读取数据到用户缓冲区，

让内核缓冲区为空的时候，默认是阻塞等待缓冲区内有数据为止。

- `MSG_DONTWAIT`：

  - 内核缓冲区不为空，则直接搬运最大len长度的数据
  - 内核缓冲区空，直接返回-1并设置`errno`为`EAGAIN`

- `MSG_PEEK`：**偷看数据**，从内核缓冲区拷贝数据到用户空间，但**不删除**内核里的数据

- `MSG_WAITALL`：**死等到底**，直到读满 `len` 长度的字节才返回（除非连接中断）

  > 搬多少数据根据len指定的最大长度来定，如果不足len，则把内核缓冲区中的数据全部复制完

- `MSG_TRUNC`：**截断返回**，返回数据包的真实长度，即使它超过了你提供的缓冲区

## 6.8 close：关闭套接字/文件描述符

```c
#include <unistd.h>

int close(int fd);
/*
    - 功能：关闭一个文件描述符，销毁对应的内核资源
    - fd：要关闭的套接字文件描述符（lfd 或 cfd）
    - 返回值：
        - 成功：0
        - 失败：-1，并设置 errno
*/
```

# 8.TCP并发服务器实现
