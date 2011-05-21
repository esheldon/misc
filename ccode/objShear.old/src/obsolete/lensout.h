#if !defined (_lensout_h)
#define _lensout_h

#include "types.h"
#include "LensSource.h"

/*
 * Normal outputs
 */
void write_header(FILE *fptr, par_struct *par_struct);
void write_lensout(FILE *fptr, lensout_struct &lensout);

void make_lensout(lensout_struct &lensout, int8 nbin);
void reset_lensout(lensout_struct &lensout);

/*
 * For pair outputs
 */
void write_pair_header(FILE *fptr, int64 nrows);
void write_pairs(FILE *fptr, int32 lindex, int32 sindex, float32 rkpc, float32 weight);

#endif /* _lensout_h */
