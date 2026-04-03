**实现**：服务端等待客户端连接，连接后服务端将客户端发送的信息打印出来

**服务端代码**：

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP   "172.16.0.6"
#define SERVER_PORT 9999

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
    int connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connect_fd == -1)
    {
        perror("accept error");
        return 1;
    }
    char client_ip[16];
    printf("连接成功，客户端ip：%s:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)), ntohs(client_addr.sin_port));
	
    /* 5.获取客户端数据 */
    char recv_buf[1024];
    int len;
    while (1)
    {
        len = recv(connect_fd, recv_buf, sizeof(recv_buf), 0);
        if (len == -1)
        {
            perror("recv error");
            return 1;
        }
        else if (len == 0)
        {
            printf("客户端连接关闭\n");
            break;
        }
        else
        {
            write(1, recv_buf, len);
        }
    }
    close(connect_fd);
    close(listen_fd);
}
```

**客户端代码**：

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_IP   "172.16.0.6"
#define SERVER_PORT 9999

int main(void)
{
    /* 1. 创建套接字 */
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd == -1)
    {
        perror("create socket error");
        return 1;
    }

    /* 2. 建立TCP连接 */
    struct sockaddr_in server_sock_addr;
    server_sock_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_sock_addr.sin_addr);
    server_sock_addr.sin_port = htons(SERVER_PORT);
    if (connect(connect_fd, (struct sockaddr *)&server_sock_addr, sizeof(server_sock_addr)) == -1)
    {
        perror("connect error");
        return 1;
    }

    /* 3. 发送数据 */
    char send_buf[] = "hello, world.\n";
    for (int i = 0; i < 5; i++)
    {
        send(connect_fd, send_buf, strlen(send_buf), 0);
        printf("客户端发送：%s", send_buf);
        sleep(1);
    }

    close(connect_fd);
}
```

# 