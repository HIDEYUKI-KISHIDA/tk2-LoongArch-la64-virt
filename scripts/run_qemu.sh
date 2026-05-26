#!/bin/bash
# QEMU launch for tk2-la64 bare-metal (Rev.6.1 @ 0x40000000)
set -euo pipefail
cd "$(dirname "$0")/.."
ELF="${1:-tk2-la64.elf}"

if [[ ! -f "$ELF" ]]; then
  echo "missing $ELF — run: make" >&2
  exit 1
fi

# WSL + QEMU 8.2: virt+loader often hits iothread mutex assert — use single TCG thread.
if grep -qi microsoft /proc/version 2>/dev/null; then
  ACCEL="${QEMU_ACCEL:-tcg,thread=single}"
  echo "WSL detected: QEMU_ACCEL=${ACCEL}" >&2
else
  ACCEL="${QEMU_ACCEL:-tcg}"
fi

exec qemu-system-loongarch64 \
  -accel "${ACCEL}" \
  -machine virt \
  -cpu la464 \
  -m 2G \
  -smp 1 \
  -nographic \
  -serial mon:stdio \
  -device "loader,file=${ELF},addr=0x40000000,cpu-num=0"
