# 1 SSH 是什么

SSH 是 **Linux 系统的登录工具**，现在广泛用于**服务器登录**和各种**加密通信**。

历史上，网络主机之间的通信是不加密的，属于明文通信。这使得通信很不安全，一个典型的例子就是服务器登录。登录远程服务器的时候，需要将用户输入的密码传给服务器，如果这个过程是明文通信，就意味着传递过程中，线路经过的中间计算机都能看到密码，这是很可怕的。

SSH 就是为了解决这个问题而诞生的，它能够加密计算机之间的通信，保证不被窃听或篡改。它还能对操作者进行认证（authentication）和授权（authorization）。**明文的网络协议可以套用在它里面，从而实现加密。**

# 2 SSH 架构

SSH 的软件架构是服务器-客户端模式（Server - Client）。在这个架构中，SSH 软件分成两个部分：向服务器发出请求的部分，称为客户端（client），OpenSSH 的实现为 ssh；接收客户端发出的请求的部分，称为服务器（server），OpenSSH 的实现为 sshd。

**本质上就是**：如果PC要和服务器进行会话（比如：建立连接后发送linux指令给服务器），就需要通过一个软件首先和服务器对应的软件建立连接，然后指令通过该软件发送到远程的linux系统中对应的用于接收指令的软件，然后系统执行该指令。

所以，在**PC上运行的软件被称为SSH客户端（用于发送指令和数据，以及接收服务器信息）**，**运行在Linux服务器上的软件被称为SSH服务器（用于接收指令和数据，以及返回信息）**。

本教程约定，大写的 SSH 表示协议，小写的 ssh 表示客户端软件。

# 3 SSH 客户端

OpenSSH的客户端是二进制程序 ssh。它在 Linux/Unix 系统的位置是`/usr/local/bin/ssh`，Windows 系统的位置是`\Program Files\OpenSSH\bin\ssh.exe`。

## 3.1 基本用法

ssh 最常见的用途就是登录服务器，这要求服务器安装并正在运行 SSH 服务器软件。

ssh 登录服务器的命令如下。

```shell
$ ssh hostname 
```

上面命令中，`hostname`是主机名，它可以是**域名**，也可能是 **IP 地址**或**局域网内部的主机名**。

**不指定用户名的情况下**，将**使用客户端的当前用户名**，作为远程服务器的登录用户名。如果要指定用户名，可以采用下面的语法。

如果要指定用户名，可以采用下面的语法。

```shell
$ ssh user@hostname 
```

用户名也可以使用`ssh`的`-l`参数指定，这样的话，用户名和主机名就不用写在一起了。

```shell
$ ssh -l username host 
```

ssh 默认连接服务器的22端口，`-p`参数可以指定其他端口。

```shell
$ ssh -p 8821 foo.com 
```

上面命令连接服务器`foo.com`的8821端口。

## 3.2 连接流程

---

**公钥和私钥**

- **公钥（Public Key）**：公开的密钥，可以分享给任何人。它通常存放在服务器上，用于加密连接请求或验证请求是否由拥有匹配私钥的人发出。

- **私钥（Private Key）**：私密的密钥，仅存放在客户端设备上且不能与他人共享。私钥用于解密由公钥加密的数据或证明身份。

可以将公钥和密钥想象成**一把完整的钥匙拆成两部分**，一部分自己保留（私钥），另一部分分发出去（公钥），在建立连接时，只有对方的手上的公钥和自己的私钥匹配，才能成功建立连接。

**注：公钥和私钥只用于建立连接，数据传输的加密使用的时一个临时密钥**

**SSH 公钥认证的工作原理**

**本质**：<u>连接建立的过程就是公钥和私钥配对的过程</u>

1. **客户端生成密钥对**：**客户端**用户生成一对密钥（公钥和私钥），例如在 Linux 中可以使用以下命令生成密钥对：

   ```shell
   $ ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
   ```

   - `-t rsa`：指定使用RSA算法生成密钥。

   - `-b 4096`：指定密钥长度为4096位。

   - `-C "your_email@example.com"`：在密钥中加入注释（一般是你的邮箱）

   这会在用户的 `~/.ssh/` 目录下创建 `id_rsa`（私钥）和 `id_rsa.pub`（公钥）。

2. **客户端生成密钥对**：服务器同样会生成一对密钥，公钥用于客户端验证服务器的身份。服务器的公钥通常存储在 `/etc/ssh/ssh_host_rsa_key.pub` 或类似位置。

3. **客户端上传公钥**：要使用密钥进行SSH连接，需要将公钥添加到服务器上的 `authorized_keys` 文件中

   ```shell
   $ ssh-copy-id -i ~/.ssh/id_rsa.pub username@remote_host
   ```

   - `-i ~/.ssh/id_rsa.pub`：公钥文件路径
   - `username@remote_host`：远程服务器的用户名和IP地址或域名

   用户将公钥（`id_rsa.pub`）上传到 SSH 服务器上的特定位置（通常是 `~/.ssh/authorized_keys` 文件）。服务器会将这个公钥用于识别合法用户。

4. **连接过程**：当你成功将公钥添加到服务器后，就可以使用私钥进行登录了

   ```shell
   $ ssh -i ~/.ssh/id_rsa username@remote_host
   ```

   - 如果没有指定 `-i` 参数，SSH会默认使用 `~/.ssh/id_rsa` 作为私钥进行连接
   - 服务器收到请求后，服务器会生成一个随机的挑战信息（challenge），发送给客户端
   - 客户端用自己的（非服务器的）私钥对该挑战进行签名，然后将签名结果发送回服务器
   - 服务器使用存储在 `authorized_keys` 文件中的（客户端的）公钥验证（**客户端签名，服务端验证**）
   - 如果验证成功，连接被允许

5. **通信过程**：

   - 身份验证完成后，客户端和服务器会协商出一个**对称加密的会话密钥**。这个会话密钥是临时的，通常每次连接都会重新生成一次。生成后，这个密钥会被用于后续的数据传输加密。
   - 一旦协商出会话密钥，后续的数据传输就会使用此对称密钥进行加密。**对称加密**（例如 AES）比非对称加密效率更高，因此适合加密大量数据传输。

6. **服务器指纹**：服务器公钥的哈希值

   一旦与服务器建立连接，客户端将当前服务器的指纹也储存在本机`~/.ssh/known_hosts`文件中。

   其作用就是看是否之前连接过某个服务器的公钥是什么，如果服务器公钥改了，客户端很快就能知道，此时就可以判断新的公钥是否可以信任，还是被恶意篡改。

   ssh 连接远程服务器后，首先有一个验证过程，验证远程服务器是否为陌生地址。

   如果是第一次连接某一台服务器，命令行会显示一段文字，表示不认识这台机器，提醒用户确认是否需要连接。

   ```
   The authenticity of host 'foo.com (192.168.121.111)' can't be established.
   ECDSA key fingerprint is SHA256:Vybt22mVXuNuB5unE++yowF7lgA/9/2bLSiO3qmYWBY.
   Are you sure you want to continue connecting (yes/no)? 
   ```

   上面这段文字告诉用户，`foo.com`这台服务器的指纹是陌生的，让用户选择是否要继续连接（输入 yes 或 no）。

   ssh 会将本机连接过的所有服务器公钥的指纹，都储存在本机的`~/.ssh/known_hosts`文件中。每次连接服务器时，通过该文件判断是否为陌生主机（陌生公钥）。

   在上面这段文字后面，输入`yes`，就可以将当前服务器的指纹也储存在本机`~/.ssh/known_hosts`文件中，并显示下面的提示。以后再连接的时候，就不会再出现警告了。

   ```
   Warning: Permanently added 'foo.com (192.168.121.111)' (RSA) to the list of known hosts 
   ```

   然后，客户端就会跟服务器建立连接。接着，ssh 就会要求用户输入所要登录账户的密码。用户输入并验证密码正确以后，就能登录远程服务器的 Shell 了。

---

## 3.3 服务器密钥变更

服务器指纹可以防止有人恶意冒充远程主机（一种用于验证服务器身份的手段）。如果服务器的密钥发生变更（比如重装了 SSH 服务器），客户端再次连接时，就会发生公钥指纹不吻合的情况。这时，客户端就会中断连接，并显示一段警告信息。

```
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!
Someone could be eavesdropping on you right now (man-in-the-middle attack)!
It is also possible that the RSA host key has just been changed.
The fingerprint for the RSA key sent by the remote host is
77:a5:69:81:9b:eb:40:76:7b:13:04:a9:6c:f4:9c:5d.
Please contact your system administrator.
Add correct host key in /home/me/.ssh/known_hosts to get rid of this message.
Offending key in /home/me/.ssh/known_hosts:36 
```

上面这段文字的意思是，该主机的公钥指纹跟`~/.ssh/known_hosts`文件储存的不一样，必须处理以后才能连接。这时，你需要确认是什么原因，使得公钥指纹发生变更，到底是恶意劫持，还是管理员变更了 SSH 服务器公钥。

如果新的公钥确认可以信任，需要继续执行连接，你可以执行下面的命令，将原来的公钥指纹从`~/.ssh/known_hosts`文件删除。

```
$ ssh-keygen -R hostname 
```

上面命令中，`hostname`是发生公钥变更的主机名。

删除了原来的公钥指纹以后，重新执行 ssh 命令连接远程服务器，将新的指纹加入`known_hosts`文件，就可以顺利连接了。

## 3.4 执行远程命令

SSH 登录成功后，用户就进入了远程主机的命令行环境，所看到的提示符，就是远程主机的提示符。这时，你就可以输入想要在远程主机执行的命令。

另一种执行远程命令的方法，是将命令直接写在`ssh`命令的后面。

```
$ ssh username@hostname command 
```

上面的命令会使得 SSH 在登录成功后，立刻在远程主机上执行命令`command`。

下面是一个例子。

```
$ ssh foo@server.example.com cat /etc/hosts 
```

上面的命令会在登录成功后，立即远程执行命令`cat /etc/hosts`。

采用这种语法执行命令时，ssh 客户端不会提供互动式的 Shell 环境，而是直接将远程命令的执行结果输出在命令行。但是，有些命令需要互动式的 Shell 环境（例如输入密码、确认操作等），这时就要使用`-t`参数。

```shell
# 报错 
$ ssh remote.server.com emacs

emacs: standard input is not a tty 

# 不报错 
$ ssh -t server.example.com emacs 
```

上面代码中，`emacs`命令需要一个互动式 Shell，所以报错。只有加上`-t`参数，ssh 才会分配一个互动式 Shell