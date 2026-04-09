# 1. 查看所有软件包及其绝对路径

```Bash
rospack list
```

# 2. 查看所有节点

```Bash
rosnode list
```

# 3. 查看节点信息

```Bash
rosnode info /rosout
```

# 4. 运行节点

```Bash
rosrun 【包名】【节点名】
```

# 5. 可视化节点间关系

```Bash
rosrun rqt_graph rqt_graph
```

# 6. topic相关命令

## 6.1 查看topic带宽

```Bash
rostopic bw 【topic】
zyc@ubuntu:~/Desktop$ rostopic bw /turtle1/cmd_vel
subscribed to [/turtle1/cmd_vel]
average: 21.91B/s
        mean: 48.00B min: 48.00B max: 48.00B window: 2
average: 17.83B/s
        mean: 48.00B min: 48.00B max: 48.00B window: 2
average: 15.03B/s
        mean: 48.00B min: 48.00B max: 48.00B window: 2
average: 13.00B/s
        mean: 48.00B min: 48.00B max: 48.00B window: 2
```

## 6.2 打印传输的消息内容

```Bash
rostopic echo 【topic】
zyc@ubuntu:~/Desktop$ rostopic echo /turtle1/cmd_vel
linear: 
  x: 2.0
  y: 0.0
  z: 0.0
angular: 
  x: 0.0
  y: 0.0
  z: 0.0
---
linear: 
  x: 2.0
  y: 0.0
  z: 0.0
angular: 
  x: 0.0
  y: 0.0
  z: 0.0
```

## 6.3 打印有关topic的信息

```Bash
rostopic info 【topic】
zyc@ubuntu:~/Desktop$ rostopic info /turtle1/cmd_vel
Type: geometry_msgs/Twist

Publishers: 
 * /teleop_turtle (http://ubuntu:39999/)

Subscribers: 
 * /turtlesim (http://ubuntu:39519/)
```

## 6.4 列出所有topic

```Bash
rostopic list
zyc@ubuntu:~/Desktop$ rostopic list
/rosout
/rosout_agg
/statistics
/turtle1/cmd_vel
/turtle1/color_sensor
/turtle1/pose
```

## 6.5 将数据发布到topic

```Bash
rostopic pub [发送频率参数] /主题名称 消息类型 "具体的数据内容"

# 发送频率参数
# -1：只发送一次
# -r 10: 以10hz的频率发送
```

- 常用场景：给topic发送json数据

> 1. 输入 `rostopic pub /turtle1/cmd_vel ` （注意结尾留个空格）。
> 2. **按两下** **`Tab`** **键**，系统会自动补全消息类型：`rostopic pub /turtle1/cmd_vel geometry_msgs/Twist `。
> 3. **再按两下** **`Tab`** **键**，见证奇迹的时刻——系统会自动把这个消息类型的**所有内部结构和默认值（通常是0.0）全部打印出来**！
> 4. 你现在得到的是这样一串极其完美的命令：
>    1. `rostopic pub /turtle1/cmd_vel geometry_msgs/Twist "linear:  x: 0.0  y: 0.0  z: 0.0 angular:  x: 0.0  y: 0.0  z: 0.0"`
>
> 你只需要用键盘的方向键，把光标移动到你需要修改的数字（比如 `linear.x`）上，改成 `2.0`，然后按下回车即可！

有哪些常用的类型

| **消息大类 / 门派** | **核心包名 (Package)** | **常用消息类型 (Type)**                                 | **内部核心数据结构示例**                               | **主要应用场景 / 作用**                                      |
| ------------------- | ---------------------- | ------------------------------------------------------- | ------------------------------------------------------ | ------------------------------------------------------------ |
| **基础数据派**      | `std_msgs`             | `String` `Int32` / `Int64` `Float32` / `Float64` `Bool` | `string data` `int32 data` `float32 data` `bool data`  | 发送最基础的控制指令、状态反馈或参数标志位。                 |
|                     |                        | ⭐ **`Header`**                                          | `time stamp` `string frame_id`                         | **极度重要**：几乎嵌在所有高级消息中，用于时间戳同步和 TF 坐标系对齐。 |
| **物理运动派**      | `geometry_msgs`        | `Point`                                                 | `float64 x, y, z`                                      | 表示三维空间中的一个绝对坐标点。                             |
|                     |                        | `Quaternion`                                            | `float64 x, y, z, w`                                   | 四元数，用于无万向节死角地表示三维旋转姿态。                 |
|                     |                        | `Pose`                                                  | `Point position` `Quaternion orientation`              | **位姿**：包含物体在空间中的位置（Point）和朝向（Quaternion）。 |
|                     |                        | ⭐ **`Twist`**                                           | `Vector3 linear` `Vector3 angular`                     | **速度指令**：控制底盘或机械臂的线速度（向前/后）和角速度（转向）。 |
| **各种感官派**      | `sensor_msgs`          | ⭐ **`Image`**                                           | `Header header` `uint32 height, width` `uint8[] data`  | **视觉处理核心**：原始图像矩阵数据，通常转为 OpenCV 格式处理。 |
|                     |                        | ⭐ **`PointCloud2`**                                     | `Header header` `uint32 width, height` `uint8[] data`  | **3D重建核心**：海量的 3D 点云坐标集合（激光雷达/深度相机产生）。 |
|                     |                        | `LaserScan`                                             | `float32[] ranges`                                     | 2D 激光雷达扫描出的一圈距离数组（常用于平面避障）。          |
|                     |                        | `Imu`                                                   | `Quaternion orientation` `Vector3 angular_velocity`... | 惯性测量单元数据（陀螺仪角速度、加速度计）。                 |
| **寻路导航派**      | `nav_msgs`             | `OccupancyGrid`                                         | `Header header` `int8[] data`                          | **2D 栅格地图**：也就是建图算法跑出来的黑白灰像素迷宫图。    |
|                     |                        | `Path`                                                  | `Header header` `PoseStamped[] poses`                  | **路径**：一系列位姿（Pose）组成的数组，告诉机器人走哪条路线。 |
|                     |                        | `Odometry`                                              | `Header header` `PoseWithCovariance pose`...           | **里程计**：机器人根据轮子编码器推算出的自身累计移动距离和速度。 |