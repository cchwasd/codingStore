@echo off
echo ���߳�SocketͨѶ��Ŀ����ű�
echo ================================

REM ����Ƿ����buildĿ¼������������򴴽�
if not exist "build" (
    echo ����buildĿ¼...
    mkdir build
)

REM ����buildĿ¼
cd build

REM ���ɹ����ļ�
echo ����CMake�����ļ�...
REM cmake .. -G "Visual Studio 16 2019" -A x64
cmake .. -G "MinGW Makefiles"

REM ���CMake�Ƿ�ɹ�
if %ERRORLEVEL% NEQ 0 (
    echo CMake����ʧ�ܣ�
    pause
    exit /b 1
)

REM ������Ŀ
echo ������Ŀ...
cmake --build . --config Release
REM cmake --build .

REM �������Ƿ�ɹ�
if %ERRORLEVEL% NEQ 0 (
    echo ����ʧ�ܣ�
    pause
    exit /b 1
)

echo ����ɹ���
echo ��ִ���ļ�λ��: build\bin\Release\socket_test.exe

REM ѯ���Ƿ����г���
echo.
set /p choice="�Ƿ����г���(y/n): "
if /i "%choice%"=="y" (
    echo ��������...
    bin\Release\socket_test.exe
)

pause 