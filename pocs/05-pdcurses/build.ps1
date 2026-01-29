# Build script for PDCurses POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building PDCurses POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Check for PDCurses library
if (-not (Test-Path "pdcurses.lib")) {
    Write-Host "[-] pdcurses.lib not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "[!] You need to build PDCursesMod first:" -ForegroundColor Yellow
    Write-Host "    1. git clone https://github.com/Bill-Gray/PDCursesMod.git" -ForegroundColor White
    Write-Host "    2. cd PDCursesMod\wincon" -ForegroundColor White
    Write-Host "    3. nmake -f Makefile.vc" -ForegroundColor White
    Write-Host "    4. copy pdcurses.lib <this-directory>\" -ForegroundColor White
    Write-Host "    5. copy ..\curses.h <this-directory>\" -ForegroundColor White
    Write-Host ""
    exit 1
}

# Check for header
if (-not (Test-Path "curses.h")) {
    Write-Host "[-] curses.h not found!" -ForegroundColor Red
    Write-Host "[!] Copy curses.h from PDCursesMod directory" -ForegroundColor Yellow
    exit 1
}

# Compile
Write-Host "[+] Compiling..." -ForegroundColor White
cl.exe /W4 /I. /Fe:curses_poc.exe curses_poc.c pdcurses.lib user32.lib advapi32.lib

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Build successful: curses_poc.exe" -ForegroundColor Green
    Write-Host "[+] Run with: .\curses_poc.exe" -ForegroundColor Cyan
    Write-Host "[+] Use arrow keys to move, 'q' to quit" -ForegroundColor Yellow
} else {
    Write-Host "[-] Build failed" -ForegroundColor Red
    exit 1
}
