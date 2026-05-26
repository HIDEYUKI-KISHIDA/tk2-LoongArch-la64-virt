# TK2 LoongArch Design Specification

> **Language:** English (this file) | [Japanese normative spec](Tk2_LoongArch_LA64_Specification_v6_2.ja.md) (full assembly listings)

**Document code:** TK2-LA64-SPEC  
**Version:** Rev.6.2 (QEMU virt implementation review)  
**Date:** 2026-05-24  
**Status:** Canonical (English operational edition)  
**Previous:** Rev.6.1

---

## Revision history (summary)

| Rev | Date | Summary |
|-----|------|---------|
| 6.2 | 2026-05-24 | tk2-la64-virt review: UART/link/DMW, PRMD+`ertn` dispatch, Appendix H |
| 6.1 | 2026-05-26 | 4-level PT, `irq_enable`, full exception entry, switch IE notes |
| 6.0 | 2026-05-26 | Struct sizes, ECFG, `idle` asm, CSR notes |

Full tables: [CHANGELOG.en.md](../CHANGELOG.en.md) / [CHANGELOG.md](../CHANGELOG.md).

---

## §0 Document control

### 0.1 Purpose

Normative design for porting **T-Kernel 2.0 (TK2)** to **LoongArch LA64**: `sysdepend`, boot, MMU, exceptions, and scheduler. Same design principles as x86_64 / AArch64 / RISC-V TK2 ports (32-bit `INT`/`UINT`, separate TRAP/TASK contexts).

### 0.2 Scope

| In scope | Out of scope (future) |
|----------|------------------------|
| Single-core, PLV0 kernel boot | SMP / IPI |
| Exceptions, timer, UART | Full user-space POSIX |
| **4-level** page table + TLB refill | KVM / virtualization extension |
| Cooperative `switch_context` | Dynamic module load |

### 0.3 Normative terms

- **MUST** — required for a conforming implementation  
- **SHOULD** — strongly recommended  
- **MAY** — optional / example  

### 0.4 References

- LoongArch Reference Manual Vol.1 / Vol.2  
- LoongArch ELF ABI  
- QEMU `loongarch64` `virt` machine  
- Linux `arch/loongarch` (CSR numbers, TLB refill conventions)

---

## §1 Architecture

### 1.1 CPU

- **ISA:** LoongArch64 (LA64)  
- **Page size:** 4 KiB  
- **Virtual address:** 48-bit effective; Linux-style split at `badv[47]`

### 1.2 MMU (vs RISC-V)

1. **4-level page table** (PGD → PUD → PMD → PTE; Linux `CONFIG_PGTABLE_LEVELS=4`)  
2. Dedicated **TLB refill** vector `CSR.TLBRENTRY` (`tlb_refill_entry`)  
3. Hardware-assisted walk: `lddir` / `ldpte` / `tlbfill`  
4. Boot: **direct map (CRMD.DA=1)**; after MMU enable: **paged mode (CRMD.PG=1)**

### 1.3 Priority

1. Single-core boot (`Boot OK` on UART)  
2. Exceptions, interrupts, 100 Hz timer  
3. TLB refill + kernel virtual map  
4. `switch_context` two-task round-robin  
5. PLV3 user + `syscall` (future)

---

## §2 Runtime environment

Detailed host/tool/QEMU steps: **[docs/ENVIRONMENT.en.md](../docs/ENVIRONMENT.en.md)**.

### 2.1 Toolchain (recommended)

| Item | Recommendation |
|------|----------------|
| GCC | `loongarch64-linux-gnu-gcc` 12+ (verified: **gcc-14-loongarch64-linux-gnu**) |
| QEMU | **8.0+**, `-bios none -kernel` |
| Model | LP64 (`-march=loongarch64 -mabi=lp64d`) |

### 2.2 QEMU launch

```bash
qemu-system-loongarch64 \
  -machine virt \
  -cpu la464 \
  -bios none \
  -kernel tk2.elf \
  -m 256M \
  -display none \
  -serial mon:stdio
```

### 2.3 QEMU `virt` map (Rev.6.2)

> **Do not use `0x1000_0000` for UART** — that is **RISC-V `virt`**, not LoongArch.

| Kind | Address | Notes |
|------|---------|--------|
| UART (NS16550) PA | `0x1fe0_01e0` | Physical |
| Kernel UART VA | `0x8000_0000_1fe0_01e0` | DMW0 direct map (uncached) |
| Kernel link VA | `0x9000_0000_0400_0000` | `linker.ld` ORIGIN |
| DRAM (reference) | `0x4000_0000` region | Board/QEMU specific |

```c
#define UART_BASE  0x800000001fe001e0UL
```

Boot code MUST initialize **DMW0/DMW1** (`dmw_init.S`) before using these VAs.

### 2.4 Privilege

| PLV | Use in TK2 |
|-----|------------|
| 0 | Kernel, exception handlers |
| 3 | Future user tasks |

---

## §2A Data types (LP64 TK2 ABI)

| Type | LA64 rule |
|------|-----------|
| INT / UINT | **32-bit `int`** (API compatibility) |
| D / UD, pointers | 64-bit `long` |
| REGINT / REGUINT | 64-bit |

---

## §3 Implementation modules (summary)

Full assembly listings: **Japanese canonical spec §3**.

### Boot order (MUST)

1. `start.S` — DMW, BSS, stack  
2. `exception_vector_init()` — ECFG timer bit, `EENTRY`, `TLBRENTRY`  
3. `kernel_main` → `timer_init()` → **`irq_enable()`** (ECFG alone is not enough; **CRMD.IE** required)  
4. Tests / scheduler  

### T3 — UART

Use `UART_BASE` from §2.3. Poll LSR bit 0x20 (THRE) before writing THR.

### T2 — Interrupt enable

```c
void irq_enable(void) {
    unsigned long crmd;
    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd |= (1UL << 2);  /* CRMD.IE */
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}
```

### T5 — Exception entry

- Clear **CRMD.IE** on entry before saving frame  
- Save `$r1..$r31`, ERA, CRMD, PRMD, ESTAT, BADV (288-byte `TK_EXFRAME_LOONGARCH`)  
- On return: restore **ERA** and **PRMD**; **`ertn` loads CRMD from PRMD**

### T5b — `tk_handle_exception`

1. ECODE `0x0B` — SYSCALL (stub), `era += 4`  
2. `ESTAT` interrupt mask — timer bit 11 → `timer_interrupt_handler()`  
3. Else — panic  

### T6 — Timer

- CSR `TCFG` (0x41), `TICLR` (0x44)  
- Frequency from `cpucfg` leaf 4  
- 100 Hz: `interval = freq / 100`, `TCFG = EN | LOOP | (interval << 2)`

### T10 — Cooperative `switch_context`

- Save/restore `ra`, `sp`, `fp`, `s0..s8`, ERA (csr 0x06), CRMD (csr 0x00)  
- **Do not** set **IE** in initial task CRMD; use `irq_disable()` before switch, `irq_enable()` after (Rev.6.1)  
- Initial `crmd = 0x08` (PLV0 | DA), not `0x0c`

### T12 — T-Kernel dispatch (`cpu_support_tk.S`) — Rev.6.2

**`ertn` copies `PRMD` → `CRMD`.** Writing CRMD alone before `ertn` does not enable interrupts.

Task resume MUST:

```asm
    ld.d    $t0, $sp, TK2_CTX_MSTATUS
    li.d    $t1, 0x0c              /* PLV0 | DA | IE */
    or      $t0, $t0, $t1
    csrwr   $t0, 0x01              /* PRMD */
    ld.d    $t0, $sp, TK2_CTX_PC
    csrwr   $t0, 0x06              /* ERA */
    /* restore sp, callee-saved */
    ertn
```

For HLNG tasks, save **`ra` into TK2_CTX_PC (ERA)** when preempted from a `dispatch()` call (same class of bug as RV64 `mepc`).

If `tk_loc_mtx` returns **`E_CTX` (`ercd=0xffe70000`)**, the task is likely running with **CRMD.IE=0** (`in_ddsp()` includes interrupt-disabled state).

### Linker (Rev.6.2)

```ld
MEMORY {
    RAM (rwx) : ORIGIN = 0x9000000004000000, LENGTH = 256M
}
SECTIONS {
    . = 0x9000000004000000;
    ...
}
```

---

## §6 Verification phases

| Phase | Content | Done when |
|-------|---------|-----------|
| 1 | Boot | UART shows `[TK2] LoongArch LA64 Boot OK` |
| 2 | Exceptions / timer | After `irq_enable()`, 100 Hz tick (poll or IRQ) |
| 3 | MMU + TLB refill | Kernel VA access without refill storm |
| 4 | Scheduler | Cooperative A/B pattern, no stack corruption |
| 5 | Syscall | PLV3 + `syscall` (future) |

Reference port expected logs: [ENVIRONMENT.en.md](../docs/ENVIRONMENT.en.md).

---

## Appendix C — Key CSRs (excerpt)

| CSR | Num | Role |
|-----|-----|------|
| CRMD | 0x00 | Current mode, IE (bit 2), DA, PG |
| PRMD | 0x01 | Pre-exception mode (**source for `ertn` → CRMD**) |
| ESTAT | 0x05 | Exception / interrupt status |
| ERA | 0x06 | Exception return address |
| ECFG | 0x04 | Interrupt enable configuration |
| EENTRY | 0x0C | Generic exception entry |
| TLBRENTRY | 0x88 | TLB refill entry |
| TCFG / TICLR | 0x41 / 0x44 | Core timer |

Full table: Japanese spec Appendix C.

---

## Appendix H — QEMU virt verified values (Rev.6.2)

| Item | Value |
|------|--------|
| QEMU | `-machine virt -cpu la464 -bios none -kernel …` |
| UART PA | `0x1fe001e0` |
| UART kernel VA | `0x800000001fe001e0` |
| Link VA | `0x9000000004000000` |
| DMW | `dmw_init.S` at boot |
| Timer | 100 Hz, `cpucfg` leaf 4 |
| Cooperative sched CRMD | `0x08` initial; `irq_enable()` after switch |
| TK2 dispatch | **PRMD `0x0c` before `ertn`** |
| Bring-up success | `Phase4 scheduler OK` |
| TK2 M5 success | `BABABABAB` + `[M5] COMPLETE` |

---

## Appendix O — Rev.6.1 → v6.2 fixes

| ID | Summary |
|----|---------|
| O-01 | UART / MMIO not RISC-V `0x10000000` |
| O-02 | Link VA + DMW |
| O-03 | TK dispatch **PRMD + `ertn`** |
| O-04 | 4-level PT wording |
| O-05 | Appendix H |
| O-06 | [ENVIRONMENT.en.md](../docs/ENVIRONMENT.en.md) |

---

**— End of English operational specification —**

For normative assembly source, CSR commentary, and historical appendices D-G, use [Tk2_LoongArch_LA64_Specification_v6_2.ja.md](Tk2_LoongArch_LA64_Specification_v6_2.ja.md) in this directory.
