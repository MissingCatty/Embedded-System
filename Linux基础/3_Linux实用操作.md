# 1.快捷键

|   按键    |                             含义                             |
| :-------: | :----------------------------------------------------------: |
| `ctrl+c`  | 1. 强制停止程序运行<br />2. 命令输入一半发现错误，另起一个命令行重新输入 |
| `ctrl+d`  | 1. 退出账户的登录<br />2. 退出某些程序的专属页面（如：python） |
| `history` | 查看历史输入过的命令<br />（技巧：通过`$ history | grep xxx`可查看包含`xxx`关键字的命令） |
|  `!xxx`   | 自动匹配之前输入过的第一个以`xxx`为前缀的命令<br />（如：之前输入过`hisory`命令，之后直接可以`his`） |
| `ctrl+r`  |              搜索第一个包含输入关键字的历史命令              |
| `ctrl+a`  |                         跳到命令开头                         |
| `ctrl+e`  |                         跳到命令结尾                         |
| `ctrl+l`  |                         清空终端内容                         |

# 2.软件安装/卸载

Linux安装软件的两种方式：

- 下载安装包自行安装
- 系统的应用商店安装

## 2.1 应用商店安装

使用`apt`命令（需要`root`权限），Ubuntu是`apt`命令，Centos是`yum`命令

`apt`：deb（Ubuntu中的安装包是`.deb`文件，Centos中的安装包是`.rpm`文件）包软件管理器，用于自动化安装配置Linux软件，并可以自动解决依赖问题

```shell
$ apt [-y][install | remove | search] 软件名称
```

- `-y`：在安装过程中出现`yes/no`自动`yes`

## 2.2 安装包安装

Ubuntu安装包为`.deb`文件，需要使用Debian包管理系统的核心工具进行安装和卸载

- **安装软件包**：使用 `dpkg -i package.deb` 命令可以安装本地的 `.deb` 文件。
- **卸载软件包**：`dpkg -r package_name` 用于卸载软件包，但会保留配置文件；而 `dpkg --purge package_name` 则会彻底移除软件包及其配置文件。
- **查询信息**：`dpkg -l` 列出所有已安装的软件包；`dpkg -L package_name` 显示指定软件包安装的所有文件列表；`dpkg -s package_name` 获取指定软件包的详细状态信息。
- **重新配置软件包**：如果安装过程中出现问题或需要重新运行配置脚本，可以使用 `dpkg-reconfigure package_name`。

# 3.systemctl命令

Linux很多（内置/第三方）软件支持使用`systemctl`命令控制：启动、停止、开机自启

能够被`systemctl`管理的软件，一般被称为“服务“

```shell
$ systemctl start | stop | status | enable | disable 服务名
```

- `enable`：开启开机自启

系统内置的服务较多，如：

- NetworkManager，主网络服务
- network，副网络服务
- firewalld，防火墙服务
- sshd，ssh服务

注：部分第三方软件在安装后没有自动集成到`systemctl`中时，可以手动添加

# 4.软链接

将文件、文件夹链接到其他位置（**快捷方式**）

```shell
$ ln -s 被链接文件（夹） 链接目的地
```

- `-s`：创建软连接

# 5.日期和时区

## 5.1 date命令

通过`date`命令在命令行中查看系统的时间

```shell
$ date [-d] [+格式化字符串]
```

- `-d`：按照给定的字符串显示日期，一般用于日期计算

  ```shell
  $ date -d "+1 day" +%Y-%m-%d	# 显示后一天的日期
  $ date -d "+1 month" +%Y-%m-%d	# 显示后一月的日期
  $ date -d "+1 year" +%Y-%m-%d	# 显示后一年的日期
  ```

  - 支持：`year`，`month`，`day`，`hour`，`minute`，`second`

- 格式化字符串：通过特定的字符串标记，控制显示的日期格式

  ```shell
  zyc@zyc-VMware-Virtual-Platform:~$ date +%Y-%m-%d
  2024-12-15
  zyc@zyc-VMware-Virtual-Platform:~$ date +%y-%m-%d
  24-12-15
  ```

  - `%Y`：年（四位数的年）
  - `%y`：年份后两位数字
  - `%m`：月份
  - `%d`：日
  - `%H`：小时
  - `%M`：分钟
  - `%S`：秒
  - `%s`：自1970-01-01 00:00:01 UTC到现在的秒数

## 5.2 修改时区

需要`root`权限

```shell
rm -f /etc/localtime	# 删除原本的本地时间
sudo ln -s /usr/share/zoneinfo/Asia/Shanghai /etc/localtime	# 创建软连接，替换原本的localtime
```

进入`/etc`下执行`ls -l | grep localtime`，得到以下结果：

```shell
zyc@zyc-VMware-Virtual-Platform:/etc$ ls -l | grep localtime
lrwxrwxrwx 1 root root 33 Oct 29 05:11 localtime -> /usr/share/zoneinfo/Asia/Shanghai
```

发现`localtime`软连接指向`/usr/share/zoneinfo/Asia/Shanghai`

## 5.3 ntp程序

安装`ntp`程序以自动校准系统时间

# 6.IP地址和主机名

## 6.1 查看和修改主机名

查看主机名：

- 命令行输入时`zyc@zyc-VMware-Virtual-Platform`，前半部分是用户名，后半部分是主机名
- 或者使用`hostname`命令

修改主机名：

```shell
$ hostnamectl set-hostname 主机名
```

## 6.2 域名解析

给一个域名`www.baidu.com`，系统需要查询DNS服务器，找到该域名对应的IP地址才能进行访问

过程为：

- 先查询本机的`/etc/hosts`文件中是否有`www.baidu.com`的记录
- 如果没有，就联网查询DNS服务器中的记录

---

**通过主机名SSH连接Linux**

- 找到`C:\WINDOWS\System32\drivers\etc\hosts`
- 输入域名（主机名）与IP地址的映射关系`192.168.71.101 Vmware`

- 之后就可以通过主机名`Vmware`远程Linux系统

## 6.3 虚拟机配置固定IP

- 动态IP：`DHCP`每次在设备重启后都会重新获取一个IP地址
  - 对于远程连接的Linux系统，如果频繁变更IP地址，会使配置很麻烦

配置静态IP的两个步骤：

- 在VMware里配置IP地址网关和<u>网段</u>（IP地址范围，通过子网和子网掩码实现）

  - 打开VMware中的”编辑-虚拟网络编辑器“
  - 点击”VMnet8“（因为是NAT模式）
  - 设置子网IP和子网掩码

- 在**Ubuntu**中手动修改配置文件，固定IP

  - `sudo nano /etc/netplan/xxx.yaml`

  - 手动配置yaml文件

    ```yaml
    network:
      version: 2
      renderer: networkd  # 或 NetworkManager，取决于系统
      ethernets:
        enp0s3:  # 替换为实际网卡名称
          dhcp4: false  # 禁用 DHCP
          addresses:
            - 192.168.1.100/24  # 静态 IP 和子网掩码
          gateway4: 192.168.1.1  # 默认网关
          nameservers:
            addresses:
              - 8.8.8.8  # 主 DNS
              - 8.8.4.4  # 备用 DNS
    ```

  - 保存并应用配置

    ```shell
    $ sudo netplan apply
    ```

## 6.4 虚拟机的NAT模式和桥接模式

- NAT模式：宿主机做虚拟机的网关路由器，多个虚拟机构成一个私有网络，虚拟机需要通过宿主机访问公共网络
- 桥接模式：虚拟机与宿主机同级，作为宿主机网络中的一个单独的设备

# 7.网络请求和下载

## 7.1 wget命令

在命令行内下载网络文件

```shell
$ wget [-b] url
```

- `-b`：后台下载，将下载进度写入到**当前工作目录**的`wget.log`文件
  - 使用`tail wget.log`打印当前时刻的下载进度，如果想要持续刷新下载进度，使用`tail -f wget.log`，只要当下载进度在变，在命令行中就持续打印

| 参数                     | 功能                         | 示例                                                         |
| ------------------------ | ---------------------------- | ------------------------------------------------------------ |
| `-O <文件名>`            | 将下载的内容保存到指定文件中 | `wget -O example.html https://example.com`                   |
| `-c`                     | 断点续传                     | `wget -c https://example.com/file.zip`                       |
| `-r`                     | 递归下载                     | `wget -r https://example.com/dir`                            |
| `-p`                     | 下载页面及其所有依赖资源     | `wget -p https://example.com/page`                           |
| `-k`                     | 转换链接为本地链接           | `wget -k -p https://example.com/page`                        |
| `--limit-rate`           | 限制下载速度                 | `wget --limit-rate=100k https://example.com/file.zip`        |
| `--mirror`               | 下载整个网站                 | `wget --mirror https://example.com`                          |
| `--header`               | 添加自定义 HTTP 请求头       | `wget --header="Authorization: Bearer token" https://example.com` |
| `--no-check-certificate` | 忽略 SSL 证书验证            | `wget --no-check-certificate https://example.com`            |

## 7.2 curl命令

发送http请求，可用于下载文件、获取信息

```shell
$ curl [-O] url
```

- `-O`：用于下载文件，当url是下载链接时，可以使用此选项保存文件

| 参数          | 功能                                | 示例                                             |
| ------------- | ----------------------------------- | ------------------------------------------------ |
| `-o <文件名>` | 将下载的内容保存到指定文件中        | `curl -o example.html https://example.com`       |
| `-O`          | 保存文件时使用服务器上的原始文件名  | `curl -O https://example.com/file.zip`           |
| `-L`          | 跟随 HTTP 重定向                    | `curl -L https://example.com/redirect`           |
| `-C -`        | 断点续传                            | `curl -C - -O https://example.com/file.zip`      |
| `-I`          | 仅显示响应头                        | `curl -I https://example.com`                    |
| `-X <方法>`   | 指定 HTTP 方法（如 GET, POST, PUT） | `curl -X DELETE https://example.com/resource`    |
| `-G`          | 将数据作为查询字符串附加到 URL      | `curl -G -d "q=test" https://example.com/search` |

# 8.端口

通俗点说，网络通信就好比两个人互相发送邮件，IP地址为居住的大楼地址，端口号为两个人具体的门牌号，这里的”人“在通信中就是两台计算机中运行的进程/服务。

同一时间，一台计算机内可能会运行多个进程，这些进程占用不同的端口来接收和处理外面发送过来的信息

例如：HTTP服务（80），HTTPS（443），SSH（22），SMTP（25），MySQL（3306）

**Linux中的端口**

Linux系统支持65535个端口，分为三类使用：

- 公认端口（1~1023）：通常用于一些系统内置或知名程序的预留使用（上面举的HTTP这些例子），**非特殊需要，不要占用这些端口**
- 注册端口（1024~49151）：通常可以随意使用，用于松散的绑定一些程序/服务
- 动态端口（49152~65535）：临时使用，不固定绑定某个程序

## 8.1 nmap命令

查看端口占用

```shell
$ nmap ip地址
```

查看本机端口可以使用`127.0.0.1`

## 8.2 netstat命令

查看端口的占用情况

```shell
$ netstat -tuln | grep xxx
```

| **命令**        | **说明**                                                   | **示例**        |
| --------------- | ---------------------------------------------------------- | --------------- |
| `netstat -a`    | 显示所有的网络连接（包括活动的连接和监听的端口）。         | `netstat -a`    |
| `netstat -t`    | 仅显示 TCP 协议的网络连接。                                | `netstat -t`    |
| `netstat -u`    | 仅显示 UDP 协议的网络连接。                                | `netstat -u`    |
| `netstat -n`    | 以数字格式显示地址和端口，不进行域名解析（加快输出）。     | `netstat -n`    |
| `netstat -l`    | 显示所有正在监听的端口（包括 TCP 和 UDP）。                | `netstat -l`    |
| `netstat -p`    | 显示进程 ID 和程序名称（需要 root 权限）。                 | `netstat -p`    |
| `netstat -an`   | 显示所有网络连接和监听的端口，采用数字格式（不解析域名）。 | `netstat -an`   |
| `netstat -tuln` | 显示 TCP 和 UDP 连接，并以数字格式显示端口和地址。         | `netstat -tuln` |
| `netstat -i`    | 显示网络接口的统计信息（如接收和发送的字节数）。           | `netstat -i`    |
| `netstat -s`    | 显示每种协议的统计信息，包括 TCP、UDP、ICMP 等。           | `netstat -s`    |
| `netstat -r`    | 显示路由表信息。                                           | `netstat -r`    |
| `netstat -c`    | 持续更新显示网络连接信息（类似实时监控）。                 | `netstat -c`    |
| `netstat -g`    | 显示多播组的相关信息。                                     | `netstat -g`    |

# 9.进程管理

## 9.1 ps命令

通过ps命令查看进程信息

```bash
$ ps [-e -f]
```

- `-e`：显示全部进程
- `-f`：以完全格式化的形式展示信息（展示全部信息）

```bash
UID          PID    PPID  C STIME TTY          TIME CMD
zyc       279195  279194  0 16:59 pts/1    00:00:00 -bash
zyc       395768  279195  0 20:00 pts/1    00:00:00 ps -f
```

- `PPID`：父进程ID
- `C`：CPU占用率
- `STIME`：启动时间
- `TTY`：启动此进程的终端序号，如显示？，表示非终端启动
- `TIME`：占用CPU的时间
- `CMD`：启动路径或启动命令

## 9.2 kill命令

关闭进程

```shell
$ kill [-9] PID
```

- `-9`：强制关闭进程
  - 如果不使用该选项，则系统向进程发送信号，要求他自行关闭，但**是否关闭需要看进程自身的处理机制**

# 10.主机状态

## 10.1 查看系统资源占用

使用`top`命令，查看CPU、内存使用情况

| **命令选项**      | **说明**                                                     | **示例**      |
| ----------------- | ------------------------------------------------------------ | ------------- |
| `top`             | 启动 `top` 命令，显示系统的实时资源使用情况。                | `top`         |
| `top -d <秒数>`   | 设置刷新间隔时间（单位：秒）。                               | `top -d 2`    |
| `top -u <用户名>` | 仅显示指定用户的进程。                                       | `top -u root` |
| `top -p <PID>`    | 仅显示指定进程 ID（PID）的进程。                             | `top -p 1234` |
| `top -n <次数>`   | 设置 `top` 刷新输出的次数，之后退出。                        | `top -n 10`   |
| `top -b`          | 以批处理模式运行 `top`，用于将输出结果导入文件（不显示交互式界面）。 | `top -b`      |
| `top -c`          | 显示进程的完整命令行（包括命令行参数）。                     | `top -c`      |
| `top -i`          | 仅显示活跃的进程，不显示状态为 `Z`（僵尸）或 `D`（不可中断）等进程。 | `top -i`      |
| `top -H`          | 显示线程级别的进程信息，而不是进程级别的。                   | `top -H`      |
| `top -s`          | 显示进程排序依据（如 CPU、内存、时间等）。                   | `top -s`      |
| `top -S`          | 显示进程的累积时间，而不是实时使用的资源。                   | `top -S`      |
| `top -w`          | 设置终端窗口的宽度（适用于交互模式）。                       | `top -w 80`   |

每5s刷新一次，按`ctrl+c`或`q`退出

```bash
PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                     
3110 zyc      20   0  547936  12288   7680 R  95.4   0.3 128:40.94 gvfsd-trash                                 
375 root      19  -1  325156 206132 205236 R  15.2   5.2  25:32.77 systemd-journal
```

- `PR`：优先级，越小越高
- `NI`：负值表示高优先级，正表示低优先级
- `VIRT`：进程使用虚拟内存，单位KB
- `RES`：进程使用物理内存，单位KB
- `SHR`：进程使用共享内存，单位KB
- `S`：进程状态（`S`休眠，`R`运行，`Z`僵死状态，`N`负数优先级，`I`：空闲状态）
- `%CPU`：进程占用CPU率
- `%MEM`：进程占用内存率
- `TIME+`：进程使用CPU时间总和
- `COMMAND`：进程名称或系统文件路径

## 10.2 磁盘监控

- 使用`df`命令，查看使用情况

  ```bash
  $ df [-h]
  ```

  - `-h`：以更加人性化的单位显示

- 使用`iostat`查看CPU和磁盘的信息

  ```bash
  $ iostat [-x][num1][num2]
  ```

  - `-x`：显示更多信息
  - `num1`：刷新间隔
  - `num2`：刷新几次

  得到很多信息，其中重要的有

  - `rKB/s`：每秒发送到设备的读取请求数
  - `wKB/s`：每秒发送到设备的写入请求数
  - `%util`：磁盘利用率

## 10.3 网络状态监控

使用`sar`命令查看网络的相关统计

```bash
$ sar -n DEV num1 num2
```

- `-n`：查看网络，`DEV`表示查看网络接口

- `num1`：刷新间隔（不填就查看一次结束）
- `num2`：查看次数（不填无限次数）

关键信息：

|   名称   |               描述               |
| :------: | :------------------------------: |
|  IFACE   |        本地网卡接口的名称        |
| rxpck/s  |        每秒钟接受的数据包        |
| txpck/s  |        每秒钟发送的数据包        |
|  rxKB/s  | 每秒钟接受的数据包大小，单位为KB |
|  txKB/s  | 每秒钟发送的数据包大小，单位为KB |
| rxcmp/s  |      每秒钟接受的压缩数据包      |
| txcmp/s  |        每秒钟发送的压缩包        |
| rxmcst/s |      每秒钟接收的多播数据包      |

# 11.环境变量

环境变量时系统在运行时，记录的一些关键性信息，以辅助系统运行（**相当于系统级的全局变量，不被单独的某个程序所独有，任何程序都能访问**）

使用`env`命令查看记录的环境变量，实际的内容为键值对

```bash
PATH=/home/zyc/anaconda3/bin:/home/zyc/anaconda3/condabin:/home/zyc/anaconda3/bin:/home/zyc/anaconda3/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus
SSH_TTY=/dev/pts/1
OLDPWD=/home/zyc
```

---

**PATH变量**

```bash
PATH=/home/zyc/anaconda3/bin:/home/zyc/anaconda3/condabin:/home/zyc/anaconda3/bin:/home/zyc/anaconda3/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
```

PATH记录了系统执行任何命令的搜索路径，例如：执行`cd`命令时，系统会在PATH所指的路径中找`cd`程序

---

**`$`符号**

该符号用于取环境变量的值

```shell
$ echo $PATH
```

返回结果

```shell
(base) zyc@Vmware:~$ echo $PATH
/home/zyc/anaconda3/bin:/home/zyc/anaconda3/condabin:/home/zyc/anaconda3/bin:/home/zyc/anaconda3/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
```

当环境变量与其他的内容混在一起时，使用

```shell
$ echo ${PATH}ABC
```

## 11.1 自行设置环境变量

**临时设置**

```shell
$ export 变量名=变量值
```

**永久生效**

- 针对当前用户生效，配置在当前用户的`~/.bashrc`文件中
  - 在该文件中写`export ...`
- 针对所有用户生效，配置在`/etc/profile`文件中
- 并通过语法`source 配置文件`，进行立刻生效，或重新打开bash生效

---

**案例：编写一个脚本文件，在任何地方都能够执行该文件，且自动输出“hello”**

思路：

- 在任意路径下创建文件`/home/zyc/operate`，在文件中写入`echo hello`
- 将文件设置为可执行`$ chmod 755 operate`
  - 此时，如果不修改环境变量，则只能在该目录下执行`$ ./operate`运行该脚本文件
- 将该文件的路径添加到`PATH`环境变量中`export PATH=${PATH}:${PWD}`
  - 加入环境变量后，可以直接通过`$ operate`来运行该脚本
- 执行该文件

# 12.文件上传和下载

`rz`命令：文件上传，直接输入`rz`命令即可

`sz`命令：文件下载，输入`sz 文件名`

# 13.压缩和解压

常见的几种压缩格式：

- `zip`：对于**Linux**，Windows和MacOS常用
- `7z`：对于Windows常用
- `rar`：对于Windows常用
- `tar`：对于**Linux**和MacOS常用
- `gz`：对于**Linux**和MacOS常用

## 13.1 `tar`和`gz`两种格式

- `tar`：称为`tarball`，归档文件，仅仅简单的讲文件组装到一个`.tar`文件内，并无文件体积的减少
- `gz`：常见为`tar.gz`，使用`gzip`压缩算法（**该算法只支持压缩一个文件**）将文件压缩到一个文件内，可极大减少压缩后的体积

针对这两种格式，使用`tar`命令，**均可以进行压缩和解压缩的操作**

```shell
$ tar [-c -v -x -f -z -C] 参数1 参数2 ... 参数N
```

- `-c`：创建压缩文件，用于压缩模式
- `-v`：显示压缩、解压过程，查看进度
- `-x`：解压模式
- `-f`：要创建的文件，或要解压的文件，此选项**必须在所有选项中处于最后一个**
- `-z`：`gzip`模式，不使用该选项就是普通的`tarball`模式
- `-C`：选择解压的目的地，用于解压模式

**常用组合**

- `tar -cvf test.tar 1.txt 2.txt 3.txt`：将三个文件**组合**到`tar`文件里
- `tar -zcvf test.tar.gz 1.txt 2.txt 3.txt`：将三个文件**压缩**到`.tar.gz`文件里

- `tar -xvf test.tar`：将压缩包解压到当前目录
- `tar -xvf test.tar -C /home/zyc`：解压`tar`压缩包到`/home/zyc`
- `tar -zxvf test.tar.gz -C /home`/zyc：解压`.tar.gz`压缩包到`/home/zyc`

## 13.2 `zip`格式

**压缩**

```bash
$ zip [-r] 参数1 参数2 ...
```

- `-r`：被压缩的内容**包含文件夹**时，需要使用`-r`选项

示例：

- `zip test.zip a.txt b.txt c.txt`：将`a.txt`，`b.txt`和`c.txt`压缩到`test.zip`文件中
- `zip test.zip test a.txt`：将文件夹`test`和文件`a.txt`压缩到`test.zip`中

---

**解压**

```bash
$ unzip [-d] 参数
```

- `-d`：指定要解压的目的地

示例：

- `unzip test.zip`：将`test.zip`解压到当前目录
- `unzip test.zip -d /home/zyc`：将`test.zip`解压到`/home/zyc`

# 14.脚本

如果有一个脚本`xxx.sh`需要在当前shell中执行，需要运行`source xxx.sh`而不是`./xxx.sh`。

因为如果执行后者，脚本中的命令会在**一个新的子 shell 中执行**，一旦脚本结束，你的**当前 shell 不会受到任何影响**
