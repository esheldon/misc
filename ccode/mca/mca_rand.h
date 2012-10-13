#ifndef _MCA_RAND_HEADER_GUARD
#define _MCA_RAND_HEADER_GUARD


/* 
   generate random integers in [0,n)

   Don't forget to seed srand48!
 */
long mca_randlong(long n);

/*

   get a random long index in [0,n) from the *complement* of the input
   current value, i.e. such that index!=current

*/
long mca_rand_complement(long current, long n);

#endif
