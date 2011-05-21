#if !defined (_sigmaCritInv_h)
#define _sigmaCritInv_h


#include <algorithm>
#include <cmath>
#include "types.h"
#include <vector>


//////////////////////////////////////////////////////////
// Inverse critical density struct for interpolation
//////////////////////////////////////////////////////////

#define NZL 100
#define NZS 50
#define NZSERR 50

using namespace std;

struct scinv {

  int32   nPoints;

  float32 zlStep;
  float32 zlMin;
  float32 zlMax;
  float32 zl[NZL];
  float32 zli[NZL];

  float32 zsStep;
  float32 zsMin;
  float32 zsMax;
  float32 zs[NZS];
  float32 zsi[NZS];

  float32 zsErrStep;
  float32 zsErrMin;
  float32 zsErrMax;
  float32 zsErr[NZSERR];
  float32 zsErri[NZSERR];

  // IDL has it backwards!
  // float64 scinv[NZL][NZS][NZSERR];
  float64 scinv[NZSERR][NZS][NZL];

};

struct scinv2d {

  int32   npoints;

  int32 nzl;
  int32 nzs;

  float32 zlStep;
  float32 zlMin;
  float32 zlMax;
  vector<float> zl;
  vector<float32> zli;

  float32 zsStep;
  float32 zsMin;
  float32 zsMax;
  vector<float32> zs;
  vector<float32> zsi;

  vector<float64> scinv;

};



//Inverse critical density
float sigmaCritInv(float H0, float omegaMatter, float zLens, float zSource);

// DL in Mpc
float sigmaCritInv(float DL, float aeta_rel_lens, float aeta_rel_source);

float sigmaCritInvInterp3d(
        float& zLens, 
        float& zSource, 
        float& zSourceErr, 
        struct scinv* sc_inv);
double 
interpolateScinvStruct3d(
        float fzsErr, 
        float fzs, 
        float fzl, 
        struct scinv* s);

float sigmaCritInvInterp2d(
        float& zLens, 
        float& zSource, 
        struct scinv2d *sc_inv);
double interpolateScinvStruct2d(
        float fzs, 
        float fzl,
        struct scinv2d *s);



#endif // _sigmaCritInv_h
