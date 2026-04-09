# 1.在Ubuntu系统上配置ROS环境

官方安装文档：https://wiki.ros.org/cn/ROS/Installation

# 2. ROS软件包和节点的关系

- 软件包：本质上就是一个**包含特定描述文件的 Linux 文件夹**。
  -  哪怕一个文件夹里全是空文件，只要里面包含以下两个文件，ROS 系统就会承认它是一个合法的包：
  - `package.xml`：这个包的“身份证”，里面写了包名、版本号、作者是谁，以及编译它需要依赖哪些其他的包。
  - `CMakeLists.txt`：给 CMake 用的编译规则文件。你在这里配置要把哪些 `.cpp` 源码编译成可执行文件。
- 节点：本质上就是 `CMakeLists.txt` 里通过 `add_executable()` 编译出来的那个 **二进制可执行文件 (Binary)**。

> 假设你们团队正在开发一个“视觉避障系统”，你们建了一个 **ROS** **软件包（Package）**，名字叫 `vision_navigation`。
>
> 在这个包的文件夹（`vision_navigation/`）里，存放了所有的东西：C++ 源码、Python 脚本、配置文件、3D 模型等。
>
> 在它的 `CMakeLists.txt` 里，你写了这样两段编译指令：
>
> 1. `add_executable(camera_reader src/read_usb.cpp)`
> 2. `add_executable(obstacle_detector src/yolo_detect.cpp)`
>
> 当你敲下 `catkin_make` 或 `colcon build` 编译完成后： 这个**软件包 (Package)** 孕育出了两个独立的 **节点 (Node)**：
>
> - 节点 1：`camera_reader`（专职负责读摄像头）
> - 节点 2：`obstacle_detector`（专职负责跑 AI 算法识别）
>
> 这时候，如果你想让系统跑起来，你需要开两个终端，分别输入：
>
> - `rosrun vision_navigation camera_reader`
> - `rosrun vision_navigation obstacle_detector`

# 3. 环境变量的刷新时机

**只要** **ROS** **的“物理目录结构”或“可执行文件名单”发生了改变，且超出了当前终端的认知，就需要刷新。** 如果只是改了代码逻辑，则完全不需要。

```Bash
source /path/to/workspace/devel/setup.bash
```

# 4. ROS1通信本质

在ROS1中，无论是“发布和订阅”还是“服务”，底层都是两个节点（进程）的**点对点**通信，且使用的是TCP协议（ROS1的底层就是TCP协议）。

- 需要保证通信双方**在通信之前先建立连接**。

## 4.1 节点订阅流程

1. 发布节点：向Master注册topic（/TOPIC）告知Master该topic的ip和port。
2. 订阅节点：向Master发送`subscribe(/TOPIC)`查询谁发布了该主题。
3. Master：向订阅者返回该topic的ip和port。
4. 订阅节点：向发布节点发送TCP连接请求建立连接
5. 开始传输数据
6. 订阅节点取消订阅时，断开tcp连接

## 4.2 有多个订阅方的情况

一个Topic被多个节点订阅，本质上就是：**发布节点的某一个****端口****与多个节点建立****TCP****连接**。

# 5. Master节点作用

Master节点的作用就是一个注册机构，负责维护 **4 张核心映射表**：

## 5.1 节点名映射表 (Node Registry)

- **映射关系**：`逻辑节点名 (String)` ➡️ `节点的 XML-RPC 管理服务器地址 (URI)`
- **作用**：让系统知道某个节点跑在网络中的哪台机器、哪个端口上。
- **举例**：
  - *输入查找*：`/camera_driver_node`
  - *映射结果*：`http://192.168.1.100:45678/`

## 5.2 主题映射表 (Topic Registry)

- **映射关系**：`逻辑主题名 (String)` ➡️ `发布该主题的节点列表 + 订阅该主题的节点列表`
- **作用**：这就是我们前面说的“媒人”记的小本本。它记录了谁在发、谁在听。
- **举例**：
  - *输入查找*：`/environment/3d_obstacles`
  - *映射结果*：
    - Publishers: `['``http://192.168.1.100:45678/']` (感知节点)
    - Subscribers: `['``http://192.168.1.101:33221/']` (规划节点)

## 5.3 服务映射表 (Service Registry)

- **映射关系**：`逻辑服务名 (String)` ➡️ `提供该服务的服务端 TCP 通信地址 (rosrpc URI)`
- **作用**：当客户端想调用某个 RPC 函数时，告诉它去哪个 IP 和端口连那个 TCP Server。
- **举例**：
  - *输入查找*：`/calculate_grasp_pose`
  - *映射结果*：`rosrpc://192.168.1.105:12345`

## 5.4 参数服务器 (Parameter Server)

*(注：虽然叫服务器，但它其实是嵌在* *Master* *进程里的一个全局字典)*

- **映射关系**：`全局参数名 (String)` ➡️ `参数值 (Int/Float/String/List)`
- **作用**：映射全局配置。所有节点启动时，都可以去 Master 这里查一下自己需要的初始配置。
- **举例**：
  - *输入查找*：`/robot/max_velocity`
  - *映射结果*：`1.5` (米/秒)

# 6. msg文件的作用

简单来说，`.msg` 文件就是：

1. **用于简便快捷地定义类**（不仅自动生成 C++ 的 `struct`，还顺手帮你把最难写的网络序列化/反序列化代码全包了）。
2. **用于网络数据传输**（充当不同进程、不同物理机之间通信的标准化“集装箱”）。
3. ➕ **用于跨语言搭桥**（同样的 `.msg`，左手自动生成 C++ 类，右手自动生成 Python 类，让 C++ 和 Python 能完美互发快递）。
