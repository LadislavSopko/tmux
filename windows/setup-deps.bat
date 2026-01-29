@echo off
:: Install dependencies via vcpkg
:: Usage: setup-deps.bat
:: Installs vcpkg and dependencies in windows/thirdparty/

setlocal
cd /d "%~dp0"
call env.bat
if errorlevel 1 exit /b 1

:: Create thirdparty folder if needed
if not exist "%~dp0thirdparty" mkdir "%~dp0thirdparty"

:: Install vcpkg if needed
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo.
    echo === Installing vcpkg ===
    cd /d "%~dp0thirdparty"
    git clone https://github.com/Microsoft/vcpkg.git
    if errorlevel 1 (
        echo Failed to clone vcpkg
        exit /b 1
    )
    cd /d "%VCPKG_ROOT%"
    call bootstrap-vcpkg.bat -disableMetrics
    if errorlevel 1 (
        echo Failed to bootstrap vcpkg
        exit /b 1
    )
    cd /d "%~dp0"
)

:: Install libevent
echo.
echo === Checking libevent ===
"%VCPKG_ROOT%\vcpkg.exe" list | findstr /C:"libevent:x64-windows" >nul
if errorlevel 1 (
    echo Installing libevent...
    "%VCPKG_ROOT%\vcpkg.exe" install libevent:x64-windows
    if errorlevel 1 (
        echo Failed to install libevent
        exit /b 1
    )
) else (
    echo libevent OK
)

:: Install pdcurses
echo.
echo === Checking pdcurses ===
"%VCPKG_ROOT%\vcpkg.exe" list | findstr /C:"pdcurses:x64-windows" >nul
if errorlevel 1 (
    echo Installing pdcurses...
    "%VCPKG_ROOT%\vcpkg.exe" install pdcurses:x64-windows
    if errorlevel 1 (
        echo Failed to install pdcurses
        exit /b 1
    )
) else (
    echo pdcurses OK
)

echo.
echo === Dependencies installed ===
