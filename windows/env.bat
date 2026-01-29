@echo off
:: Environment setup for tmux Windows port
:: Usage: call env.bat
:: This sets up VS environment and VCPKG_ROOT

if defined TMUX_ENV_LOADED goto :eof

:: Find Visual Studio
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

echo [ENV] Loading Visual Studio...
call "%VCVARS%"
if errorlevel 1 (
    echo [ENV] Failed to load VS environment
    exit /b 1
)

:: VCPKG - inside project thirdparty folder
set "VCPKG_ROOT=%~dp0thirdparty\vcpkg"
set "CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

:: Mark as loaded (avoid reloading in same session)
set "TMUX_ENV_LOADED=1"

echo [ENV] Ready. VCPKG_ROOT=%VCPKG_ROOT%
