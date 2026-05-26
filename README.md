# tk2-LoongArch-la64-virt

> **Language:** [English](README.en.md) · [日本語](README.md)

**LoongArch LA64** 向け T-Kernel 2.0 コミュニティポート（QEMU `virt`）。**RISC-V（RV64）とは別リポジトリ**です。

| 項目 | パス |
|------|------|
| **設計仕様 Rev.6.2（同梱）** | [docs/spec/](docs/spec/) — `TK2-LA64-SPEC` |
| ドキュメント索引 | [docs/README.md](docs/README.md) |
| ビルド | [docs/BUILD.ja.md](docs/BUILD.ja.md) / [docs/BUILD.en.md](docs/BUILD.en.md) |
| QEMU（WSL 注意） | [docs/QEMU-WSL.ja.md](docs/QEMU-WSL.ja.md) / [docs/QEMU-WSL.en.md](docs/QEMU-WSL.en.md) |
| ライセンス | [LICENSE](LICENSE)（MIT） / [NOTICE](NOTICE)（TK2 等） |

## Build（WSL 推奨）

```bash
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make clean && make
```

成果物: `tk2-la64.elf`（リンク先 **0x9000_0000_0400_0000**、DMW1 ウィンドウ。仕様は [docs/spec/](docs/spec/)）。

## Run

- **WSL 同梱 QEMU 8.2** は `qemu_mutex_lock_iothread_impl` で落ちることが多い → **Windows QEMU** 経由: `bash scripts/run-qemu-wsl.sh` または `scripts/run-qemu-win.ps1`
- 詳細: [docs/QEMU-WSL.ja.md](docs/QEMU-WSL.ja.md)

```bash
make && bash scripts/run-qemu-wsl.sh              # bring-up
make TK2=1 && bash scripts/run-qemu-wsl.sh -Tk2   # T-Kernel 2.0 + M5
```

## Status（2026-05）

- Phase 1–4 bring-up: UART、タイマ IRQ、協調 A/B スケジューラ（Rev.6.2 整合）
- **`make TK2=1`**: 公式 `tkernel_2` リンク + M5 デュアルタスク（`loc_mtx` は PRMD+`ertn` 修正済み）
- MMU Phase C: コードあり・本番検証は今後
