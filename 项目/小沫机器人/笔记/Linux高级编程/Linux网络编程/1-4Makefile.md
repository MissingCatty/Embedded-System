# 1.为什么要Makefile

一个工程中的源文件不计其数，其按类型、功能、模块分别放在若干个目录中，Makefile 文件定义了一系列的规则来指定哪些文件需要先编译，哪些文件需要后编译，哪些文件需要重新编译

# 2.Makefile文件命名和规则

文件命名是有规定的必须为`makefile`或`Makefile`

## 2.1 Makefile规则

```
目标 ...: 依赖 ...
	命令（shell命令）
```

- 目标：最后要生成的文件
- 依赖：生成目标所需的文件
- 命令：如何操作用依赖生成目标（Tab缩进）

Makefile中的其他规则一般是为第一条规则服务的

## 2.2 Makefile案例

某个项目中结构如下：

```
.
├── add.cpp
├── div.cpp
├── main.cpp
├── math.h
├── mul.cpp
└── sub.cpp
```

要编译src里的内容，并和main组装成一个可执行程序，需要：

- g++编译`src`下的所有源文件
- g++编译并链接main.cpp

```makefile
app: add.cpp div.cpp mul.cpp sub.cpp main.cpp
	g++ add.cpp div.cpp mul.cpp sub.cpp main.cpp -o app
```

现在把目录整理一下：

```bash
.
├── inc
│   └── math.h
├── main.cpp
├── Makefile
└── src
    ├── add.cpp
    ├── div.cpp
    ├── mul.cpp
    └── sub.cpp
```

最简单的写法是：

```makefile
app: src/add.cpp src/div.cpp src/mul.cpp src/sub.cpp main.cpp
	g++ src/add.cpp src/div.cpp src/mul.cpp src/sub.cpp main.cpp -Iinc -o app
```

规范一点可以：

- 把所需源文件写到一个变量里
- 把头文件目录写到一个变量里

```makefile
SRC = src/add.cpp src/div.cpp src/mul.cpp src/sub.cpp
INC = inc
CXX = g++

app: $(SRC) main.cpp
	$(CXX) $(SRC) main.cpp -I$(INC) -o app
```

再高级一点，如果源文件需要删除或新增，每次都要手动指定十分麻烦，可以使用makefile中的函数功能：

```makefile
SRC = $(wildcard src/*.cpp)
INC = inc
CXX = g++

app: $(SRC) main.cpp
	$(CXX) $(SRC) main.cpp -I$(INC) -o app
```

# 3.Makefile的工作原理

命令执行之前，先检查规则中的依赖是否存在

- 如果存在，就去执行命令

- 如果依赖不存在，就**向下检查其他的规则**，查看是否有其他规则是生成这个依赖的，如果找到了，就执行这个规则下的命令

  ```makefile
  app:sub.o add.o mult.o div.o main.o
  	gcc sub.o add.o mult.o div.o main.o -o app
  
  sub.o:sub.c
  	gcc -c sub.c -o sub.o
  
  add.o:add.c
  	gcc -c add.c -o add.o
  
  mult.o:mult.c
  	gcc -c mult.c -o mult.o
  
  div.o:div.c
  	gcc -c div.c -o div.o
  
  main.o:main.c
  	gcc -c main.c -o main.o
  ```

检查更新，在执行规则中的命令时，会比较目标和依赖的生成或修改时间

- 如果目标的修改时间在依赖的修改时间之后，则说明依赖没变过，目标不更新，命令也就不执行
- 如果。。。

所以，**依赖的本质作用就是检查生成的目标文件和依赖文件的时间戳的**。如果依赖指定的有问题，那么很可能导致有些情况命令不会被执行。

# 4.变量

- 自定义变量

  ```makefile
  var=hello
  ```

- 预定义变量：可以直接使用的变量，不需要自己定义

  - `AR`：默认值`ar`，打包静态库的工具
  - `CC`：默认值`cc`，C编译器名
  - `CXX`：默认值`g++`，C++编译器名

  下面三个只能用在规则的命令里：

  - `$@`：目标名
  - `$<`：第一个依赖文件名
  - `$^`：所有依赖文件

- 获取变量值：`$(变量)`

# 5.符号

- `%`：通配符，匹配一个字符串

  > `%.o:%.c`中两个`%`表示的同一个字符串

  ```makefile
  # 定义变量
  src=sub.o add.o mult.o div.o main.o
  target=app
  
  $(target):$(src)
  	$(CC) $(src) -o $(target)
  
  # 每一个.o文件都是由.c文件得到的
  %.o:%.c
  	$(CC) -c $< -o $@
  ```

# 6.函数

1. `$(wildcard PATTERN)`

   - 功能：获取指定目录下指定类型的文件列表
   - 返回：若干个文件的文件列表，之间使用空格间隔

   ```
   $(wildcard *.c ./src/*.c)
   返回：a.c b.c ...
   ```

2. `$(patsubst PATTERN,replacement,text)`

   - 功能：在text中查找符合pattern的字符串，如果匹配则替换

   ```makefile
   $(patsubst %.c,%.o,a.c b.c)
   返回：a.o b.o ...
   ```

# 7.伪目标

在规则上方加上`.PHONY`，表示将下面紧跟的规则中的目标设置成伪目标，表示**该目标不会生成对应的文件**。

```makefile
.PHONY:clean
clean:
	rm ...
```

- 注意：伪目标下的命令执行与否不根据时间戳，所以伪目标一定会执行

