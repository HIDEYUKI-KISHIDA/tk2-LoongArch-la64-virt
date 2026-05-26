# TK2 LoongArch 設計仕様書

> **Language:** [English](Tk2_LoongArch_LA64_Specification_v6_2.en.md) | [Japanese](Tk2_LoongArch_LA64_Specification_v6_2.ja.md)

**ドキュメントコード:** TK2-LA64-SPEC  
**バージョン:** Rev.6.2（QEMU virt 実装照合版）  
**発行日:** 2026年5月24日  
**ステータス:** 正式正本  
**前バージョン:** Rev.6.1

---

## 改訂履歴

| Rev | 日付 | 概要 |
|-----|------|------|
| 6.2 | 2026-05-24 | tk2-la64-virt 照合。UART/リンク/DMW/PRMD+ertn ディスパッチ修正、§0.2 4 段 PT 統一、付録 H 追加（付録 O） |
| 6.1 | 2026-05-26 | Rev.6.0 再査読。4 段ページテーブル（Linux 準拠）・`mmu_map_page` 葉インデックス修正・`irq_enable` 追加・例外入口フル実装・`tk_handle_exception` 判定順・協調 `switch_context` の CRMD.IE 注記（付録 G） |
| 6.0 | 2026-05-26 | Rev.5 技術査読。構造体サイズ誤り・`idle` 文法エラー・ECFG 初期化欠落・`$r21` ABI 表記・`mmu_enable` 物理アドレス注記等（付録 E 参照） |
| 5.0 | 2026-05-24 | Rev.4 技術査読。CSR 誤番号・TLB リフィル・MMU 有効化・ABI/データ型・検証計画を補完 |
| 4.0 | 2026-05-?? | exception/TLB/switch 整合性修正 |
| 3.0 | 2026-05-?? | TLB リフィル・`mmu_map_page` 追加 |
| 2.0 | 2025-06-?? | 初回技術評価 12 件 |
| 1.0 | —        | ドラフト |

> Rev.3〜4 の正確な日付は確認次第 `??` を置換すること。詳細変更一覧は同梱の [`CHANGELOG.md`](CHANGELOG.md) を参照。

---

## §0 文書管理

### 0.1 目的

T-Kernel 2.0（TK2）を **LoongArch LA64（64bit）** へ移植するための、カーネル `sysdepend`・ブート・MMU・例外・スケジューラの**設計仕様正本**である。x86_64 / AArch64 / RISC-V 版と同一の設計思想（INT/UINT 32bit 維持、TRAP/TASK コンテキスト分離）に従う。

### 0.2 適用範囲

| 含む | 含まない（将来） |
|------|------------------|
| シングルコア・PLV0 カーネル起動 | SMP / IPI |
| 例外・タイマ・UART | 完全なユーザ空間 POSIX |
| **4 段**ページテーブル + TLB リフィル | KVM / 仮想化拡張 |
| 協調的 `switch_context` | 動的モジュールロード |

### 0.3 規範用語

- **必須 (MUST)** — 準拠実装が満たすこと  
- **推奨 (SHOULD)** — 強く推奨（理由があれば逸脱可）  
- **参考 (MAY)** — 実装例・将来拡張

### 0.4 参照仕様

- LoongArch Reference Manual, Volume 1 / Volume 2  
- LoongArch ELF ABI specification  
- QEMU `loongarch64` `virt` マシン  
- Linux `arch/loongarch`（CSR 番号・TLB リフィル慣習の参照実装）

---

## §1 スコープとアーキテクチャ前提

### 1.1 対象 CPU

- **ISA:** LoongArch64（LA64）  
- **ページサイズ:** 4 KiB 固定  
- **仮想アドレス:** 48bit 有効（`badv[47]` でカーネル/ユーザ空間を分割する Linux 慣習に準拠）

### 1.2 MMU モデル（RISC-V との差異）

LoongArch は Sv39 型の単純な SATP 切替だけでは足りない。

1. **4 段ページテーブル**（PGD → PUD → PMD → PTE。Linux `CONFIG_PGTABLE_LEVELS=4` と同型）  
2. **TLB リフィル専用例外ベクタ** `CSR.TLBRENTRY`（`tlb_refill_entry`）  
3. **`lddir` / `ldpte` / `tlbfill`** によるハードウェア支援ウォーク  
4. ブート初期は **直接アドレス翻訳（CRMD.DA=1）**、MMU 有効化後は **ページテーブルモード（CRMD.PG=1）**

### 1.3 開発優先度

1. シングルコア・カーネルモード起動（UART に `Boot OK`）  
2. 例外・割り込み・100Hz タイマ  
3. TLB リフィル + カーネル仮想マップ  
4. `switch_context` による 2 タスクラウンドロビン  
5. PLV3 ユーザモード + `syscall` 命令（将来）

---

## §2 実行環境

### 2.1 ツールチェーン（推奨）

| 項目 | 推奨 |
|------|------|
| GCC | `loongarch64-linux-gnu-gcc` **12.x 以降**（参考実装: **gcc-14-loongarch64-linux-gnu**） |
| Binutils | 対応する `ld` / `objcopy` |
| QEMU | **8.0 以降**（`-bios none -kernel` サポート）。WSL では **Windows 版 QEMU** 推奨 |
| Make | GNU Make（**WSL/Linux で実行**。PowerShell 単体不可） |
| C モデル | LP64（`int` 32bit、`long`/pointer 64bit） |

> **動作環境の詳細**（WSL2、日本語パス対策、`TKERNEL2_ROOT`）はリポジトリ同梱の [`docs/ENVIRONMENT.ja.md`](../docs/ENVIRONMENT.ja.md) を参照。

```bash
# ビルド例
loongarch64-linux-gnu-gcc -static -nostdlib -fno-PIE -fno-stack-protector \
  -Wl,-T,tk2_loongarch.ld -o tk2.elf ...
```

### 2.2 QEMU 起動（必須オプション）

```bash
qemu-system-loongarch64 \
  -machine virt \
  -bios none \
  -kernel tk2.elf \
  -m 256M \
  -nographic \
  -serial mon:stdio
```

デバッグ時:

```bash
  -s -S    # GDB :1234 で停止起動
```

### 2.3 QEMU `virt` MMIO・メモリマップ（Rev.6.2 確定値）

> **注意:** `0x1000_0000` の UART は **RISC-V `virt` の値**であり、LoongArch `virt` では使用しない（Rev.6.1 以前の誤記を修正）。

| 種別 | アドレス | サイズ | 用途 |
|------|----------|--------|------|
| UART（NS16550）PA | 0x1fe0_01e0 | 0x100 | シリアル（物理） |
| UART カーネル VA | 0x8000_0000_1fe0_01e0 | — | DMW0 直映像（XKPRANGE uncached） |
| カーネルリンク VA | 0x9000_0000_0400_0000 | — | `linker.ld` の `ORIGIN` |
| DRAM（参考） | 0x4000_0000 付近 | — | 物理 DRAM ベース（SoC/QEMU 定義に従う） |

```c
/* uart.c — QEMU loongarch64 virt */
#define UART_BASE  0x800000001fe001e0UL
```

ブート時に **DMW0/DMW1** を設定し、上記 VA で MMIO/DRAM にアクセスする（§3.1 T1 `dmw_init.S`）。

### 2.4 特権レベル

| PLV | 名称 | TK2 での用途 |
|-----|------|----------------|
| 0 | カーネル | カーネル・例外ハンドラ |
| 1〜2 | — | 未使用 |
| 3 | ユーザ | 将来のユーザタスク |

---

## §2A データ型（LP64・TK2 ABI）

他アーキテクチャ TK2 移植と同方針: **API 互換の `INT`/`UINT` は 32bit 維持**、レジスタ幅は `REGINT`/`REGUINT` を新設。

| 型 | RV64/x86_64 方針 | LA64 正式 |
|----|------------------|-----------|
| INT / UINT | 32bit `int` | **32bit `int`（維持）** |
| D / UD | 64bit `long` | 64bit `long` |
| SIZE / ADDR | 64bit | 64bit `unsigned long` |
| REGINT / REGUINT | 64bit | 64bit `long` / `unsigned long` |
| VP / CVP | 64bit pointer | 64bit pointer |

> **注意:** 32bit CPU 向けバイナリのそのまま実行は非目標。**ソース/構造体 ABI 互換**を指す。

---

## 付録 B. 汎用レジスタと ABI

> **読み方:** 付録 B・C は CSR/ABI の参照表。実装コードは **§3** を正とする。

| レジスタ | ABI 名 | 役割 | 退避（タスク切替） |
|----------|--------|------|-------------------|
| $r0 | zero | 常に 0 | 不要 |
| $r1 | ra | 戻りアドレス | **保存**（協調切替） |
| $r2 | tp | TLS / per-CPU | 別途管理 |
| $r3 | sp | スタックポインタ | **保存** |
| $r4–$r11 | a0–a7 | 引数・戻り値 | Caller |
| $r12–$r20 | t0–t8 | 一時（9本） | Caller |
| $r21 | — | **OS予約（ABI規定）** | — |
| $r22 | fp | フレームポインタ | **保存** |
| $r23–$r31 | s0–s8 | Callee-saved（9本） | **保存** |

> **注意:** `$r21` は LoongArch ELF ABI で明示的に OS 予約とされており、アプリケーション・ライブラリのいずれも使用してはならない。TK2 カーネルはこれを per-CPU ポインタ等のカーネル専用スクラッチとして将来利用できる。

- **例外フレーム:** $r1–$r31（$r0 除く）+ ERA/CRMD/PRMD/ESTAT/BADV  
- **タスク切替:** `ra`, `sp`, `fp`, `s0`–`s8`, `ERA`, `CRMD`（`TK_TASKCTX_LOONGARCH`）

---

## 付録 C. CSR 一覧（実装で使用）

| CSR | 番号 | 用途 |
|-----|------|------|
| CRMD | 0x00 | 現モード PLV/IE/DA/PG |
| PRMD | 0x01 | 例外前モード（`ertn` で CRMD に復帰） |
| ECFG | 0x04 | 例外・割り込みマスク設定 |
| ESTAT | 0x05 | 例外・割り込み要因 |
| ERA | 0x06 | 例外 PC |
| BADV | 0x07 | 例外 VA |
| EENTRY | 0x0C | 一般例外ベクタ |
| PGDL | 0x19 | ユーザ側 PGD（`badv[47]=0`）**物理アドレスを設定** |
| PGDH | 0x1A | カーネル側 PGD（`badv[47]=1`）**物理アドレスを設定** |
| PGD | 0x1B | 現在 PGD（PGDL/PGDH のどちらかを自動反映・読み出し専用） |
| TCFG | 0x41 | タイマ設定 |
| TICLR | 0x44 | タイマ割り込みクリア |
| TLBRENTRY | 0x88 | TLB リフィルベクタ |
| TLBRBADV | 0x89 | TLB リフィル時の障害 VA |
| TLBRERA | 0x8A | TLB リフィル ERA |
| TLBRSAVE | 0x8B | リフィル用スクラッチ（第1レジスタ退避） |
| TLBRELO0 | 0x8C | リフィル用 TLBELO0（**スクラッチ不可・`ldpte` の出力先**） |
| TLBRELO1 | 0x8D | リフィル用 TLBELO1（**スクラッチ不可・`ldpte` の出力先**） |
| MERRENTRY | 0x93 | マシンエラーベクタ |

> **追記（Rev.6.0）:** `PGD`（0x1B）は読み出し専用の合成レジスタ。`PGDL`/`PGDH` に設定する値はページテーブルの**物理アドレス**であること（DA モード中は VA = PA のため実害はないが、PG モード移行後を見越して明記する）。

**CRMD 主要ビット（実装参照）:**

| ビット | 名 | 意味 |
|--------|-----|------|
| 1:0 | PLV | 特権レベル |
| 2 | IE | 割り込み有効 |
| 3 | DA | 直接アドレスモード |
| 4 | PG | ページテーブルモード |

初期タスク（MMU 前）: `CRMD = PLV0 | DA` → **`0x08`**  
MMU 有効後: `CRMD = PLV0 | PG` → **`0x10`**（IE は ECFG 設定後に別途セット）

> **注意:** `CRMD.DA` と `CRMD.PG` は排他。両方同時に 1 にしてはならない。遷移は必ず DA=1,PG=0 → DA=0,PG=1 の順で atomic に行うこと。

**ECFG 割り込みマスク（初期化で設定）:**

| ビット範囲 | 名称 | 推奨初期値 |
|-----------|------|-----------|
| IS[1:0] | ソフトウェア割り込み | 0（無効） |
| IS[9:2] | ハードウェア割り込み 0–7 | 0（無効、必要なデバイスのみ有効化） |
| IS[10] | PMI | 0 |
| IS[11] | コアタイマ | **1**（タイマ init 後に有効化） |
| IS[12] | IPI | 0（SMP 未対応のため無効） |

---

## §3 実装仕様（確定コード）

### 3.1 T1 — ブート (`start.S`)

```asm
#define LOONGARCH_CSR_CRMD   0x0
#define LOONGARCH_CSR_CPUID  0x20

    .section .text.init
    .globl _start
_start:
    # 割り込み無効: CRMD.IE = 0
    csrrd   $t0, LOONGARCH_CSR_CRMD
    li.d    $t1, ~(1UL << 2)
    and     $t0, $t0, $t1
    csrwr   $t0, LOONGARCH_CSR_CRMD

    # hart0 以外は待機
    csrrd   $t0, LOONGARCH_CSR_CPUID
    bnez    $t0, .Lhalt

    la      $sp, _stack_top
    la      $tp, _percpu_data

    la      $t0, __bss_start
    la      $t1, __bss_end
.Lbss_clear:
    bgeu    $t0, $t1, .Lbss_done
    st.d    $zero, $t0, 0
    addi.d  $t0, $t0, 8
    b       .Lbss_clear
.Lbss_done:

    bl      exception_vector_init
    bl      kernel_main

.Lhalt:
    idle    0
    b       .Lhalt
```

> Rev.4 の `bstrins.d` より **`and` による IE クリア**を規範とする（可読性・検証容易性）。

### 3.2 T2 — 例外ベクタ登録 (`exception.c`)

ベクタ登録と同時に **ECFG でコアタイマ割り込みを有効化**する。タイマ割り込みは `timer_init()` 呼び出し後でなければカウントが始まらないが、マスクは事前に開けておく。

```c
#define CSR_ECFG       0x04
#define CSR_EENTRY     0x0C
#define CSR_TLBRENTRY  0x88
#define CSR_MERRENTRY  0x93

/* ECFG.IS[11] = コアタイマ割り込み有効ビット */
#define ECFG_TIMER_BIT (1U << 11)

extern void exception_entry(void);
extern void tlb_refill_entry(void);
extern void machine_error_entry(void);

void exception_vector_init(void)
{
    unsigned long addr;

    /* コアタイマ割り込みのみ許可（他は timer_init / irq_enable で個別に開ける） */
    __asm__ volatile("csrwr %0, %1" :: "r"((unsigned long)ECFG_TIMER_BIT),
                     "i"(CSR_ECFG));

    addr = (unsigned long)&exception_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_EENTRY));

    addr = (unsigned long)&tlb_refill_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_TLBRENTRY));

    addr = (unsigned long)&machine_error_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_MERRENTRY));
}

/*
 * ECFG.IS[11] だけでは割り込みは届かない。CRMD.IE も 1 にする。
 * 呼び出し順: exception_vector_init() → timer_init() → irq_enable()
 */
void irq_enable(void)
{
    unsigned long crmd;

    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd |= (1UL << 2);   /* CRMD.IE = 1 */
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}

void irq_disable(void)
{
    unsigned long crmd;

    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd &= ~(1UL << 2);  /* CRMD.IE = 0 */
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}
```

`kernel_main` では `timer_init()` の直後に `irq_enable()` を呼ぶ（`start.S` で `exception_vector_init()` は済み）。

### 3.3 T3 — UART (`uart.c`)

```c
#define UART_BASE       0x800000001fe001e0UL
#define UART_THR        (*(volatile unsigned char *)(UART_BASE + 0x00))
#define UART_LSR        (*(volatile unsigned char *)(UART_BASE + 0x05))
#define UART_LSR_THRE   0x20

void uart_putchar(int c)
{
    while (!(UART_LSR & UART_LSR_THRE))
        ;
    UART_THR = (unsigned char)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putchar('\r');
        uart_putchar(*s++);
    }
}
```

### 3.4 T4 — コンテキスト構造体 (`loongarch_ctx.h`)

```c
#ifndef LOONGARCH_CTX_H
#define LOONGARCH_CTX_H

typedef struct {
    unsigned long r[31];    /* 0..30 : $r1..$r31  → 248 bytes */
    unsigned long era;      /* offset 248 (31*8)  */
    unsigned long crmd;     /* offset 256 (32*8)  */
    unsigned long prmd;     /* offset 264 (33*8)  */
    unsigned long estat;    /* offset 272 (34*8)  */
    unsigned long badv;     /* offset 280 (35*8)  */
} TK_EXFRAME_LOONGARCH;     /* 合計 288 bytes */

typedef struct {
    unsigned long ra;       /* offset   0: $r1  */
    unsigned long sp;       /* offset   8: $r3  */
    unsigned long fp;       /* offset  16: $r22 */
    unsigned long s[9];     /* offset  24..88: $r23..$r31（9 * 8 = 72 bytes） */
    unsigned long era;      /* offset  96 */
    unsigned long crmd;     /* offset 104 */
} TK_TASKCTX_LOONGARCH;     /* 合計 112 bytes（14 フィールド × 8） */

_Static_assert(sizeof(TK_EXFRAME_LOONGARCH) == 288,
               "TK_EXFRAME_LOONGARCH must be 288 bytes");
_Static_assert(sizeof(TK_TASKCTX_LOONGARCH) == 112,
               "TK_TASKCTX_LOONGARCH must be 112 bytes");

#endif
```

> **Rev.6.0 修正:** `s[9]` は offset 24 から始まり 24 + 72 = 96 で終端。`era` は offset 96、`crmd` は offset 104 となり構造体サイズは **112 bytes**。Rev.5.0 の「120 bytes (15\*8)」はフィールド数を 15 と数え間違えた誤りであった。`switch_context` のオフセット計算（12\*8=96、13\*8=104）と整合していることを確認済み。

### 3.5 T5 — 例外エントリ (`exception_entry.S`)

```asm
    .section .text
    .align  4
    .globl  exception_entry
exception_entry:
    # ネスト防止: IE クリア（CRMD.IE = 0）
    csrrd   $t0, 0x00
    li.d    $t1, ~(1UL << 2)
    and     $t0, $t0, $t1
    csrwr   $t0, 0x00

    addi.d  $sp, $sp, -288

    st.d    $r1,  $sp,  0*8
    st.d    $r2,  $sp,  1*8
    st.d    $r3,  $sp,  2*8
    st.d    $r4,  $sp,  3*8
    st.d    $r5,  $sp,  4*8
    st.d    $r6,  $sp,  5*8
    st.d    $r7,  $sp,  6*8
    st.d    $r8,  $sp,  7*8
    st.d    $r9,  $sp,  8*8
    st.d    $r10, $sp,  9*8
    st.d    $r11, $sp, 10*8
    st.d    $r12, $sp, 11*8
    st.d    $r13, $sp, 12*8
    st.d    $r14, $sp, 13*8
    st.d    $r15, $sp, 14*8
    st.d    $r16, $sp, 15*8
    st.d    $r17, $sp, 16*8
    st.d    $r18, $sp, 17*8
    st.d    $r19, $sp, 18*8
    st.d    $r20, $sp, 19*8
    st.d    $r21, $sp, 20*8
    st.d    $r22, $sp, 21*8
    st.d    $r23, $sp, 22*8
    st.d    $r24, $sp, 23*8
    st.d    $r25, $sp, 24*8
    st.d    $r26, $sp, 25*8
    st.d    $r27, $sp, 26*8
    st.d    $r28, $sp, 27*8
    st.d    $r29, $sp, 28*8
    st.d    $r30, $sp, 29*8
    st.d    $r31, $sp, 30*8

    # 元の sp を復元して保存（例外フレームの $r3 スロットへ）
    addi.d  $t0, $sp, 288
    st.d    $t0, $sp, 2*8          /* 元の sp */

    csrrd   $t0, 0x06
    st.d    $t0, $sp, 31*8        /* ERA   */
    csrrd   $t0, 0x00
    st.d    $t0, $sp, 32*8        /* CRMD  */
    csrrd   $t0, 0x01
    st.d    $t0, $sp, 33*8        /* PRMD  */
    csrrd   $t0, 0x05
    st.d    $t0, $sp, 34*8        /* ESTAT */
    csrrd   $t0, 0x07
    st.d    $t0, $sp, 35*8        /* BADV  */

    move    $a0, $sp
    bl      tk_handle_exception

    ld.d    $t0, $sp, 31*8
    csrwr   $t0, 0x06             /* ERA 復元 */
    ld.d    $t0, $sp, 33*8
    csrwr   $t0, 0x01             /* PRMD 復元（ertn で CRMD へ反映） */

    ld.d    $r1,  $sp,  0*8
    ld.d    $r2,  $sp,  1*8
    ld.d    $r4,  $sp,  3*8
    ld.d    $r5,  $sp,  4*8
    ld.d    $r6,  $sp,  5*8
    ld.d    $r7,  $sp,  6*8
    ld.d    $r8,  $sp,  7*8
    ld.d    $r9,  $sp,  8*8
    ld.d    $r10, $sp,  9*8
    ld.d    $r11, $sp, 10*8
    ld.d    $r12, $sp, 11*8
    ld.d    $r13, $sp, 12*8
    ld.d    $r14, $sp, 13*8
    ld.d    $r15, $sp, 14*8
    ld.d    $r16, $sp, 15*8
    ld.d    $r17, $sp, 16*8
    ld.d    $r18, $sp, 17*8
    ld.d    $r19, $sp, 18*8
    ld.d    $r20, $sp, 19*8
    ld.d    $r21, $sp, 20*8
    ld.d    $r22, $sp, 21*8
    ld.d    $r23, $sp, 22*8
    ld.d    $r24, $sp, 23*8
    ld.d    $r25, $sp, 24*8
    ld.d    $r26, $sp, 25*8
    ld.d    $r27, $sp, 26*8
    ld.d    $r28, $sp, 27*8
    ld.d    $r29, $sp, 28*8
    ld.d    $r30, $sp, 29*8
    ld.d    $r31, $sp, 30*8
    ld.d    $r3,  $sp,  2*8       /* sp は最後に復元 */
    addi.d  $sp, $sp, 288
    ertn
```

### 3.6 T5b — 例外 C ディスパッチ (`exception_c.c`)

```c
#include "loongarch_ctx.h"

/* ESTAT.IS[12:0] = 割り込みステータスビット群 */
#define CSR_ESTAT_IS_MASK  0x00001FFFUL
/* ESTAT.Ecode[21:16] = 例外コード */
#define ESTAT_ECODE(es)    (((es) >> 16) & 0x3F)

extern void timer_interrupt_handler(void);
extern void irq_dispatch(unsigned long estat);
extern void syscall_handler(TK_EXFRAME_LOONGARCH *ef);

void tk_handle_exception(TK_EXFRAME_LOONGARCH *ef)
{
    unsigned long estat = ef->estat;
    unsigned int  ecode  = ESTAT_ECODE(estat);

    /*
     * ディスパッチ順（Rev.6.1）:
     *   1) ECODE（SYSCALL 等の同期例外）
     *   2) ESTAT.IS（割り込み）
     * SYSCALL と IS が同時に立つことは通常ないが、実装は ECODE を先に判定する。
     */
    if (ecode == 0x0B) {   /* SYSCALL */
        syscall_handler(ef);
        ef->era += 4;      /* syscall 命令の次へ */
        return;
    }

    if (estat & CSR_ESTAT_IS_MASK) {
        if (estat & (1UL << 11)) {   /* IS[11]: コアタイマ割り込み */
            timer_interrupt_handler();
        } else {
            irq_dispatch(estat);
        }
        return;
    }

    uart_puts("PANIC: unhandled exception\n");
    for (;;)
        __asm__ volatile("idle 0");
}
```

> **Rev.6.0 修正:** Rev.5.0 の `idle 0;` は C 文として無効（コンパイルエラー）。`__asm__ volatile("idle 0")` に訂正した。LoongArch の `idle` 命令は ISA レベル命令であり、C からは必ずインライン asm で呼び出すこと。  
> **Rev.6.1 修正:** 同期例外（ECODE）を IS マスクより先に判定。タイマ到達には `irq_enable()`（§3.2）が別途必要。

### 3.7 T6 — タイマ (`timer.c`)

```c
#define CSR_TCFG    0x41
#define CSR_TICLR   0x44
#define TCFG_EN     (1UL << 0)
#define TCFG_LOOP   (1UL << 1)
/* TCFG.InitVal = bits[63:2]、カウント初期値（単位: タイマクロック） */

static unsigned long timer_freq(void)
{
    /*
     * cpucfg leaf 4 でタイマ周波数（Hz）を取得する。
     * 戻り値は 32bit（unsigned int）。QEMU virt では通常 10 MHz 前後。
     * 実機では SoC ドキュメントを確認すること。
     */
    unsigned int val;
    __asm__ volatile("cpucfg %0, %1" : "=r"(val) : "r"(4U));
    return (unsigned long)val;
}

void timer_init(void)
{
    unsigned long freq     = timer_freq();
    unsigned long interval = freq / 100UL;   /* 100 Hz */
    /* InitVal は bits[63:2] に格納: interval を 2 ビット左シフト */
    unsigned long tcfg = TCFG_EN | TCFG_LOOP | (interval << 2);
    __asm__ volatile("csrwr %0, %1" :: "r"(tcfg), "i"(CSR_TCFG));
}

void timer_interrupt_handler(void)
{
    /* TICLR.CLR (bit 0) に 1 を書いてタイマ割り込みをクリア */
    __asm__ volatile("csrwr %0, %1" :: "r"(1UL), "i"(CSR_TICLR));
    timer_handler();
}
```

### 3.8 T7 — ページテーブル (`mmu.c`)

```c
#define PAGE_SIZE     4096UL
#define PTE_V         (1UL << 0)   /* Valid */
#define PTE_P         (1UL << 7)   /* Physical page present */
#define PTE_W         (1UL << 8)   /* Writable */
#define PTE_PLV0      (0UL << 2)   /* PLV=0 でアクセス可 */
#define PTE_MAT_CC    (1UL << 4)   /* Memory Access Type: Cached Coherent */

/*
 * 48bit 仮想アドレス / 4KiB ページ / 4段テーブル (9+9+9+9+12)
 * Linux arch/loongarch/include/asm/pgtable.h と同型:
 *   bit[47:39] → PGD  (VA_PGD_SHIFT  = 39)
 *   bit[38:30] → PUD  (VA_PUD_SHIFT  = 30)
 *   bit[29:21] → PMD  (VA_PMD_SHIFT  = 21)
 *   bit[20:12] → PTE  (VA_PAGE_SHIFT = 12)  ← 葉インデックス
 *   bit[11:0]  → ページ内オフセット
 *
 * Rev.6.0 の VA_PTE_SHIFT=21 は PMD 境界であり、葉テーブル index としては誤りだった。
 */
#define VA_PGD_SHIFT   39
#define VA_PUD_SHIFT   30
#define VA_PMD_SHIFT   21
#define VA_PAGE_SHIFT  12
#define PFN_MASK       0x0000FFFFFFFFF000UL

void mmu_map_page(unsigned long *pgd, unsigned long va,
                  unsigned long pa, unsigned long flags)
{
    unsigned int gi = (va >> VA_PGD_SHIFT)  & 0x1FF;
    unsigned int ui = (va >> VA_PUD_SHIFT)  & 0x1FF;
    unsigned int mi = (va >> VA_PMD_SHIFT)  & 0x1FF;
    unsigned int pi = (va >> VA_PAGE_SHIFT) & 0x1FF;
    unsigned long *pud, *pmd, *pte;

    if (!(pgd[gi] & PTE_V)) {
        pud = boot_alloc_page();
        pgd[gi] = ((unsigned long)pud & PFN_MASK) | PTE_V;
    }
    pud = (unsigned long *)(pgd[gi] & PFN_MASK);

    if (!(pud[ui] & PTE_V)) {
        pmd = boot_alloc_page();
        pud[ui] = ((unsigned long)pmd & PFN_MASK) | PTE_V;
    }
    pmd = (unsigned long *)(pud[ui] & PFN_MASK);

    if (!(pmd[mi] & PTE_V)) {
        pte = boot_alloc_page();
        pmd[mi] = ((unsigned long)pte & PFN_MASK) | PTE_V;
    }
    pte = (unsigned long *)(pmd[mi] & PFN_MASK);

    pte[pi] = (pa & PFN_MASK) | PTE_V | PTE_P | PTE_W | PTE_PLV0
              | PTE_MAT_CC | flags;

    __asm__ volatile("invtlb 0x5, $zero, %0" :: "r"(va) : "memory");
}
```

> **Rev.6.1 修正:** 4 段ウォークに合わせ葉インデックスを `(va >> 12)` に変更。TLB リフィル（§3.10）の `lddir` も level 3→2→1 に統一。  
> **Rev.6.0 注記:** `invtlb` op=0x5 は指定 VA の全 ASID エントリ無効化（カーネル恒等マップ向け）。

### 3.9 T8 — MMU 有効化（Rev.5.0 新設・Rev.6.0 補記）

ブート直後は **DA モード（CRMD.DA=1, CRMD.PG=0）**。以下の手順を**この順序で**実施してから **PG モード**へ遷移する。

1. `boot_alloc` でカーネル PGD を構築し、DRAM/MMIO/UART をマップ  
2. `CSR.PGDH` / `CSR.PGDL` に PGD の**物理アドレス**を設定（DA 中は VA=PA）  
3. **DMW**（直接マップウィンドウ）でカーネル恒等マップ領域を維持  
   - QEMU virt では DMW0/DMW1 のデフォルト値を確認すること（ボード定義 `board/qemu_virt.h` に分離推奨）  
4. TLB を全無効化（`invtlb 0x0, $zero, $zero`）  
5. `ibar 0` で命令フェッチキャッシュを同期  
6. `CRMD.DA=0`, `CRMD.PG=1` を **1 回の `csrwr`** でアトミックに切替

```c
void mmu_enable(unsigned long *kernel_pgd)
{
    /*
     * DA モード中は VA = PA のため kernel_pgd をそのまま渡してよい。
     * PGDL/PGDH には物理アドレスが必要（PG モード移行後も参照されるため）。
     */
    __asm__ volatile("csrwr %0, %1" :: "r"((unsigned long)kernel_pgd),
                     "i"(0x1A) : "memory"); /* PGDH: カーネル側 PGD */
    __asm__ volatile("csrwr %0, %1" :: "r"((unsigned long)kernel_pgd),
                     "i"(0x19) : "memory"); /* PGDL: ユーザ側（同一 PGD で可） */

    /* TLB 全無効化 */
    __asm__ volatile("invtlb 0x0, $zero, $zero" ::: "memory");
    /* 命令フェッチ同期 */
    __asm__ volatile("ibar 0" ::: "memory");

    /* CRMD: DA=0, PG=1 をアトミックに切替（PLV/IE は変更しない） */
    unsigned long crmd;
    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd &= ~(1UL << 3);   /* DA = 0 */
    crmd |=  (1UL << 4);   /* PG = 1 */
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}
```

> **注意:** `mmu_enable()` の直後から命令フェッチ・データアクセスともに TLB 経由になる。呼び出し元のコードおよびスタック領域が PG モード用ページテーブルに**マップ済みであること**を事前に確認すること。マップ漏れは即座に TLB リフィル例外 → パニックを引き起こす。

### 3.10 T9 — TLB リフィル（Rev.5.0 修正版・Rev.6.0 確定）

**Rev.4 からの修正点:**

| 項目 | Rev.4（誤） | Rev.5.0 以降（正） |
|------|-------------|---------------|
| 障害 VA CSR | 0x1D | **0x89 (TLBRBADV)** |
| 第 2 スクラッチ | 0x8C を TLBRSAVE2 として使用 | **禁止**（0x8C は TLBRELO0・`ldpte` 出力先） |
| 空間判定ビット | bit 63 | **badv[47]** |

```asm
#define CSR_TLBRBADV  0x89
#define CSR_PGDL      0x19
#define CSR_PGDH      0x1A
#define CSR_TLBRSAVE  0x8B

    .globl tlb_refill_entry
    .align 4
tlb_refill_entry:
    /* $t0 を TLBRSAVE に退避（$t1 はリフィル中のみ使用） */
    csrwr   $t0, CSR_TLBRSAVE

    /* 障害 VA を読み、bit[47] でユーザ/カーネル空間を判定 */
    csrrd   $t1, CSR_TLBRBADV
    srli.d  $t0, $t1, 47
    andi    $t0, $t0, 1
    bnez    $t0, 1f
    csrrd   $t0, CSR_PGDL      /* ユーザ空間: PGDL を起点 */
    b       2f
1:
    csrrd   $t0, CSR_PGDH      /* カーネル空間: PGDH を起点 */
2:
    /*
     * lddir/ldpte によるハードウェア支援ページウォーク (4段, 4KiB ページ)
     *   lddir $t0, $t0, 3  → PGD[gi]
     *   lddir $t0, $t0, 2  → PUD[ui]
     *   lddir $t0, $t0, 1  → PMD[mi]
     *   ldpte $t0, 0/1     → 葉 PTE（TLBRELO0/1）
     */
    lddir   $t0, $t0, 3
    lddir   $t0, $t0, 2
    lddir   $t0, $t0, 1
    ldpte   $t0, 0
    ldpte   $t0, 1
    tlbfill

    /* $t0 を復元して返す */
    csrrd   $t0, CSR_TLBRSAVE
    ertn
```

> **注記:** `lddir` の level は RM Vol.1 §7.6 に従う（3=PGD, 2=PUD, 1=PMD）。Rev.6.1 では Linux 4 段テーブルに合わせ 3→2→1 をすべて実行する。`ldpte` は TLBRELO0/1（0x8C/0x8D）へ書き込むため汎用スクラッチに使わない。

### 3.11 T10 — `switch_context` (`switch.S`)

オフセットは `TK_TASKCTX_LOONGARCH` の定義（§3.4）と完全に一致している（Rev.6.0 で構造体サイズ確定後に再検証済み）。

> **Rev.6.1 — 協調切替と CRMD.IE:** `switch_context` は **CRMD.IE を変更しない**。保存した `crmd` をそのまま次タスクへ渡す。スケジューラは切替直前に `irq_disable()`（IE クリア）し、復帰後 `irq_enable()` で再開する（RISC-V TK2 の MIE/MPIE 扱いと同型）。協調切替中にタイマ割り込みが入るとスタック破壊の恐れがある。

```asm
    .globl switch_context
# void switch_context(TK_TASKCTX_LOONGARCH *prev, TK_TASKCTX_LOONGARCH *next)
# $a0 = prev, $a1 = next
switch_context:
    /* prev コンテキスト保存 */
    st.d    $ra,  $a0,  0*8    /* offset   0: ra  */
    st.d    $sp,  $a0,  1*8    /* offset   8: sp  */
    st.d    $fp,  $a0,  2*8    /* offset  16: fp  */
    st.d    $s0,  $a0,  3*8    /* offset  24: s0  */
    st.d    $s1,  $a0,  4*8    /* offset  32: s1  */
    st.d    $s2,  $a0,  5*8    /* offset  40: s2  */
    st.d    $s3,  $a0,  6*8    /* offset  48: s3  */
    st.d    $s4,  $a0,  7*8    /* offset  56: s4  */
    st.d    $s5,  $a0,  8*8    /* offset  64: s5  */
    st.d    $s6,  $a0,  9*8    /* offset  72: s6  */
    st.d    $s7,  $a0, 10*8    /* offset  80: s7  */
    st.d    $s8,  $a0, 11*8    /* offset  88: s8  */
    csrrd   $t0, 0x06
    st.d    $t0,  $a0, 12*8    /* offset  96: era */
    csrrd   $t0, 0x00
    st.d    $t0,  $a0, 13*8    /* offset 104: crmd */

    /* next コンテキスト復元 */
    ld.d    $ra,  $a1,  0*8
    ld.d    $sp,  $a1,  1*8
    ld.d    $fp,  $a1,  2*8
    ld.d    $s0,  $a1,  3*8
    ld.d    $s1,  $a1,  4*8
    ld.d    $s2,  $a1,  5*8
    ld.d    $s3,  $a1,  6*8
    ld.d    $s4,  $a1,  7*8
    ld.d    $s5,  $a1,  8*8
    ld.d    $s6,  $a1,  9*8
    ld.d    $s7,  $a1, 10*8
    ld.d    $s8,  $a1, 11*8
    ld.d    $t0,  $a1, 12*8
    csrwr   $t0, 0x06          /* ERA 復元 */
    ld.d    $t0,  $a1, 13*8
    csrwr   $t0, 0x00          /* CRMD 復元 */
    jr      $ra
```

### 3.12 T11 — 初期タスクスタック

```c
void build_initial_task_stack(TK_TASKCTX_LOONGARCH *ctx,
                              void (*entry)(void),
                              unsigned long *stack_top)
{
    int i;
    for (i = 0; i < 9; i++)
        ctx->s[i] = 0;
    ctx->fp  = 0;
    ctx->sp  = (unsigned long)stack_top;
    ctx->ra  = (unsigned long)entry;
    ctx->era = (unsigned long)entry;
    /*
     * MMU 有効化前: CRMD = PLV0 | DA = 0x08
     * MMU 有効化後: CRMD = PLV0 | PG = 0x10
     * タスクが初めてスケジュールされる時点の MMU 状態に合わせて設定する。
     */
    ctx->crmd = 0x08UL;   /* 初期値: DA モード（mmu_enable 後は 0x10 に更新） */
}
```

### 3.13 T12 — T-Kernel ディスパッチ復帰（`cpu_support_tk.S`）

T-Kernel 2.0 ポートでは `dispatch_entry` がタスクを **`ertn` で復帰**する。LoongArch では **`ertn` が `CSR.PRMD` の内容を `CSR.CRMD` に反映**する（`csrwr` で CRMD を書いても `ertn` で上書きされる）。

> **Rev.6.2 — 実装照合:** `tk_loc_mtx` が `E_CTX`（`ercd=0xffe70000`）で失敗する典型原因は、タスク実行時に **CRMD.IE=0** のままであること。`in_ddsp()` は `isDI(get_crmd())` を含む。

タスク復帰直前の必須手順:

```asm
    ld.d    $t0, $sp, TK2_CTX_MSTATUS   /* 保存済み CRMD イメージ */
    li.d    $t1, 0x0c                   /* PLV0 | DA | IE */
    or      $t0, $t0, $t1
    csrwr   $t0, 0x01                   /* PRMD — ertn で CRMD へ反映 */
    ld.d    $t0, $sp, TK2_CTX_PC
    csrwr   $t0, 0x06                   /* ERA */
    ld.d    $t0, $sp, TK2_CTX_SP
    move    $sp, $t0
    ertn
```

HLNG タスクで `dispatch()` から戻る場合は、保存時に **`ra` を `TK2_CTX_PC`（ERA）に保存**する（RISC-V ポートの `mepc` 問題と同型）。

割り込みハンドラからの `ertn` でも、IE を有効にして復帰する場合は **PRMD** を更新する（CRMD 直書きのみにしない）。

---

## §4 システムコール・将来拡張

### 4.1 システムコール ABI（PLV3 将来）

| 項目 | 規約 |
|------|------|
| 命令 | `syscall 0` |
| 番号 | `$a7` |
| 引数 | `$a0`–`$a5` |
| 戻り値 | `$a0`（エラー副値を `$a1` で渡す場合は実装定義） |
| 例外コード | ECODE = 0x0B（SYSCALL） |
| ERA 補正 | カーネルが `era += 4` で `syscall` の次命令へ進む |

### 4.2 SMP（将来）

- `start.S` の `.Lhalt` を IPI 待ちに拡張  
- per-CPU `$r21` / `TLBRSAVE` スクラッチ  
- スピンロック（`amswap_db` 等、LoongArch アトミック命令）  
- `ECFG.IS[12]`（IPI 割り込み）を有効化

---

## §5 リンカスクリプト (`tk2_loongarch.ld`)

```ld
OUTPUT_ARCH(loongarch)
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x9000000004000000, LENGTH = 256M
}

SECTIONS {
    . = 0x9000000004000000;

    .text : {
        *(.text.init)
        *(.text .text.*)
    } > RAM

    .rodata : { *(.rodata .rodata.*) } > RAM

    .data : {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss .bss.* COMMON)
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_bottom = .;
        . += 0x4000;   /* 16 KiB カーネルスタック */
        _stack_top = .;
    } > RAM

    .percpu (NOLOAD) : {
        _percpu_data = .;
        . += 0x1000;   /* 4 KiB per-CPU データ領域 */
    } > RAM

    _heap_start = .;
}
```

---

## §6 検証フェーズ（完了条件）

| Phase | 内容 | 完了条件 |
|-------|------|----------|
| 1 | ブート | UART に `[TK2] Boot OK` |
| 2 | 例外・タイマ | `irq_enable()` 後、100Hz でハンドラカウンタ増加（UART にカウント出力） |
| 3 | MMU + TLB リフィル | カーネル仮想アドレスで UART/DRAM アクセス、リフィル例外なし |
| 4 | スケジューラ | 2 タスクがラウンドロビン、スタック破壊なし（スタックキャナリで確認） |
| 5 | システムコール | PLV3 タスクから `syscall` でカーネル API（将来） |

---

## 付録 A. Rev.2〜5 技術評価サマリ（統合）

| No. | 箇所 | 内容 | Rev |
|-----|------|------|-----|
| 13 | T1 | IE クリア手順 | v5.0: `and` 方式を規範化 |
| 14 | T5 | PRMD/ERA 復元 | v5.0: 維持 + IE 入口クリア |
| 15 | T6 | cpucfg / TCFG | v5.0: 維持、v6.0: cpucfg 32bit 戻り値注記追加 |
| 16 | T7 | mmu_map_page | v5.0: invtlb 追加、v6.0: invtlb opcode 確定 |
| 17 | T9 | TLB リフィル | v5.0: CSR・bit47 修正、v6.0: lddir/ldpte 注記確定 |
| 18 | T10 | switch オフセット | v5.0: 維持、v6.0: 構造体サイズ確定で再検証 |
| 19 | T4 | 構造体サイズ誤り（120→112） | **v6.0: 修正** |
| 20 | T5b | `idle 0;` 文法エラー | **v6.0: `__asm__ volatile` に修正** |
| 21 | T2 | ECFG 初期化欠落 | **v6.0: exception_vector_init に追加** |
| 22 | 付録B | `$r21` OS予約の説明不足 | **v6.0: 注記追加** |
| 23 | 付録C | PGDL/PGDH 物理アドレス要件の明記不足 | **v6.0: 注記追加** |
| 24 | T7/T9 | 葉 PTE index・TLB ウォーク段数の誤り | **v6.1: 4 段 PGD/PUD/PMD/PTE** |
| 25 | T2 | CRMD.IE 未設定で割り込み未到達 | **v6.1: irq_enable/irq_disable** |
| 26 | T5 | 例外入口のレジスタ保存省略 | **v6.1: 全文記載** |
| 27 | T5b | 例外ディスパッチ順 | **v6.1: ECODE 優先** |
| 28 | T10 | 協調切替時の IE 扱い | **v6.1: §3.11 注記** |

---

## 付録 D. Rev.4 技術査読一覧（v5.0 入力）

| ID | 重大度 | Rev.4 の問題 | v5.0 対応 |
|----|--------|--------------|------|
| L-01 | 致命 | TLBRBADV = 0x1D | **0x89** |
| L-02 | 致命 | 0x8C をスクラッチに使用 | **TLBRELO0 — 禁止** |
| L-03 | 重大 | 空間判定 bit 63 のみ | **badv[47]** |
| L-04 | 重大 | 例外入口 IE 未クリア | §3.5 |
| L-05 | 中 | ESTAT 判定曖昧 | §3.6 |
| L-06 | 中 | 初期 CRMD=0x10 の説明不足 | 付録 C + §3.12 |
| L-07 | 重大 | MMU 有効化手順欠落 | §3.9 T8 |
| L-08 | 中 | データ型・ツールチェーン欠落 | §2A, §2.1 |
| L-09 | 軽微 | T8 番号欠落 | 再採番 |
| L-10 | 軽微 | Markdown 構造不統一 | 全文 |

---

## 付録 E. Rev.5 技術査読一覧（v6.0 入力）

| ID | 重大度 | Rev.5.0 の問題 | v6.0 対応 |
|----|--------|----------------|-----------|
| M-01 | **致命** | `exception_c.c` の `idle 0;` は C 文として不正（コンパイルエラー） | `__asm__ volatile("idle 0")` に修正（§3.6） |
| M-02 | 重大 | `TK_TASKCTX_LOONGARCH` サイズコメント「120 bytes (15\*8)」が誤り（正: 112 bytes, 14 フィールド） | §3.4 のコメント・注記を修正 |
| M-03 | 重大 | `exception_vector_init` に ECFG 初期化なし → タイマ割り込み未到達のリスク | `exception_vector_init` に ECFG 設定を追加（§3.2） |
| M-04 | 中 | PGDL/PGDH に渡す値が「物理アドレス」である要件が未明記 | 付録 C・§3.9 に注記追加 |
| M-05 | 中 | `VA_PTE_SHIFT = 21` を葉 index と誤解（実際は PMD 境界） | v6.0 は注記のみ。**v6.1 で `VA_PAGE_SHIFT=12` に修正**（付録 G N-01） |
| M-06 | 中 | `$r21` の ABI 上の位置付けが曖昧（単に `—` 表記） | 付録 B に「OS 予約（ABI 規定）」と注記追加 |
| M-07 | 軽微 | `invtlb` のオペコード（op フィールド）が未指定 | §3.8 に op=0x5 の根拠と使い分けを追記 |
| M-08 | 軽微 | 改訂履歴 Rev.3/4 の日付が「2026-05」と月精度のみ | `??` プレースホルダに変更し確認指示を追記 |
| M-09 | 軽微 | `lddir` レベル引数の意味が未説明 | §3.10 のコメントに level=3/1 の定義を追記 |
| M-10 | 軽微 | `TLBRELO1`（0x8D）が CSR 表に未掲載 | 付録 C に追記 |

---

## 付録 F. 既知の制限（Rev.6.2）

1. **SMP 未対応** — hart1+ は `idle` ループ。  
2. **ユーザ PLV3** — 仕様のみ。ページテーブルの PLV3 ビットは将来。  
3. **DMW 詳細** — QEMU virt 前提。実 SoC はボード定義が必要。  
4. **フル POSIX / TK2 サービスコール** — 本書は最小 `sysdepend` のみ。  
5. **検証コード** — 各 `T*` はリファレンス実装。**tk2-la64-virt** が Phase 1–4 および M5 の統合テスト。  
6. **FPU / LSX / LASX** — 例外フレームに浮動小数点レジスタは含まない。  
7. **3 段ページテーブル** — Rev.6.0 以前の PGD→PMD→PTE 短縮ウォークは Rev.6.1 で廃止（Linux 4 段に統一）。

---

## 付録 G. Rev.6.0 技術査読一覧（v6.1 入力）

| ID | 重大度 | Rev.6.0 の問題 | v6.1 対応 |
|----|--------|----------------|-----------|
| N-01 | **致命** | `mmu_map_page` の葉 index が `(va>>21)` — bit[20:12] が未使用 | 4 段化、`VA_PAGE_SHIFT=12`（§3.8） |
| N-02 | **致命** | ECFG のみで CRMD.IE=0 のまま割り込み未到達 | `irq_enable()` 追加（§3.2） |
| N-03 | 重大 | `exception_entry.S` の `$r4..$r30` が省略表記 | 保存/復元を全文記載（§3.5） |
| N-04 | 中 | `tk_handle_exception` が IS を ECODE より先に判定 | ECODE 0x0B を先に判定（§3.6） |
| N-05 | 中 | 協調 `switch_context` の IE 扱い未記載 | §3.11 に `irq_disable`/`irq_enable` 要件 |
| N-06 | 軽微 | 構造体サイズのコンパイル時検証なし | `_Static_assert`（§3.4） |
| N-07 | 中 | TLB リフィルが PUD をスキップ（3 段前提） | `lddir` 3→2→1（§3.10） |

---

## 付録 H. QEMU virt 検証済みプラットフォーム値（Rev.6.2）

参考実装 **tk2-la64-virt** で Phase 1–4 および T-Kernel M5（`make TK2=1`）を通した値。実 SoC ではボード定義に置き換える。

| 項目 | 値 | 備考 |
|------|-----|------|
| QEMU | `-machine virt -cpu la464 -bios none -kernel …` | Windows 版 8.x 推奨 |
| UART PA | `0x1fe001e0` | NS16550 |
| UART VA（カーネル） | `0x800000001fe001e0` | DMW0 直映像 |
| リンク VA | `0x9000000004000000` | `linker.ld` / `Kernel.ld` |
| DMW0 / DMW1 | `dmw_init.S` で Linux 起動互換の初期値 | ブート直後 |
| タイマ | CSR TCFG / TICLR、100 Hz | `cpucfg` leaf 4 |
| 協調 sched CRMD | 初期 `0x08`（IE off）、切替後 `irq_enable()` | §3.11 |
| TK2 dispatch | `ertn` 前 **PRMD** に `0x0c` | §3.13、`loc_mtx` E_CTX 回避 |
| 成功ログ（bring-up） | `Phase4 scheduler OK` | 無限 ABAB はデモ上限で打切可 |
| 成功ログ（TK2 M5） | `BABABABAB` + `[M5] COMPLETE` | mutex + rot_rdq |

動作環境の手順: [`docs/ENVIRONMENT.ja.md`](../docs/ENVIRONMENT.ja.md)

---

## 付録 O. Rev.6.1 技術査読一覧（v6.2 入力）

| ID | 重大度 | Rev.6.1 の問題 | v6.2 対応 |
|----|--------|----------------|-----------|
| O-01 | **致命** | UART / MMIO が RISC-V `0x10000000` のまま | §2.3 / §3.3 / 付録 H |
| O-02 | 重大 | リンク `0x40000000` のみ（DMW 未整合） | §5 linker / 付録 H |
| O-03 | **致命** | T-Kernel dispatch が CRMD のみ更新 | §3.13 PRMD + `ertn` |
| O-04 | 軽微 | §0.2「3 段 PT」表記 | 4 段に統一 |
| O-05 | 中 | 実装照合表なし | 付録 H |
| O-06 | 中 | ホスト/WSL/QEMU 手順が分散 | `docs/ENVIRONMENT.ja.md` |

---

**— 文書終端 —**
