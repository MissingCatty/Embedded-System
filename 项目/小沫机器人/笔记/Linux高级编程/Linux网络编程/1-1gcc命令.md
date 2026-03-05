# 1.GCC

GCC 原名为 GNU C语言编译器（GNU C Compiler）

GCC（GNU Compiler Collection，GNU编译器套件）是由 GNU 开发的编程语言译器。GNU 编译器套件包括 C、C++、Objective-C、Java、Ada 和 Go 语言前端，也包括了这些语言的库（如 libstdc++、libgcj 等）

GCC 不仅支持 C 的许多“方言”，也可以区别不同的 C 语言标准；可以使用命令行选项来控制编译器在翻译源代码时应该遵循哪个 C 标准。例如，当使用命令行参数 -std=c99 启动 GCC 时，编译器支持 C99 标准。

安装命令 （版本大于等于 4.8.5）

```bash
sudo apt install gcc g++
```

查看版本

```bash
gcc -v
g++ -v
```

# 2.命令测试

直接编译源文件成为可执行文件并执行

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.c -o app
zyc@ubuntu:~/Linux/Lesson01$ ./app
```

---

`-E`：测试预处理指令

```BASH
zyc@ubuntu:~/Linux/Lesson01$ gcc test.c -E -o test.i
```

---

`-S`：测试编译指令

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.i -S -o test.s
```

---

`-c`：测试汇编指令

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.s -c -o test.o
```

- 汇编，但不进行链接

---

执行`.o`文件

```
zyc@ubuntu:~/Linux/Lesson01$ gcc test.o -o test
zyc@ubuntu:~/Linux/Lesson01$ ./test
```

---

`-D`：测试指令，表示打开某一个宏

```c
#include <stdio.h>

#define PI 3.14

int main(void)
{
  int a = PI + 3;
#ifdef DEBUG
  printf("This is a debug sentence.\n");
#endif // DEBUG
  printf("hello world.\n");
}
```

不指定宏定义，中间的语句被忽略

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.c -o test
zyc@ubuntu:~/Linux/Lesson01$ ./test
hello world.
```

指定之后

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.c -o test -DDEBUG
zyc@ubuntu:~/Linux/Lesson01$ ./test 
This is a debug sentence.
hello world.
```

---

`-Wall`：编译时输出所有`warning`

```bash
zyc@ubuntu:~/Linux/Lesson01$ gcc test.c -o test -DDEBUG -Wall
test.c: In function ‘main’:
test.c:7:7: warning: unused variable ‘a’ [-Wunused-variable]
    7 |   int a = PI + 3;
      |
```

---

`-On`：`n=0,1,2,3`，编译器的优化选项的四个级别，`-O0`表示没有优化，`-O1`为缺省值，`-O3`优化级别最高

---

`-std`：指定C的方言，`-std=c99`表示使用的C99语法，gcc默认的方言为`GNU C`