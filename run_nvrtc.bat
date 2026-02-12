cd build\example
Debug\example.exe
cd ..\..

copy /Y build\example\kernel.shm kernel-launcher\auto-benchmark\temp\kernel.shm
copy /Y build\example\first_cuda.cu kernel-launcher\auto-benchmark\temp\first_cuda.cu

cd kernel-launcher\auto-benchmark
launcher.exe --run-single --use-config temp/conf.json
