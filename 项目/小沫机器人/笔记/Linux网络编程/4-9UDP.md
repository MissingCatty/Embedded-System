UDP的流程比较简单，服务端只需要：

- `socket()`
- `bind()`
- `sendto()`
- `recvfrom()`

或者是：

- `socket()`
- `bind()`
- `connect()`
- `sendto()`
- `recvfrom()`

---

**客户端**

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    char msg[] = "hello udp";
    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr));

    close(sockfd);
    return 0;
}
```

---

**服务端**

```c
#include <stdio.h>      // printf, perror, snprintf
#include <string.h>     // memset, strlen
#include <unistd.h>     // close
#include <arpa/inet.h>  // htons, htonl, ntohs, inet_ntop
#include <sys/socket.h> // socket, bind, recvfrom, sendto

int main() {
    // 1. 创建 UDP 套接字
    // AF_INET     : IPv4
    // SOCK_DGRAM  : UDP
    // 0           : 默认协议
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // 定义服务端地址结构体和客户端地址结构体
    struct sockaddr_in servaddr, cliaddr;

    // 先清零，避免结构体里有脏数据
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // 2. 设置服务端地址信息
    servaddr.sin_family = AF_INET;                // IPv4
    servaddr.sin_port = htons(8888);              // 绑定端口 8888，注意要转成网络字节序
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定本机所有网卡地址

    // 3. 绑定本地地址和端口
    // 这样客户端才能向这个端口发送数据
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // 接收缓冲区
    char buf[1024];

    // 4. 循环接收多个客户端的数据
    while (1) {
        // len 在 recvfrom 之前要先初始化
        socklen_t len = sizeof(cliaddr);

        // 5. 接收数据
        // buf                     : 接收数据存放位置
        // sizeof(buf) - 1         : 留一个位置给 '\0'
        // 0                       : flags，一般填 0
        // &cliaddr                : 接收“是谁发来的”
        // &len                    : 地址结构长度
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&cliaddr, &len);

        // 如果接收失败，打印错误并继续下一轮
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        // 6. 把收到的数据补成字符串结束符，方便 printf 打印
        buf[n] = '\0';

        // 7. 把客户端 IP 从网络地址格式转成点分十进制字符串
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));

        // ntohs(cliaddr.sin_port) 把网络字节序端口转回主机字节序
        printf("from %s:%d -> %s\n", ip, ntohs(cliaddr.sin_port), buf);

        // 8. 组织回复内容
        char reply[1024];
        snprintf(reply, sizeof(reply), "server reply: %s", buf);

        // 9. 回复给刚才发消息的那个客户端
        // 这里用的是 recvfrom 得到的 cliaddr
        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&cliaddr, len);
    }

    // 这句理论上到不了，因为上面 while(1) 一直循环
    close(sockfd);
    return 0;
}
```

