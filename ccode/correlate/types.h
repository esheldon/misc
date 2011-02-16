#include <vector>
#include "Array.h"

#if !defined (_types_h)
#define _types_h

typedef char                    int8;
typedef unsigned char           uint8;
typedef short int               int16;
typedef unsigned short int      uint16;
typedef int                     int32;
typedef unsigned int            uint32;
typedef float                   float32;
typedef double                  float64;
#ifdef _WIN32
typedef __int64                 int64;
typedef unsigned __int64        uint64;
#else
typedef long long               int64;
typedef unsigned long long      uint64;
#endif



using namespace std;

typedef struct {

    int index;

    double ra;
    double dec;

    float z;

    // To be filled in
    float DA;

} primary_struct;

typedef struct {

    double ra;
    double dec;

    float gflux;
    float rflux;
    float iflux;

    int htm_index;

} secondary_struct;

typedef struct {

    double ra;
    double dec;

    int htm_index;

} random_secondary_struct;


typedef struct {

    // Will not be output, just here for convenience.
    int8 nrad;
    int8 nlum;
    int8 nkgmr;

    int index;
    int totpairs;

    // For calculating mean radius
    vector<float> rsum;

    // Sums over model fluxes in radial bins.
    vector<float> kgflux;
    vector<float> krflux;
    vector<float> kiflux;

    // This is when we are binning by color/lum/rad.  Not allocated
    // otherwise
    Array::array3<int> counts;
    Array::array3<float> lum;

    // This is when just binning by rad.  Not allocated otherwise.
    vector<int> radcounts;
    vector<float> radlum;

} output_struct;

typedef struct {

    int index;
    int totpairs;

    // For calculating mean radius
    vector<float> rsum;
    vector<int> counts;

} edge_output_struct;


typedef struct {

    int nel;
    int min;
    int max;

    int *data;

} rev_struct;

typedef struct {

    // Introduced at version 0.9
    float version;

    char sample[250];
    char primary_file[250];
    char secondary_file[250];
    char rev_file[250];
    char kcorr_file[250];
    char output_file[250];

    // Type:                       sec tags
    // 1: data-data || rand-data   ra,dec,fg,fr,fi,htm_index
    // 2: data-rand || rand-rand   ra,dec,htm_index
    int corrtype;
    int output_type;

    float h;
    float omega_m;

    // radial binning
    int8 nrad;
    float rmin;
    float rmax;

    // lum binning

    // lumband is the band in which we check the luminosity
    // bounds and output the luminosity values
    int lumband;
    int8 nlum;
    float loglmin;
    float loglmax;
    float lmin;
    float lmax;

    // color binning
    int8 nkgmr;
    float kgmrmin;
    float kgmrmax;

    int8 comoving;
    int8 depth;


    // secondary calculations

    float logRmin;
    float logRmax;
    float logBinsize;

    int nprimary;
    int nsecondary;

} par_struct;



#endif /* _types_h */
