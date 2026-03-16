使用 GDB 调试的时候，GDB 默认只能跟踪一个进程，**可以在 fork 函数调用之前**，通过指令设置 GDB 调试工具跟踪父进程或者是跟踪子进程，**默认跟踪父进程**。

设置调试父进程或者子进程：

```bash
set follow-fork-mode [parent（默认） | child]
```

---

设置fork后的调试模式：

```
set detach-on-fork [on | off]
```

**规定fork出来的新进程怎么处理？**（是脱离GDB掌控继续执行，还是被GDB管理被挂起）。默认为 on，表示调试当前进程的时候，被fork出来的进程继续运行，如果为 off，调试当前进程的时候，被fork出来的进程被 GDB 挂起。

> 当程序执行：
>
> ```
> fork();
> ```
>
> 会出现：
>
> ```
> parent process
> child process
> ```
>
> 这时 GDB 要决定：
>
> ```
> 另一个怎么办？
> ```
>
> **detach-on-fork = on（默认）**
>
> ```
> 父进程 → 继续调试
> 子进程 → 脱离调试，继续运行
> ```
>
> 结构：
>
> ```
> GDB
>  │
>  └── parent (调试)
> 
> child (自由运行)
> ```
>
> **detach-on-fork = off**
>
> ```
> 父进程 → 调试
> 子进程 → 也被 GDB 控制
> ```
>
> 结构：
>
> ```
> GDB
>  ├── parent
>  └── child
> ```
>
> 然后可以：
>
> ```
> info inferiors
> inferior 1
> inferior 2
> ```
>
> 切换调试。

---

查看调试的进程id：

```
info inferiors
```

> 只会列出当前被 GDB 管理（调试）的进程。
>
> 注意：**该命令列出来的所有进程，即被GDB控制管理的进程，除了当前被调试的进程外，其他进程都默认被挂起**

---

切换当前调试的进程：

```
inferior id
```

---

将某个进程拉进GDB管理：

```
attach pid
```

---

使进程脱离 GDB 调试：

```
detach inferiors id
```

> 让进程脱离GDB管理，进程继续执行