@echo off

setlocal
set "folder=.\build"
for /D %%i in ("%folder%\*.*") do (
    del /s /q %%i\*
    rd /s /q %%i
)
del /s /q "%folder%"



cmake -S .\ -B .\build  -G "MinGW Makefiles"
pushd .\build
mingw32-make
popd

setlocal