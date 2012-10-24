# also tried -msse2 -mfpmath=sse  but didn't speed up cpu code
gcc -I/usr/local/cuda/include/ -O2 -std=gnu99 example-2d-xyarr-gpuclust.c -o example-2d-xyarr-gpuclust -lOpenCL -lm
