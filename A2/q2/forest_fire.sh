#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="${SCRIPT_DIR}/forest_fire"

# Compile if binary doesn't exist or source is newer
if [ ! -f "$BINARY" ] || [ "${SCRIPT_DIR}/forest_fire.cpp" -nt "$BINARY" ]; then
    g++ -O2 -std=c++17 -o "$BINARY" "${SCRIPT_DIR}/forest_fire.cpp"
fi

"$BINARY" "$1" "$2" "$3" "$4" "$5" "$6"