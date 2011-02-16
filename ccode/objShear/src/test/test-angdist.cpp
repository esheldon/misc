#include <stdlib.h>
#include <stdio.h>
#include "../angDist.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("usage: test-angdist z\n");
        exit(45);
    }

    float z = atof(argv[1]);

    float omega_m = 0.30;
    float H0 = 100.0;

    float d = angDist(H0, omega_m, z);

    printf("omega_m: %f\n", omega_m);
    printf("H0: %f\n", H0);
    printf("DL(%f) = %f Mpc\n", z, d);

}
