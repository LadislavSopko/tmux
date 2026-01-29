@echo off
setlocal

:: Try VS2026 first, then VS2022
set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo ERROR: Visual Studio not found
    exit /b 1
)

echo === Loading Visual Studio Environment ===
call "%VCVARS%"
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
echo === Running Tests ===
ctest --test-dir build --output-on-failure
set TEST_RESULT=%errorlevel%

echo.
echo === Build Complete ===
echo Executables are in: %~dp0build\bin
dir /b build\bin\*.exe 2>nul

if %TEST_RESULT% neq 0 (
    echo.
    echo WARNING: Some tests failed!
    exit /b %TEST_RESULT%
)

echo.
echo All tests passed!
