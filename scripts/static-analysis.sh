#!/bin/sh
#
# Run the static analysers over the keyboard sources.
#
# Two passes, because they disagree about what is worth saying and the overlap
# is small:
#
#   cppcheck     - flow-sensitive, finds leaks, uninitialised reads and
#                  out-of-bounds accesses without needing to compile
#   clang-tidy   - compiles for real, so it sees through templates and Qt
#                  macros, and runs the clang static analyser's path-sensitive
#                  checks (see .clang-tidy for the selection)
#
# Both are optional: the script reports which ones it found and skips the rest,
# so it is useful on a machine that only has one of them installed.
#
# Usage:
#   scripts/static-analysis.sh [--cppcheck-only] [--tidy-only]
#                              [--compile-commands DIR] [-- <extra tidy args>]
#
# Exit status is 0 only when every analyser that ran reported nothing.

set -eu

top_dir=$(cd "$(dirname "$0")/.." && pwd)
cd "$top_dir"

sources="src tests"
run_cppcheck=yes
run_tidy=yes
compile_commands=""
tidy_extra=""

while [ $# -gt 0 ]; do
    case "$1" in
        --cppcheck-only) run_tidy=no ;;
        --tidy-only)     run_cppcheck=no ;;
        --compile-commands)
            shift
            compile_commands="${1:-}"
            ;;
        --) shift; tidy_extra="$*"; break ;;
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

status=0
ran_any=no

if [ "$run_cppcheck" = yes ] && command -v cppcheck >/dev/null 2>&1; then
    ran_any=yes
    echo "== cppcheck =="

    # Everything cppcheck knows how to say, including the style category: the
    # tree is expected to come out clean under it, so a finding here is a finding
    # to fix rather than a category to switch off.
    #
    # The suppressions below are about what cppcheck cannot see, not about what
    # we would rather not hear:
    #
    #   missingInclude/missingIncludeSystem
    #       it is not given the Qt, Maliit, presage or hunspell include paths, by
    #       design - resolving them would tie this script to a sysroot
    #   unknownMacro
    #       Q_OBJECT, Q_DECLARE_PRIVATE, Q_DISABLE_COPY, Q_DECLARE_LOGGING_CATEGORY
    #       and friends are moc constructs its preprocessor does not model
    #   unusedFunction
    #       this tree is a plugin and two static libraries; "unused" here means
    #       "no caller in this tree", which is true of everything Maliit calls
    #
    # Anything narrower is an inline cppcheck-suppress comment at the site, with
    # the reason next to it.
    cppcheck \
        --enable=all \
        --inconclusive \
        --std=c++17 \
        --language=c++ \
        --force \
        --quiet \
        --inline-suppr \
        --error-exitcode=1 \
        --suppress=missingInclude \
        --suppress=missingIncludeSystem \
        --suppress=unknownMacro \
        --suppress=unusedFunction \
        --suppress=unmatchedSuppression \
        --suppress=checkersReport \
        -I src -I src/lib -I src/plugin -I src/view -I . \
        $sources || status=1
    echo
fi

if [ "$run_tidy" = yes ] && command -v clang-tidy >/dev/null 2>&1; then
    if [ -z "$compile_commands" ] && [ -f compile_commands.json ]; then
        compile_commands="$top_dir"
    fi

    if [ -z "$compile_commands" ]; then
        cat >&2 <<'EOF'
clang-tidy needs a compilation database and none was found.

The test suite needs nothing but Qt, so the quickest database to generate is
the one for it:

    mkdir build-tidy && cd build-tidy
    qmake6 ../tests/tests.pro
    bear -- make -j"$(nproc)"
    cd .. && scripts/static-analysis.sh --tidy-only --compile-commands build-tidy

That covers the sources the tests compile in. For the whole plugin, configure
the top level project instead - which does need Maliit, presage and hunspell
installed, or a target sysroot:

    qmake6 ../luneos-keyboard.pro CONFIG+=notests

For a cross build the database has to name the target sysroot, since that is
where the headers live; the include paths in the generated Makefiles under the
OE build directory already do.

Skipping clang-tidy.
EOF
    else
        ran_any=yes
        echo "== clang-tidy =="
        # Only the files the database knows about: clang-tidy cannot compile a
        # file it has no command line for, and says so once per file.
        files=$(find $sources -name '*.cpp' -not -name 'moc_*' | sort \
            | while read -r f; do
                  if grep -q "\"$(basename "$f")\"" "$compile_commands/compile_commands.json" 2>/dev/null \
                     || grep -q "/$f\"" "$compile_commands/compile_commands.json" 2>/dev/null; then
                      echo "$f"
                  fi
              done)

        if [ -z "$files" ]; then
            echo "none of the sources appear in $compile_commands/compile_commands.json" >&2
            status=1
        else
            # clang-tidy exits 0 for findings unless WarningsAsErrors is set, and
            # setting that in .clang-tidy would also fail the build for anyone
            # running it through their editor. So judge it by what it printed:
            # "N warnings generated." is its own progress reporting and does not
            # count, a line naming a check in brackets does.
            tidy_log=$(mktemp)
            # shellcheck disable=SC2086
            clang-tidy -p "$compile_commands" --quiet $tidy_extra $files \
                >"$tidy_log" 2>&1 || status=1
            cat "$tidy_log"
            if grep -qE '\[[a-z0-9-]+-[a-z0-9.-]+\]$' "$tidy_log"; then
                status=1
            fi
            rm -f "$tidy_log"
        fi
        echo
    fi
fi

if [ "$ran_any" = no ]; then
    echo "No analyser available. Install cppcheck and/or clang-tidy." >&2
    exit 2
fi

if [ "$status" -eq 0 ]; then
    echo "static analysis: clean"
else
    echo "static analysis: findings above" >&2
fi

exit "$status"
