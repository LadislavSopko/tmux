@echo off
:: Build tmux Windows port
:: Usage: build.bat
:: Prerequisite: run setup-deps.bat first (once)

setlocal
cd /d "%~dp0"
call env.bat
if errorlevel 1 exit /b 1

echo.
echo === Configuring CMake ===
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="%CMAKE_TOOLCHAIN_FILE%"
if errorlevel 1 (
    echo CMake configure failed
    exit /b 1
)

echo.
echo === Building ===
cmake --build build
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo.
echo === Build Complete ===
echo Executables: %~dp0build\bin
dir /b build\bin\*.exe 2>nul
