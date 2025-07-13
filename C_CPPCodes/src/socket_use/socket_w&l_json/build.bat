@echo off
echo 多线程Socket通讯项目编译脚本
echo ================================

REM 检查是否存在build目录，如果不存在则创建
if not exist "build" (
    echo 创建build目录...
    mkdir build
)

REM 进入build目录
cd build

REM 生成构建文件
echo 生成CMake构建文件...
REM cmake .. -G "Visual Studio 16 2019" -A x64
cmake .. -G "MinGW Makefiles"

REM 检查CMake是否成功
if %ERRORLEVEL% NEQ 0 (
    echo CMake生成失败！
    pause
    exit /b 1
)

REM 编译项目
echo 编译项目...
cmake --build . --config Release
REM cmake --build .

REM 检查编译是否成功
if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo 编译成功！
echo 可执行文件位置: build\bin\Release\socket_test.exe

REM 询问是否运行程序
echo.
set /p choice="是否运行程序？(y/n): "
if /i "%choice%"=="y" (
    echo 启动程序...
    bin\Release\socket_test.exe
)

pause 