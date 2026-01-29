# Build script for ConPTY POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building ConPTY POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    Write-Host "    Or run: " -ForegroundColor Yellow
    Write-Host '    & "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1"' -ForegroundColor White
    exit 1
}

# Compile
cl.exe /W4 /Fe:conpty_poc.exe conpty_poc.c

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Build successful: conpty_poc.exe" -ForegroundColor Green
    Write-Host "[+] Run with: .\conpty_poc.exe" -ForegroundColor Cyan
    Write-Host "[+] Or with PowerShell: .\conpty_poc.exe powershell.exe" -ForegroundColor Cyan
} else {
    Write-Host "[-] Build failed" -ForegroundColor Red
    exit 1
}
