# 1.多进程版

如果想4-3中那样，服务器只和一个客户端连接，

```c
// 等待连接
int connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
...
// 收发数据
while (1) {
    recv(connect_fd, recv_buf, sizeof(recv_buf), 0);
    ...
}
```

这么写是没有问题的。

因为连接完成就只剩收发数据了，此时并不会还有其他的客户端发起连接请求并发送数据。

但是在并发服务器中，会有多个客户端一起连接并发送数据的情况，对于服务端来说，就要使用多线程或多进程来实现。

通常使用的架构是：**父进程负责接收客户端的连接请求，多个子进程负责收发对应的客户端数据**。

**服务端**

```c
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/signal.h>

#define SERVER_IP "172.16.0.6"
#define SERVER_PORT 9999

// 销毁僵尸进程
void catch_child(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main(void)
{
    signal(SIGCHLD, catch_child);

    /* 1. 创建套接字 */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("create socket error");
        return 1;
    }

    /* 2. 绑定ip地址和端口 */
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);
    saddr.sin_port = htons(SERVER_PORT);
    if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        perror("socket bind error");
        return 1;
    }

    /* 3. 开启监听 */
    if (listen(listen_fd, 10) == -1)
    {
        perror("socket listen error");
        return 1;
    }

    /* 4. 等待被连接 */
    struct sockaddr_in client_addr;
    printf("服务器正在等待被连接...\n");
    socklen_t addrlen = sizeof(client_addr);
    int connect_fd;
    int process_id;
    while (1)
    {
        connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connect_fd == -1)
        {
            perror("accept error");
            continue;
        }
        char client_ip[16];
        printf("[连接成功]，客户端ip：%s:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)), ntohs(client_addr.sin_port));

        /* 5. 开辟子进程 */
        pid_t process_id = fork();
        if (process_id < 0)
        {
            perror("child process create error");
            close(connect_fd);
        }
        else if (process_id == 0)
        {
            /* 6.获取客户端数据 */
            close(listen_fd); // 关闭listen文件描述符
            char recv_buf[1024];
            int len;
            while (1)
            {
                len = recv(connect_fd, recv_buf, sizeof(recv_buf), 0);
                if (len < 0)
                {
                    perror("recv error");
                    break;
                }
                else if (len == 0)
                {
                    printf("客户端连接关闭\n");
                    break;
                }
                else
                {
                    printf("子进程%d接收：", getpid());
                    fflush(stdout);
                    write(1, recv_buf, len);
                }
            }
            close(connect_fd);
            exit(0);
        }
        else
        {
            // 父进程不负责连接，所以关闭
            close(connect_fd);
        }
    }
    close(listen_fd);
    return 0;
}
```

几个注意点：

- 当子进程调用`exit`结束之后，会成为僵尸进程，占用进程号，但会给父进程发送`SIGCHLD`信号，父进程收到信号后，可以调用`wait/waitpid`函数去回收子进程资源
- 子进程刚开始的时候由于用不到`listen_fd`，虽然不影响程序执行，出于规范考虑，应该关闭

**客户端**

和原来一样

# 2.多线程版

思路与多进程类似，主线程负责监听端口连接，每来一个客户端连接，开一个子线程负责通信。

```c
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <pthread.h>

#define SERVER_IP "172.16.0.6"
#define SERVER_PORT 9999

void *communicate(void *arg)
{
    int connect_fd = *(int *)arg;
    
    /*正确写法
        int connect_fd = *(int *)arg;
    	free(arg);
    */
    
    char recv_buf[1024];
    int len;
    while (1)
    {
        len = recv(connect_fd, recv_buf, sizeof(recv_buf), 0);
        if (len == -1)
        {
            perror("recv error");
            break;
        }
        else if (len == 0)
        {
            printf("客户端连接关闭\n");
            break;
        }
        else
        {
            printf("子线程%ld接收：", pthread_self());
            fflush(stdout);
            write(1, recv_buf, len);
        }
    }
    close(connect_fd);
    pthread_exit(NULL);
}

int main(void)
{
    /* 1. 创建套接字 */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("create socket error");
        return 1;
    }

    /* 2. 绑定ip地址和端口 */
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);
    saddr.sin_port = htons(SERVER_PORT);
    if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        perror("socket bind error");
        return 1;
    }

    /* 3. 开启监听 */
    if (listen(listen_fd, 10) == -1)
    {
        perror("socket listen error");
        return 1;
    }

    /* 4. 等待被连接 */
    struct sockaddr_in client_addr;
    printf("服务器正在等待被连接...\n");
    socklen_t addrlen = sizeof(client_addr);
    while (1)
    {
        int connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connect_fd == -1)
        {
            perror("accept error");
            free(connect_fd_p);
            break;
        }
        char client_ip[16];
        printf("[连接成功]，客户端ip：%s:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)), ntohs(client_addr.sin_port));

        /* 5. 开辟子线程 */
        pthread_t tid;
        pthread_create(&tid, NULL, communicate, &connect_fd);
        pthread_detach(tid);
    }
    close(listen_fd);
    return 0;
}
```

上述程序有个错误：

```c
/* 主线程 */
int connect_fd = accept(...);
// ...
pthread_create(&tid, NULL, communicate, &connect_fd);

/* 子线程 */
int connect_fd = *(int *)arg;
```

所有线程的`connect_fd`都是获取自主线程的同一个局部变量，当线程A被创建，还没来得及执行`int connect_fd = *(int *)arg;`主线程又`accept`更新了`connect_fd`的值，导致前后不一致。

**解决办法**：每次使用malloc申请一个`connect_fd`，然后在子线程里free掉

> 这就是多线程和多进程方式的区别，多进程根本不需要管共享内存

# 3.多进程和多线程完成TCP并发通信的区别

1. **资源开销**

   - **多进程**：`fork()` 是一个“重量级”操作。操作系统需要为子进程复制父进程的内存空间、文件描述符表、页表等。虽然现代 Linux 有“写时复制（COW）”技术优化，但创建和销毁进程的开销依然很大。

   - **多线程**： 线程是“轻量级”的进程。新开辟的线程和主线程**共享**同一块内存地址空间（只拥有自己独立的栈空间和寄存器）。创建和销毁线程的速度比进程快得多，占用的内存也极小。

     多线程共享：

     - **文件描述符表**
     - **全局变量与静态变量**
     - **堆区内存**
     - **信号处理函数**：如果你在主线程里写了 `signal(SIGINT, my_handler);`（比如按 Ctrl+C 时的处理逻辑），这个规则对所有线程都生效。一个进程只能有一套信号处理规则。

2. **内存隔离与数据共享**

   - **多进程：天生隔离。** 子进程和父进程的变量互不干扰。客户端 A 和客户端 B 对应的子进程想交换数据非常麻烦，必须使用进程间通信（IPC，如管道、共享内存、消息队列）。

     **多线程：天生共享。** 所有线程共享全局变量、堆区内存。客户端 A 的线程可以直接读写客户端 B 线程的数据。**但是**，这也带来了灾难性的副作用：**资源竞争**。你必须引入“互斥锁（Mutex）”来防止两个线程同时修改同一个变量导致数据错乱。

3. **文件描述符的处理**

   - **多进程：** 子进程复制了父进程的 `listen_fd`（监听套接字）和 `connect_fd`（通信套接字）。所以我们在子进程里要 `close(listen_fd)`，在父进程里要 `close(connect_fd)`。
   - **多线程：** 所有线程共享文件描述符！如果你在新线程里手贱写了一句 `close(listen_fd)`，**整个服务器的监听通道就直接关闭了**，主线程再也无法 `accept` 新连接。多线程下，主线程不能关闭 `connect_fd`，子线程也绝对不能关闭 `listen_fd`。

4. **健壮性与稳定性**

   - **多进程：极度稳定。** 一个子进程因为代码 Bug（比如数组越界、段错误）崩溃了，只会死它自己，其他客户端的子进程和主进程依然坚挺。这也是 Nginx 等老牌服务器默认采用多进程架构的原因。
   - **多线程：一死全死。** 只要有一个线程触发了段错误（Segmentation Fault）或者除以零，整个进程会直接崩溃，所有连接的客户端会瞬间全部断开。