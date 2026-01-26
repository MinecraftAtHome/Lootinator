@echo off
setlocal

for /L %%i in (0,1,100) do (
    python .\assertions.py %%i > out_test.txt
    python .\assertions_brute.py %%i > out_expected.txt
    python .\test_checker.py out_test.txt out_expected.txt > out_checker.txt
    if errorlevel 1 (
        echo Test %%i failed!
        exit /b 1
    )
    echo Test %%i OK
)

endlocal
