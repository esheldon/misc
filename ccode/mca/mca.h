/*
   mca - a library implementing affine invariant MCMC as
   outlined in Goodman & Weare 2010.

   Some implementation inspiration from the Emcee python version.

   I made a choice to use have mca_run() take in the value a rathern than put
   this in some "self" struct.  If we decide to keep more such data around
   between runs, I might change this.

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
#ifndef _MCA_HEADER_GUARD
#define _MCA_HEADER_GUARD


#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#define MCA_LOW_VAL (-DBL_MAX)

/*
   The main structure for mcmc chains.
*/

struct mca_chain {
    size_t nwalkers;
    size_t steps_per_walker;
    size_t npars;

    // indexed by (iwalker,istep,ipar)
    //    npars*steps_per_walker*iwalker + npars*istep + ipar
    // where istep is in [0,steps_per_walker)
    // or ignoring walkers by (istep,ipar)
    //    npars*istep + ipar
    // where istep is in [0,nwalkers*nsteps_per_walker)

    double *pars;

    // index by (iwalker,istep)
    //   steps_per_walker*iwalker + istep
    // or ignoring walkers by (istep)
    double *lnprob;

    int *accept;
};

#define MCA_CHAIN(mca) (mca)->chain

#define MCA_CHAIN_NPARS(chain)  (chain)->npars
#define MCA_CHAIN_NWALKERS(chain)  (chain)->nwalkers
#define MCA_CHAIN_NSTEPS(chain)  \
    (chain)->steps_per_walker*(chain)->nwalkers
#define MCA_CHAIN_NSTEPS_BYWALKER(chain)  (chain)->steps_per_walker


#define MCA_CHAIN_PAR_BYWALKER(chain, iwalker, istep, ipar)    \
    (chain)->pars[                                             \
        (chain)->npars*(chain)->steps_per_walker*(iwalker)       \
      + (chain)->npars*(istep)                                   \
      + (ipar)                                                   \
    ]
#define MCA_CHAIN_PAR(mca, istep, ipar)    \
    (chain)->pars[ (chain)->npars*(istep)  + (ipar) ]


#define MCA_CHAIN_LNPROB_BYWALKER(chain, iwalker, istep)        \
    (chain)->lnprob[                               \
        (chain)->steps_per_walker*(iwalker)          \
      + (istep)                                           \
    ]
#define MCA_CHAIN_LNPROB(chain, istep)    \
    (chain)->lnprob[(istep)]

#define MCA_CHAIN_ACCEPT_BYWALKER(chain, iwalker, istep)        \
    (chain)->accept[                               \
        (chain)->steps_per_walker*(iwalker)          \
      + (istep)                                           \
    ]
#define MCA_CHAIN_ACCEPT(chain, istep)    \
    (chain)->accept[(istep)]



struct mca_chain *mca_chain_new(size_t nwalkers,
                                size_t steps_per_walker,
                                size_t npars);
struct mca_chain *mca_chain_del(struct mca_chain *self);

void mca_chain_print(struct mca_chain *chain, FILE *stream);

struct mca_chain *mca_make_guess(double *centers, 
                                 double *ballsizes,
                                 size_t npars, 
                                 size_t nwalkers);


/*
   stats calculated from mca_chain s
*/
struct mca_stats {
    size_t npars;

    double *mean;

    /* [i,j] -> npar*i + j */
    double *cov;
};


#define MCA_STATS_NPARS(stats) (stats)->npars

#define MCA_STATS_MEAN(stats, i) ({                                           \
    double _mn=-MCA_LOW_VAL;                                                  \
    if ((i) >= (stats)->npars) {                                              \
        fprintf(stderr,                                                       \
            "stats error: mean index %lu out of bounds [0,%lu)\n",            \
                        (i), (stats)->npars);                                 \
        fprintf(stderr,"returning %.16g\n", _mn);                             \
    } else {                                                                  \
        _mn = (stats)->mean[(i)];                                             \
    }                                                                         \
    _mn;                                                                      \
})

#define MCA_STATS_COV(stats, i, j) ({                                         \
    double _cov=-MCA_LOW_VAL;                                                 \
    if ((i) >= (stats)->npars || (j) >= (stats)->npars) {                     \
        fprintf(stderr,                                                       \
            "stats error: cov index (%lu,%lu) out of bounds [0,%lu)\n",       \
                        (i), (j),(stats)->npars);                             \
        fprintf(stderr,"returning %.16g\n", _cov);                            \
    } else {                                                                  \
        _cov= (stats)->cov[(i)*(stats)->npars + (j)];                         \
    }                                                                         \
    _cov;                                                                     \
})




struct mca_stats *mca_stats_new(size_t npar);
struct mca_stats *mca_stats_del(struct mca_stats *self);

/*
   Calculate means of each parameter and full covariance matrix
*/

struct mca_stats *mca_chain_stats(struct mca_chain *chain);
/* print 
     mean +/- err
   for each parameter

   This is for human viewing.
*/
void mca_stats_print(struct mca_stats *self, FILE *stream);
/* 
   this version good for machine reading

   npar
   mean1 mean2 mean3 ....
   cov11 cov12 cov13 ....
   cov21 cov22 cov23 ....
   cov31 cov32 cov33 ....

*/
void mca_stats_print_full(struct mca_stats *self, FILE *stream);

/*

   Fill the chain with MCMC steps.

   The *last* set of walkers in the "start" chain will be the starting point
   for the chain.  This way you can feed a start as a single chain, e.g.  from
   mca_make_guess() or as the last step of a previous burn-in run.

   parameters
   ----------
   a  double
     The scale for the g(z) random number function.  a=2 gives
     acceptance rate ~.5.  4 around .35-.4 but these depend
     on the dimensionality
   start mca_chain
     To be used as the starting point.  When starting burn-in, get
     this by calling mca_make_guess().  For a post-burn run just
     send the chain from the burn-in run.
   chain mca_chain
     The chain to be filled.  It's size determines the number
     of steps.
   lnprob_func function pointer
     The function to call for lnprob
   userdata void*
     Either NULL or some data for use by the lnprob func.  The
     user must cast it to the right type.

   Procedure
   ---------

   loop over steps
     loop over walkers
       choose a random walker from the complement
       make a stretch move
       if accept
           copy new pars
       else 
           copy old pars
*/

void mca_run(double a,
             const struct mca_chain *start,
             struct mca_chain *chain,
             double (*lnprob_func)(double *pars, size_t npars, void *userdata),
             void *userdata);

/* copy the last step in the start chain to the first step
   in the chain */
void mca_set_start(const struct mca_chain *start,
                   struct mca_chain *chain);

/*
   make a stretch move

   comp_pars are from a randomly drawn walker from the set complement of the
   current walker.

   newpar = comp_par + z*(par-comp_par)

   where z is drawn from the mca_rand_gofz, which is
        1/sqrt(z) in the range [1/a,a]

   Note this is a *vectoral* move; z is a scalar in that vectoral equation.

   On return the value of z is set and the newpars are filled in.

*/
void mca_stretch_move(double a,
                      const double *pars, 
                      const double *comp_pars, 
                      size_t ndim,
                      double *newpars,
                      double *z);

/*
   Determine of a stretch move should be accepted

   Returns 1 if yes 0 if no
*/
int mca_accept(int ndim,
               double lnprob_old,
               double lnprob_new,
               double z);

/*
   copy the parameters
*/
void mca_copy_pars(const double *pars_src, double *pars_dst, size_t npars);

/* 
   generate random integers in [0,n)

   Don't forget to seed srand48!
 */
long mca_rand_long(long n);

/*

   get a random long index in [0,n) from the *complement* of the input
   current value, i.e. such that index!=current

*/
long mca_rand_complement(long current, long n);

/*
   generate random numbers 
       {
       { 1/sqrt(z) if z in (1/a,a)
       {     0     otherwise
       {

   When used in the affine invariant mcmc, a value
   of a=2 gives about 50% acceptance rate
 */
double mca_rand_gofz(double a);


/*

   Generate gaussian random numbers with variance 1 and mean 0.  This is not
   used by the affine invariant sampler, but will probably be useful for test
   programs that generate random data.

   Note we get two per run but I'm only using one.
*/
double mca_randn();


#endif
