#ifndef _DIMS_H_
#define _DIMS_H


// just add more if statements if you need higher dimensions
#define MAXDIMS 11

#if defined NDIM1
#define NDIM 1
#elif defined NDIM2
#define NDIM 2
#elif defined NDIM3
#define NDIM 3
#elif defined NDIM4
#define NDIM 4
#elif defined NDIM5
#define NDIM 5
#elif defined NDIM6
#define NDIM 6
#elif defined NDIM7
#define NDIM 7
#elif defined NDIM8
#define NDIM 8
#elif defined NDIM9
#define NDIM 9
#elif defined NDIM10
#define NDIM 10
#elif defined NDIM11
#define NDIM 11
#else
// default is dimensions 5
#define NDIM 5
#endif


#endif
