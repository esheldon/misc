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