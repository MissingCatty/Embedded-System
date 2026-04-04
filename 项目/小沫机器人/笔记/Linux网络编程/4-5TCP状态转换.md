

![image-20211123124438914](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2026%2F03%2F45e029e9b800fb767a2f68af8fa49601.png)

![image-20211123124611335](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/2026%2F03%2Feb399a4538ea7e3597b5af269848923b.png)

# 1.被动连接（绿色）

所有状态都是对服务器来说的

- **服务器收到xxx，转为xxx态**
- **服务器发送xxx(除ACK)，转为xxx态**

---

**服务端等待被连接**

- 服务端状态：`Listen`

此时服务端监听端口，等待客户端连接请求。

---

**三次握手**

1. 客户端发送SYN请求建立连接，服务端**收到**SYN请求
   - 服务端状态：由`Listen`转为`SYN_RCVD`（SYN已接收）
2. 服务端返回`SYN+ACK`应答，**收到**服务端响应的`ACK`
   - 服务端状态：由`SYN_RCVD`转为`ESTABLISHED`（连接已建立）

---

**数据通信**

服务器收到客户端数据报，立刻返回ACK，没有状态切换

---

**四次挥手**

1. 服务端收到客户端发送的`FIN`，返回`ACK`
   - 服务端状态：由`ESTABLISHED`转为`CLOSE_WAIT`
2. 服务端向客户端发送`FIN`
   - 服务端状态：由`CLOSE_WAIT`转为`LAST_ACK`
3. 收到客户端发送的`ACK`
   - 服务端状态：由`LAST_ACK`转为`CLOSED`

# 2.主动连接（红色）

**三次握手**

1. 服务器发送`SYN`
   - 转为`SYN_SENT`
2. 客户端回`ACK`，服务器回`ACK`
   - 转为`ESTABLISH`

---

**数据通信**

无状态切换

---

**四次挥手**

1. 服务器发送`FIN`
   - 由`ESTABLISHED`转为`FIN_WAIT1`
2. 收到客户端的`ACK`，等待客户端的`FIN`
   - 由`FIN_WAIT1`转为`FIN_WAIT2`
3. 收到客户端的`FIN`，回复`ACK`
   - 由`FIN_WAIT2`转为`CLOSED`