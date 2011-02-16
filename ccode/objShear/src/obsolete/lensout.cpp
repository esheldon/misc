#include "lensout.h"

using namespace std;

void write_lensout(FILE *fptr, lensout_struct &lensout)
{

  int8 nbin = lensout.nbin;

  fwrite(&lensout.zindex, sizeof(int32), 1, fptr);

  fwrite(&lensout.tot_pairs, sizeof(int32), 1, fptr);
  fwrite(lensout.npair,      sizeof(int32), nbin, fptr);

  fwrite(lensout.rmax_act, sizeof(float32), nbin, fptr);
  fwrite(lensout.rmin_act, sizeof(float32), nbin, fptr);

  fwrite(lensout.rsum, sizeof(float32), nbin, fptr);

  fwrite(lensout.sigma,       sizeof(float32), nbin, fptr);
  fwrite(lensout.sigmaerr,    sizeof(float32), nbin, fptr);
  fwrite(lensout.orthosig,    sizeof(float32), nbin, fptr);
  fwrite(lensout.orthosigerr, sizeof(float32), nbin, fptr);

  fwrite(lensout.sigerrsum,      sizeof(float32), nbin, fptr);
  fwrite(lensout.orthosigerrsum, sizeof(float32), nbin, fptr);

  fwrite(&lensout.weight, sizeof(float32), 1, fptr);

  fwrite(lensout.wsum,  sizeof(float32), nbin, fptr);
  fwrite(lensout.owsum, sizeof(float32), nbin, fptr);
  fwrite(lensout.wscritinvsum,  sizeof(float32), nbin, fptr);

  fwrite(&lensout.sshsum,   sizeof(float32), 1, fptr);
  fwrite(&lensout.wsum_ssh, sizeof(float32), 1, fptr);
  fwrite(&lensout.ie,       sizeof(float32), 1, fptr);

}

#include <iostream>
void write_header(FILE *fptr, par_struct *par_struct)
{

  int8 nbin;
  nbin = par_struct->nbin;

  // Required header keywords
  fprintf(fptr, "NROWS  = %15d\n", par_struct->nlens);
  fprintf(fptr, "FORMAT = BINARY\n");
  fprintf(fptr, "BYTE_ORDER = NATIVE_LITTLE_ENDIAN\n");  
  fprintf(fptr, "IDLSTRUCT_VERSION = 0.9\n");

  // optional keywords

  fprintf(fptr, "nlens = %d         # Number of lenses\n", par_struct->nlens);

  fprintf(fptr, "h = %f             # Hubble parameter/100 km/s\n", par_struct->h);
  fprintf(fptr, "omega_m = %f       # omega_m, flat assumed\n", par_struct->omega_m);

  fprintf(fptr, "sigmacrit_style = %d # How sigmacrit was calculated\n", par_struct->sigmacrit_style);
  fprintf(fptr, "shape_correction_style = %d # How shape correction was performed\n", par_struct->shape_correction_style);

  fprintf(fptr, "logbin = %d        # logarithmic binning?\n", par_struct->logbin);
  fprintf(fptr, "nbin = %d          # Number of bins\n", par_struct->nbin);
  fprintf(fptr, "binsize = %f       # binsize if not log\n", par_struct->binsize);

  fprintf(fptr, "rmin = %f          # mininum radius (kpc)\n", par_struct->rmin);
  fprintf(fptr, "rmax = %f          # maximum radius (kpc)\n", par_struct->rmax);

  fprintf(fptr, "comoving = %d      # comoving radii?\n", par_struct->comoving);

  fprintf(fptr, "htm_depth = %d     # depth of HTM search tree\n", par_struct->depth);


  // field descriptions

  fprintf(fptr, "zindex 0L\n");

  fprintf(fptr, "tot_pairs 0L\n");
  fprintf(fptr, "npair lonarr(%d)\n", nbin);

  fprintf(fptr, "rmax_act fltarr(%d)\n", nbin);
  fprintf(fptr, "rmin_act fltarr(%d)\n", nbin);

  fprintf(fptr, "rsum fltarr(%d)\n", nbin);

  fprintf(fptr, "sigma fltarr(%d)\n", nbin);
  fprintf(fptr, "sigmaerr fltarr(%d)\n", nbin);
  fprintf(fptr, "orthosig fltarr(%d)\n", nbin);
  fprintf(fptr, "orthosigerr fltarr(%d)\n", nbin);

  fprintf(fptr, "sigerrsum fltarr(%d)\n", nbin);
  fprintf(fptr, "orthosigerrsum fltarr(%d)\n", nbin);

  fprintf(fptr, "weight 0.0\n");

  fprintf(fptr, "wsum fltarr(%d)\n", nbin);
  fprintf(fptr, "owsum fltarr(%d)\n", nbin);
  fprintf(fptr, "wscritinvsum fltarr(%d)\n", nbin);
  
  fprintf(fptr, "sshsum 0.0\n");
  fprintf(fptr, "wsum_ssh 0.0\n");
  fprintf(fptr, "ie 0.0\n");

  fprintf(fptr, "END\n");
  fprintf(fptr, "\n");

}


void make_lensout(lensout_struct &lensout, int8 nbin)
{

  lensout.nbin = nbin;

  lensout.npair          = (int32 *) calloc(nbin, sizeof(int32));

  lensout.rmax_act       = (float32 *) calloc(nbin, sizeof(float32));
  lensout.rmin_act       = (float32 *) calloc(nbin, sizeof(float32));

  lensout.rsum           = (float32 *) calloc(nbin, sizeof(float32));

  lensout.sigma          = (float32 *) calloc(nbin, sizeof(float32));
  lensout.sigmaerr       = (float32 *) calloc(nbin, sizeof(float32));
  lensout.orthosig       = (float32 *) calloc(nbin, sizeof(float32));
  lensout.orthosigerr    = (float32 *) calloc(nbin, sizeof(float32));

  lensout.sigerrsum      = (float32 *) calloc(nbin, sizeof(float32));
  lensout.orthosigerrsum = (float32 *) calloc(nbin, sizeof(float32));

  lensout.wsum           = (float32 *) calloc(nbin, sizeof(float32));
  lensout.owsum          = (float32 *) calloc(nbin, sizeof(float32));
  lensout.wscritinvsum   = (float32 *) calloc(nbin, sizeof(float32));

  reset_lensout(lensout);

}

void reset_lensout(lensout_struct &lensout)
{

  lensout.tot_pairs = 0;
  lensout.weight = 0.0;
  lensout.sshsum = 0.0;
  lensout.wsum_ssh = 0.0;
  lensout.ie = 0.0;

  for (int8 i=0; i<lensout.nbin; i++)
    {
      lensout.npair[i] = 0;

      lensout.rmax_act[i] = 0.0;
      lensout.rmin_act[i] = 0.0;
      
      lensout.rsum[i] = 0.0;
      
      lensout.sigma[i] = 0.0;
      lensout.sigmaerr[i] = 0.0;
      lensout.orthosig[i] = 0.0;
      lensout.orthosigerr[i] = 0.0;
      
      lensout.sigerrsum[i] = 0.0;
      lensout.orthosigerrsum[i] = 0.0;
      
      lensout.wsum[i] = 0.0;
      lensout.owsum[i] = 0.0;
      lensout.wscritinvsum[i] = 0.0;
    }

}


/* 
 *
 * For writing pair indices to an output file
 *
 */


/*
 * This just writes a int32 0 into the first slot of the file, 
 * to be filled in at the end
 */

void write_pair_header(FILE *fptr, int64 nrows)
{
    fwrite(&nrows, sizeof(int64), 1, fptr);
}

void write_pairs(FILE *fptr, int32 lindex, int32 sindex, float32 rkpc, float32 weight)
{
  fwrite(&lindex, sizeof(int32), 1, fptr);
  fwrite(&sindex, sizeof(int32), 1, fptr);
  fwrite(&rkpc, sizeof(float32), 1, fptr);
  fwrite(&weight, sizeof(float32), 1, fptr);
}


