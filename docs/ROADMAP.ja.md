# tk2-LoongArch-la64-virt ロードマップ（LoongArch LA64 のみ）

> **Language:** [English](ROADMAP.en.md) · [日本語](ROADMAP.ja.md)

**同梱仕様:** [spec/README.md](spec/README.md) — `TK2-LA64-SPEC` Rev.6.2  
**上流 TK2:** `B-Free-master/archive/tkernel_2-master`（または `TKERNEL2_ROOT`）

| Phase | 内容 | 状態 |
|-------|------|------|
| 1 | UART ブート `Boot OK` | 実装済み |
| 2 | 例外・コアタイマ 100Hz（`irq_enable`） | 実装済み |
| 3 | MMU 4 段 + TLB リフィル | コードあり・本番検証は今後 |
| 4 | 協調 `switch_context` A/B | 実装済み |
| 5 | QEMU で Phase 1–4 実行確認 | **WSL 8.2 は QEMU クラッシュ** → Windows QEMU 推奨 |
| 6 | `make TK2=1` で tkernel_2 リンク | **実装済み**（M5 デュアルタスク） |
| 7 | PLV3 / syscall | 仕様のみ |

## リポジトリ

```
tk2-LoongArch-la64-virt/   ← 本リポジトリ（LA64 のみ、RV64 とは別）
```

## ビルド

```bash
sudo apt install gcc-14-loongarch64-linux-gnu binutils-loongarch64-linux-gnu
cd /mnt/c/Users/h_kis/Desktop/tk2-LoongArch-la64-virt
make clean && make
```

## 実行

- WSL: `make qemu`（環境によっては不可）  
- **推奨:** `bash scripts/run-qemu-wsl.sh` または PowerShell `run-qemu-win.ps1`

## Phase 7 以降

ユーザ PLV3、`syscall` ABI、最小 libc（将来）。
