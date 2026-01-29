# Build script for Named Pipes POC
# Run from Developer PowerShell for VS or with cl.exe in PATH

$ErrorActionPreference = "Stop"

Write-Host "[+] Building Named Pipes POC..." -ForegroundColor Cyan

# Check if cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "[-] cl.exe not found. Please run from Developer PowerShell for VS" -ForegroundColor Red
    exit 1
}

# Compile server
Write-Host "[+] Compiling server..." -ForegroundColor White
cl.exe /W4 /Fe:pipe_server.exe pipe_server.c
if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] Server build failed" -ForegroundColor Red
    exit 1
}

# Compile client
Write-Host "[+] Compiling client..." -ForegroundColor White
cl.exe /W4 /Fe:pipe_client.exe pipe_client.c
if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] Client build failed" -ForegroundColor Red
    exit 1
}

Write-Host "[+] Build successful!" -ForegroundColor Green
Write-Host ""
Write-Host "[+] Usage:" -ForegroundColor Cyan
Write-Host "    Terminal 1: .\pipe_server.exe" -ForegroundColor White
Write-Host "    Terminal 2: .\pipe_client.exe" -ForegroundColor White
