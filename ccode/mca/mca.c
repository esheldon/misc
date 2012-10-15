/*
    for docs see mca.h

    Copyright (C) 2012  Erin Sheldon

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mca.h"

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
    self->accept=calloc(nwalkers*steps_per_walker,sizeof(int));
    if (self->pars==NULL) {
        fprintf(stderr,"Could not allocate mca_chain accept\n");
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


void mca_chain_print(struct mca_chain *chain, FILE *stream)
{
    size_t nwalkers=MCA_CHAIN_NWALKERS(chain);
    size_t steps_per_walker=MCA_CHAIN_NSTEPS_BYWALKER(chain);
    size_t npars=MCA_CHAIN_NPARS(chain);

    size_t nsteps=nwalkers*steps_per_walker;

    fprintf(stream,"%lu %lu %lu\n", nwalkers, steps_per_walker, npars);
    for (size_t istep=0; istep<nsteps; istep++) {
        fprintf(stream,"%d %.16g ", 
                MCA_CHAIN_ACCEPT(chain,istep), 
                MCA_CHAIN_LNPROB(chain,istep));
        for (size_t ipar=0; ipar<npars; ipar++) {
            fprintf(stream,"%.16g ", MCA_CHAIN_PAR(chain, istep, ipar));
        }
        fprintf(stream,"\n");
    }
}

struct mca_chain *mca_make_guess(double *centers, 
                                 double *ballsizes,
                                 size_t npars, 
                                 size_t nwalkers)
{
    struct mca_chain *chain=mca_chain_new(nwalkers,1,npars);

    for (size_t iwalk=0; iwalk<nwalkers; iwalk++) {
        MCA_CHAIN_LNPROB_BYWALKER(chain,iwalk,0) = MCA_LOW_VAL;
    }
    for (size_t ipar=0; ipar<npars; ipar++) {

        double center=centers[ipar];
        double ballsize=ballsizes[ipar];

        for (size_t iwalk=0; iwalk<nwalkers; iwalk++) {
            double val = center + ballsize*(drand48()-0.5)*2;
            MCA_CHAIN_PAR_BYWALKER(chain, iwalk, 0, ipar) = val; 
        }
    }

    return chain;
}


struct mca_stats *mca_stats_new(size_t npars)
{
    struct mca_stats *self=calloc(1,sizeof(struct mca_stats));
    if (self==NULL) {
        fprintf(stderr,"Could not allocate struct mca_stats\n");
        exit(EXIT_FAILURE);
    }

    self->mean = calloc(npars, sizeof(double));
    if (self->mean == NULL) {
        fprintf(stderr,"Could not allocate mca_stats mean array\n");
        exit(EXIT_FAILURE);
    }
    self->cov = calloc(npars*npars, sizeof(double));
    if (self->cov == NULL) {
        fprintf(stderr,"Could not allocate mca_stats cov array\n");
        exit(EXIT_FAILURE);
    }

    self->npars=npars;
    return self;
}

struct mca_stats *mca_stats_del(struct mca_stats *self)
{
    if (self) {
        free(self->mean);
        free(self->cov);
        free(self);
    }
    return NULL;
}

struct mca_stats *mca_chain_stats(struct mca_chain *chain)
{
    size_t npars = MCA_CHAIN_NPARS(chain);
    size_t nsteps = MCA_CHAIN_NSTEPS(chain);
    double ival=0, jval=0;
    struct mca_stats *self=mca_stats_new(npars);

    for (size_t istep=0; istep<nsteps; istep++) {

        for (size_t ipar=0; ipar<npars; ipar++) {

            ival=MCA_CHAIN_PAR(chain,istep,ipar);
            self->mean[ipar] += ival;

            for (size_t jpar=ipar; jpar<npars; jpar++) {

                if (ipar==jpar) {
                    jval=ival;
                } else {
                    ival=MCA_CHAIN_PAR(chain,istep,jpar);
                }

                self->cov[ipar*npars + jpar] += ival*jval;
            }

        }
    }

    for (size_t ipar=0; ipar<npars; ipar++) {
        self->mean[ipar] /= nsteps;
    }

    for (size_t ipar=0; ipar<npars; ipar++) {
        double imean=self->mean[ipar];

        for (size_t jpar=ipar; jpar<npars; jpar++) {
            size_t index=ipar*npars + jpar;

            double jmean=self->mean[jpar];

            self->cov[index] /= nsteps;
            self->cov[index] -= imean*jmean;

            if (ipar!=jpar) {
                self->cov[jpar*npars + ipar] = self->cov[index];
            }
        }
    }

    return self;
}

void mca_stats_print(struct mca_stats *self, FILE *stream)
{
    size_t npars = MCA_STATS_NPARS(self);

    for (size_t ipar=0; ipar< npars; ipar++) {
        double mn=MCA_STATS_MEAN(self,ipar);
        double var=MCA_STATS_COV(self,ipar,ipar);
        double err=sqrt(var);
        fprintf(stream,"%.16g +/- %.16g\n",mn,err);
    }
}
void mca_stats_print_full(struct mca_stats *self, FILE *stream)
{
    size_t npars = MCA_STATS_NPARS(self);

    fprintf(stream,"%lu\n", npars);
    for (size_t ipar=0; ipar< npars; ipar++) {
        double mn=MCA_STATS_MEAN(self,ipar);
        fprintf(stream,"%.16g", mn);
    }
    fprintf(stream,"\n");
    for (size_t ipar=0; ipar< npars; ipar++) {
        for (size_t jpar=0; jpar< npars; jpar++) {
            double cov=MCA_STATS_COV(self,ipar,jpar);
            fprintf(stream,"%.16g ",cov);
        }
        fprintf(stream,"\n");
    }
}

void mca_run(struct mca_chain *chain,
             double a,
             const struct mca_chain *start,
             double (*lnprob_func)(const double *, size_t, const void *),
             const void *userdata)
{
    double z=0;
    size_t nwalkers=MCA_CHAIN_NWALKERS(chain);
    size_t npars=MCA_CHAIN_NPARS(chain);
    size_t steps_per_walker=MCA_CHAIN_NSTEPS_BYWALKER(chain);
    double *pars_old=NULL,*pars_new=NULL;
    double lnprob_old=0, lnprob_new=0;

    mca_set_start(start, chain);


    for (size_t istep=1; istep<steps_per_walker; istep++) {
        for (size_t iwalker=0; iwalker<nwalkers; iwalker++) {

            pars_old   =&MCA_CHAIN_PAR_BYWALKER(chain,   iwalker,istep-1,0);
            lnprob_old = MCA_CHAIN_LNPROB_BYWALKER(chain,iwalker,istep-1);

            pars_new=&MCA_CHAIN_PAR_BYWALKER(chain,iwalker,istep,0);

            long comp_walker=mca_rand_complement(iwalker,nwalkers);
            double *comp_pars=&MCA_CHAIN_PAR_BYWALKER(chain,comp_walker,istep-1,0);

            // note this copies the new pars into our chain; we will copy over
            // if not accepted
            mca_stretch_move(a, pars_old, comp_pars, npars, 
                             pars_new, &z);

            //fprintf(stderr,"    calculate lnprob\n");
            lnprob_new = (*lnprob_func)(pars_new,npars,userdata);
            //fprintf(stderr,"lnprob: %.16g\n", lnprob_new);
            //fprintf(stderr,"    done\n");

            int accept = mca_accept(npars, lnprob_old, lnprob_new, z);
            MCA_CHAIN_ACCEPT_BYWALKER(chain,iwalker,istep) = accept;

            if (!accept) {
                // copy the older pars over
                mca_copy_pars(pars_old, pars_new, npars);
                MCA_CHAIN_LNPROB_BYWALKER(chain,iwalker,istep) = lnprob_old;
            } else {
                MCA_CHAIN_LNPROB_BYWALKER(chain,iwalker,istep) = lnprob_new;
            }

        }
    }
}


void mca_stretch_move(double a,
                      const double *pars, 
                      const double *comp_pars, 
                      size_t ndim,
                      double *newpars,
                      double *z)
{
    (*z) = mca_rand_gofz(a);
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
 /* copy the last step in the start chain to the first step
   in the chain */
void mca_set_start(const struct mca_chain *start,
                   struct mca_chain *chain)
{
    size_t steps_per=MCA_CHAIN_NSTEPS_BYWALKER(start);
    size_t nwalkers=MCA_CHAIN_NWALKERS(start);
    size_t npars=MCA_CHAIN_NPARS(start);

    for (size_t iwalker=0; iwalker<nwalkers; iwalker++) {
        MCA_CHAIN_ACCEPT_BYWALKER(chain,iwalker,0) = 1;
        MCA_CHAIN_LNPROB_BYWALKER(chain,iwalker,0) = 
            MCA_CHAIN_LNPROB_BYWALKER(start,iwalker,(steps_per-1));
        for (size_t ipar=0; ipar<npars; ipar++) {

            MCA_CHAIN_PAR_BYWALKER(chain,iwalker,0,ipar) = 
                MCA_CHAIN_PAR_BYWALKER(start,iwalker,(steps_per-1),ipar);

        }
    }
}


                 
void mca_copy_pars(const double *pars_src, double *pars_dst, size_t npars)
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

/*
   Generate normal random numbers.

   Note we get two per run but I'm only using one.
*/

double mca_randn() 
{
    double x1, x2, w, y1;//, y2;
 
    do {
        x1 = 2.*drand48() - 1.0;
        x2 = 2.*drand48() - 1.0;
        w = x1*x1 + x2*x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.*log( w ) ) / w );
    y1 = x1*w;
    //y2 = x2*w;
    return y1;
}

