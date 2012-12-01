# also tried -msse2 -mfpmath=sse  but didn't speed up cpu code
gcc -I/usr/local/cuda/include/ -O2 -std=gnu99 example-event.c -o example-event -lOpenCL -lm
