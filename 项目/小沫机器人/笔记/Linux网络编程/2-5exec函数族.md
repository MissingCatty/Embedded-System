# 1.概念

`exec` 函数族是 **Unix / Linux 系统编程**（C语言）中用于 **执行新程序、替换当前进程映像** 的一组函数。它们定义在：

```c
#include <unistd.h>
```

核心特点：

- **用一个新程序替换当前进程**
- **不会创建新进程**
- **执行成功后不会返回**
- 通常和 **fork()** 配合使用

当调用 `exec` 时：

1. 当前进程的 **代码段、数据段、堆、栈** 被新程序替换
2. **进程ID（PID）不变**
3. 从新程序的 `main()` 开始执行
4. 原程序后面的代码 **不会执行**

> 示例：
>
> ```
> printf("before exec\n");
> execl("/bin/ls","ls","-l",NULL);
> printf("after exec\n");
> ```
>
> 运行结果：
>
> ```
> before exec
> (执行ls命令)
> ```
>
> 不会打印：
>
> ```
> after exec
> ```
>
> 因为进程已经被替换。

# 2.函数

## 2.1 execl

```c
int execl(const char *path, const char *arg, ...);
execl(path, arg0, arg1, arg2, ..., NULL);
```

| 参数 | 含义                       |
| ---- | -------------------------- |
| path | 要执行程序的**完整路径**   |
| arg  | 第一个参数（通常是程序名） |
| ...  | 后续参数                   |
| NULL | 参数结束标志               |

示例：

```c
execl("/bin/ls","ls","-l","/home",NULL);
```

执行效果：

```
ls -l /home
```

## 2.2 execlp

```c
int execlp(const char *file, const char *arg, ...);
```

| 参数 | 含义                         |
| ---- | ---------------------------- |
| file | 程序名（系统会在PATH中搜索） |
| arg  | 第一个参数                   |
| ...  | 后续参数                     |
| NULL | 参数结束                     |

区别：

- execl  → 必须写完整路径
- execlp → 自动搜索file

系统会在：

```
/bin
/usr/bin
/usr/local/bin
```

中寻找 `ls`。

## 2.3 execle

```c
int execle(const char *path, const char *arg, ..., char *const envp[]);
```

| 参数 | 含义         |
| ---- | ------------ |
| path | 程序完整路径 |
| arg  | 第一个参数   |
| ...  | 后续参数     |
| NULL | 参数结束     |
| envp | 环境变量数组 |

示例：

```c
char *env[] = {"PATH=/bin", "USER=test", NULL};	// 必须以NULL结尾

execle("/bin/ls", "ls", "-l", NULL, env);
```

## 2.4 execv

函数原型：

```c
int execv(const char *path, char *const argv[]);
```

| 参数 | 含义                     |
| ---- | ------------------------ |
| path | 程序路径（**完整路径**） |
| argv | 参数数组                 |

参数数组格式：

```
argv[0] = 程序名
argv[1] = 参数1
argv[2] = 参数2
...
argv[n] = NULL
```

示例

```c
char *argv[] = {"ls", "-l", "/home", NULL};

execv("/bin/ls", argv);
```

执行：

```bash
ls -l /home
```

## 2.5 execvp

```c
int execvp(const char *file, char *const argv[]);
```

| 参数 | 含义                   |
| ---- | ---------------------- |
| file | 程序名（**PATH查找**） |
| argv | 参数数组               |

区别：

- execv  → 必须写完整路径
- execvp → 自动PATH搜索

示例

```c
char *argv[] = {"ls", "-l", NULL};

execvp("ls", argv);
```

## 2.6 execve（最底层）

```c
int execve(
    const char *pathname,
    char *const argv[],
    char *const envp[]
);
```

| 参数     | 含义         |
| -------- | ------------ |
| pathname | 程序路径     |
| argv     | 参数数组     |
| envp     | 环境变量数组 |

示例

```c
char *argv[] = {"ls", "-l", NULL};
char *envp[] = {NULL};

execve("/bin/ls", argv, envp);
```

【注意】：execve 是所有 exec 的底层实现，其他函数内部都会调用 `execve()`。