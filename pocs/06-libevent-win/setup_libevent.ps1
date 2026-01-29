# Setup script for libevent via vcpkg
# Run from Developer PowerShell for VS

$ErrorActionPreference = "Stop"

Write-Host "[+] libevent Setup Script (via vcpkg)" -ForegroundColor Cyan

# Check if vcpkg exists
$vcpkgExe = $null
if ($env:VCPKG_ROOT -and (Test-Path "$env:VCPKG_ROOT\vcpkg.exe")) {
    $vcpkgExe = "$env:VCPKG_ROOT\vcpkg.exe"
} elseif (Test-Path "C:\vcpkg\vcpkg.exe") {
    $vcpkgExe = "C:\vcpkg\vcpkg.exe"
} else {
    Write-Host "[-] vcpkg not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "[!] To install vcpkg:" -ForegroundColor Yellow
    Write-Host "    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg" -ForegroundColor White
    Write-Host "    cd C:\vcpkg" -ForegroundColor White
    Write-Host "    .\bootstrap-vcpkg.bat" -ForegroundColor White
    Write-Host ""
    Write-Host "[!] Then run this script again" -ForegroundColor Yellow
    exit 1
}

Write-Host "[+] Found vcpkg: $vcpkgExe" -ForegroundColor Green

# Install libevent
Write-Host "[+] Installing libevent:x64-windows..." -ForegroundColor White
& $vcpkgExe install libevent:x64-windows

if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] vcpkg install failed" -ForegroundColor Red
    exit 1
}

# Integrate
Write-Host "[+] Running vcpkg integrate..." -ForegroundColor White
& $vcpkgExe integrate install

Write-Host ""
Write-Host "[+] Setup complete!" -ForegroundColor Green
Write-Host "[+] Now run: .\build.ps1" -ForegroundColor Cyan
