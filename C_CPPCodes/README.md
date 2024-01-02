- C/C++ 项目结构

```
+---build.sh / build.bat
+---CMakeLists.txt
+---Makefile
+---build
+---include
|   +---googletest
|   |   +---gmock
|   |   |   \---internal
|   |   |       \---custom
|   |   +---gtest
|   |   |   \---internal
|   |   |       \---custom
|   |   \---lib
|   +---nlohmann
|   +---spdlog
|   |   +---cfg
|   |   +---details
|   |   +---fmt
|   |   |   \---bundled
|   |   +---lib
|   |   |   +---cmake
|   |   |   |   \---spdlog
|   |   |   \---pkgconfig
|   |   \---sinks
|   \---sqlite3
|       \---lib
+---out
|   \---log
+---resources
+---src
\---tst

说明 :
1. build 目录为 Cmake 生成的中间路径，存放编译生成的文件
2. build.sh 脚本为基于 CMake 的编译脚本
3. include 目录为本项目中的头文件，第三方依赖项目文件(头文件、库文件)
4. src 目录为本项目中的源文件，(包含 CMakeLists.txt 文件)
5. out 目录为本项目编译生成的可执行文件、库文件等
6. tst 目录为本项目测试代码路径，(包含 CMakeLists.txt 文件)
7. CMakeLists.txt 为项目根路径上的 CMake 配置文件
8. Makefile 为项目根路径上的 make 配置文件
9. resources 目录为资源文件路径

```

- 运行环境

```sh
>g++ --version # mingw64
g++ (x86_64-posix-seh-rev0, Built by MinGW-Builds project) 13.2.0

>cmake --version
cmake version 3.26.3

VSCode V1.75

boost: V1.84.0
googletest: v1.14.0
spdlog: v1.12.0
sqlite: v3.34
nlohmann: v3.11.3

```

