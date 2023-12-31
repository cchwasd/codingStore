@echo off setlocal
set "folder=.\build"
for /D %%i in ("%folder%\*.*") do (
    del /s /q %%i\*
    rd /s /q %%i
)
del /s /q "%folder%"

setlocal

cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
cd ..
