# ビルド・実行 — LoongArch LA64（tk2-LoongArch-la64-virt）

> **Language:** [English](BUILD.en.md) · [日本語](BUILD.ja.md)

## ツールチェーン（Ubuntu 24.04 / WSL）

```bash
sudo apt update
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu qemu-system-misc
```

パッケージ名は **`gcc-loongarch64-linux-gnu` ではない**（`gcc-14-loongarch64-linux-gnu`）。

## ビルド

**本リポジトリのルート**で実行（パスは環境に合わせて読み替え）:

```bash
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make print-cc
make clean && make
```

成果物:

- `tk2-la64.elf` — Phase 1–4 ブリングアップ
- `make TK2=1` → `build/tk2-la64-tk.elf` — T-Kernel 2.0 + M5

リンク先頭: **0x9000_0000_0400_0000**（DMW。同梱 [仕様](../spec/Tk2_LoongArch_LA64_Specification_v6_2.ja.md) Rev.6.2）。

## QEMU 起動

### A. Windows（推奨）

PowerShell には **GNU make がありません**（`make` は WSL 専用）。

1. https://www.qemu.org/download/#windows で `qemu-system-loongarch64.exe` をインストール  
2. PowerShell:

```powershell
cd "C:\Users\h_kis\Desktop\tk2-LoongArch-la64-virt"
.\scripts\run-qemu-win.ps1        # bring-up (tk2-la64.elf)
.\scripts\run-qemu-win.ps1 -Tk2   # T-Kernel（事前に WSL で make TK2=1）
```

ELF が無いときは先に WSL で `make` / `make TK2=1` を実行してください。

日本語を含むパスでは QEMU に ELF を直接渡すと文字化けすることがあります。その場合は WSL 用スクリプト（`%TEMP%` にコピーして `-kernel`）を使います:

```bash
chmod +x scripts/run-qemu-wsl.sh
./scripts/run-qemu-wsl.sh -Tk2
```

### B. WSL

```bash
make qemu
```

`qemu_mutex_lock_iothread_impl` で落ちる場合は [QEMU-WSL.ja.md](QEMU-WSL.ja.md) 参照。

## 成功ログ

```
[TK2] LoongArch LA64 Boot OK
  PASS: timer tick
...
TK2-LA64: Phase4 scheduler OK
```

## T-Kernel 2.0 統合

```bash
make TK2=1
# → build/tk2-la64-tk.elf
```

上流パスは `Makefile` の `TKERNEL2_ROOT`（デフォルトは `Desktop/B-Free-master/archive/tkernel_2-master` があればそこ）。
