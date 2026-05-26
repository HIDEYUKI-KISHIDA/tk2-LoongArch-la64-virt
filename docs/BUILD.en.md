# Build and run — LoongArch LA64 (tk2-LoongArch-la64-virt)

> **Language:** English (this file) | [Japanese](BUILD.ja.md)

## Toolchain (Ubuntu 24.04 / WSL)

```bash
sudo apt update
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu qemu-system-misc
```

The package is **`gcc-14-loongarch64-linux-gnu`**, not `gcc-loongarch64-linux-gnu`.

## Build

From the **repository root** (adjust the path if yours differs):

```bash
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make print-cc
make clean && make
```

Artifacts:

- `tk2-la64.elf` — bring-up (Phase 1–4)
- `make TK2=1` → `build/tk2-la64-tk.elf` — T-Kernel 2.0 + M5

Link address: **0x9000000004000000** (DMW; see bundled [spec](../spec/Tk2_LoongArch_LA64_Specification_v6_2.en.md)).

## QEMU

### A. Windows (recommended)

PowerShell has **no GNU `make`** — build in WSL, run QEMU on Windows.

1. Install `qemu-system-loongarch64.exe` from https://www.qemu.org/download/#windows  
2. PowerShell:

```powershell
cd "C:\Users\h_kis\Desktop\tk2-LoongArch-la64-virt"
.\scripts\run-qemu-win.ps1        # bring-up
.\scripts\run-qemu-win.ps1 -Tk2   # after: make TK2=1
```

If the ELF is missing, run `make` / `make TK2=1` in WSL first.

Paths with non-ASCII characters may confuse QEMU; the repo includes `scripts/run-qemu-wsl.sh` which copies the ELF to `%TEMP%` and uses `-kernel`.

```bash
chmod +x scripts/run-qemu-wsl.sh
./scripts/run-qemu-wsl.sh -Tk2
```

### B. WSL-native QEMU

```bash
make qemu
```

If QEMU aborts, see [QEMU-WSL.en.md](QEMU-WSL.en.md).

## Success log (bring-up)

```
[TK2] LoongArch LA64 Boot OK
  PASS: timer tick
...
TK2-LA64: Phase4 scheduler OK
```

## T-Kernel 2.0 (`TK2=1`)

```bash
make TK2=1
# → build/tk2-la64-tk.elf
```

Upstream path is set in the `Makefile` (`TKERNEL2_ROOT`), defaulting under `Desktop/B-Free-master/archive/tkernel_2-master` when present.
