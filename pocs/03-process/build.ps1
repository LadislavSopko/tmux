# Build script for Process Management POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building Process Management POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Compile
cl.exe /W4 /Fe:process_poc.exe process_poc.c

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Build successful: process_poc.exe" -ForegroundColor Green
    Write-Host "[+] Run with: .\process_poc.exe" -ForegroundColor Cyan
} else {
    Write-Host "[-] Build failed" -ForegroundColor Red
    exit 1
}
