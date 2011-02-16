#ifndef _lens_source_h
#define _lens_source_h

#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "types.h"
#include "angDist.h"
#include "sigmaCritInv.h"
#include "lens_constants.h"

#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"


using namespace std;

struct lens_old {

  int32 zindex;

  float64 ra;
  float64 dec;
  float64 clambda;
  float64 ceta;

  float32 z;
  float32 angmax;
  float32 DL;

  float32 mean_scinv;

  int16 pixelMaskFlags;

};

struct source_old {

  int16 stripe;
  
  float32 e1;
  float32 e2;
  float32 e1e1err;
  float32 e1e2err;
  float32 e2e2err;

  float64 clambda;
  float64 ceta;

  float32 z;
  float32 zerr;

  int32 htm_index;

};

// this is where we treat z as truth
/*
struct source_ztrue {
  
  float64 clambda;
  float64 ceta;

  float32 e1;
  float32 e2;
  // error in each component, so we can use as-is in the tangential shear
  // since it is a projection
  float32 err;
  float32 photoz_z;
  int32 htm_index;
  // structure padding so we can just read the whole chunk of data
  int32 padding;

};
*/


struct source_pixel {

  int32 htm_index;
  int16 nobj;

  float32 e1;
  float32 e2;
  float32 e1e1err;
  float32 e1e2err;
  float32 e2e2err;

  float32 aeta1;
  float32 aeta2;

  float64 clambda;
  float64 ceta;

};

struct lensout {
    
  // nbin will not be output, it is here for convenience.
  short nbin;

  int32 zindex;
    
  int32 tot_pairs;

  vector<int32> npair;
    
  vector<float> rmax_act;
  vector<float> rmin_act;
    
  vector<float> rsum;

  vector<float> sigma;
  vector<float> sigmaerr;
  vector<float> orthosig;
  vector<float> orthosigerr;

  vector<float> sigerrsum;
  vector<float> orthosigerrsum;


  float weight;
        
  vector<float> wsum;
  vector<float> owsum;
  vector<float> wscritinvsum;
        
  float sshsum;
  float wsum_ssh;
        
  float ie;

};



struct par {

  string lens_file;
  string source_file;
  string rev_file;
  string scinv_file;
  string output_file;
  string pair_file;

  /* Is this for pixel-lensing? */
  int16 pixel_lensing;

  /* output info for each pair? */
  int16 dopairs;

  float32 h;
  float32 omega_m;

  int16 sigmacrit_style;
  int16 shape_correction_style;

  int16 logbin;
  int16 nbin;
  
  float32 logRmin;
  float32 logRmax;
  float32 logBinsize;

  // in kpc
  float32 binsize;

  // in kpc
  float32 rmin;
  float32 rmax;

  int16 comoving;
  
  int16 depth;

  float32 zbuffer;

  int32 nlens;
  int32 nsource;
  int32 nrev;

  int32 min_htm_index;
  int32 max_htm_index;

  int32 sample;
  char catalog[250];

};


//////////////////////////////////////////////////
// Class to deal with lens and source data
//////////////////////////////////////////////////

class LensSource {

    public: 
        LensSource(); // default constructor: do nothing
        LensSource(string par_file); // read parameters from this file

        // Reading data
        void ReadPar(string file);
        void ReadScatOld(string file);
        void ReadScatPixel(string scat_file);
        void ReadLcat(string file);
        void ReadRev(string file);
        void ReadScinv2d(string scinv_file);

        int32 GetIdlstructNrows(ifstream& f);

        // Calculate aeta lens/source if using that method
        void CalcAeta();

        // Convenience functions
        int32                GetNsource();
        int32                GetNlens();
        float32              GetScinvFactor();
        float32              GetAetaZero();

        // Test what we have read
        void TestScat();
        void TestScatPixel();
        void TestLcat();
        void TestRev();

        // Output file
        void MakeLensout();
        void ResetLensout();
        void WriteHeader();
        void WriteLensout();

        void WritePairHeader();
        void WritePairs(
                int32 rLensIndex,
                int32 rSourceIndex,
                float32 rRkpc,
                float32 rWeight);
        void WriteTotPairs();

        // Testing quadrants around a lens
        int32 TestQuad(
                int32& bad12, 
                int32& bad23, 
                int32& bad34,
                int32& bad41, 
                double& theta);

        // Great circle distance and angle
        void CalcGcircSurvey(
                double lam1, double eta1, 
                double lam2, double eta2,
                double& dis,  double& theta);


        // HTM searching
        void IntersectHtm(
                double ra, 
                double dec, 
                double cosangle, 
                vector<uint64> &idlist);
        void FindHtmSources(vector<uint64> &idlist, vector<int> &slist);
        void FindNearbySources(int32 rLensIndex);

        // All the shear calculations
        void CalculateSourceShear(
                int32 pSourceSubIndex, 
                int32 pLensIndex, 
                float sig_inv);
        void ProcessSources(int32 rLensIndex);
        void ProcessSourcesQuad(int32 rLensIndex);
        void ComputeLensAverages();
        void PrintMeans(int32 rLensIndex);

        
        void DoTheMeasurement();

    protected:

        // Adhere to mCamelHumpNames for internal variables.
        struct par mPars;

        vector<struct source_old> mSources;
        vector<struct source_pixel> mPixelSources;
        vector<struct lens_old> mLenses;

        float mScinvFactor;
        float mAetaZero;
        vector<float> mAetaRelLens;
        vector<float> mAetaRelSource;

        vector<int32> mRev;

        struct scinv2d mScinv;

        htmInterface *mHTM;  // The htm interface
        const SpatialIndex *mSpatialIndex;

        // This is a temporary variable for holding indices
        vector<int> mSindices;
        // A temp variable for holding radii
        vector<double> mSrkpc;
        // A temp variable for holding theta
        vector<double> mStheta;
        // A temp variable for holding the radial bin

        // Various Running totals 
        float mXpysum, mXmysum, mXysum;
        float mRmaxAct, mRminAct;
        float mSigwsum, mSigsum;

        // output 
        int64 mTotPairs;
        struct lensout mLensout;

        // Output files
        FILE* mFilePtr;
        ofstream mOfstream;
        ofstream mPairOfstream;

        // Are we debuggin?
        int16 mDebug;
        int16 mDebug2;
        // maximum source ellip distribution
        double mMaxe;
};


#endif /* _lens_source_h */
