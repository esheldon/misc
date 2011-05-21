#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include "LensSource.h"
#include "read_cat.h"

using namespace std;

PAR_STRUCT read_par(char *file)
{

  PAR_STRUCT par_struct;

  char name[100];
  char val[100];

  FILE *fptr;
  if ( ! (fptr = fopen(file, "r") ) )
    {
      cout << "Could not open par file: " << file << endl;
      exit(1);
    }

  cout << endl <<
    "Reading parameters from file " << file << endl;

  fscanf(fptr, "%s %s", name, &par_struct.lens_file);
  fscanf(fptr, "%s %s", name, &par_struct.source_file);
  fscanf(fptr, "%s %s", name, &par_struct.rev_file);
  fscanf(fptr, "%s %s", name, &par_struct.scinv_file);
  fscanf(fptr, "%s %s", name, &par_struct.output_file);
  fscanf(fptr, "%s %s", name, &par_struct.pair_file);

  fscanf(fptr, "%s %d", name, &par_struct.pixel_lensing);
  fscanf(fptr, "%s %d", name, &par_struct.dopairs);

  fscanf(fptr, "%s %f", name, &par_struct.h);
  fscanf(fptr, "%s %f", name, &par_struct.omega_m);

  fscanf(fptr, "%s %d", name, &par_struct.sigmacrit_style);
  fscanf(fptr, "%s %d", name, &par_struct.shape_correction_style);

  fscanf(fptr, "%s %d", name, &par_struct.logbin);
  fscanf(fptr, "%s %d", name, &par_struct.nbin);

  fscanf(fptr, "%s %f", name, &par_struct.binsize);

  fscanf(fptr, "%s %f", name, &par_struct.rmin);
  fscanf(fptr, "%s %f", name, &par_struct.rmax);

  fscanf(fptr, "%s %d", name, &par_struct.comoving);

  fscanf(fptr, "%s %d", name, &par_struct.depth);

  fscanf(fptr, "%s %f", name, &par_struct.zbuffer);

  // Note, not currently reading these since old par files didn't
  // have them.  Its OK since they are at the end

  //fscanf(fptr, "%s %d", name, &par_struct.sample);
  //fscanf(fptr, "%s %s", name, &par_struct.catalog);

  cout << " lens_file = " << par_struct.lens_file << endl;
  cout << " source_file = " << par_struct.source_file << endl;
  cout << " rev_file = " << par_struct.rev_file << endl;
  cout << " scinv_file = " << par_struct.scinv_file << endl;
  cout << " output_file = " << par_struct.output_file << endl;
  cout << " pairs_file = " << par_struct.pair_file << endl;
  cout << " pixel_lensing? = " << (int) par_struct.pixel_lensing << endl;
  cout << " dopairs? = " << (int) par_struct.dopairs << endl;

  cout << " h = " << par_struct.h << endl;
  cout << " omega_m = " << par_struct.omega_m << endl;

  cout << " sigmacrit_style = " << (int) par_struct.sigmacrit_style << endl;
  cout << " shape_correction_style = " << 
    (int) par_struct.shape_correction_style << endl;

  cout << " logbin = " << (int) par_struct.logbin << endl;
  cout << " nbin = " << (int) par_struct.nbin << endl;

  cout << " binsize = " << par_struct.binsize << endl;

  cout << " rmin = " << par_struct.rmin << endl;
  cout << " rmax = " << par_struct.rmax << endl;

  cout << " comoving = " << (int) par_struct.comoving << endl;

  cout << " depth = " << (int) par_struct.depth << endl;
  cout << " zbuffer = " << par_struct.zbuffer << endl;
  //cout << " min_htm_index = " << (int) par_struct.min_htm_index << endl;

  fclose(fptr);

  // What type of binning?
  if (par_struct.logbin)
    {
      printf("\nBinning in log\n");
      par_struct.logRmin = log10(par_struct.rmin);
      par_struct.logRmax = log10(par_struct.rmax);

      par_struct.logBinsize = 
	( par_struct.logRmax - par_struct.logRmin )/par_struct.nbin;

      cout << " logRmin = " << par_struct.logRmin << endl;
      cout << " logRmax = " << par_struct.logRmax << endl;
      cout << " logBinsize = " << par_struct.logBinsize << endl;
    }



  return(par_struct);

}

source_struct *read_scat(char *file, int32 &nsource)
{

  source_struct *scat;
  FILE *fptr;
  char c;

  int nlines, row;


  if (! (fptr = fopen(file, "r")) )
    {
      cout << "Cannot open source file " << file << endl;
      exit(45);
    }

  /* Read the number of rows */
  nsource = nrows(fptr);

  cout << endl 
       << "Reading " << nsource << " sources from file " << file << endl;

  /* That leaves us sitting on the first newline. 
     There are SOURCE_HEADER_LINES header lines in total */

  nlines = 0;
  cout << "Skipping "<<SOURCE_HEADER_LINES<<" header lines"<<endl;
  while (nlines < SOURCE_HEADER_LINES) 
    {
      c = getc(fptr);
      if (c == '\n') nlines++;

      //printf("%c",c);
    }

  scat = (source_struct *) calloc( nsource, sizeof(source_struct) );

  for (row=0;row< (nsource);row++)
    {
      fread( &scat[row].stripe, sizeof(int16), 1, fptr);

      fread( &scat[row].e1, sizeof(float), 1, fptr);
      fread( &scat[row].e2, sizeof(float), 1, fptr);
      fread( &scat[row].e1e1err, sizeof(float), 1, fptr);
      fread( &scat[row].e1e2err, sizeof(float), 1, fptr);
      fread( &scat[row].e2e2err, sizeof(float), 1, fptr);

      fread( &scat[row].clambda, sizeof(double), 1, fptr);
      fread( &scat[row].ceta, sizeof(double), 1, fptr);
      
      fread( &scat[row].photoz_z, sizeof(float), 1, fptr);
      fread( &scat[row].photoz_zerr, sizeof(float), 1, fptr);
      
      fread( &scat[row].htm_index, sizeof(int), 1, fptr);
    }

  return(scat);

}

source_pixel_struct *read_scat_pixel(char *file, int32 &npix)
{

  source_pixel_struct *scat;
  FILE *fptr;

  if (! (fptr = fopen(file, "r")) )
    {
      cout << "Cannot open source file " << file << endl;
      exit(45);
    }

  fread(&npix, sizeof(int32), 1, fptr);

  cout << endl
       << "Reading " << npix << " source pixels from file " << file << endl;
 
  scat = (source_pixel_struct *) calloc(npix, sizeof(source_pixel_struct));

  for (int32 row=0; row < npix; row++)
    {

      fread(&scat[row].htm_index, sizeof(int32), 1, fptr);

      fread(&scat[row].nobj, sizeof(int16), 1, fptr);

      fread(&scat[row].e1, sizeof(float32), 1, fptr);
      fread(&scat[row].e2, sizeof(float32), 1, fptr);
      fread(&scat[row].e1e1err, sizeof(float32), 1, fptr);
      fread(&scat[row].e1e2err, sizeof(float32), 1, fptr);
      fread(&scat[row].e2e2err, sizeof(float32), 1, fptr);

      fread(&scat[row].aeta1, sizeof(float32), 1, fptr);
      fread(&scat[row].aeta2, sizeof(float32), 1, fptr);

      fread(&scat[row].clambda, sizeof(float64), 1, fptr);
      fread(&scat[row].ceta, sizeof(float64), 1, fptr);
    }

  return(scat);

}

lens_struct *read_lcat(char *file, int32 &nlens)
{

  lens_struct *lcat;
  FILE *fptr;
  char c;

  int nlines, row;

  if (! (fptr = fopen(file, "r")) )
    {
      cout << "Cannot open lens file " << file << endl;
      exit(45);
    }

  /* Read the number of rows */
  nlens = nrows(fptr);
  cout << endl 
       << "Reading " << nlens << " lenses from file " << file << endl;

  /* That leaves us sitting on the first newline. 
     There are LENS_HEADER_LINES header lines in total */

  nlines = 0;
  cout << "Skipping "<<LENS_HEADER_LINES<<" header lines"<<endl;
  while (nlines < LENS_HEADER_LINES) 
    {
      c = getc(fptr);
      if (c == '\n') nlines++;

      //printf("%c",c);
    }


  lcat = (lens_struct *) calloc( nlens, sizeof(lens_struct) );

  for (row=0;row< (nlens);row++)
    {
      fread( &lcat[row].zindex, sizeof(int32), 1, fptr);

      fread( &lcat[row].ra, sizeof(float64), 1, fptr);
      fread( &lcat[row].dec, sizeof(float64), 1, fptr);      
      fread( &lcat[row].clambda, sizeof(float64), 1, fptr);
      fread( &lcat[row].ceta, sizeof(float64), 1, fptr);
      
      fread( &lcat[row].z, sizeof(float32), 1, fptr);
      fread( &lcat[row].angmax, sizeof(float32), 1, fptr);
      fread( &lcat[row].DL, sizeof(float32), 1, fptr);

      fread( &lcat[row].mean_scinv, sizeof(float32), 1, fptr);
      
      fread( &lcat[row].pixelMaskFlags, sizeof(int16), 1, fptr);

    }

  return(lcat);

}

int nrows(FILE *fptr)
{

  short i;
  char c;
  char nrows_string[11];

  /* 9 chars for "NROWS  = " */
  for (i=0;i<9;i++)
    c = getc(fptr);

  /* now read nrows */
  for (i=0;i<11;i++)
    {
      nrows_string[i] = getc(fptr);
    }
  
  return( atoi(nrows_string) );

}

int32 *read_rev(char *file, int32 &nrev, int32 &minid, int32 &maxid)
{

  int32 *rev;
  FILE *fptr;

  if (! (fptr = fopen(file, "r")) )
    {
      cout << "Cannot open reverse indices file " << file << endl;
      exit(45);
    }
  
  fread( &nrev, sizeof(int32), 1, fptr);
  fread( &minid, sizeof(int32), 1, fptr);
  fread( &maxid, sizeof(int32), 1, fptr);

  cout << endl 
       << "Reading " << nrev << " rev data from file " << file << endl;

  rev = (int32 *) calloc(nrev, sizeof(int32));
  fread( rev, sizeof(int32), nrev, fptr);

  fclose(fptr);

  return(rev);

}



SCINV_STRUCT2D* read_scinv2d(char* file)
{
    SCINV_STRUCT2D* s;
    FILE* fptr;

    cout <<"Reading scinv file: "<<file<<endl;
    if (! (fptr = fopen(file, "r")) ) {
      cout << "Cannot open scinv file " << file << endl;
      exit(45);
    }

    s = (SCINV_STRUCT2D *) calloc( 1, sizeof(SCINV_STRUCT2D) );

    fread( &s->npoints, sizeof(int32), 1, fptr);
    fread( &s->nzl, sizeof(int32), 1, fptr);
    fread( &s->nzs, sizeof(int32), 1, fptr);

    // Now that we know the sizes, allocate the arrays->
    s->zl  = (float32 *) calloc(s->nzl, sizeof(float32));
    s->zli = (float32 *) calloc(s->nzl, sizeof(float32));
    s->zs  = (float32 *) calloc(s->nzs, sizeof(float32));
    s->zsi = (float32 *) calloc(s->nzs, sizeof(float32));

    s->scinv = (float64 *) calloc(s->nzs*s->nzl, sizeof(float64));


    fread( &s->zlStep, sizeof(float32), 1, fptr);
    fread( &s->zlMin, sizeof(float32), 1, fptr);
    fread( &s->zlMax, sizeof(float32), 1, fptr);
    fread( s->zl, sizeof(float32), s->nzl, fptr);
    fread( s->zli, sizeof(float32), s->nzl, fptr);

    fread( &s->zsStep, sizeof(float32), 1, fptr);
    fread( &s->zsMin, sizeof(float32), 1, fptr);
    fread( &s->zsMax, sizeof(float32), 1, fptr);
    fread( s->zs, sizeof(float32), s->nzs, fptr);
    fread( s->zsi, sizeof(float32), s->nzs, fptr);

    float64* scinv = s->scinv;
    for (int i=0; i<s->nzl; i++)
        for (int j=0; j<s->nzs; j++)
        {
            fread( &scinv[i*s->nzl + j], sizeof(float64), 1, fptr);
        }

    fclose(fptr);
    return(s);
}

