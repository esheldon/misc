/*
 
 A brute force n body code, just for fun.

 This code is c99, make sure to compile it with a c99 compiler!
 e.g. with gcc, use 

   gcc --std=c99 -o nb nb.c

 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct universe {
    double xmax;
    double ymax;
    double zmax;
};

struct particle {
    double x;
    double y;
    double z;

    double vx;
    double vy;
    double vz;
};

struct particles {
    size_t size;
    struct particle* p;
};

struct particles* particles_new(size_t n) {
    struct particles* particles;
    particles = malloc(sizeof(struct particles));
    if (particles == NULL) {
        printf("Could not malloc struct particles");
        exit(EXIT_FAILURE);
    }

    particles->size = n;
    particles->p = malloc(n*sizeof(struct particle));
    if (particles == NULL) {
        printf("Could not malloc %ld struct particle", n);
        exit(EXIT_FAILURE);
    }


    return particles;

}

void initialize(struct particles* particles) {
    // just place them uniformly in the box with zero velocity

    for (size_t i=0; i<particles->size; i++) {
        particles->p[i].vx=0;
        particles->p[i].vy=0;
        particles->p[i].vz=0;

    }
}

int main(int argc, char* argv) {

    struct universe universe;
    size_t n = 1000;
    struct particles* particles;

    particles = particles_new(n);

    initialize(particles);

    /* don't bother to clean up... */
}
