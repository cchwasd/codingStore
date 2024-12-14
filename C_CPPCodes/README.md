- C/C++ 项目结构

```
├─── build.sh / build.bat
├─── CMakeLists.txt
├─── Makefile
├─── build
├─── include
├─── bin
├─── tests
└─── src

说明 :
1. build 目录为 Cmake 生成的中间路径，存放编译生成的文件
2. build.sh/build.bat 脚本为基于 CMake 的编译脚本
3. include 目录用来存放所有的头文件，第三方依赖项目文件(头文件、库文件)
4. src 目录用来存放所有的源文件，(包含 CMakeLists.txt 文件)
5. bin 目录为本项目编译生成的可执行文件、库文件等
6. tests 目录用来存放所有的测试代码
7. CMakeLists.txt 为项目根路径上的 CMake 配置文件
8. Makefile 为项目根路径上的 make 配置文件

```

编译环境：
Windows: VSCode + MinGW + cmake
