#include <iostream>
#include <stdio.h>
#include <string.h>

#include "neu_net.h"

using namespace std;

int main(int argc, char **argv) {

  if (argc < 3) {
    cerr << "usage: neu_fit weights_file nfitfile" << endl;
    exit(1);
  }

  int DO_SED = 0; //set this to non-zero integer to do SED type fitting

  char wfilename[256];
  char dfilename[256];
  char prefix[256];
  const double ZCUT = 1.5;
  
  strcpy(dfilename, argv[2]);
  strcpy(wfilename, argv[1]);

  neu_net *net = new neu_net();
  net->read_param("run.param");
  net->initialize();
  net->read_weights(wfilename);
  
  net->read_trainfile(dfilename);

  net->calc_zphot();
  int NN = net->get_NFILTERS();
  
  Vec_DP zspec = net->get_zspec();
  Vec_DP zphot = net->get_zphot();
  Vec_DP stype = net->get_stype();
  Vec_DP zphot_err = net->get_sigma_zphot();
  Mat_DP mags = net->get_mags();
  Mat_DP magerrs = net->get_magerrs();
  
  int temp = 0;
  double tempd;

  cout << "NN = " << NN << endl;

  FILE *out = NULL;
  FILE *out2 = NULL;
  
  
  out = fopen("bzphot.tbl", "w");
  

  for (int i = 0; i < zspec.size(); ++i) {
    fprintf(out, "%lf %lf %lf ", zphot[i], zspec[i], zphot_err[i]);
    for(int j=0; j<mags.ncols(); ++j)
      fprintf(out, "%lf ", mags[i][j]);
    /*
    for(int j=0; j<mags.ncols(); ++j)
      fprintf(out, "%lf ", magerrs[i][j]);
    */
    fprintf(out, "\n");
  }

  /*
  for (int i = 0; i < zspec.size(); ++i) {
    fprintf(out, "%lf %lf ", zphot[i], zspec[i]);
    fprintf(out, "\n");
  }
  */
  fclose(out);
  if (DO_SED != 0)
    fclose(out2);
  
  
}
