/* 
   
calculate_area MaskFile

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector_ulong.h>
#include <gsl/gsl_vector_char.h>
#include <gsl/gsl_vector_int.h>
#include <gsl/gsl_matrix_long.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "sdsspix.c"


/* Minimum size we want to resolve 
   in square degrees */
#define Amin 0.05
/* Chance of missing Amin sized region */
#define Pmax 0.02

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

unsigned long n_gal, n_masks, n_masks_iter, n_thetabins;
unsigned long n_rand, n_bbox, n_tmp, n_mc_iter;
unsigned long n_zbins, n_typebins, n_bins;
monte_carlo_struct *tmp;
gal_struct *gal, *rand_gal, *sub_rand_gal;
bbox_struct *bbox;
wtheta_struct mean_wtheta, mean_sub_wtheta, single_wtheta;
wtheta_struct *wtheta, *sub_wtheta;
theta_struct *theta;
superpixnum_struct *mask_struct;
gsl_vector_ulong *mask_superpixnum_array;
gsl_vector_ulong *gal_pixnum_array, *rand_pixnum_array, *sub_rand_pixnum_array;
gsl_vector *zbin_array, *parameter_array, *eigen_parameter_array;
gsl_vector *eigen_parameter_error_array, *z_kcorr_array, *type_kcorr_array;
gsl_vector *dist_array, *sintheta_array;
gsl_matrix *covar_matrix, *eigenvec_covar_matrix, *trans_eigenvec_covar_matrix;
gsl_matrix *kcorr_matrix;
gsl_integration_workspace *w;
gsl_spline *dist_spline;
gsl_interp_accel *acc;
gsl_rng *mt19937_rand;
double theta_min, theta_max, rand_iter, survey_area;
double omega_m, omega_l, h, H_0;
unsigned long iter, bbox_iter, n_superpix;
int superpix_resolution, pixel_resolution;

int main(int argc, char *argv[])
{
  extern unsigned long n_masks;
  extern unsigned long n_bbox;
  extern bbox_struct *bbox;
  extern gsl_vector_ulong *mask_superpixnum_array;
  extern gsl_integration_workspace *w;
  extern gsl_interp_accel *acc;
  extern gsl_rng *mt19937_rand;
  extern int superpix_resolution;
  double bins_per_decade;
  double unit_double;
  double lammin, lammax, etamin, etamax;
  gsl_vector *fixed_theta_array;
  gsl_vector_ulong *mask_pixnum_array;
  gsl_vector_int *mask_resolution_array;
  const gsl_rng_type *T;

  gsl_vector_int *stripe_array;

  /* function prototypes */

  void CalculateMean();

  double CalculateActualArea();
  double CalculateArea(double lammin, double lammax, 
		       double etamin, double etamax);


  FILE *MaskFile;
  unsigned long n_fixed_thetabins;
  unsigned long i,c,j,k;
  unsigned long bbox_finder, n_masks_old;
  unsigned long pixnum, n_stripe;
  int resolution;

  /* My variables E.S.S. */
  double survey_area;
  char *mask_file_string;

  if (argc < 2) {
    printf("-Syntax: calculate_area maskfile\n");
    return(-1);
  }

  /* Cast the input variables */
  mask_file_string = (char *) argv[1];

  /* printf("\nMaskFile: %s\n\n", mask_file_string); */

  MaskFile = fopen(mask_file_string,"r");

  assign_parameters();

  gsl_rng_env_setup();

  gsl_rng_default_seed = time(NULL);

  T = gsl_rng_default;
  mt19937_rand = gsl_rng_alloc(T);
  w = gsl_integration_workspace_alloc(1000);
  acc = gsl_interp_accel_alloc();

  superpix_resolution = 4;

  /* 
     Setting up the number of pixels in each axis and the number of bins for
     theta 
  */

  n_fixed_thetabins = 50;
  bins_per_decade = 6.0;

  fixed_theta_array = gsl_vector_alloc(n_fixed_thetabins);


  for (i=0,unit_double=-3.0*bins_per_decade;i<n_fixed_thetabins;i++) {
    fixed_theta_array->data[i] = pow(10.0,unit_double/bins_per_decade);
    unit_double += 1.0;
  }

  /* 
     Now we read through the the mask file to find out how many masks 
     (subtracting one for the survey boundary on the first line of the mask file) 
  */

  /* 
   * Deal with the Mask File
   */

  n_masks=0;
  while ((c = getc(MaskFile)) != EOF) {
    if (c == '\n') n_masks++;
  }
  
  printf("There are %lu masks in %s\n",n_masks,mask_file_string);

  rewind(MaskFile);

  n_stripe = 0;
  bbox_finder = 1;
  n_masks_old = n_masks;
  while ((bbox_finder == 1) && (n_stripe < n_masks_old)) {
    fscanf(MaskFile,"%lu %i\n", &pixnum, &resolution);
    if (resolution < 0) {
      n_stripe++;
      n_masks--;
    } else {
      bbox_finder = 0;
    }
  }

  rewind(MaskFile);
  
  printf("Found %lu stripes in %s\n",n_stripe,mask_file_string);

  stripe_array = gsl_vector_int_alloc(n_stripe);

  for (i=0;i<n_stripe;i++)
    fscanf(MaskFile,"%i %i\n",&stripe_array->data[i],&resolution);

  gsl_sort_vector_int(stripe_array);

  n_bbox = 1;

  for (i=1;i<n_stripe;i++) {
    if (stripe_array->data[i] > stripe_array->data[i-1]+1) n_bbox++;
  }

  if (!(bbox=malloc(n_bbox*sizeof(bbox_struct)))) {
    printf("Couldn't allocate bbox_struct memory...\n");
    exit(1);
  }

  printf("Found %lu bounding regions...\n",n_bbox);

  for (i=0;i<n_bbox;i++) bbox[i].n_stripe = 1;

  j = 0;
  for (i=1;i<n_stripe;i++) {
    if (stripe_array->data[i] == stripe_array->data[i-1]+1) {
      bbox[j].n_stripe++;
    } else {
      j++;
    }
  }

  for (i=0;i<n_bbox;i++) {
    if (!(bbox[i].stripe_bound=
	  malloc(bbox[i].n_stripe*sizeof(stripe_struct)))) {
      printf("Couldn't allocate stripe_struct memory...\n");
      exit(1);
    }
  }

  j = k = 0;
  bbox[0].stripe_bound[k].stripe = stripe_array->data[0];
  for (i=1;i<n_stripe;i++) {
    if (stripe_array->data[i] == stripe_array->data[i-1]+1) {
      k++;
      bbox[j].stripe_bound[k].stripe = stripe_array->data[i];
    } else {
      j++;
      k = 0;
      bbox[j].stripe_bound[k].stripe = stripe_array->data[i];
    }
  }


  for (i=0;i<n_bbox;i++) {
    printf("BBOX %lu:\n\t",i+1);
    primary_bound(bbox[i].stripe_bound[0].stripe,
                  &lammin,&lammax,&etamin,&etamax);
    bbox[i].stripe_bound[0].lammin = lammin; 
    bbox[i].stripe_bound[0].lammax = lammax; 
    bbox[i].stripe_bound[0].etamin = etamin; 
    bbox[i].stripe_bound[0].etamax = etamax; 
    bbox[i].lammin = lammin;
    bbox[i].lammax = lammax;
    bbox[i].etamin = etamin;
    bbox[i].etamax = etamax;
    for (j=0;j<bbox[i].n_stripe;j++) {
      printf("%i ",bbox[i].stripe_bound[j].stripe);
      primary_bound(bbox[i].stripe_bound[j].stripe,
                    &lammin,&lammax,&etamin,&etamax);
      bbox[i].stripe_bound[j].lammin = lammin; 
      bbox[i].stripe_bound[j].lammax = lammax; 
      bbox[i].stripe_bound[j].etamin = etamin; 
      bbox[i].stripe_bound[j].etamax = etamax; 
      if (lammax > bbox[i].lammax) bbox[i].lammax = lammax;
      if (lammin < bbox[i].lammin) bbox[i].lammin = lammin;
      if (etamax > bbox[i].etamax) bbox[i].etamax = etamax;
      if (etamin < bbox[i].etamin) bbox[i].etamin = etamin;
    }
    printf("\n");
    bbox[i].n_gal = 0;
  }

  mask_pixnum_array = gsl_vector_ulong_alloc(n_masks);
  mask_resolution_array = gsl_vector_int_alloc(n_masks);
  
  for (i=0;i<n_masks;i++) 
    fscanf(MaskFile,"%lu %i\n",&mask_pixnum_array->data[i],
	   &mask_resolution_array->data[i]);
  
  fclose(MaskFile);
  
  n_superpix = find_n_superpix(superpix_resolution, mask_pixnum_array, 
			       mask_resolution_array, n_masks);
  
  printf("%lu masks span %lu superpixels...\n",n_masks,n_superpix);
  
  if (!(mask_struct=malloc(n_superpix*sizeof(superpixnum_struct)))) {
    printf("Couldn't allocate superpixnum_struct memory...\n");
    exit(1);
  }
  
  mask_superpixnum_array = gsl_vector_ulong_alloc(n_superpix);
  
  make_superpix_struct(superpix_resolution,mask_pixnum_array,
		       mask_resolution_array,n_masks,mask_struct,n_superpix);
  
  for (i=0;i<n_superpix;i++) {
    mask_superpixnum_array->data[i] = mask_struct[i].superpixnum;
  }
  
  gsl_vector_ulong_free(mask_pixnum_array);
  gsl_vector_int_free(mask_resolution_array);


  survey_area = CalculateActualArea();
  printf("Survey Area: %lf square degrees\n", survey_area);
  return(0);
}
  


double CalculateArea(double lammin, double lammax, 
                     double etamin, double etamax)
{
  extern double deg2Rad, strad2Deg;
  double area;
  
  area = sin(deg2Rad*lammax) - sin(deg2Rad*lammin);
  area *= strad2Deg*(deg2Rad*(etamax - etamin));

  return area;

}





double CalculateActualArea()
{
  int area_resolution, n_dropped, masked, n_partial, n_check;
  gsl_vector_ulong *pixnum_array;
  double lammin, lammax, area, unmasked_area, tarea;
  unsigned long i, j, k, n, m, jlo, ilo, n_pixel, x_min, x_max,y_min,y_max;
  unsigned long pixnum, nx, ny;

  area_resolution = 256;
  nx = nx0*area_resolution;
  ny = ny0*area_resolution;

  printf("Calculating unmasked area for each section...\n");

  for (n=0;n<n_bbox;n++) {

    tarea = 0.0;
    n_pixel = 0;

    for (i=0;i<bbox[n].n_stripe;i++) {
      lammin = bbox[n].stripe_bound[i].lammin;
      lammax = bbox[n].stripe_bound[i].lammax;
      
      area_index(area_resolution,lammin,lammax,
		 bbox[n].stripe_bound[i].etamin,
		 bbox[n].stripe_bound[i].etamax,
		 &x_min,&x_max,&y_min,&y_max);
      n_pixel += (x_max - x_min + 1)*(y_max - y_min + 1);
    }

    pixnum_array = gsl_vector_ulong_alloc(n_pixel);
    
    m = 0;
    for (k=0;k<bbox[n].n_stripe;k++) {
      
      lammin = bbox[n].stripe_bound[k].lammin;
      lammax = bbox[n].stripe_bound[k].lammax;
      
      area_index(area_resolution,lammin,lammax,
		 bbox[n].stripe_bound[k].etamin,
		 bbox[n].stripe_bound[k].etamax,
		 &x_min,&x_max,&y_min,&y_max);
      for (j=y_min;j<=y_max;j++) {
	for (i=x_min;i<=x_max;i++) {
	  pixnum_array->data[m] = nx*j + i;
	  m++;
	}
      }
    }

    gsl_sort_vector_ulong(pixnum_array);
    
    n_dropped = n_partial = n_check = 0;
    
    for (m=0;m<n_pixel;m++) {
      area = pix_area(area_resolution,pixnum_array->data[m]);
      unmasked_area = 1.0;
      
      superpix(area_resolution,pixnum_array->data[m],
	       superpix_resolution,&pixnum);
      
      masked = 0;
      lhunt(mask_superpixnum_array,pixnum,&jlo);
      
      if (jlo <= n_superpix - 1) {
	n_check++;
	if (mask_superpixnum_array->data[jlo] == pixnum) {
	  if (mask_struct[jlo].n_pixel == 1) {
	    unmasked_area = 0.0;
	    n_dropped++;
	  } else {
	    for (k=0;k<mask_struct[jlo].n_res;k++) {
	      if (mask_struct[jlo].res_struct[k].resolution == 
		  area_resolution) {
		if (mask_struct[jlo].res_struct[k].n_pixel == 1) {
		  ilo = 0;
		} else {
		  lhunt(mask_struct[jlo].res_struct[k].pixnum,
			pixnum_array->data[m],&ilo);
		}
		if (ilo <= mask_struct[jlo].res_struct[k].n_pixel-1) {
		  if (mask_struct[jlo].res_struct[k].pixnum->data[ilo] ==
		      pixnum_array->data[m]) {
		    unmasked_area = 0.0;
		    n_dropped++;
		  }
		}
	      }
	      if (mask_struct[jlo].res_struct[k].resolution < 
		  area_resolution) {
		superpix(area_resolution,pixnum_array->data[m],
			 mask_struct[jlo].res_struct[k].resolution,&pixnum);
		if (mask_struct[jlo].res_struct[k].n_pixel == 1) {
		  ilo = 0;
		} else {
		  lhunt(mask_struct[jlo].res_struct[k].pixnum,pixnum,&ilo);
		}
		if (ilo <= mask_struct[jlo].res_struct[k].n_pixel-1) {
		  if (mask_struct[jlo].res_struct[k].pixnum->data[ilo] == 
		      pixnum) {
		    unmasked_area = 0.0;
		    n_dropped++;
		  }
		}
	      }
	      if (mask_struct[jlo].res_struct[k].resolution > 
		  area_resolution) {
		for (j=0;j<mask_struct[jlo].res_struct[k].n_pixel;j++) {
		  superpix(mask_struct[jlo].res_struct[k].resolution,
			   mask_struct[jlo].res_struct[k].pixnum->data[j],
			   area_resolution,&pixnum);
		  if (pixnum_array->data[m] == pixnum) {
		    unmasked_area -= 
		      1.0*area_resolution*area_resolution/
		      (mask_struct[jlo].res_struct[k].resolution*
		       mask_struct[jlo].res_struct[k].resolution);
		    masked = 1;
		  }
		}
	      }
	    }     
	  }
	}
      }
      if (masked == 1) n_partial++;
      tarea += unmasked_area*area;
    }
    
    gsl_vector_ulong_free(pixnum_array);
    printf("BBOX: %lu Area: %lf square degrees\n",n+1,tarea); 
    
    survey_area += tarea;
    
  }


  return(survey_area);
} /* end CalculateActualArea */


