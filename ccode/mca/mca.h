/*
   mca - a library implementing affine invariant MCMC as
   outlined in Goodman & Weare 2010.

   Some implementation inspiration from the Emcee python version.
*/
#ifndef _MCA_HEADER_GUARD
#define _MCA_HEADER_GUARD

#include <stdlib.h>
#include <stdio.h>

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
};


struct mca {
    // holds the pars and lnprob
    struct mca_chain *chain;

    // This is the function to calculate the log(prob)
    double (*lnprob_func)(double *pars);

    // The user can store data here for the log(prob) function
    // This field will not be set or freed by mca_run()
    void *userdata;
};

// here istep is within the walker's sub-chain
#define MCA_PAR_BYWALKER(mca, iwalker, istep, ipar)                 \
    (mca)->chain->pars[                                             \
        (mca)->chain->npars*(mca)->chain->steps_per_walker*iwalker  \
      + (mca)->chain->npars*istep                                   \
      + ipar                                                        \
    ]

// here istep is the overall step, [0,nwalkers*steps_per_walker)
#define MCA_PAR(mca, iwalker, istep, ipar)    \
    (mca)->chain->pars[                       \
        (mca)->chain->npars*istep             \
      + ipar                                  \
    ]

#define MCA_LNPROB_BYWALKER(mca, iwalker, istep)        \
    (mca)->chain->lnprob[                               \
        (mca)->chain->steps_per_walker*iwalker          \
      + istep                                           \
    ]

#define MCA_LNPROB(mca, iwalker, istep)    \
    (mca)->chain->lnprob[istep]



/*
*/
struct mca *mca_new(size_t nwalkers,
                    size_t steps_per_walker,
                    size_t npars,
                    double (*lnprob_func)(double *pars),
                    void *userdata);
struct mca *mca_del(struct mca *self);

struct mca_chain *mca_chain_new(size_t nwalkers,
                                size_t steps_per_walker,
                                size_t npars);
struct mca_chain *mca_chain_del(struct mca_chain *self);




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
void mca_copy_pars(double *pars_src, double *pars_dst, size_t npars);

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



#endif
