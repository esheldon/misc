// $Id: calcMagErr4.cpp,v 1.2 2007/02/19 23:13:39 oyachai Exp $
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "nr.h"

void find_NR_NMAG(const char *filename, int *NR, int *NMAG) {
  //This function is used to find out the number of rows and number of magnitudes in a table file
  //Find out the number of rows
  FILE *in = fopen(filename, "r");
  char line[256];
  char *token = NULL;
  fgets(line, 256, in);
  token = strtok(line, " \n");
  *NMAG = 0;
  int NS = 3;
  while (token != NULL) {
    (*NMAG)++;
    token = strtok(NULL, " \n");
  }
  *NMAG = *NMAG - NS;
  //Find out the number of rows
  *NR = 0;
  while (!feof(in)) {
    (*NR)++;
    fgets(line, 256, in);
  }
  printf("NR = %d, NMAG = %d\n", *NR, *NMAG);
}

void read_data(const char *filename, int NR, int NMAG, double *zs, double *zp, double *sed, double *mags) {
  FILE *in = fopen(filename, "r");
  if (in == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", filename);
    exit(1);
  }

  int i, j;
  for(i=0; i<NR; ++i) {
    fscanf(in, "%lf %lf %lf", &zp[i], &zs[i], &sed[i]);
    for(j=0; j<NMAG; ++j) {
      fscanf(in, "%lf", &mags[i*NMAG + j]);
    }
  }
  fclose(in);
}

void find_NR_NMAG2(const char *filename, int *NR, int *NMAG) {
  //This function is used to find out the number of rows and number of magnitudes in the input photometric set
  //The column format for this input should be:
  // 1: zphot
  // 2-5: g, r, i, z

  //Find out the number of rows
  FILE *in = fopen(filename, "r");
  char line[256];
  char *token = NULL;
  fgets(line, 256, in);
  token = strtok(line, " ");
  *NMAG = 0;
  int NS = 1;
  while (token != NULL) {
    (*NMAG)++;
    token = strtok(NULL, " \n");
  }
  *NMAG = *NMAG - NS;
  //Find out the number of rows
  *NR = 0;
  while (!feof(in)) {
    (*NR)++;
    fgets(line, 256, in);
  }
  printf("NR = %d, NMAG = %d\n", *NR, *NMAG);
}

void read_data2(const char *filename, int NR, int NMAG, double *zp, double *mags) {
  //The column format for this input should be:
  // 1: zphot
  // 2-5: g, r, i, z
  FILE *in = fopen(filename, "r");
  if (in == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", filename);
    exit(1);
  }

  int i, j;
  for(i=0; i<NR; ++i) {
    fscanf(in, "%lf", &zp[i]);
    for(j=0; j<NMAG; ++j) {
      fscanf(in, "%lf", &mags[i*NMAG + j]);
    }
  }
  fclose(in);
}

void write_etbl(const char *filename, int NR, int NMAG, double *zp, double *ze, double *mags) {
  FILE *out = fopen(filename, "w");
  if (out == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", filename);
    exit(1);
  }

  int i, j;
  for(i=0; i<NR; ++i) {
    fprintf(out, "%lf %lf", zp[i], ze[i]);
    for(j=0; j<NMAG; ++j) {
      fprintf(out, " %lf", mags[i*NMAG + j]);
    }
    fprintf(out, "\n");
  }
  fclose(out);
}

double calcSigma(Vec_DP &zs, Vec_DP &zp) {
  if (zs.size() < 2) {
    return 0.0;
  }
  double r = 0.0;
  for(int i=0; i<zs.size(); ++i) {
    r += (zs[i]-zp[i])*(zs[i]-zp[i]);
  }
  return sqrt(r/zs.size());
}

double calcSigma68(Vec_DP &zs, Vec_DP &zp) {
  if (zs.size() < 2) {
    return 0.0;
  }
  Vec_DP d(zs.size());
  for(int i=0; i<zs.size(); ++i) {
    d[i] = fabs(zs[i]-zp[i]);
  }
  NR::sort(d);
  return d[(2*zs.size())/3];
}

int main(int argc, char **argv) {

  if (argc < 3) {
    fprintf(stderr, "Usage: %s train_tbl photo_tbl\n", argv[0]);
    exit(1);
  }

  int NNEAR = 100;

  int tNR, tNMAG;
  int pNR, pNMAG;

  find_NR_NMAG(argv[1], &tNR, &tNMAG);
  find_NR_NMAG(argv[2], &pNR, &pNMAG);

  double *tzp = (double*)malloc(sizeof(double)*tNR);
  double *tzs = (double*)malloc(sizeof(double)*tNR);
  double *tsd = (double*)malloc(sizeof(double)*tNR);
  double *tm = (double*)malloc(sizeof(double)*tNR*tNMAG);

  double *pzp = (double*)malloc(sizeof(double)*pNR);
  double *pzs = (double*)malloc(sizeof(double)*pNR);
  double *psd = (double*)malloc(sizeof(double)*pNR);
  double *pze = (double*)malloc(sizeof(double)*pNR);
  double *pm = (double*)malloc(sizeof(double)*pNR*pNMAG);

  if (tNMAG != pNMAG) {
    fprintf(stderr, "number of magnitudes does not match\n");
    exit(3);
  }

  read_data(argv[1], tNR, tNMAG, tzs, tzp, tsd, tm);
  read_data(argv[2], pNR, pNMAG, pzs, pzp, psd,  pm);
  
  Vec_DP d(tNR);
  Vec_DP ds(NNEAR);
  Vec_INT indx(NNEAR);
  Vec_DP zptemp(NNEAR);
  Vec_DP zstemp(NNEAR);

  int t10 = pNR / 10;
  for(int i=0; i<pNR; ++i) {
    if (i % t10 == 0) {
      printf(" %4.2f\n", (float)(i/t10)*10.0);
      fflush(NULL);
    }
    for(int j=0; j<tNR; ++j) {
      d[j] = 0.0;
      for(int k=0; k<tNMAG; ++k) {
	d[j] -= (pm[i*pNMAG+k]-tm[j*tNMAG+k])*(pm[i*pNMAG+k]-tm[j*tNMAG+k]);
      }
    }
    NR::hpsel(d, indx);
    for(int j=0; j<indx.size(); ++j) {
      ds[j] = d[indx[j]];
      zptemp[j] = tzp[indx[j]];
      zstemp[j] = tzs[indx[j]];
    }
    //double s1 = calcSigma(zstemp, zptemp);
    double s2 = calcSigma68(zstemp, zptemp);
    pze[i] = s2;
  }

  write_etbl("output.etbl", pNR, pNMAG, pzp, pze, pm);

  return 0;
}
