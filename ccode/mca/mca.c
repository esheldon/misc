#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mca.h"

struct mca *mca_new(size_t nwalkers,
                    size_t steps_per_walker,
                    size_t npars,
                    double (*lnprob_func)(double *pars),
                    void *userdata)
{
    struct mca *self=calloc(1,sizeof(struct mca));
    if (self==NULL) {
        fprintf(stderr,"Could not allocate struct mca\n");
        exit(EXIT_FAILURE);
    }

    self->lnprob_func = lnprob_func;
    self->userdata=userdata;

    self->chain=mca_chain_new(nwalkers,steps_per_walker,npars);

    return self;
}
struct mca *mca_del(struct mca *self)
{
    if (self) {
        self->chain=mca_chain_del(self->chain);
        free(self);
    }
    return NULL;
}

struct mca_chain *mca_chain_new(size_t nwalkers,
                                size_t steps_per_walker,
                                size_t npars)
{
    struct mca_chain *self=calloc(1,sizeof(struct mca_chain));
    if (self==NULL) {
        fprintf(stderr,"Could not allocate struct mca_chain\n");
        exit(EXIT_FAILURE);
    }

    self->pars=calloc(nwalkers*steps_per_walker*npars,sizeof(double));
    if (self->pars==NULL) {
        fprintf(stderr,"Could not allocate mca_chain pars\n");
        exit(EXIT_FAILURE);
    }
    self->lnprob=calloc(nwalkers*steps_per_walker,sizeof(double));
    if (self->pars==NULL) {
        fprintf(stderr,"Could not allocate mca_chain lnprob\n");
        exit(EXIT_FAILURE);
    }

    self->nwalkers=nwalkers;
    self->steps_per_walker=steps_per_walker;
    self->npars=npars;

    return self;

}
struct mca_chain *mca_chain_del(struct mca_chain *self)
{
    if (self) {
        free(self->pars);
        free(self->lnprob);
        free(self);
    }
    return NULL;
}

void mca_run(struct mca *mca,
             double **pars, 
             int nwalkers,
             int npars)
{

}
void mca_stretch_move(double a,
                      const double *pars, 
                      const double *comp_pars, 
                      size_t ndim,
                      double *newpars,
                      double *z)
{
    *z = mca_rand_gofz(a);
    for (size_t i=0; i<ndim; i++) {

        double val=pars[i];
        double cval=comp_pars[i];

        newpars[i] = cval + (*z)*(val-cval); 
    }
}

int mca_accept(int ndim,
               double lnprob_old,
               double lnprob_new,
               double z)
{
    double lnprob_diff = (ndim - 1.)*log(z) + lnprob_new - lnprob_old;
    double r = drand48();

    if (lnprob_diff > log(r)) {
        return 1;
    } else {
        return 0;
    }
}
                  
void mca_copy_pars(double *pars_src, double *pars_dst, size_t npars)
{
    memcpy(pars_dst, pars_src, npars*sizeof(double));
}

long mca_rand_long(long n)
{
    return lrand48() % n;
}

long mca_rand_complement(long current, long n)
{
    long i=current;
    while (i == current) {
        i = mca_rand_long(n);
    }
    return i;
}

double mca_rand_gofz(double a)
{
    // ( (a-1) rand + 1 )^2 / a;

    double z = (a - 1.)*drand48() + 1.;

    z = z*z/a;

    return z;
}
