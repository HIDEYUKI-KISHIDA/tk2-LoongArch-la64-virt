#!/bin/bash
# Launch Windows QEMU from WSL (ASCII temp path; avoids Japanese path mojibake).
set -euo pipefail
cd "$(dirname "$0")/.."

TK2=0
for a in "$@"; do
  case "$a" in
    -Tk2|--tk2) TK2=1 ;;
  esac
done

WIN_USER="$(sed -n 's|.*/Users/\([^/]*\)/.*|\1|p' <<< "$(pwd)")"
[ -z "$WIN_USER" ] && WIN_USER="h_kis"
TEMP_DIR="/mnt/c/Users/${WIN_USER}/AppData/Local/Temp"

if [ "$TK2" -eq 1 ]; then
  SRC="build/tk2-la64-tk.elf"
  DST="${TEMP_DIR}/tk2-la64-tk.elf"
else
  SRC="tk2-la64.elf"
  DST="${TEMP_DIR}/tk2-la64.elf"
fi

if [ ! -f "$SRC" ]; then
  echo "missing $SRC - run: make${TK2:+ TK2=1}" >&2
  exit 1
fi

cp -f "$SRC" "$DST"
WIN_DST="C:\\Users\\${WIN_USER}\\AppData\\Local\\Temp\\$(basename "$DST")"
WIN_LOG="C:\\Users\\${WIN_USER}\\AppData\\Local\\Temp\\tk2-la64-serial.log"
LOG="${TEMP_DIR}/tk2-la64-serial.log"
rm -f "$LOG"

QEMU="/mnt/c/Program Files/qemu/qemu-system-loongarch64.exe"
if [ ! -f "$QEMU" ]; then
  echo "QEMU not found: $QEMU" >&2
  exit 1
fi

echo "Kernel: $WIN_DST"
echo "Serial log: $LOG"
echo "Exit QEMU: Ctrl+A then X (or wait for timeout below)"
echo ""

timeout 12 "$QEMU" -accel tcg,thread=single -machine virt -cpu la464 -m 2G -smp 1 \
  -display none -serial "file:${WIN_LOG}" \
  -kernel "${WIN_DST}" || true

echo "--- guest serial output ---"
if [ -f "$LOG" ]; then
  cat "$LOG"
else
  echo "(no serial log file — QEMU may have failed to start)"
fi
