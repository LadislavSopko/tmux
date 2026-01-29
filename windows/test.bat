@echo off
:: Run tests for tmux Windows port
:: Usage: test.bat [test_name]
:: Examples: test.bat          (run all)
::           test.bat test_pty (run specific)

setlocal
cd /d "%~dp0"
call env.bat
if errorlevel 1 exit /b 1

if "%1"=="" (
    echo.
    echo === Running All Tests ===
    ctest --test-dir build --output-on-failure
) else (
    echo.
    echo === Running %1 ===
    build\bin\%1.exe
)
