#!/usr/bin/env bash

# hilariously complicated, I know

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${SCRIPT_DIR}/../src"
OUT_FILE="${SCRIPT_DIR}/../compile_commands.json"

to_unix_path() {
  local p="$1"
  p="${p//$'\r'/}"
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -u "$p" 2>/dev/null | tr -d '\r' || printf '%s\n' "$p"
  else
    printf '%s\n' "$p"
  fi
}

find_compiledb() {
  local candidate base scripts_dir cmd sys_scripts pip_location pip_prefix

  if command -v compiledb >/dev/null 2>&1; then
    command -v compiledb
    return 0
  fi

  for cmd in python3 python py; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
      continue
    fi

    # Ask Python directly where script entrypoints are installed.
    sys_scripts="$("$cmd" -c "import sysconfig; print(sysconfig.get_path('scripts') or '')" 2>/dev/null || true)"
    if [ -n "$sys_scripts" ]; then
      sys_scripts="$(to_unix_path "$sys_scripts")"
      for candidate in "$sys_scripts/compiledb" "$sys_scripts/compiledb.exe"; do
        if [ -f "$candidate" ]; then
          printf '%s\n' "$candidate"
          return 0
        fi
      done
    fi

    base="$("$cmd" -m site --user-base 2>/dev/null || true)"
    if [ -n "$base" ]; then
      base="$(to_unix_path "$base")"
      for scripts_dir in "$base/bin" "$base/Scripts"; do
        for candidate in "$scripts_dir/compiledb" "$scripts_dir/compiledb.exe"; do
          if [ -f "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
          fi
        done
      done
    fi

    # Fallback: derive Scripts/bin from pip show Location.
    pip_location="$("$cmd" -m pip show compiledb 2>/dev/null | sed -n 's/^Location:[[:space:]]*//p' | head -n 1 || true)"
    if [ -n "$pip_location" ]; then
      pip_location="$(to_unix_path "$pip_location")"
      pip_prefix="${pip_location%/Lib/site-packages}"
      if [ "$pip_prefix" = "$pip_location" ]; then
        pip_prefix="${pip_location%/lib/python*/site-packages}"
      fi
      for scripts_dir in "$pip_prefix/Scripts" "$pip_prefix/bin"; do
        for candidate in "$scripts_dir/compiledb" "$scripts_dir/compiledb.exe"; do
          if [ -f "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
          fi
        done
      done
    fi
  done

  return 1
}

COMPILEDB_BIN="$(find_compiledb)" || {
  echo "Error: could not locate compiledb. Install it with 'pip install compiledb' or add it to PATH." >&2
  exit 1
}

cd "$SRC_DIR"

make clean
make sdl2w_rebuild
build_failed=0
if ! "$COMPILEDB_BIN" make -j8; then
  build_failed=1
fi

if [ ! -f compile_commands.json ]; then
  echo "Error: compiledb did not produce compile_commands.json." >&2
  exit 1
fi

mv compile_commands.json "$OUT_FILE"

if [ "$build_failed" -ne 0 ]; then
  echo "Warning: build/link step failed, but compile_commands.json was generated." >&2
fi