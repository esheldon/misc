#include <stdio.h>
#include <stdlib.h>
#include "../cosmo.h"

int main(int argc, char** argv) {
    double H0=70;
    int flat=1;
    double omega_m=0.3;
    double omega_l=0.7;
    double omega_k=0.0;

    struct cosmo* c=cosmo_new(H0,flat,omega_m,omega_l,omega_k);
    cosmo_print(c);

    free(c);
}
