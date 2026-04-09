# 1. 发布者代码

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

int main(int argc, char **argv)
{
  // 初始化ROS节点
  ros::init(argc, argv, "talker");
  // 创建节点句柄
  ros::NodeHandle n;
  // 注册话题发布者
  ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);
  // 设置发布频率10Hz
  ros::Rate loop_rate(10);

  int count = 0;
  while (ros::ok())
  {
    // 构造消息内容
    std_msgs::String msg;
    std::stringstream ss;
    ss << "hello world " << count;
    msg.data = ss.str();

    // 打印日志并发布消息
    ROS_INFO("%s", msg.data.c_str());	// 底层是c风格的字符串，需要的是const char *，而msg.data是std::string类型
    chatter_pub.publish(msg);

    // 处理回调+控制频率
    ros::spinOnce();
    loop_rate.sleep();
    ++count;
  }
  return 0;
}
```

逐行查看每个语句的含义

---

```cpp
ros::init(argc, argv, "talker");
```

- 功能：把这个普通 C++ 程序，登记成一个 ROS 节点，并把 ROS 运行前要做的基础准备先做好。
- 主要完成：
  - ROS 初始化
  - 记录这个节点的名字
  - 处理命令行里的 ROS 专用参数与重映射规则
  - 并把库内部状态标记为“已经初始化”

---

```cpp
ros::NodeHandle n;
```

- 功能：拿到一个操作 ROS 系统的句柄，之后就通过它和 ROS 系统打交道，比如：
  - 发布话题：`n.advertise(...)`
  - 订阅话题：`n.subscribe(...)`
  - 读写参数：`n.getParam(...)` / `n.setParam(...)`
  - 创建服务端/客户端：`n.advertiseService(...)` / `n.serviceClient(...)`
  - 创建定时器：`n.createTimer(...)`

---

```cpp
ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);
```

- 功能：创建一个“发布者”对象 `chatter_pub`，并把当前节点注册成 `chatter` 这个话题的发布者。

着重讲一下`advertise`函数：

```cpp
Publisher advertise(const std::string& topic, uint32_t queue_size, bool latch=false)
/*
	- topic: 话题名
	- queue_size: 发送队列大小，最多允许多少条“待发送的输出消息”排队，等着投递给订阅者。
	- latch: 锁存发布，如果开启，publisher 会保存最后一条消息，新订阅者之后连上来时会立刻收到那条“最后消息”。
*/
```

> **发送队列是什么？**
>
> 假设你发得很快，但订阅者来不及收，或者网络一时处理不过来，那么消息不会立刻全丢掉，而是先放进这个发送队列里。这个数字越大，能缓存的待发送消息越多；太小的话，在发布速度高的时候更容易丢掉旧消息。

---

```cpp
ros::spinOnce();
```

- 功能：处理一次当前节点收到的回调任务。

  你先把“回调”理解成：

  - 收到订阅消息后要执行的函数
  - 服务请求来了要执行的函数
  - 定时器到了要执行的函数

  这些不会凭空自己执行，ROS 需要一个地方去**“取出并处理”这些事件**。 

  `ros::spinOnce()` 做的就是：**检查一下有没有待处理的回调，有的话处理一轮，然后立刻返回。**

---

```cpp
loop_rate.sleep();
```

- 功能：让整个程序休眠一会儿

# 2.订阅者代码

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"

// 消息回调函数：收到消息时自动调用
void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
  // 打印收到的消息
  ROS_INFO("I heard: [%s]", msg->data.c_str());
}

int main(int argc, char **argv)
{
  // 初始化ROS节点，节点名：listener
  ros::init(argc, argv, "listener");
  // 创建节点句柄
  ros::NodeHandle n;

  // 订阅chatter话题，队列长度10，回调函数chatterCallback
  ros::Subscriber sub = n.subscribe("chatter", 10, chatterCallback);

  // 循环等待消息，处理回调
  ros::spin();

  return 0;
}
```

---

```cpp
ros::spin();
```

- 功能：进入 ROS 的回调事件循环，一直处理回调，直到节点退出。（也就是进入一个循环，不断处理 callbacks）

  **主要适合那种“节点的大部分工作都在订阅回调里完成”的程序**。

# 3.CMakeList.txt

要将上述代码能够通过`catkin_make`编译，还需要修改CMakeLisit.txt文件

```cmake
# 防止 CMake 版本太老，某些命令不支持
cmake_minimum_required(VERSION 2.8.3)

# 定义工程的名字
project(beginner_tutorials)

# 查找 catkin 构建系统，并且要求找到这些依赖包
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  genmsg
)

# 声明这个包里有自定义消息文件，要处理 msg/Num.msg
add_message_files(
  DIRECTORY msg
  FILES Num.msg
)

# 声明这个包里有自定义服务文件，要处理 srv/AddTwoInts.srv
add_service_files(
  DIRECTORY srv
  FILES AddTwoInts.srv
)

# 根据前面声明的 .msg 和 .srv 文件，生成消息/服务代码
generate_messages(
  DEPENDENCIES std_msgs
)

# 声明这是一个 catkin 包，并导出这个包的构建信息
# 相当于告诉其他包：我这个包叫什么，我导出了什么，我依赖哪些 catkin 包，别人如何链接我
catkin_package()

# （生成节点）把 src/talker.cc 编译成一个可执行程序 talker
add_executable(talker src/talker.cc)

# 让 talker 链接 catkin 提供的库
# （因为talker.cc里使用了：ros::init，NodeHandle，Publisher，ROS_INFO）
target_link_libraries(talker ${catkin_LIBRARIES})

# add_depencies: 给一个顶层 target 添加对其他顶层 target 的依赖，确保被依赖的 target 先构建，再构建当前 target。 
# talker 这个目标依赖于消息生成目标，必须先把消息代码生成好，再编译 talker
add_dependencies(talker beginner_tutorials_generate_messages_cpp)

# 把 src/listener.cc 编译成一个可执行程序 listener
add_executable(listener src/listener.cc)

# 给 listener 链接 ROS/catkin 相关库
target_link_libraries(listener ${catkin_LIBRARIES})

# 让 listener 在编译前先等待本包消息代码生成完成
add_dependencies(listener beginner_tutorials_generate_messages_cpp)
```

- 注意：如果没有自定义msg文件，仅仅使用的是ROS中定义的标准数据类型，就不需要执行
  - add_message_files
  - generate_messages
  - add_dependencies

# 4.编译流程

写完CMake后，执行编译：

```bash
zyc@ubuntu:~/catkin_ws$ catkin_make -j8
```

运行主节点：

```bash
roscore
```

刷新环境变量（因为目录结构改了）：

```bash
source devel/setup.bash
```

运行节点：

```bash
rosrun beginner_tutorials listener
rosrun beginner_tutorials talker
```

