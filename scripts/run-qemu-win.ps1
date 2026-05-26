# Run tk2-la64-virt on Windows QEMU (no GNU make required).
# Usage:
#   .\scripts\run-qemu-win.ps1           # bring-up tk2-la64.elf
#   .\scripts\run-qemu-win.ps1 -Tk2      # T-Kernel build build/tk2-la64-tk.elf
#
# Copies ELF to %TEMP% (ASCII path) before load — avoids Japanese path mojibake
# when PowerShell is started from WSL.

param(
    [switch]$Tk2,
    [string]$Qemu = "",
    [string]$LoaderElf = ""
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent

if ($Tk2) {
    $ElfName = "tk2-la64-tk.elf"
    $Elf = Join-Path $Root "build\$ElfName"
} else {
    $ElfName = "tk2-la64.elf"
    $Elf = Join-Path $Root $ElfName
}

if (-not (Test-Path -LiteralPath $Elf)) {
    Write-Host "ELF not found: $Elf" -ForegroundColor Red
    Write-Host "Build in WSL first:"
    if ($Tk2) {
        Write-Host '  cd "/mnt/c/Users/h_kis/Desktop/Tk2_LoongArch_設計仕様書/tk2-la64-virt" && make TK2=1'
    } else {
        Write-Host '  cd "/mnt/c/Users/h_kis/Desktop/Tk2_LoongArch_設計仕様書/tk2-la64-virt" && make'
    }
    exit 1
}

if ($LoaderElf -eq "") {
    $LoaderElf = Join-Path $env:TEMP $ElfName
}
Copy-Item -LiteralPath $Elf -Destination $LoaderElf -Force

if ($Qemu -eq "") {
    $cmd = Get-Command qemu-system-loongarch64.exe -ErrorAction SilentlyContinue
    if ($cmd) { $Qemu = $cmd.Source }
}
if ($Qemu -eq "") {
    $candidates = @(
        "C:\Program Files\qemu\qemu-system-loongarch64.exe",
        "C:\Program Files\QEMU\qemu-system-loongarch64.exe",
        "$env:LOCALAPPDATA\Programs\QEMU\qemu-system-loongarch64.exe"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { $Qemu = $c; break }
    }
}

if ($Qemu -eq "" -or -not (Test-Path $Qemu)) {
    Write-Host "qemu-system-loongarch64.exe not found." -ForegroundColor Red
    Write-Host "Install from https://www.qemu.org/download/#windows"
    exit 1
}

$qemuArgs = @(
    "-accel", "tcg,thread=single",
    "-machine", "virt",
    "-cpu", "la464",
    "-m", "2G",
    "-smp", "1",
    "-nographic",
    "-serial", "stdio",
    "-kernel", $LoaderElf
)

Write-Host "QEMU:   $Qemu"
Write-Host "Source: $Elf"
Write-Host "Loader: $LoaderElf"
Write-Host ""
Write-Host "Exit: Ctrl+A then X"
Write-Host ""

& $Qemu @qemuArgs
