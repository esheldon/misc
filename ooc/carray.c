#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int num=10000000;
    int i;
    float tot=0;
    float* ptr;

    float* data = (float*) malloc(num*sizeof(float));

    ptr = data;
    for (i=0;i<num;i++) {
        *ptr = i;
        ptr++;
    }

    ptr = data;
    for (i=0;i<num;i++) {
        tot += *ptr;
        ptr++;
    }

    printf("total: %f\n", tot);

}
