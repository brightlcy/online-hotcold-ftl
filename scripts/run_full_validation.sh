#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh scripts/run_fast_tests.sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DATLAS_SANITIZERS=ON
cmake --build build-sanitize --parallel 2
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-sanitize --output-on-failure
