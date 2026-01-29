# Build script for libevent POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building libevent POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Try to find libevent via vcpkg
$vcpkgRoot = $env:VCPKG_ROOT
if (-not $vcpkgRoot) {
    $vcpkgRoot = "C:\vcpkg"
}

$libeventInclude = "$vcpkgRoot\installed\x64-windows\include"
$libeventLib = "$vcpkgRoot\installed\x64-windows\lib"

# Check if libevent is installed via vcpkg
if (Test-Path $libeventInclude) {
    Write-Host "[+] Found libevent via vcpkg" -ForegroundColor Green
    $includeFlag = "/I`"$libeventInclude`""
    $libPath = "/LIBPATH:`"$libeventLib`""
} else {
    # Check for local headers/libs
    if (Test-Path "event2") {
        Write-Host "[+] Found local libevent headers" -ForegroundColor Green
        $includeFlag = "/I."
        $libPath = ""
    } else {
        Write-Host "[-] libevent not found!" -ForegroundColor Red
        Write-Host ""
        Write-Host "[!] Install libevent via vcpkg:" -ForegroundColor Yellow
        Write-Host "    vcpkg install libevent:x64-windows" -ForegroundColor White
        Write-Host ""
        Write-Host "[!] Or download and extract libevent headers/libs to this directory" -ForegroundColor Yellow
        exit 1
    }
}

# Compile
Write-Host "[+] Compiling..." -ForegroundColor White

$compileCmd = "cl.exe /W4 $includeFlag /Fe:libevent_poc.exe libevent_poc.c /link $libPath event.lib event_core.lib ws2_32.lib advapi32.lib"
Write-Host "[+] Command: $compileCmd" -ForegroundColor Gray

Invoke-Expression $compileCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Build successful: libevent_poc.exe" -ForegroundColor Green
    Write-Host "[+] Run with: .\libevent_poc.exe" -ForegroundColor Cyan
} else {
    Write-Host "[-] Build failed" -ForegroundColor Red
    Write-Host ""
    Write-Host "[!] If linking fails, ensure libevent DLLs are in PATH or same directory" -ForegroundColor Yellow
    exit 1
}
