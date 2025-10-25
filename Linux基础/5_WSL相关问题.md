# WSL2 迁移到 D 盘（或其他盘符）完整指南

本文档将指导你如何使用 `wsl` 命令自带的 `export` 和 `import` 功能，将已安装的 WSL2 发行版（如 Ubuntu）从 C 盘安全迁移到 D 盘。

这个过程的核心是：**备份 -> 卸载 -> 恢复到新位置**。

**请在 Windows PowerShell 或 CMD 中执行以下所有命令。**

## 第一步：准备工作

在开始迁移之前，我们需要获取发行版名称并确保它已关闭。

### 1. 查看已安装的发行版

你需要知道你要迁移的发行版的确切名称。

```
wsl -l -v
```

你会看到类似这样的输出，请记下 `NAME` 列中的名称（本例中为 `Ubuntu`）：

```
  NAME      STATE           VERSION
* Ubuntu    Stopped         2
```

### 2. 关闭所有 WSL 实例

为确保导出过程顺利，请关闭所有正在运行的 WSL 实例。

```
wsl --shutdown
```

## 第二步：导出 (备份)

此步骤会将你当前的 WSL 系统打包为一个 `.tar` 备份文件。请确保目标位置（如 D 盘）有足够的空间。

1. 在 D 盘创建一个用于存放备份文件的文件夹（例如 `D:\wsl-backup`）：

   ```
   mkdir D:\wsl-backup
   ```

2. 执行导出命令：

   ```
   # 语法: wsl --export <发行版名称> <导出的.tar文件路径>
   # 示例:
   wsl --export Ubuntu D:\wsl-backup\ubuntu.tar
   ```

   *请将 `Ubuntu` 替换为你的发行版名称。* *此过程可能需要几分钟，具体取决于你的系统大小。*

## 第三步：注销 (卸载 C 盘原版)

**确认上一步的 `.tar` 文件已成功生成后**，执行此操作。这将从 C 盘删除原来的 WSL 系统并释放空间。

```
# 语法: wsl --unregister <发行版名称>
# 示例:
wsl --unregister Ubuntu
```

*执行后，你可以再次运行 `wsl -l -v` 确认该发行版已被卸载。*

## 第四步：导入 (恢复到 D 盘)

现在，我们将备份文件重新导入到你指定D盘新位置。

1. 在 D 盘为你D盘的WSL创建一个新家（例如 `D:\WSL\Ubuntu`），这是未来 WSL 系统文件 (`.vhdx`) **实际存放**的地方：

   ```
   mkdir D:\WSL\Ubuntu
   ```

2. 执行导入命令：

   ```
   # 语法: wsl --import <新发行版名称> <安装位置> <tar文件路径> --version 2
   # 示例:
   wsl --import Ubuntu D:\WSL\Ubuntu D:\wsl-backup\ubuntu.tar --version 2
   ```

   - **新发行版名称**: 可以和原来一样（例如 `Ubuntu`）。
   - **安装位置**: 你在上一步创建的文件夹（`D:\WSL\Ubuntu`）。
   - **tar文件路径**: 你在第二步中导出的备份文件。
   - `--version 2`: 必须指定，以确保使用 WSL2。

## 第五步：迁移后收尾工作 (重要)

迁移已完成，但还有最后一步配置需要处理。

### 1. 修复默认登录用户

通过 `import` 恢复的系统，默认会使用 `root` 用户登录，而不是你原来的用户。我们需要将其改回。

1. 首先，以 `root` 身份登录你的 WSL：

   ```
   # 将 Ubuntu 替换为你的发行版名称
   wsl -d Ubuntu -u root
   ```

2. 在 `root` 终端中（提示符为 `#`），使用 `nano` 或 `vi` 编辑器创建或修改 `/etc/wsl.conf` 文件：

   ```
   nano /etc/wsl.conf
   ```

3. 将以下内容**准确**地输入到文件中。如果你之前配置过 `systemd`，请确保格式正确：

   ```
   [user]
   default = your_username
   
   [boot]
   systemd = true
   ```

   - **重要**：将 `your_username` 替换为你**原来**的 Linux 用户名（例如 `peter`, `john`, `zyc` 等）。
   - 如果你不需要 `systemd`，可以只保留 `[user]` 部分。
   - 确保 `[user]` 和 `[boot]` 是顶格写的，它们是不同的配置节。

4. 保存文件并退出。 *(在 `nano` 中: 按 `Ctrl+X` -> 按 `Y` -> 按 `Enter`)*

5. 退出 `root` 终端：

   ```
   exit
   ```

6. **回到 PowerShell/CMD**，再次执行 `wsl --shutdown` 来让新配置生效：

   ```
   wsl --shutdown
   ```

7. 现在，当你再次启动 `wsl` 或 `wsl -d Ubuntu` 时，就应该会以你的默认用户（`your_username`）登录了。

### 2. 设置默认发行版 (可选)

如果你有多个 WSL 发行版，你可能希望将这个新迁移的设置为默认（即在 PowerShell 中输入 `wsl` 时自动进入的系统）。

```
wsl -s Ubuntu
```

### 3. 清理备份文件 (可选)

确认 D 盘的 WSL 一切运行正常后，你可以删除第二步中生成的临时 `.tar` 备份文件以节省空间。

```
del D:\wsl-backup\ubuntu.tar
```