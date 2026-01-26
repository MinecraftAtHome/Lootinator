#!/usr/bin/env bash
set -euo pipefail

for ((i=1; i<=10; i++)); do
    python ./assertions.py "$i" > out_test.txt
    python ./assertions_brute.py "$i" > out_expected.txt
    python ./test_checker.py out_test.txt out_expected.txt > out_checker.txt

    exit_code=$?
    if [ "$exit_code" -ne 0 ]; then
        echo "Test $i failed!"
        exit 1
    fi

    echo "Test $i OK"
done
