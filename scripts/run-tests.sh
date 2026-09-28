#!/bin/sh
#
# Configure, build and run the unit test suite.
#
# This configures tests/tests.pro rather than the top level project, on purpose:
# the suite compiles the sources it covers straight into each test binary, so it
# needs nothing but Qt. Maliit, presage, hunspell and LunaNext are not required,
# which means the tests run on a plain developer machine and in CI without a
# LuneOS sysroot. Pass --full to build the whole project instead, which does
# need all of it.
#
# By default the tests are built with AddressSanitizer and
# UndefinedBehaviorSanitizer. Much of what they cover is parsing files and
# capability bitmaps that something else wrote, where an out-of-bounds read is
# the failure mode and a passing test without an instrumented build proves
# little. Pass --no-sanitizers for a plain build.
#
# Usage:
#   scripts/run-tests.sh [--no-sanitizers] [--full] [--build-dir DIR] [--keep]
#                        [--qmake PATH] [-- <extra qmake args>]
#
# Environment:
#   QMAKE           qmake binary to use (default: qmake6, then qmake)
#   QT_QPA_PLATFORM forced to "offscreen" unless already set
#
# Cross builds: this script runs the binaries it builds, so it is for a native
# build. To build the suite for a device, drop CONFIG+=notests from the qmake
# invocation the recipe makes and run "make -C tests check" on the target.

set -eu

top_dir=$(cd "$(dirname "$0")/.." && pwd)
build_dir="$top_dir/build-tests"
sanitizers=yes
full=no
keep=no
qmake_bin="${QMAKE:-}"
qmake_extra=""

while [ $# -gt 0 ]; do
    case "$1" in
        --no-sanitizers) sanitizers=no ;;
        --full)          full=yes ;;
        --keep)          keep=yes ;;
        --build-dir)     shift; build_dir="${1:-}" ;;
        --qmake)         shift; qmake_bin="${1:-}" ;;
        --)              shift; qmake_extra="$*"; break ;;
        -h|--help)
            sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "unknown argument: $1" >&2
            exit 2
            ;;
    esac
    shift
done

if [ -z "$qmake_bin" ]; then
    for candidate in qmake6 qmake; do
        if command -v "$candidate" >/dev/null 2>&1; then
            qmake_bin="$candidate"
            break
        fi
    done
fi

if [ -z "$qmake_bin" ]; then
    echo "No qmake found. Set QMAKE or pass --qmake." >&2
    exit 2
fi

sanitizer_args=""
if [ "$sanitizers" = yes ]; then
    sanitizer_args="CONFIG+=sanitizer CONFIG+=sanitize_address CONFIG+=sanitize_undefined"

    # Without this an overflow that UBSan merely prints keeps the exit status at
    # zero and the run looks clean.
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-}:print_stacktrace=1:halt_on_error=1"
    UBSAN_OPTIONS="${UBSAN_OPTIONS#:}"
    export UBSAN_OPTIONS

    # Qt allocates a fair amount it never frees at exit by design; leak reporting
    # here is noise, while the use-after-free and overflow detection is the point.
    ASAN_OPTIONS="${ASAN_OPTIONS:-}:detect_leaks=0"
    ASAN_OPTIONS="${ASAN_OPTIONS#:}"
    export ASAN_OPTIONS
fi

if [ "$keep" = no ]; then
    rm -rf "$build_dir"
fi
mkdir -p "$build_dir"

: "${QT_QPA_PLATFORM:=offscreen}"
export QT_QPA_PLATFORM

if [ "$full" = yes ]; then
    project="$top_dir/luneos-keyboard.pro"
    check_dir="tests"
else
    project="$top_dir/tests/tests.pro"
    check_dir="."
fi

echo "== configuring $project in $build_dir =="
cd "$build_dir"
# shellcheck disable=SC2086
"$qmake_bin" "$project" $sanitizer_args $qmake_extra

echo "== building =="
make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

echo "== running tests =="
# "make check" recurses into each test directory and runs the binary in place.
make -C "$check_dir" check
