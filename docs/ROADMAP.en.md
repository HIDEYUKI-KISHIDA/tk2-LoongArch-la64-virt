# tk2-LoongArch-la64-virt roadmap (LoongArch LA64 only)

> **Language:** English (this file) | [Japanese](ROADMAP.ja.md)

**Bundled spec:** [docs/spec/](docs/spec/) — `TK2-LA64-SPEC` Rev.6.2  
**Upstream TK2:** `B-Free-master/archive/tkernel_2-master` (or `TKERNEL2_ROOT`)

| Phase | Content | Status |
|-------|---------|--------|
| 1 | UART boot `Boot OK` | Done |
| 2 | Exceptions + 100 Hz core timer (`irq_enable`) | Done |
| 3 | 4-level MMU + TLB refill | Code present; validation TBD |
| 4 | Cooperative `switch_context` A/B | Done |
| 5 | QEMU bring-up on Windows / WSL script | **Prefer Windows QEMU** (WSL 8.2 crash) |
| 6 | `make TK2=1` link to official `tkernel_2` | Done (M5 dual-task) |
| 7 | PLV3 / `syscall` | Spec only |

## Repository layout

This tree is **`tk2-LoongArch-la64-virt`** on the Desktop (LA64 only; independent of RV64).

## Build

```bash
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make clean && make
```

## Run

- WSL: `make qemu` (may fail on some QEMU builds)  
- **Recommended:** `bash scripts/run-qemu-wsl.sh` or PowerShell `run-qemu-win.ps1`

## After Phase 7

User PLV3, syscall ABI, minimal libc (future work).
