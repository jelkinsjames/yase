#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_TARGET="${BUILD_TARGET:-}"

cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" "$@"

if [[ -n "${BUILD_TARGET}" ]]; then
  cmake --build "${BUILD_DIR}" --target "${BUILD_TARGET}"
else
  cmake --build "${BUILD_DIR}"
fi

