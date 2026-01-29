@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo Failed to load VS environment
    exit /b 1
)

cd /d "%~dp0"
echo.
echo === Configuring CMake ===
cmake -B build -G Ninja
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
echo Executables are in: %~dp0build\bin
dir /b build\bin\*.exe 2>nul
