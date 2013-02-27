/*
  Generate gaussian random deviates

  Don't forget to seed the random number generator using
  srand48(some integer).

  for example, use time from time.h
    time_t t1;
    (void) time(&t1);
    srand48((long) t1);
*/
double randn();

/*
   Generate a poisson deviate.

   This is apparently from Knuth
*/
long poisson(double lambda);
