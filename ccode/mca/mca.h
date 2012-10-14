#ifndef _MCA_HEADER_GUARD
#define _MCA_HEADER_GUARD

/*
   make a stretch move

   comp_pars are from a randomly drawn walker from the set complement of the
   current walker.

   newpar = comp_par + z*(par-comp_par)

   where z is drawn from the mca_rand_gofz, which is
        1/sqrt(z) in the range [1/a,a]

  On return the value of z is set and the
  newpars are filled in.

*/
void mca_stretch_move(double a,
                      const double *pars, 
                      const double *comp_pars, 
                      size_t npars,
                      double *newpars,
                      double *z);


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
