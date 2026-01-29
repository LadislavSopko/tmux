# Build script for Console Events POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building Console Events POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Compile
cl.exe /W4 /Fe:events_poc.exe events_poc.c

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Build successful: events_poc.exe" -ForegroundColor Green
    Write-Host "[+] Run with: .\events_poc.exe" -ForegroundColor Cyan
    Write-Host "[+] Then try Ctrl+C, resize window, Ctrl+Break to exit" -ForegroundColor Yellow
} else {
    Write-Host "[-] Build failed" -ForegroundColor Red
    exit 1
}
