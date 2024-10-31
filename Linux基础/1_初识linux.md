# 1 Linux系统组成

- 系统内核：提供最核心的功能，主要是管理硬件调度的能力
  - CPU调度
  - 内存调度
  - 文件系统调度
  - 网络通讯
  - IO调度
- 系统级应用：可以理解为出厂自带的程序
  - 文件管理器
  - 任务管理器
  - 图片查看
  - 音乐播放

![image-20241028195554243](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241028195554243.png)

注：[内核源码下载地址](https://www.kernel.org)

# 2 Linux发行版

由于Linux的内核是完全开源免费的，如果将其拿来，并使用内核自己编写一整套系统级程序，最后封装成一个完整的Linux系统，那么该完整的系统就被称为“Linux 发行版”

市面上有许多Linux发行版，如：Ubuntu、Centos、debian...

这些不同的发行版本，

- 所有的基础命令是完全相同的
- 部分操作不同（软件安装）