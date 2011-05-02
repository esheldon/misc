/*
 
 A brute force n body code, just for fun.  For now, G, masses, and universe
 size are unity.

 This code is c99, make sure to compile it with a c99 compiler!
 e.g. with gcc, use 

   gcc --std=c99 -o nb nb.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>


struct particle {
    
    double x[3];  // position
    double v[3];  // velocity
};

// not currently used
struct particles {
    int64_t size;
    struct particle* data;
};

struct universe {
    int64_t nperside;
    int64_t nparticles;
    int64_t nstep;

    double xmax[3];
    double tstep;

    double xsoft;
    double xsoft2;

    struct particle* particles;
};

void print_universe(struct universe* u) {
    printf("xmax: %lf\n", u->xmax[0]);
    printf("nparticles: %ld\n", u->nparticles);
    printf("nstep: %ld\n", u->nstep);
    printf("tstep: %lf\n", u->tstep);
    printf("xsoft: %lf\n", u->xsoft);
}

struct particles* particles_new(int64_t n) {
    struct particles* particles;
    particles = malloc(sizeof(struct particles));
    if (particles == NULL) {
        printf("Could not malloc struct particles");
        exit(EXIT_FAILURE);
    }

    particles->size = n;
    particles->data = malloc(n*sizeof(struct particle));
    if (particles == NULL) {
        printf("Could not malloc %ld struct particle", n);
        exit(EXIT_FAILURE);
    }


    return particles;

}

void set_initial_conditions(struct universe* u) {

    // for now just place them uniformly in the box with zero velocity
    double dx = u->xmax[0]/u->nperside;
    struct particle* particles = u->particles;

    int64_t i=0;
    double x,y,z;
    for (int64_t ix=0; ix< u->nperside; ix++) {
        x = (ix+0.5)*dx;
        for (int64_t iy=0; iy< u->nperside; iy++) {
            y = (iy+0.5)*dx;
            for (int64_t iz=0; iz< u->nperside; iz++) {
                z = (iz+0.5)*dx;

                particles[i].v[0]=0;
                particles[i].v[1]=0;
                particles[i].v[2]=0;

                particles[i].x[0] = x;
                particles[i].x[1] = y;
                particles[i].x[2] = z;

                i += 1;
            }
        }
    }
}


struct universe* universe_new(int64_t nstep, int64_t nperside, double xmax, double tstep, double xsoft) {

    struct universe* u;
    u = malloc(sizeof(struct universe));
    if (u == NULL) {
        printf("Could not malloc struct universe");
        exit(EXIT_FAILURE);
    }

    u->nstep = nstep;
    u->nperside = nperside;
    u->nparticles = nperside*nperside*nperside;
    u->xmax[0] = u->xmax[1] = u->xmax[2] = xmax;
    u->tstep = tstep;
    u->xsoft = xsoft;
    u->xsoft2 = xsoft*xsoft;

    u->particles = malloc(u->nparticles*sizeof(struct particle));
    if (u->particles == NULL) {
        printf("Could not malloc %ld struct particle", u->nparticles);
        exit(EXIT_FAILURE);
    }

    set_initial_conditions(u);

    return u;
}


FILE* open_output(const char* filename) {
    FILE* fptr = fopen(filename, "w");
    if (fptr==NULL) {
        printf("Could not open file: '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
    return fptr;
}

void write_header(FILE* fptr, struct universe* u) {
    fwrite(&u->nstep,      sizeof(int64_t), 1, fptr);
    fwrite(&u->nparticles, sizeof(int64_t), 1, fptr);
    fwrite(&u->xmax,       sizeof(double),  1, fptr);
    fwrite(&u->tstep,      sizeof(double),  1, fptr);
}
void write_step(FILE* fptr, struct universe* u, int64_t step) {
    struct particle* p = u->particles;

    fwrite(&step, sizeof(int64_t), 1, fptr);
    for (int64_t i=0; i< u->nparticles; i++) {
        fwrite(p[i].x, sizeof(double), 3, fptr);
        fwrite(p[i].v, sizeof(double), 3, fptr);
    }
}

// magnitude squared
double mag2(double x[3]) {
    return x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
}

void get_accel(struct universe* u, double x[3], double accel[3]) {

    // brute force calculation of accel

    double deltax[3];
    double r2, r2inv, r2invsqrt;
    double A;

    accel[0]=accel[1]=accel[2]=0;
    for (int64_t i=0; i< u->nparticles; i++) {
        deltax[0] = u->particles[i].x[0] - x[0];
        deltax[1] = u->particles[i].x[1] - x[1];
        deltax[2] = u->particles[i].x[2] - x[2];

        r2 = mag2(deltax) + u->xsoft2;
        if (r2 > 0) {
            r2inv = 1./r2;

            // note assuming G=1 and masses=1
            A = r2inv;

            // now get vector accel

            r2invsqrt = sqrt(r2inv);
            accel[0] += A*deltax[0]*r2invsqrt;
            accel[1] += A*deltax[1]*r2invsqrt;
            accel[2] += A*deltax[2]*r2invsqrt;
        }
    }
}

void print_particle(struct particle* p) {
    printf("  x: [%lf, %lf, %lf]\n", p->x[0], p->x[1], p->x[2]);
    printf("  v: [%lf, %lf, %lf]\n", p->v[0], p->v[1], p->v[2]);
}

void take_step(struct universe* u) {

    // brute force our way: 
    //  - calculate the force on each particle from all other particles
    //  - calculate the dp = F*dt
    //  - add this dp to the existing dp
    //  - assume the particle had that momentum for the whole timestep
    //  - x -> x+dx = x + v*dt

    double accel[3];
    for (int64_t i=0; i< u->nparticles; i++) {
        get_accel(u, u->particles[i].x, accel);

        // update the momentum
        u->particles[i].v[0] += accel[0]*u->tstep;
        u->particles[i].v[1] += accel[1]*u->tstep;
        u->particles[i].v[2] += accel[2]*u->tstep;

        // assume particle moved at that speed for the whole time step. Note
        // again, assuming mass=1
        
        u->particles[i].x[0] += u->particles[i].v[0]*u->tstep;
        u->particles[i].x[1] += u->particles[i].v[1]*u->tstep;
        u->particles[i].x[2] += u->particles[i].v[2]*u->tstep;

        /*
        if (i == 0) {
            print_particle(&u->particles[i]);
        }
        */
    }
}

int main(int argc, char* argv) {

    struct universe* universe;
    double xmax=1.0;
    int64_t nperside=10;
    int64_t nstep=10000;
    //int64_t nstep=1;
    double tstep=0.00001;
    int64_t step;

    double xsoft = 0.05;

    universe = universe_new(nstep, nperside, xmax, tstep, xsoft);

    printf("Universe parameters:\n");
    print_universe(universe);

    FILE* fptr=open_output("tests/test-nb.dat");

    write_header(fptr, universe);

    // write out first step from initial conditions
    step=0;
    write_step(fptr, universe, step);
    
    for (step=1; step < nstep; step++) {
        if ( ((step+1) % 10) == 0) {
            printf("step %ld/%ld\n", (step+1), nstep);
        }
        take_step(universe);
        write_step(fptr, universe, step);
    }

    fclose(fptr);

    /* don't bother to clean up... */

}
