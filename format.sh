#!/usr/bin/env bash
set -e

echo "Formatting C/C++ files..."

find . \
    \( -path "./build" -o -path "./.git" -o -path "./external" \) -prune -o \
    -type f \
    \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" \
       -o -name "*.h" -o -name "*.hh" -o -name "*.hpp" -o -name "*.hxx" \) \
    -exec clang-format-16 -i {} +

echo "Done."