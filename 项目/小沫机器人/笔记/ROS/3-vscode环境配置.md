在`~/catkin_ws/.vscode/c_cpp_properties.json`里放入ros的头文件：

```JSON
{
  "configurations": [
    {
      "name": "ROS",
      "compilerPath": "/usr/bin/g++",
      "includePath": [
        "/opt/ros/noetic/include/**",
        // "${workspaceFolder}/devel/include/**",
        "${workspaceFolder}/src/**"
      ],
      "defines": [],
      "cStandard": "c11",
      "cppStandard": "c++14",
      "intelliSenseMode": "linux-gcc-x64"
    }
  ],
  "version": 4
}
```