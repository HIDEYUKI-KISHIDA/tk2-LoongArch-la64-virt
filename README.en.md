# tk2-LoongArch-la64-virt

> **Language:** English (this file) | [Japanese](README.md)

Community port of **T-Kernel 2.0** to **LoongArch LA64** on QEMU `virt`. **Not** the RISC-V `tk2-rv64-virt` project.

| Item | Path |
|------|------|
| **Design spec Rev.6.2 (bundled)** | [docs/spec/](docs/spec/) — `TK2-LA64-SPEC` |
| Doc index | [docs/README.en.md](docs/README.en.md) |
| Build | [docs/BUILD.en.md](docs/BUILD.en.md) |
| QEMU / WSL | [docs/QEMU-WSL.en.md](docs/QEMU-WSL.en.md) |
| License | [LICENSE](LICENSE) (MIT) / [NOTICE](NOTICE) (TK2, etc.) |

## Build (WSL recommended)

```bash
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make clean && make
```

Output: `tk2-la64.elf` (linked at **0x9000000004000000**, DMW1 window — see [docs/spec/](docs/spec/)).

## Run

- **QEMU 8.2 inside WSL** often crashes with `qemu_mutex_lock_iothread_impl` → use **Windows QEMU** via `bash scripts/run-qemu-wsl.sh` or `scripts/run-qemu-win.ps1`
- Details: [docs/QEMU-WSL.en.md](docs/QEMU-WSL.en.md)

```bash
make && bash scripts/run-qemu-wsl.sh              # bring-up
make TK2=1 && bash scripts/run-qemu-wsl.sh -Tk2   # T-Kernel 2.0 + M5
```

## Status (2026-05)

- Phase 1–4: UART, timer IRQ, cooperative A/B scheduler (aligned with spec Rev.6.2)
- **`make TK2=1`**: official `tkernel_2` link + M5 dual-task (`loc_mtx` fixed with PRMD before `ertn`)
- MMU Phase C: code present; full validation pending

