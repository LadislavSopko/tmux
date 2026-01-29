# Setup script to download and build PDCursesMod
# Run from Developer PowerShell for VS

$ErrorActionPreference = "Stop"

Write-Host "[+] PDCursesMod Setup Script" -ForegroundColor Cyan

# Check if nmake is available
$nmake = Get-Command nmake.exe -ErrorAction SilentlyContinue
if (-not $nmake) {
    Write-Host "[-] nmake.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Check if git is available
$git = Get-Command git.exe -ErrorAction SilentlyContinue
if (-not $git) {
    Write-Host "[-] git.exe not found" -ForegroundColor Red
    exit 1
}

# Clone PDCursesMod if not present
if (-not (Test-Path "PDCursesMod")) {
    Write-Host "[+] Cloning PDCursesMod..." -ForegroundColor White
    git clone https://github.com/Bill-Gray/PDCursesMod.git
} else {
    Write-Host "[+] PDCursesMod directory exists" -ForegroundColor Yellow
}

# Build wincon version
Write-Host "[+] Building PDCursesMod (wincon)..." -ForegroundColor White
Push-Location PDCursesMod\wincon

nmake -f Makefile.vc clean 2>$null
nmake -f Makefile.vc

if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Copy files
Write-Host "[+] Copying files..." -ForegroundColor White
Copy-Item "PDCursesMod\wincon\pdcurses.lib" "." -Force
Copy-Item "PDCursesMod\curses.h" "." -Force
Copy-Item "PDCursesMod\panel.h" "." -Force

Write-Host "[+] Setup complete!" -ForegroundColor Green
Write-Host "[+] Now run: .\build.ps1" -ForegroundColor Cyan
