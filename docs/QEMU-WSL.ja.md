# QEMU on WSL2

> **Language:** [English](QEMU-WSL.en.md) · [日本語](QEMU-WSL.ja.md)

## 症状

`qemu_mutex_lock_iothread_impl: assertion failed` — QEMU 8.2.2 付近の不具合。ELF は正常（`make` 成功）。

## 対処

1. **Windows 版** `qemu-system-loongarch64.exe` を WSL から `scripts/run-qemu-wsl.sh` で起動する、または PowerShell で `run-qemu-win.ps1`  
2. または Linux ネイティブで QEMU 9+  
3. `tcg,thread=single` だけでは WSL では直らないことが多い

## 成功ログ

`[TK2] LoongArch LA64 Boot OK` → `PASS: timer tick` → スケジューラ / M5 出力。
