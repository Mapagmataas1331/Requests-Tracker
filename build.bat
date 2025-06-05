@echo off
setlocal enabledelayedexpansion

if not exist build (
    mkdir build
)

cd build
echo.

cmake -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 (
    echo CMake configuration failed.
    pause >nul
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause >nul
    exit /b 1
)

cd ..

if not exist .out (
    mkdir .out
)

xcopy /Y /E /I build\\Release\\* .out\\

echo.
echo Build and copy completed!
echo Press any Button to run program.
pause >nul

cd .out
cls

echo.
echo Directory contents:
dir /b .
echo.
echo Program started.
echo ------------------------------
RequestTracker.exe
echo ------------------------------
echo Program exited.
pause >nul