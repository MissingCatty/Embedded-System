# 1.移植步骤

## 1.1 下载源码

进入lvgl官方github仓库：[lvgl/lvgl: Embedded graphics library to create beautiful UIs for any MCU, MPU and display type.](https://github.com/lvgl/lvgl)

去release中下载最新版的源码并解压。

将源码中的`./lv_conf.h`，`./lv_version.h`，`./lvgl.h`，和`./src`目录全部复制到Keil项目目录`Lib/lvgl`中

**注：所有源文件都要加到目录里，但可以挑选部分.c文件进入项目结构中**

## 1.2 保留移植文件

1. 修改`lv_conf_template.h`为`lv_conf.h`
2. 打开`lv_conf.h`，修改宏为1以启用该配置文件
3. `lv_conf_internal.h`文件中`LV_CONF_INCLUDE_SIMPLE`，在keil项目中添加宏`LV_CONF_INCLUDE_SIMPLE`

## 1.3 在Keil中添加.c文件

添加的源文件包含：

```
./src/*.c
./src/core/*.c
./src/display/*.c
./src/draw/*.c
./src/draw/sw/*.c
./src/draw/sw/blend/*.c
./src/drivers/display/ili9341/*.c [这是对应的显示屏，按需更换驱动代码]
./src/font/*.c
./src/indev/*.c
./src/layouts/*.c
./src/layouts/flex/*.c
./src/layouts/grid/*.c
./src/libs/bin_decoder/*.c
./src/misc/*.c
./src/misc/cache/*.c
./src/osal/cache/lv_os.c + lv_freertos.c + lv_os_none.c
./src/stdlib/*.c
./src/stdlib/builtin/*.c
./src/stdlib/clib/*.c
./src/themes/default + mono + simple/*c
./src/tick/*.c
./src/widgets/*.c
```

**注：选择widgets引入某个控件的时候需要在`lv_conf.h`文件里打开对应的宏**