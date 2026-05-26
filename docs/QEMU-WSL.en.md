# QEMU on WSL2

> **Language:** English (this file) | [Japanese](QEMU-WSL.ja.md)

## Symptom

`qemu_mutex_lock_iothread_impl: assertion failed` — seen with QEMU 8.2.x on WSL. The **ELF is fine** (`make` succeeds).

## Workaround

1. Run **Windows** `qemu-system-loongarch64.exe` from WSL via `scripts/run-qemu-wsl.sh`, or use `scripts/run-qemu-win.ps1` from PowerShell  
2. Or use QEMU 9+ on native Linux  
3. `-accel tcg,thread=single` alone often **does not** fix WSL 8.2

## Expected serial log

`[TK2] LoongArch LA64 Boot OK` → `PASS: timer tick` → scheduler / M5 output.
