#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT_DIR/numerics/strlen_bench"

if [ "$#" -gt 0 ]; then
  sizes=("$@")
else
  sizes=(64 256 1024 4096 65536 1048576 16777216)
fi

for size in "${sizes[@]}"; do
  make -B -C "$ROOT_DIR" STRLEN_BENCH_LEN="$size" strlen-bench >/dev/null
  "$BIN"
  printf '\n'
done
