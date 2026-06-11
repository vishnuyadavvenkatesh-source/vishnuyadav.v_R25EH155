@echo off
echo Compiling 2D Graphics Editor...
gcc -Wall -Wextra -std=c99 -o editor.exe editor.c
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)
echo Compilation successful. Running editor...
editor.exe
