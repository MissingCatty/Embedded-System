# 1.准备工作

要移植 FreeRTOS，肯定需要一个基础工程，基础工程越简单越好。

源码的移植主要是在`FreeRTOSv202212.01\FreeRTOS\Source`下：

![image-20241220121650073](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220121650073.png)

需要移植的内容包含：

- 所有`.c`文件
- portable文件夹中的`keil`，`MemMang`和`RVDS`
- `include`文件夹及其中所有内容

# 2. 开始移植

---

**添加文件**

- 新建STM32基础工程（略过）
- 将1中提到的三部分文件复制到工程根目录内，`portable`中只保留`keil`，`MemMang`和`RVDS`
  - 修改`include`文件夹为`FreeRTOS_Inc`
  - 修改`portable`文件夹为`FreeRTOS_Port`
  - 创建`FreeRTOS_Core`文件夹，并将复制来的`.c`文件移动进去

![image-20241220123704225](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220123704225.png)

---

**加入Keil工程**

将上面添加好的文件加入到Keil的工程里，包括两步：新建分组并添加文件，添加头文件路径

**新建分组并添加文件**

- 在Keil里创建`FreeRTOS_CORE`和`FreeRTOS_PORTABLE`分组

- 将`~/FreeRTOS_Core/`和`~/FreeRTOS_Port/`路径下的所有文件分别加入到分组里

  - `~/FreeRTOS_Core/`下所有`.c`文件

  - `~/FreeRTOS_Port/MemMang/`下的`heap_4.c`

    `heap_4.c` 是 `MemMang`文件夹中的，前面说了 `MemMang` 是跟内存管理相关的，里面有 5 个 `.c` 文件：`heap_1.c`、`heap_2.c`、`heap_3.c`、`heap_4.c` 和 `heap_5.c`。这 5 个 `.c` 文件是五种不同的内存管理方法，都可以用来作为FreeRTOS 的内存管理文件，只是它们的实现原理不同，各有利弊。

  - `~/FreeRTOS_Port/RVDS/ARM_CM3`中的 `port.c` 文件

![image-20241220125338554](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220125338554.png)

**添加头文件路径**

![image-20241220125555754](https://zyc-learning-1309954661.cos.ap-nanjing.myqcloud.com/machine-learning-pic/image-20241220125555754.png)

---

**添加`FreeRTOSConfig.h`文件**

- 在官方例程文件夹`FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_STM32F103_Keil/`里找
- 复制到`~/FreeRTOS_Inc/`文件夹里

---

**编译测试**

如果没有问题，则移植成功