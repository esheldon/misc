#if !defined (_corrobj_h)
#define _corrobj_h

#include <cstdio>
#include <cmath>
#include <iostream>

#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"


#include "Array.h"
#include "types.h"
#include "constants.h"

#include "kcorr.h"
#include "binner.h"
#include "gcirc.h"
#include "angDist.h"

// Minimum angle in radians (10 arcseconds)
//#define MINIMUM_ANGLE 4.8481368e-05
// 20 arcsec
//#define MINIMUM_ANGLE 9.6962736e-05
#define MINIMUM_ANGLE 0.0

#define PRIMARY_HEADER_LINES 10
#define SECONDARY_HEADER_LINES 12
#define RANDOM_SECONDARY_HEADER_LINES 9

class corrobj {

 public:
  // Constructor: read the parameter file and store. Also initialize the
  // htm interface.
  corrobj();
  corrobj(char *parfile); 
  
  ~corrobj(); // destructor

  // I/O
  void read(char *parfile); // Read the data: kcorr, primary, secondary according to the type
  void read_par(char *parfile);

  int nrows(FILE *fptr);
  void read_primary();
  void read_secondary();
  void read_random_secondary();
  void read_rev();

  void make_output();
  void reset_output();  
  void write_header(FILE *fptr);
  void write_output(FILE *fptr);

  // For when we just save the pairs
  void write_pairindex_header(FILE *fptr);

  void make_edge_output();
  void reset_edge_output();
  void write_edge_header(FILE *fptr);
  void write_edge_output(FILE *fptr);



  // Tools for doing the correlation function
  void intersect(double ra, double dec, float DA, vector<uint64> &idList);
  void get_seclist(double ra, double dec, float DA, vector <uint64> &idList, 
		   vector<int> &seclist, vector<float> &radlist);
  void bin_by_colorlumrad(float z, float DLum, 
			  vector<int> & seclist, vector<float> &radlist);
  int write_colorlumrad_pairs(int index, 
				float z, 
				float DLum, 
				vector<int>   &seclist, 
				vector<float> &radlist);

  void bin_by_rad(vector<int> & seclist, vector<float> &radlist);

  void correlate();

  void printstuff(int index, int bigStep);

 protected:

  // Output file pointer
  FILE *mFptr;
  FILE *mIndexFptr;



  // Data structures
  par_struct par;
  kcorr_table *kcorr;
  binner *binobj;

  vector<primary_struct> primary;
  vector<secondary_struct> secondary;
  vector<random_secondary_struct> random_secondary;

  int nrev;
  int min_htmind;
  int max_htmind;
  vector<int> rev;

  output_struct output;
  edge_output_struct edge_output;

  htmInterface *htm;  // The htm interface
  const SpatialIndex *spatialIndex;

  // Keep track of max mag
  float max_kept_imag;

  // Timers
  time_t t0;

};


#endif // _corrobj_h
