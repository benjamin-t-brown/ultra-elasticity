#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="${SCRIPT_DIR}/.."
TOOLS_DIR="${REPO_ROOT}/sdl2w/src/build/tools"
RUN_DIR="${REPO_ROOT}/src"
ASSETS_DIR="${REPO_ROOT}/src/assets"
SPRITES_FILE="${ASSETS_DIR}/sprites.txt"
ANIMS_FILE="${ASSETS_DIR}/anims.txt"
SOUNDS_FILE="${ASSETS_DIR}/sounds.txt"

if [ ! -d "${TOOLS_DIR}" ]; then
  echo "Error: tools directory not found at ${TOOLS_DIR}" >&2
  echo "Build the tools first: (cd sdl2w/src && make tools)" >&2
  exit 1
fi

ANIMS_BIN="${TOOLS_DIR}/Anims"
if [ -f "${TOOLS_DIR}/Anims.exe" ]; then
  ANIMS_BIN="${TOOLS_DIR}/Anims.exe"
fi

if [ ! -f "${ANIMS_BIN}" ]; then
  echo "Error: Anims binary not found in ${TOOLS_DIR}" >&2
  echo "Build it first: (cd sdl2w/src && make tools)" >&2
  exit 1
fi

if [ ! -f "${SPRITES_FILE}" ]; then
  echo "Error: sprites file not found at ${SPRITES_FILE}" >&2
  exit 1
fi

if [ ! -f "${ANIMS_FILE}" ]; then
  echo "Error: anims file not found at ${ANIMS_FILE}" >&2
  exit 1
fi

if [ ! -f "${SOUNDS_FILE}" ]; then
  echo "Error: sounds file not found at ${SOUNDS_FILE}" >&2
  exit 1
fi

if [ ! -d "${RUN_DIR}" ]; then
  echo "Error: run directory not found at ${RUN_DIR}" >&2
  exit 1
fi



cd "${RUN_DIR}"
cmd=(
  "${ANIMS_BIN}"
  --assets-dir "${ASSETS_DIR}"
  --sprites-file "${SPRITES_FILE}"
  --anims-file "${ANIMS_FILE}"
  --sounds-file "${SOUNDS_FILE}"
  "$@"
)
printf 'Running:'
printf ' %q' "${cmd[@]}"
printf '\n'
"${ANIMS_BIN}" \
  --assets-dir "${ASSETS_DIR}" \
  --sprites-file "${SPRITES_FILE}" \
  --anims-file "${ANIMS_FILE}" \
  --sounds-file "${SOUNDS_FILE}" \
  "$@"
