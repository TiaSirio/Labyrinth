@echo off
cd /d "%~dp0"
cd ../../..

cmake -S . -B ./build/ -G "MinGW Makefiles"
cd build/
mingw32-make.exe && mingw32-make.exe Shaders