#!/bin/bash
# WSL: bash build.sh
set -e
cd "$(dirname "$0")"
sed -i 's/\r$//' Makefile 2>/dev/null || true
make clean
make "$@"
