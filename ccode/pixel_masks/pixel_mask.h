#define FLAGS_MASKED       0x1   /* central point is masked */
#define FLAGS_QUAD1_MASKED 0x2   /* Quadrant masked */
#define FLAGS_QUAD2_MASKED 0x4
#define FLAGS_QUAD3_MASKED 0x8
#define FLAGS_QUAD4_MASKED 0x10

#define FLAGS_QUAD1_MASKED_MONTE 0x20  /* Quadrant masked, monte carlo */
#define FLAGS_QUAD2_MASKED_MONTE 0x40
#define FLAGS_QUAD3_MASKED_MONTE 0x80
#define FLAGS_QUAD4_MASKED_MONTE 0x100

/* Minimum size we want to resolve 
   in square degrees */
#define Amin 0.05
/* Chance of missing Amin sized region */
#define Pmax 0.02

/* Function Prototypes */

int ApplyMask(double *lambdav, double *etav, unsigned long n_gal, 
		short *is_unmaskedv, unsigned long *n_unmasked);

int ApplyEdgeMask(double *lambdav, double *etav, float *maxangle, 
		    unsigned long n_gal, 
		    short *maskflags);
int getCircleLamEta(double *lambdav, double *etav, float *maxangle,
		    unsigned long n_gal, float psi, short quadrant, 
		    double *circle_lambda, double *circle_eta);
int getRandLamEta(double lambda, double eta, float R, short quadrant, 
		  double *rand_lambda, 
		  double *rand_eta);

/* Ryan's functions in pixel_util.c */

void assign_parameters();
void pix2ang(int resolution, unsigned long pixnum, double *lambda, double *eta);
void ang2pix(int resolution, double lambda, double eta, unsigned long *pixnum);
void downsample(int resolution, gsl_matrix *inmap, gsl_matrix *outmap);
void upsample(int resolution, gsl_matrix *inmap, gsl_matrix *outmap);
void superpix(int hi_resolution, unsigned long hi_pixnum, 
	      int lo_resolution, unsigned long *lo_pixnum);
void subpix(int resolution, unsigned long pixnum, unsigned long *sub_pixnum1, 
	    unsigned long *sub_pixnum2, unsigned long *sub_pixnum3, 
	    unsigned long *sub_pixnum4);
void pix_bound(int resolution, unsigned long pixnum, 
	       double *lammin, double *lammax, double *etamin, double *etamax);
double pix_area(int resolution, unsigned long pixnum);
void pix2xyz(int resolution, unsigned long pixnum, 
	     double *x, double *y, double *z);
void area_index(int resolution, double lammin, double lammax, double etamin, 
		double etamax, unsigned long *x_min, unsigned long *x_max, 
		unsigned long *y_min, unsigned long *y_max);
void area_index_stripe(int resolution, int stripe, 
		       unsigned long *x_min, unsigned long *x_max, 
		       unsigned long *y_min, unsigned long *y_max);
void sort_mask_resolution(gsl_vector_int *resolution_array, 
			  gsl_vector_ulong *pixnum_array, unsigned long n_mask);
void sort_mask_pixnum(gsl_vector_ulong *pixnum_array, 
		      gsl_vector_int *resolution_array, unsigned long n_mask,
		      gsl_vector_int *resolution_region_array, 
		      gsl_vector_ulong *resolution_start_array,
		      gsl_vector_ulong *resolution_finish_array, int n_res);
int find_n_res(gsl_vector_int *resolution_array, unsigned long n_mask);
long find_n_superpix(int superpix_resolution, gsl_vector_ulong *pixnum_array,
		     gsl_vector_int *resolution_array, unsigned long n_mask);
void find_resolution_bounds(gsl_vector_int *resolution_array, 
			    unsigned long n_mask,
			    gsl_vector_int *resolution_region_array, 
			    gsl_vector_ulong *resolution_start_array,
			    gsl_vector_ulong *resolution_finish_array);
void find_superpix_bounds(gsl_vector_ulong *superpix_array, 
			  unsigned long n_mask,
			  gsl_vector_ulong *superpix_region_array, 
			  gsl_vector_ulong *superpix_start_array,
			  gsl_vector_ulong *superpix_finish_array);
void make_resolution_struct(gsl_vector_ulong *pixnum_array, 
			    gsl_vector_int *resolution_array, 
			    unsigned long n_pixel, 
			    resolution_struct *res_struct, int n_res);
void make_superpix_struct(int superpix_resolution,
			  gsl_vector_ulong *pixnum_array, 
			  gsl_vector_int *resolution_array, 
			  unsigned long n_pixel, 
			  superpixnum_struct *superpix_struct, 
			  unsigned long n_superpix);
void rand_pixel_position(int resolution, unsigned long pixnum, 
			 double *lambda, double *eta);
double stripe_inclination(int stripe);
void primary_bound(int stripe, double *lammin, double *lammax, 
		   double *etamin, double *etamax);
void hunt(gsl_vector *xx, double x, long *jlo);
void ihunt(gsl_vector_int *xx, int x, long *jlo);
void lhunt(gsl_vector_ulong *xx, unsigned long x, unsigned long *jlo);






/* Typedef statements */

typedef struct {
  double z, type, r, covar_zz, covar_tz, covar_tt, prob, lam, eta;
} monte_carlo_struct;

typedef struct {
  double x, y, z, prob;
  int iter;
} gal_struct;

typedef struct {
  double eta, lam, prob;
  long bbox;
  int iter;
} master_gal_struct; 

typedef struct {
  double thetamin, thetamax, sinthetamin, sinthetamax;
} theta_struct;

typedef struct {
  double lammin, lammax, area, total_gal, total_rand;
  long n_gal;
} iter_struct;

typedef struct {
  double lammin, lammax, etamin, etamax;
  int stripe;
} stripe_struct;

typedef struct {
  int n_stripe;
  unsigned long n_gal;
  stripe_struct *stripe_bound;
  iter_struct *iter_bound;
  double lammin, lammax, etamin, etamax;
} bbox_struct;

typedef struct {
  gsl_vector *gal_gal, *gal_rand, *rand_rand;
  gsl_vector *wtheta, *wtheta_error, *counter, *int_counter;
} wtheta_struct;
