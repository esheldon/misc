

//-*-c++-*-

#include "nr.h"
#include <sys/time.h>
#include <string>

#include <stdio.h>

using std::string;

class neu_net {

 private:
  int N_LAYERS;
  int N_FILTERS;
  int N_DATAPOINTS;
  int N_W;
  int MAX_LSIZE;
  int NTARGET;  //number of training targes.  By default, target 0 is zspec, target 1 is SED type
  

  Mat_DP nnet;
  Mat_DP nbeta;
  Mat3D_DP nweights;
  Mat3D_DP ndw;
  Mat3D_DP ndedw;
  Vec_INT nlayer_size;
  
  double **net;
  double **beta;
  double **weights;
  double **dw;
  double **dedw;
  int *layer_size;

  Vec_DP zdat;
  Vec_DP stype;
  Vec_DP zphot;
  Vec_DP sigma_zphot;
  Vec_DP stype_phot;
  Vec_DP zscaled;
  Mat_DP mags;
  Mat_DP mag_errs;

  double eta;
  double alpha;
  double wbeta;
  double mbias;

  double max_z;
  double min_z;
  double max_s;
  double min_s;
  double max_mag;
  double min_mag;

  double K; //gain factor in the sigmoid function

 public:
  //constructor
  neu_net();

  //destructor function 
  ~neu_net();
  
  int initialize();
  void randomizeWeights();
  int read_trainfile(char *filename);
  int read_datafile(char *filename);
  //double E();
  //double new_E();
  int minimize();
  int new_minimize();
  int set_eta(double d);
  int set_alpha(double d);
  int set_wbeta(double d);
  int set_mbias(double d);
  int set_K(double d);
  int print_z();
  int write_z(char *filename);
  int write_sed(char *filename);
  double z_score();
  double z_scatter();

  double normalize(double mag); //normalizes the magnitude to -1 <-> 1
  double unnormalize(double mag); //unnormalizes the magnitude
  int calc_zphot(); //calculates and sets zphot, also calculates derivative error
  int calcZphot(); //calculates and sets zphot

  //access functions
  int get_NL();
  int get_NDATA();
  int get_NFILTERS();
  Vec_DP get_zphot();
  Vec_DP get_sigma_zphot();
  Vec_DP get_zspec();
  Vec_DP get_sphot();
  Vec_DP get_stype();
  Vec_INT get_nlayersize();
  Mat_DP get_mags();
  Mat_DP get_magerrs();

  //weghts
  int set_weights(Vec_I_DP &w);
  int get_weights(Vec_O_DP &w);
  int write_weights(char *filename);
  int read_weights(char *filename);
  int rw(char *filename);

  double EE(Vec_I_DP &w);
  void dE(Vec_I_DP &w, Vec_O_DP &dde);
  int minimize(Vec_IO_DP &w, Vec_I_DP &dde);
  int maximize(Vec_IO_DP &w, Vec_I_DP &dde);

  //misc
  double scale_z(double z);
  double unscale_z(double z);
  double scale_sed(double s);
  double unscale_sed(double s);

  //clean up
  int clean();

  inline double sigmoid(double d);
  inline double sigmoid_prime(double d);
  double inv_sigmoid(double d);

  //read a parameter file to get all of the required parameters
  void read_param(const char *paramfile);

};

double ran2(long *idum);

