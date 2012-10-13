#ifndef _MCA_RAND_HEADER_GUARD
#define _MCA_RAND_HEADER_GUARD


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
