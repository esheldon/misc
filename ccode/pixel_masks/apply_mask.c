/* 
   
Modified from Ryan Scranton's w(theta) code to just apply the
mask to input lambda/eta points.
Erin Sheldon 23-May-2003

apply_mask GalaxyFile MaskFile OutputFile

Ryan's comments:
   This code is intended to calculate w(theta) from a dataset and a mask file.
   The dataset needs to be two columns (ostensibly RA and DEC).  The mask
   file should contain coordinates for the upper left and lower right corners
   of the rectangular areas that have been cut out of the survey area for one
   reason or another.  Thus, the mask file should have four columns:
   
   RAmin DECmax RAmax DECmin

   The first line of the mask file should contain the coordinates for the area 
   that the survey covers in the same fashion. The data file and mask file 
   need to be given in the command line; usage is 
  
   calc_wtheta datafile maskfile

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
#include "pixel_util.c"
#include "pixel_mask.h"
#include "ApplyMask.c"

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
  extern unsigned long n_gal, n_masks, n_masks_iter, n_thetabins;
  extern unsigned long n_rand, n_bbox, n_tmp, n_mc_iter;
  extern unsigned long n_zbins, n_typebins, n_bins;
  extern monte_carlo_struct *tmp;
  extern gal_struct *gal, *rand_gal, *sub_rand_gal;
  extern bbox_struct *bbox;
  extern wtheta_struct mean_wtheta, mean_sub_wtheta, single_wtheta;
  extern wtheta_struct *wtheta, *sub_wtheta;
  extern theta_struct *theta;
  extern gsl_vector *theta_array, *sintheta_array;
  extern gsl_vector_ulong *mask_superpixnum_array;
  extern gsl_integration_workspace *w;
  extern gsl_spline *dist_spline;
  extern gsl_interp_accel *acc;
  extern gsl_rng *mt19937_rand;
  extern double theta_min, theta_max, rand_iter, omega_m, omega_l, h, H_0;
  extern unsigned long iter, bbox_iter;
  extern int superpix_resolution;
  double LAMmin, LAMmax, ETAmin, ETAmax, upper_abs_mag, lower_abs_mag;
  double dtheta, rect_LAMmin,rect_LAMmax, tot_rand, bins_per_decade,all_gal;
  double temp_lam, temp_eta, temp_mag, upper_mag,lower_mag,unit_double,prob;
  double temp_r, temp_abs_r, temp_z, temp_type, temp_prob, z_min, z_max;
  double upper_z, lower_z, upper_type, lower_type, dz, z_length;
  double default_lower_mag, default_upper_mag, default_lower_abs_mag;
  double default_upper_abs_mag, default_lower_z, default_upper_z;
  double default_upper_type, default_lower_type, min_prob,result,error;
  double temp_covar_zz, temp_covar_tz, temp_covar_tt, mean_prob;
  double lammin, lammax, etamin, etamax;
  master_gal_struct *master_gal;
  gsl_vector *fixed_theta_array;
  gsl_vector_ulong *mask_pixnum_array, *tmp_pixnum_array;
  gsl_vector_int *mask_resolution_array;
  const gsl_rng_type *T;
  double DistInt(double z, void *param);
  gsl_function dist_int;
  gsl_vector_int *stripe_array;
  gsl_permutation *gal_index;

  /* function prototypes */
  void FindPixelResolution();
  void GenerateProbabilities(double lower_z, double upper_z,
			     double lower_type, double upper_type,
			     double lower_abs_mag, double upper_abs_mag);
  void CalculateMean();
  void CalculateIterArea();
  double CalculateArea(double lammin, double lammax, 
		       double etamin, double etamax);
  void Correlate(double dtheta, gsl_vector_char *output_tag);
  void CalculateBias(unsigned char *bias_file);
  int double_match(double x, double y);
  int ApplyMask(double *lambdav, double *etav, unsigned long n_gal, 
		short *is_unmaskedv, unsigned long *n_unmasked);

  FILE *GalaxyFile, *MaskFile, *OutputFile, *SubOutputFile, *BiasFile;
  FILE *MeanOutputFile, *MeanBiasFile, *VarianceFile, *SubMeanOutputFile;
  FILE *CovarOutputFile, *KCorrectionFile;
  gsl_vector_char *output_file;
  double Log_theta, dLog_theta;
  unsigned long n_obj,n_thetamin,n_thetamax, n_fixed_thetabins, sum_obj;
  unsigned long jlo;
  unsigned long i,c,j,k,n,m,col,fld,run,id,bit,select_bit,max_mask,max_gal;
  unsigned long bbox_finder, n_masks_old, generate_auto_tag, made_auto_tag;
  unsigned long default_n_mc_iter, pixnum, n_stripe;
  int resolution;

  double *lambdav, *etav;
  short *is_unmaskedv;
  double LAM,ETA;
  unsigned long ilo, n_unmasked;
  int is_unmasked, stripe_iter;
  int bbox_num, stripe_num;
  
  float tmp1,tmp2;

  if (argc < 4) {
    printf("Usage: %s GalaxyFile MaskFile OutputFile\n",
	   argv[0]);
    exit(1);
  }

  printf("\nGalaxyFile: %s\nMaskFile: %s\n\n", argv[1],argv[2]);

  GalaxyFile = fopen(argv[1],"r");
  MaskFile = fopen(argv[2],"r");

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
     Now we read through the data file and the mask file to find out how long
     each is and thus how many galaxies and how many masks (subtracting one for
     the survey boundary on the first line of the mask file) 
  */

  /*
    First the Galaxy File

    # of galaxies
  */

  n_obj=0;
  while ((c = getc(GalaxyFile)) != EOF) {
    if (c == '\n') n_obj++;
  }
  n_gal = n_obj;

  /* 
     allocate the vectors for lambda/eta 
  */

  if (!(bbox=malloc(n_bbox*sizeof(bbox_struct)))) 
    {
      printf("Couldn't allocate bbox_struct memory...\n");
      return(-1);
    }
  if (!(lambdav = calloc(n_gal, sizeof(double))))
    {
      printf("Couldn't allocate lambdav memory...\n");
      return(-1);
    }
  if (!(etav = calloc(n_gal, sizeof(double))))
    {
      printf("Couldn't allocate etav memory...\n");
      return(-1);
    }
  if (!(is_unmaskedv = calloc(n_gal, sizeof(short))))
    {
      printf("Couldn't allocate is_unmaskedv memory...\n");
      return(-1);
    }

  /* read in the positions */
  rewind(GalaxyFile);
  
  for (i=0;i<n_gal;i++) 
    {
      if(fscanf(GalaxyFile,"%lf %lf",
		&temp_lam, &temp_eta) != 2)
	{
	  printf("Failure!!\n");
	  return(-1);
	}
      
      lambdav[i] = temp_lam;
      etav[i] = temp_eta;

    }
  
  printf("There are %u objects in %s\n",n_obj,argv[1]);

  fclose(GalaxyFile);

  /* 
   * Now deal with the Mask File
   */

  n_masks=0;
  while ((c = getc(MaskFile)) != EOF) {
    if (c == '\n') n_masks++;
  }
  
  printf("There are %u masks in %s\n",n_masks,argv[2]);

  rewind(MaskFile);

  n_stripe = 0;
  bbox_finder = 1;
  n_masks_old = n_masks;
  while ((bbox_finder == 1) && (n_stripe < n_masks_old)) {
    fscanf(MaskFile,"%u %i\n", &pixnum, &resolution);
    if (resolution < 0) {
      n_stripe++;
      n_masks--;
    } else {
      bbox_finder = 0;
    }
  }

  rewind(MaskFile);
  
  printf("Found %d stripes in %s\n",n_stripe,argv[2]);

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

  printf("Found %u bounding regions...\n",n_bbox);

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
    printf("BBOX %u:\n\t",i+1);
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
    fscanf(MaskFile,"%u %i\n",&mask_pixnum_array->data[i],
	   &mask_resolution_array->data[i]);
  
  fclose(MaskFile);
  
  n_superpix = find_n_superpix(superpix_resolution, mask_pixnum_array, 
			       mask_resolution_array, n_masks);
  
  printf("%d masks span %i superpixels...\n",n_masks,n_superpix);
  
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
    

  /* 
     loop over the galaxy positions. Start with the
     object assumed as unmasked
  */

  printf("\nApplying the mask\n");
  ApplyMask(lambdav, etav, n_gal, is_unmaskedv, &n_unmasked);

  printf("\nUnmasked galaxies: %d/%d\n\n", n_unmasked, n_gal);

  printf("Writing to Output File: %s\n",argv[3]);

  OutputFile = fopen(argv[3], "w");

  for(i=0;i<n_gal;i++) 
    {
      fprintf(OutputFile, "%lf %lf %d\n", 
	      lambdav[i], etav[i], is_unmaskedv[i]);
    }

  fclose(OutputFile);

  return(0);
}
  

int double_match(double x, double y)
{
  double tolerance;

  tolerance = 1.0e-5;

  if ((x >= 0.0) && (y >= 0.0)) { 
    if ((y <= (1.0+tolerance)*x) && (y >= (1.0-tolerance)*x)) {
      return 1;
    } else {
      return 0;
    }
  } else {
    if ((y >= (1.0+tolerance)*x) && (y <= (1.0-tolerance)*x)) {
      return 1;
    } else {
      return 0;
    }
  }

}
   		   
int double_le(double x, double y) {
  double tolerance;
  
  tolerance = 1.0e-6;

  if (x <= y + tolerance) {
    return 1;
  } else {
    return 0;
  }
}

int double_ge(double x, double y) {
  double tolerance;
  
  tolerance = 1.0e-6;

  if (x >= y - tolerance) {
    return 1;
  } else {
    return 0;
  }
}

double CalculateArea(double lammin, double lammax, 
                     double etamin, double etamax)
{
  extern double pi, deg2Rad, strad2Deg;
  double area;
  
  area = sin(deg2Rad*lammax) - sin(deg2Rad*lammin);
  area *= strad2Deg*(deg2Rad*(etamax - etamin));

  return area;

}



double DistInt(double z, void *param)
{
  extern double omega_m, omega_l;
  double omega_k,a;
  
  omega_k = 1.0 - omega_m - omega_l;
  a = 1.0/(1.0 + z);

  return 1.0/sqrt(omega_m/(a*a*a) + omega_l + omega_k/(a*a));
}

double FindDistance(double z)
{
  return gsl_spline_eval(dist_spline,z,acc);
}


void GenerateProbabilities(double lower_z, double upper_z,
                           double lower_type, double upper_type,
                           double lower_abs_mag, double upper_abs_mag)
{
  extern unsigned long n_tmp, n_mc_iter;
  void FindEigenValues(long n);
  double FindDistance(double z);
  double KCorrection(double z, double type);
  double z, type, abs_r, dist, kcorr, total, mean_dist, mean_kcorr, tmp_total;
  gsl_vector *tmp_parameter_array, *new_parameter_array;
  long i, j, k, n, ok_result, n_bins,n_neg;
  unsigned long jlo;
  FILE *RealizationCheck;

  tmp_parameter_array = gsl_vector_alloc(2);
  new_parameter_array = gsl_vector_alloc(2);
  covar_matrix = gsl_matrix_alloc(2,2);
  eigenvec_covar_matrix = gsl_matrix_alloc(2,2);
  trans_eigenvec_covar_matrix = gsl_matrix_alloc(2,2);
  parameter_array = gsl_vector_alloc(2);
  eigen_parameter_array = gsl_vector_alloc(2);
  eigen_parameter_error_array = gsl_vector_alloc(2);
  
  n_bins = 100;
  
  n_neg = 0;

  mean_dist = mean_kcorr = tmp_total = 0.0;

  for (n=0;n<n_tmp;n++) {
    
    FindEigenValues(n);
    
    total = 0.0;

    if  (tmp[n].prob > 0.0) {
  
      for (i=0;i<n_mc_iter;i++) {
        for (j=0;j<2;j++)
          tmp_parameter_array->data[j] = eigen_parameter_array->data[j] + 
            eigen_parameter_error_array->data[j]*
            gsl_ran_ugaussian(mt19937_rand);

    
        for (k=0;k<2;k++) {
          new_parameter_array->data[k] = 0.0;
          for (j=0;j<2;j++) {
            new_parameter_array->data[k] += 
              tmp_parameter_array->data[j]*
              eigenvec_covar_matrix->data[k*eigenvec_covar_matrix->tda+j];
          }
        }

        z = new_parameter_array->data[0];
        type = new_parameter_array->data[1];

        if (double_le(type,upper_type) && double_ge(type,lower_type) &&
            double_le(z,upper_z) && double_ge(z,lower_z)) {
          dist = FindDistance(z);
          kcorr = KCorrection(z,type);
          abs_r = tmp[n].r - 5.0*dist - 25.0 - kcorr;
          mean_dist += dist;
          mean_kcorr += kcorr;
          tmp_total += 1.0;
          if (double_le(abs_r,upper_abs_mag) && 
              double_ge(abs_r,lower_abs_mag)) 
            total += 1.0;
        }
      }
      total /= 1.0*n_mc_iter;
      tmp[n].prob *= total;
    } else {
      n_neg++;
    }
  }    

  printf("Had to drop %u galaxies due to singular covariance matrices\n",
         n_neg);

}    


void FindEigenValues(long n)
{
  extern unsigned long n_bins, n_obj, n_iter, n_gal;
  extern double upper_type, lower_type;
  double trace, det, e1, e2, norm, a, b, c;
  int i,j, k, eigen_match, is_positive, n_rot, n_drop,n_neg, ok_result;

  a = tmp[n].covar_zz;
  b = tmp[n].covar_tt;
  c = tmp[n].covar_tz;
  
  det = a*b - c*c;

  if (det > 1.0e-12) {
  
    parameter_array->data[0] = tmp[n].z;
    parameter_array->data[1] = tmp[n].type;

    trace = a + b;
       
    eigen_parameter_error_array->data[0] = e1 = 
      0.5*trace + sqrt(0.25*trace*trace - det);
    eigen_parameter_error_array->data[1] = e2 = 
      0.5*trace - sqrt(0.25*trace*trace - det);

    norm = sqrt((b - e1 + c)*(b - e1 + c) + (e1 - a - c)*(e1 - a - c));
    
    gsl_matrix_set(eigenvec_covar_matrix,0,0,(b-e1+c)/norm);
    gsl_matrix_set(eigenvec_covar_matrix,1,0,(e1-a-c)/norm); 
    gsl_matrix_set(eigenvec_covar_matrix,0,1,-1.0*(e1-a-c)/norm); 
    gsl_matrix_set(eigenvec_covar_matrix,1,1,(b-e1+c)/norm);
    

    /* covar_matrix->data[0*covar_matrix->tda+0] = covar_zz_array->data[n];
       covar_matrix->data[0*covar_matrix->tda+1] = 
       covar_matrix->data[1*covar_matrix->tda+0] = covar_tz_array->data[n];
       covar_matrix->data[1*covar_matrix->tda+1] = covar_tt_array->data[n];
       
       gsl_eigen_symmv(covar_matrix,eigen_parameter_error_array,
       eigenvec_covar_matrix,eigen_w);
       
       gsl_eigen_symmv_sort(eigen_parameter_error_array,eigenvec_covar_matrix,
       GSL_EIGEN_SORT_VAL_DESC);
    
       if (n == 1) {
       printf("%e %e\n",e1,eigen_parameter_error_array->data[0]);
       printf("%e %e\n",e2,eigen_parameter_error_array->data[1]);
       } */

    is_positive = 1;
    
    n_neg = 0;
    
    for (i=0;i<2;i++) {
      if (eigen_parameter_error_array->data[i] < 0.0) {
        is_positive = 0;
        n_neg++;
        eigen_parameter_error_array->data[i] = 0.0;
        for (j=0;j<2;j++) 
          eigenvec_covar_matrix->data[j*eigenvec_covar_matrix->tda+i] = 0.0;
      }
    }
    
    for (i=0;i<2;i++) 
      for (j=0;j<2;j++) 
        trans_eigenvec_covar_matrix->
          data[i*trans_eigenvec_covar_matrix->tda+j] = 
          eigenvec_covar_matrix->data[j*eigenvec_covar_matrix->tda+i];
    
    for (i=0;i<2;i++) {
      /* Here we take the square root since we're interested in the error
         rather than the variance. */
      eigen_parameter_error_array->data[i] = 
        sqrt(eigen_parameter_error_array->data[i]);
      eigen_parameter_array->data[i] = 0.0;
      for (j=0;j<2;j++) {
        eigen_parameter_array->data[i] += 
          parameter_array->data[j]*
          trans_eigenvec_covar_matrix->
          data[i*trans_eigenvec_covar_matrix->tda+j];
      }
    }
  } else {
    tmp[n].prob = -1.0;
  }
  
}

double KCorrection(double z, double type)
{
  double dz, dtype, kcorr_hi, kcorr_lo, dkcorr, kcorr_upper, kcorr_lower;
  unsigned long jlo, ilo;

  hunt(z_kcorr_array,z,&jlo);
  hunt(type_kcorr_array,type,&ilo);

  if ((jlo == n_zbins - 1) && (ilo < n_typebins - 1)) {
    kcorr_hi = kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo];
    kcorr_lo = kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo+1];
    dkcorr = kcorr_hi - kcorr_lo;
    dtype = type_kcorr_array->data[ilo+1] - type_kcorr_array->data[ilo];
    return kcorr_lo + (type-type_kcorr_array->data[ilo])*dkcorr/dtype;
  }

  if ((jlo < n_zbins - 1) && (ilo == n_typebins - 1)) {
    kcorr_hi = kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo];
    kcorr_lo = kcorr_matrix->data[(jlo+1)*kcorr_matrix->tda+ilo];
    dkcorr = kcorr_hi - kcorr_lo;
    dz = z_kcorr_array->data[jlo+1] - z_kcorr_array->data[jlo];
    return kcorr_lo + (z-z_kcorr_array->data[jlo])*dkcorr/dz;
  }

  if ((jlo == n_zbins - 1) && (ilo == n_typebins - 1)) {
    return kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo];
  }


  kcorr_lo = kcorr_matrix->data[(jlo+1)*kcorr_matrix->tda+ilo];
  kcorr_hi = kcorr_matrix->data[(jlo+1)*kcorr_matrix->tda+ilo+1];
  dkcorr = kcorr_hi - kcorr_lo;
  dtype = type_kcorr_array->data[ilo+1] - type_kcorr_array->data[ilo];
  kcorr_upper = kcorr_lo + (type-type_kcorr_array->data[ilo])*dkcorr/dtype;

  kcorr_lo = kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo];
  kcorr_hi = kcorr_matrix->data[jlo*kcorr_matrix->tda+ilo+1];
  dkcorr = kcorr_hi - kcorr_lo;
  kcorr_lower = kcorr_lo + (type-type_kcorr_array->data[ilo])*dkcorr/dtype;

  dkcorr = kcorr_upper - kcorr_lower;
  dz = z_kcorr_array->data[jlo+1] - z_kcorr_array->data[jlo];

  return kcorr_lo + (z - z_kcorr_array->data[jlo])*dkcorr/dz;
}

void FindPixelResolution()
{
  extern int superpix_resolution, pixel_resolution;
  long i, j, k, nx, ny, pixnum;
  double x1,x2,y1,y2,z1,z2,costheta,sintheta, sinthetamax, sinthetamin;
  int all_good, small_good, lat_good, eta_good, resolution;

  all_good = small_good = lat_good = eta_good = 0;

  pixel_resolution = 4096;
  
  nx = nx0*pixel_resolution;
  ny = ny0*pixel_resolution;

  sinthetamax = theta[n_thetabins-1].sinthetamax;
  
  while ((small_good < ny - 2) || (eta_good < 0)) {
      
    all_good = small_good = lat_good = eta_good = 0;
    
    pixel_resolution /= 2;
    
    nx = nx0*pixel_resolution;
    ny = ny0*pixel_resolution;
    
    pixnum = nx*ny/2;
    pix2xyz(pixel_resolution,pixnum,&x1,&y1,&z1);
    pixnum = nx*ny/2 + 1;
    pix2xyz(pixel_resolution,pixnum,&x2,&y2,&z2);
    costheta = (x1*x2 + y1*y2 + z1*z2);
    sintheta = 1.0 - costheta*costheta;
    if (sintheta > sinthetamax) eta_good++;
    
    pixnum = nx;
    pix2xyz(pixel_resolution,pixnum,&x2,&y2,&z2);
    for (j=2;j<ny;j++) {
      /* for (k=1;k<=pixel_resolution/4;k++) {
         pixnum = nx*j + k;
         pix2xyz(pixel_resolution,pixnum,&x2,&y2,&z2);
         costheta = (x1*x2 + y1*y2 + z1*z2);
         sintheta = 1.0 - costheta*costheta;
         if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) lat_good++;
         }
      */
      pixnum = nx*j + 0;
      pix2xyz(pixel_resolution,pixnum,&x2,&y2,&z2);
      costheta = (x1*x2 + y1*y2 + z1*z2);
      sintheta = 1.0 - costheta*costheta;
      if (sintheta > sinthetamax) small_good++;
      x1 = x2;
      y1 = y2;
      z1 = z2;
    }
    all_good += eta_good + lat_good + small_good;
    printf("%i: %i/1 %i/%i\n",pixel_resolution, eta_good,small_good,ny);
  }  

  /*  pixel_resolution /= 2; */
    
  printf("Setting pixel search resolution to %i...\n",pixel_resolution);

}

void CalculateIterArea()
{
  int area_resolution, n_dropped, masked, n_partial, n_check;
  gsl_vector_ulong *pixnum_array, *superpixnum_array;
  double lammin, lammax, area, unmasked_area, lam, eta;
  unsigned long i, j, k, n, m, jj, jlo, ilo, n_pixel, x_min, x_max,y_min,y_max;
  unsigned long pixnum, nx, ny;
  FILE *PixelCheck;

  area_resolution = 256;
  nx = nx0*area_resolution;
  ny = ny0*area_resolution;

  printf("Calculating unmasked area for each section...\n");

  for (n=0;n<n_bbox;n++) {
    for (jj=0;jj<iter;jj++) {

      /* if ((n == n_bbox-1) && (jj == iter-1)) 
	 PixelCheck = fopen("PixelCheck","w"); */

      bbox[n].iter_bound[jj].area = 0.0;

      n_pixel = 0;
      
      for (i=0;i<bbox[n].n_stripe;i++) {
	if (bbox[n].iter_bound[jj].lammin < bbox[n].stripe_bound[i].lammin) {
	  lammin = bbox[n].stripe_bound[i].lammin;
	} else {
	  lammin = bbox[n].iter_bound[jj].lammin;
	}
	if (bbox[n].iter_bound[jj].lammax > bbox[n].stripe_bound[i].lammax) {
	  lammax = bbox[n].stripe_bound[i].lammax;
	} else {
	  lammax = bbox[n].iter_bound[jj].lammax;
	}
	area_index(area_resolution,lammin,lammax,
		   bbox[n].stripe_bound[i].etamin,
		   bbox[n].stripe_bound[i].etamax,
		   &x_min,&x_max,&y_min,&y_max);
	n_pixel += (x_max - x_min + 1)*(y_max - y_min + 1);
      }
      
      pixnum_array = gsl_vector_ulong_alloc(n_pixel);
      
      m = 0;
      for (k=0;k<bbox[n].n_stripe;k++) {
	if (bbox[n].iter_bound[jj].lammin < bbox[n].stripe_bound[k].lammin) {
	  lammin = bbox[n].stripe_bound[k].lammin;
	} else {
	  lammin = bbox[n].iter_bound[jj].lammin;
	}
	if (bbox[n].iter_bound[jj].lammax > bbox[n].stripe_bound[k].lammax) {
	  lammax = bbox[n].stripe_bound[k].lammax;
	} else {
	  lammax = bbox[n].iter_bound[jj].lammax;
	}
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
	/* if ((n == n_bbox-1) && (jj == iter-1)) {
	   pix2ang(area_resolution,pixnum_array->data[m],&lam,&eta);
	   fprintf(PixelCheck,"%d %d %lf %lf %lf\n",pixnum_array->data[m],
	   superpixnum_array->data[m],unmasked_area,lam,eta);
	   } */
	/* if (n_check > 0) printf("%lf %lf %d\n",unmasked_area,area,
	   n_dropped); */
	bbox[n].iter_bound[jj].area += unmasked_area*area;
      }
    
      gsl_vector_ulong_free(pixnum_array);
      printf("%lf %lf %d %d %d %d\n",bbox[n].iter_bound[jj].area,
	     CalculateArea(bbox[n].iter_bound[jj].lammin,
			   bbox[n].iter_bound[jj].lammax,
			   bbox[n].etamin,bbox[n].etamax),
	     n_pixel,n_check,n_dropped,n_partial); 
      /* if ((n == n_bbox-1) && (jj == iter-1)) fclose(PixelCheck); */

    }
  }
}


void MakeRandomCatalog(gsl_vector_char *output_tag)
{
  extern unsigned long iter, bbox_iter;
  double LAM, ETA, LAM_length, ETA_length, max_seg;
  double z_min, z_max, z_length, z;
  gsl_permutation *gal_index;
  gsl_vector *LAM_array, *ETA_array;
  gsl_vector_ulong *tmp_pixnum_array;
  unsigned long jlo, ilo;
  unsigned long idum, stripe_iter;
  unsigned long i,n,j,k,is_unmasked, pixnum;
  gsl_vector_char *random_catalog_file;
  FILE *RandomCatalogFile;

  LAM_length = bbox[bbox_iter].lammax - bbox[bbox_iter].lammin;
  ETA_length = bbox[bbox_iter].etamax - bbox[bbox_iter].etamin;
  
  if (ETA_length < LAM_length) {
    max_seg = LAM_length;
    
    z_max = sin(deg2Rad*bbox[bbox_iter].lammax);
    z_min = sin(deg2Rad*bbox[bbox_iter].lammin);
    z_length = z_max - z_min;
  } else {
    max_seg = ETA_length;

    z_max = sin(deg2Rad*bbox[bbox_iter].etamax);
    z_min = sin(deg2Rad*bbox[bbox_iter].etamin);
    z_length = z_max - z_min;
    
    z_min = sin(deg2Rad*bbox[bbox_iter].lammin);
  }
  
  LAM_array = gsl_vector_alloc(bbox[bbox_iter].n_gal);
  ETA_array = gsl_vector_alloc(bbox[bbox_iter].n_gal);
  tmp_pixnum_array = gsl_vector_ulong_alloc(bbox[bbox_iter].n_gal);
  gal_index = gsl_permutation_alloc(bbox[bbox_iter].n_gal);

  random_catalog_file = gsl_vector_char_alloc(125);

  sprintf(random_catalog_file->data,"random_catalog%u",bbox_iter);
  /* RandomCatalogFile = fopen(random_catalog_file->data,"w"); */

  printf("Making random catalog...\n");
  
  for (i=0;i<iter;i++) bbox[bbox_iter].iter_bound[i].total_rand = 0.0;
 
  k=1;
  for (i=0;i<bbox[bbox_iter].n_gal;i++) {
    is_unmasked = 0;

    while (is_unmasked == 0) {
      is_unmasked = 0;
      ETA = max_seg*gsl_rng_uniform(mt19937_rand) + bbox[bbox_iter].etamin;
      z = z_length*gsl_rng_uniform(mt19937_rand) + z_min;
      LAM = asin(z)/deg2Rad;

      if ((LAM <= bbox[bbox_iter].lammax) && 
	  (LAM >= bbox[bbox_iter].lammin) &&
	  (ETA <= bbox[bbox_iter].etamax) && 
	  (ETA >= bbox[bbox_iter].etamin)) {

	for (j=0;j<bbox[bbox_iter].n_stripe;j++) {
	  if ((ETA <= bbox[bbox_iter].stripe_bound[j].etamax) && 
	      (ETA >= bbox[bbox_iter].stripe_bound[j].etamin)) {
	    stripe_iter = j;
	    j = bbox[bbox_iter].n_stripe;
	  }
	}
	
	if ((LAM <= bbox[bbox_iter].stripe_bound[stripe_iter].lammax) && 
	    (LAM >= bbox[bbox_iter].stripe_bound[stripe_iter].lammin)) 
	  is_unmasked = 1;

	if ((n_masks > 0) && (is_unmasked == 1)) {
	  ang2pix(superpix_resolution,LAM,ETA,&pixnum);
	  
	  lhunt(mask_superpixnum_array,pixnum,&jlo);
	  
	  if (jlo < n_superpix) {
	    if (pixnum == mask_superpixnum_array->data[jlo]) {
	      if (mask_struct[jlo].n_pixel == 1) {
		is_unmasked = 0;
	      } else {
		for (k=0;k<mask_struct[jlo].n_res;k++) {
		  ang2pix(mask_struct[jlo].res_struct[k].resolution,
			  LAM,ETA,&pixnum);
		  if (mask_struct[jlo].res_struct[k].n_pixel == 1) {
		    ilo = 0;
		  } else {
		    lhunt(mask_struct[jlo].res_struct[k].pixnum,pixnum,&ilo);
		  }
		  if (ilo < mask_struct[jlo].res_struct[k].n_pixel) {
		    if (mask_struct[jlo].res_struct[k].pixnum->data[ilo] ==
			pixnum) is_unmasked = 0;
		  }
		}
	      }
	    }
	  }    
	}
      }
    }
    LAM_array->data[i] = LAM;
    ETA_array->data[i] = ETA;
    ang2pix(pixel_resolution,LAM,ETA,&tmp_pixnum_array->data[i]);
  }  

  printf("Sorting...\n");

  gsl_sort_vector_ulong_index(gal_index,tmp_pixnum_array);
  
  for (j=0;j<bbox[bbox_iter].n_gal;j++) {
    i = gal_index->data[j];

    rand_pixnum_array->data[j] = tmp_pixnum_array->data[i];

    rand_gal[j].x = -1.0*sin(deg2Rad*LAM_array->data[i]);
    rand_gal[j].y = 
      cos(LAM_array->data[i]*deg2Rad)*cos(deg2Rad*ETA_array->data[i]+etaPole); 
    rand_gal[j].z = 
      cos(LAM_array->data[i]*deg2Rad)*sin(deg2Rad*ETA_array->data[i]+etaPole);
	
    for (k=0;k<iter;k++) {
      if ((LAM_array->data[i] <= bbox[bbox_iter].iter_bound[k].lammax) &&
	  (LAM_array->data[i] >= bbox[bbox_iter].iter_bound[k].lammin)) {
	jlo = k;
	k = iter;
      }
    }

    rand_gal[j].iter = iter*bbox_iter + jlo;
    rand_gal[j].prob = gal[i].prob;
    bbox[bbox_iter].iter_bound[jlo].total_rand += rand_gal[j].prob;

    /* fprintf(RandomCatalogFile,"%e %e %u\n",LAM_array->data[i], 
       ETA_array->data[i],rand_pixnum_array->data[j]);*/
  }
  
  /*  fclose(RandomCatalogFile); */

  for (i=1;i<bbox[bbox_iter].n_gal;i++) {
    if (rand_pixnum_array->data[i] < rand_pixnum_array->data[i-1]) {
      printf("Random pixels not sorted properly.  Bailing...\n");
      exit(1);
    }
  }

  printf("Making sub-sample random catalog...\n");
  
  n=0;
  for (i=0;i<iter;i++) {
    LAM_length = bbox[bbox_iter].iter_bound[i].lammax - 
      bbox[bbox_iter].iter_bound[i].lammin;
  
    if (ETA_length < LAM_length) {
      max_seg = LAM_length;
      
      z_max = sin(deg2Rad*bbox[bbox_iter].iter_bound[i].lammax);
      z_min = sin(deg2Rad*bbox[bbox_iter].iter_bound[i].lammin);
      z_length = z_max - z_min;
    } else {
      max_seg = ETA_length;
      
      z_max = sin(deg2Rad*bbox[bbox_iter].etamax);
      z_min = sin(deg2Rad*bbox[bbox_iter].etamin);
      z_length = z_max - z_min;
    
      z_min = sin(deg2Rad*bbox[bbox_iter].iter_bound[i].lammin);
    }

    for (j=0;j<bbox[bbox_iter].iter_bound[i].n_gal;j++) {
      is_unmasked = 0;
      while (is_unmasked == 0) {
	is_unmasked = 0;
	ETA = max_seg*gsl_rng_uniform(mt19937_rand) + bbox[bbox_iter].etamin;
	z = z_length*gsl_rng_uniform(mt19937_rand) + z_min;
	LAM = asin(z)/deg2Rad;
	
	if ((LAM <= bbox[bbox_iter].iter_bound[i].lammax) && 
	    (LAM >= bbox[bbox_iter].iter_bound[i].lammin) &&
	    (ETA <= bbox[bbox_iter].etamax) && 
	    (ETA >= bbox[bbox_iter].etamin)) {
	
	  for (k=0;k<bbox[bbox_iter].n_stripe;k++) {
	    if ((ETA <= bbox[bbox_iter].stripe_bound[k].etamax) && 
		(ETA >= bbox[bbox_iter].stripe_bound[k].etamin)) {
	      stripe_iter = k;
	      k = bbox[bbox_iter].n_stripe;
	    }
	  }
	
	  if ((LAM <= bbox[bbox_iter].stripe_bound[stripe_iter].lammax) && 
	      (LAM >= bbox[bbox_iter].stripe_bound[stripe_iter].lammin)) 
	    is_unmasked = 1;
	}

	if ((is_unmasked == 1) && (n_masks > 0)) {
	  ang2pix(superpix_resolution,LAM,ETA,&pixnum);
	  
	  lhunt(mask_superpixnum_array,pixnum,&jlo);
	  
	  if (jlo <= n_superpix-1) {
	    if (pixnum == mask_superpixnum_array->data[jlo]) {
	      if (mask_struct[jlo].n_pixel == 1) {
		is_unmasked = 0;
	      } else {
		for (k=0;k<mask_struct[jlo].n_res;k++) {
		  ang2pix(mask_struct[jlo].res_struct[k].resolution,
			  LAM,ETA,&pixnum);
		  if (mask_struct[jlo].res_struct[k].n_pixel == 1) {
		    ilo = 0;
		  } else {
		    lhunt(mask_struct[jlo].res_struct[k].pixnum,pixnum,&ilo);
		  }
		  if (ilo < mask_struct[jlo].res_struct[k].n_pixel) {
		    if (mask_struct[jlo].res_struct[k].pixnum->data[ilo] ==
			pixnum) is_unmasked = 0;
		  }
		}
	      }
	    }
	  }    
	}
      }
      LAM_array->data[n] = LAM;
      ETA_array->data[n] = ETA;
      ang2pix(pixel_resolution,LAM,ETA,&tmp_pixnum_array->data[n]);
      n++;
    }
  }  

  printf("Sorting...\n");

  gsl_sort_vector_ulong_index(gal_index,tmp_pixnum_array);

  for (j=0;j<bbox[bbox_iter].n_gal;j++) {
    i = gal_index->data[j];

    sub_rand_pixnum_array->data[j] = tmp_pixnum_array->data[i];

    sub_rand_gal[j].x = -1.0*sin(deg2Rad*LAM_array->data[i]);
    sub_rand_gal[j].y = 
      cos(LAM_array->data[i]*deg2Rad)*cos(deg2Rad*ETA_array->data[i]+etaPole); 
    sub_rand_gal[j].z = 
      cos(LAM_array->data[i]*deg2Rad)*sin(deg2Rad*ETA_array->data[i]+etaPole);
	
    for (k=0;k<iter;k++) {
      if ((LAM_array->data[i] <= bbox[bbox_iter].iter_bound[k].lammax) &&
	  (LAM_array->data[i] >= bbox[bbox_iter].iter_bound[k].lammin)) {
	jlo = k;
	k = iter;
      }
    }

    sub_rand_gal[j].iter = iter*bbox_iter + jlo;

    sub_rand_gal[j].prob = gal[i].prob;
    /* bbox[bbox_iter].iter_bound[k].total_rand += rand[j].prob; */

    /*    fprintf(RandomCatalogFile,"%e %e %e %e %i\n",randLAM_array[j],
	  rand_matrix[j][1],rand_matrix[j][2],rand_matrix[j][3],
	  rand_iter_array[j]);  */
  }
  
  for (i=1;i<bbox[bbox_iter].n_gal;i++) {
    if (sub_rand_pixnum_array->data[i] < sub_rand_pixnum_array->data[i-1]) {
      printf("Sub-random pixels not sorted properly.  Bailing...\n");
      exit(1);
    }
  }

  for (i=0;i<iter;i++) 
    printf("%i, %1.2lf - %1.2lf : %i %1.2lf %1.1lf : %1.2lf %1.2lf %1.2lf\n",
	   i,bbox[bbox_iter].iter_bound[i].lammin,
	   bbox[bbox_iter].iter_bound[i].lammax,
	   bbox[bbox_iter].iter_bound[i].n_gal,
	   bbox[bbox_iter].iter_bound[i].total_gal,
	   bbox[bbox_iter].iter_bound[i].total_rand,
	   bbox[bbox_iter].iter_bound[i].n_gal/
	   bbox[bbox_iter].iter_bound[i].area,
	   bbox[bbox_iter].iter_bound[i].total_gal/
	   bbox[bbox_iter].iter_bound[i].area,
	   bbox[bbox_iter].iter_bound[i].total_rand/
	   bbox[bbox_iter].iter_bound[i].area);


  gsl_vector_free(LAM_array);
  gsl_vector_free(ETA_array);
  gsl_vector_ulong_free(tmp_pixnum_array);
  gsl_permutation_free(gal_index);
  gsl_vector_char_free(random_catalog_file);

  printf("Done.\n");

  
}

void Correlate(double dtheta, gsl_vector_char *output_tag)
{
  extern unsigned long n_gal, n_thetabins, n_rand, n_bbox;
  extern unsigned long iter, bbox_iter;
  void MakeRandomCatalog(gsl_vector_char *output_tag);
  double w_ratio, pi, sintheta, sinthetamax, costheta, sinthetamin;
  double lam_range, dd_dr, dd_ls, dd_ham, prob_pair;
  unsigned long jlo,start_gal,start_rand,start_sub_rand;
  unsigned long end_gal,end_rand,end_sub_rand;
  unsigned long start_i, start_n, stat_check, last_n_gal, last_n_rand,i,j,m,n;
  unsigned long start_obj, end_obj;
  gsl_vector_char *status_file;
  unsigned long k, no_random_catalog, pixnum, nx, ny, x, y, max_pixnum;
  unsigned long x_center,y_center, x_iter, y_iter, xmin, xmax, ymin, ymax;
  unsigned long bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax, min_pixnum;
  FILE *MethodCompare,*MethodSCompare,*StatusFile;

  status_file = gsl_vector_char_alloc(100);
  
  sprintf(status_file->data,"status%s",output_tag->data);

  nx = nx0*pixel_resolution;
  ny = ny0*pixel_resolution;
  
  sinthetamax = theta[n_thetabins-1].sinthetamax;
  sinthetamin = theta[0].sinthetamin;

  start_i = 0;
  start_n = 0;

  area_index_stripe(pixel_resolution,bbox[bbox_iter].stripe_bound[0].stripe,
		    &bbox_xmin,&bbox_xmax,&bbox_ymin,&bbox_ymax);
  /*  bbox_ymin--;
      bbox_ymax++; */

  for (i=1;i<bbox[bbox_iter].n_stripe;i++) {
    area_index_stripe(pixel_resolution,bbox[bbox_iter].stripe_bound[i].stripe,
		      &xmin,&xmax,&ymin,&ymax);
    /* ymin--;
       ymax++; */
    if (xmin < bbox_xmin) bbox_xmin = xmin;
    if (xmax > bbox_xmax) bbox_xmax = xmax;
    if (ymin < bbox_ymin) bbox_ymin = ymin;
    if (ymax > bbox_ymax) bbox_ymax = ymax;
  }

  printf("Correlating galaxies starting at %u %u %u %u...\n",start_i,
	 start_n,bbox[bbox_iter].n_gal,n_rand);

  for (n=start_n;n<n_rand;n++) {
    stat_check = 1;
   
    MakeRandomCatalog(output_tag);

    for (i=start_i;i<bbox[bbox_iter].n_gal;i++) {
      
      y_center = gal_pixnum_array->data[i]/nx;
      x_center = gal_pixnum_array->data[i] - nx*y_center;

      ymin = y_center - 1;
      ymax = y_center + 1;
      xmin = x_center - 1;
      xmax = x_center + 1;

      if (ymin < bbox_ymin) ymin = bbox_ymin;
      if (xmin < bbox_xmin) xmin = bbox_xmin;
      if (ymax > bbox_ymax) ymax = bbox_ymax;
      if (xmax > bbox_xmax) xmax = bbox_xmax;

      /* printf("%d: %d %d %d %d %d\n",i,gal_pixnum_array->data[i],
	 xmin,xmax,ymin,ymax); */
      
      for (y=ymin;y<=ymax;y++) {

	min_pixnum = nx*y + xmin;
	max_pixnum = nx*y + xmax;
	
	/* printf("%d %d\n",min_pixnum,max_pixnum); */

	if (n==0) {
	  lhunt(gal_pixnum_array,min_pixnum,&start_gal);
	  while ((start_gal > bbox[bbox_iter].n_gal-1) && 
		 (min_pixnum < max_pixnum)) {
	    min_pixnum++;
	    lhunt(gal_pixnum_array,min_pixnum,&start_gal);
	  }


	  /* printf("\tgal-gal: %d %d\n",start_gal,bbox[bbox_iter].n_gal); */
	  if (start_gal < bbox[bbox_iter].n_gal) {
	    if (gal_pixnum_array->data[start_gal] <= min_pixnum) {
	      while ((gal_pixnum_array->data[start_gal] == min_pixnum) &&
		     (start_gal >= 1)) start_gal--;
	    
	      if (start_gal < 0) start_gal = 0;
	    
	      end_gal = start_gal;

	      while ((gal_pixnum_array->data[end_gal] <= max_pixnum) &&
		     (end_gal < bbox[bbox_iter].n_gal)) { 
	      
		if (end_gal > i) {
		  prob_pair = gal[i].prob*gal[end_gal].prob;
		  costheta = (gal[i].x*gal[end_gal].x + 
			      gal[i].y*gal[end_gal].y + 
			      gal[i].z*gal[end_gal].z);
		  sintheta = 1.0 - costheta*costheta;
		  if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) {
		    hunt(sintheta_array,sintheta,&jlo);
		    if (jlo < n_thetabins) {
		      for (k=0;k<n_bbox*iter;k++) 
			if ((gal[end_gal].iter != k) && (gal[i].iter != k)) 
			  wtheta[k].gal_gal->data[jlo] += prob_pair;
		      if (gal[end_gal].iter == gal[i].iter) 
			sub_wtheta[gal[i].iter].gal_gal->data[jlo] += 
			  prob_pair;
		      single_wtheta.gal_gal->data[jlo] += prob_pair;
		    }
		  }
		}
		end_gal++;
	      }
	    }
	  } 
	}
	/* printf("\t\tgal-gal: %d %d %d\n",start_gal,end_gal,
	   bbox[bbox_iter].n_gal); */
	
	min_pixnum = nx*y + xmin;
	max_pixnum = nx*y + xmax;
	
	/* printf("%d %d\n",min_pixnum,max_pixnum); */

	lhunt(rand_pixnum_array,min_pixnum,&start_rand);
	while ((start_rand > bbox[bbox_iter].n_gal-1) && 
	       (min_pixnum < max_pixnum)) {
	  min_pixnum++;
	  lhunt(rand_pixnum_array,min_pixnum,&start_rand);
	}


	/* printf("\tgal-rand: %d %d\n",start_rand,bbox[bbox_iter].n_gal); */
	if (start_rand < bbox[bbox_iter].n_gal) {
	  if (rand_pixnum_array->data[start_rand] <= min_pixnum) {
	    while ((rand_pixnum_array->data[start_rand] == min_pixnum) && 
		   (start_rand >= 1)) start_rand--;
	  
	    if (start_rand < 0) start_rand = 0;

	    end_rand = start_rand;

	    while ((rand_pixnum_array->data[end_rand] <= max_pixnum) &&
		   (end_rand < bbox[bbox_iter].n_gal)) { 

	      prob_pair = gal[i].prob*rand_gal[end_rand].prob;
	    
	      costheta = (gal[i].x*rand_gal[end_rand].x + 
			  gal[i].y*rand_gal[end_rand].y + 
			  gal[i].z*rand_gal[end_rand].z);
	      sintheta = 1.0 - costheta*costheta;
	      if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) {
		hunt(sintheta_array,sintheta,&jlo);
		if (jlo < n_thetabins) {
		  for (k=0;k<n_bbox*iter;k++) {
		    if ((rand_gal[end_rand].iter != k) && 
			(gal[i].iter != k)) {
		      wtheta[k].gal_rand->data[jlo] += prob_pair;
		    }
		  }
		  single_wtheta.gal_rand->data[jlo] += prob_pair;
		}
	      }
	      end_rand++;
	    }
	  }
	}
	/* printf("\t\tgal-rand: %d %d %d\n",start_rand,
	   end_rand,bbox[bbox_iter].n_gal); */

	min_pixnum = nx*y + xmin;
	max_pixnum = nx*y + xmax;
	
	/* printf("%d %d\n",min_pixnum,max_pixnum); */

	lhunt(sub_rand_pixnum_array,min_pixnum,&start_sub_rand);
	while ((start_sub_rand > bbox[bbox_iter].n_gal-1) && 
	       (min_pixnum <= max_pixnum)) {
	  min_pixnum++;
	  lhunt(sub_rand_pixnum_array,min_pixnum,&start_sub_rand);
	}
	/* printf("\tgal-sub_rand: %d %d\n",start_sub_rand,
	   bbox[bbox_iter].n_gal); */

	if (start_sub_rand < bbox[bbox_iter].n_gal) {
	  if (sub_rand_pixnum_array->data[start_sub_rand] <= min_pixnum) {
	    while ((sub_rand_pixnum_array->data[start_sub_rand] == min_pixnum) 
		   && (start_sub_rand >= 1)) start_sub_rand--;
	  
	    if (start_sub_rand < 0) start_sub_rand = 0;
	    
	    end_sub_rand = start_sub_rand;

	    while ((sub_rand_pixnum_array->data[end_sub_rand] <= 
		    max_pixnum) &&
		   (end_sub_rand < bbox[bbox_iter].n_gal)) { 
	      
	      prob_pair = gal[i].prob*sub_rand_gal[end_sub_rand].prob;
	    
	      costheta = (gal[i].x*sub_rand_gal[end_sub_rand].x + 
			  gal[i].y*sub_rand_gal[end_sub_rand].y + 
			  gal[i].z*sub_rand_gal[end_sub_rand].z);
	      sintheta = 1.0 - costheta*costheta;
	      if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) {
		hunt(sintheta_array,sintheta,&jlo);
		if (jlo < n_thetabins) {
		  if (sub_rand_gal[end_sub_rand].iter == gal[i].iter) 
		    sub_wtheta[gal[i].iter].gal_rand->data[jlo] += prob_pair;
		}
	      }
	      end_sub_rand++;
	    }
	  }
	}      
	/* printf("\t\tgal-sub_rand: %d %d %d\n",start_sub_rand,
	   end_sub_rand,bbox[bbox_iter].n_gal); */
      }

      y_center = rand_pixnum_array->data[i]/nx;
      x_center = rand_pixnum_array->data[i] - nx*y_center;

      ymin = y_center - 1;
      ymax = y_center + 1;
      xmin = x_center - 1;
      xmax = x_center + 1;

      if (ymin < bbox_ymin) ymin = bbox_ymin;
      if (xmin < bbox_xmin) xmin = bbox_xmin;
      if (ymax > bbox_ymax) ymax = bbox_ymax;
      if (xmax > bbox_xmax) xmax = bbox_xmax;
      
      /* printf("rand %d: %d %d %d %d %d\n",i,rand_pixnum_array->data[i],
	 xmin,xmax,ymin,ymax); */

      for (y=ymin;y<=ymax;y++) {

	min_pixnum = nx*y + xmin;
	max_pixnum = nx*y + xmax;

	/* printf("rand %d %d\n",min_pixnum,max_pixnum);  */

	lhunt(rand_pixnum_array,min_pixnum,&start_rand);
	while ((start_rand > bbox[bbox_iter].n_gal-1) && 
	       (min_pixnum < max_pixnum)) {
	  min_pixnum++;
	  lhunt(rand_pixnum_array,min_pixnum,&start_rand);
	}
	
	/* printf("\trand-rand: %d %d\n",start_rand,
	   bbox[bbox_iter].n_gal);  */

	if (start_rand < bbox[bbox_iter].n_gal) {
	  if (rand_pixnum_array->data[start_rand] <= min_pixnum) {
	    while ((rand_pixnum_array->data[start_rand] == min_pixnum) &&
		   (start_rand >= 1)) start_rand--;
	    
	    if (start_rand < 0) start_rand = 0;
	  
	    end_rand = start_rand;

	    while ((rand_pixnum_array->data[end_rand] <= max_pixnum) &&
		   (end_rand < bbox[bbox_iter].n_gal)) { 
	      if (end_rand > i) {
	    
		prob_pair = rand_gal[i].prob*rand_gal[end_rand].prob;
	      
		costheta = (rand_gal[i].x*rand_gal[end_rand].x + 
			    rand_gal[i].y*rand_gal[end_rand].y + 
			    rand_gal[i].z*rand_gal[end_rand].z);
		sintheta = 1.0 - costheta*costheta;
		if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) {
		  hunt(sintheta_array,sintheta,&jlo);
		  if (jlo < n_thetabins) {
		    for (k=0;k<n_bbox*iter;k++) 
		      if ((rand_gal[end_rand].iter != k) && 
			  (rand_gal[i].iter != k)) 
			wtheta[k].rand_rand->data[jlo] += prob_pair;
		    single_wtheta.rand_rand->data[jlo] += prob_pair;
		  }
		}
	      }
	      end_rand++;
	    }
	  }
	}
	/* printf("\t\trand-rand: %d %d %d\n",start_rand,end_rand,
	   bbox[bbox_iter].n_gal); */
      }

      y_center = sub_rand_pixnum_array->data[i]/nx;
      x_center = sub_rand_pixnum_array->data[i] - nx*y_center;

      ymin = y_center - 1;
      ymax = y_center + 1;
      xmin = x_center - 1;
      xmax = x_center + 1;

      if (ymin < bbox_ymin) ymin = bbox_ymin;
      if (xmin < bbox_xmin) xmin = bbox_xmin;
      if (ymax > bbox_ymax) ymax = bbox_ymax;
      if (xmax > bbox_xmax) xmax = bbox_xmax;

      /* printf("sub_rand %d: %d %d %d %d %d\n",i,
	 sub_rand_pixnum_array->data[i],xmin,xmax,ymin,ymax); */

      for (y=ymin;y<=ymax;y++) {

	min_pixnum = nx*y + xmin;
	max_pixnum = nx*y + xmax;

	/* printf("sub_rand %d %d\n",min_pixnum,max_pixnum);  */

	lhunt(sub_rand_pixnum_array,min_pixnum,&start_sub_rand);
	while ((start_sub_rand > bbox[bbox_iter].n_gal-1) && 
	       (min_pixnum < max_pixnum)) {
	  min_pixnum++;
	  lhunt(sub_rand_pixnum_array,min_pixnum,&start_sub_rand);
	}
	
	/* printf("\tsub_rand-sub_rand: %d %d\n",start_sub_rand,
	   bbox[bbox_iter].n_gal); */

	if (start_sub_rand < bbox[bbox_iter].n_gal) {
	  if (sub_rand_pixnum_array->data[start_sub_rand] <= min_pixnum) {
	    while ((sub_rand_pixnum_array->data[start_sub_rand] == min_pixnum)
		   && (start_sub_rand >= 1)) start_sub_rand--;
	    
	    if (start_sub_rand < 0) start_sub_rand = 0;
	    
	    end_sub_rand = start_sub_rand;

	    while ((sub_rand_pixnum_array->data[end_sub_rand] <= 
		    max_pixnum) &&
		   (end_sub_rand < bbox[bbox_iter].n_gal)) { 
	      
	      if (end_sub_rand > i) {
		prob_pair = 
		  sub_rand_gal[i].prob*sub_rand_gal[end_sub_rand].prob;
		
		costheta = (sub_rand_gal[i].x*sub_rand_gal[end_sub_rand].x + 
			    sub_rand_gal[i].y*sub_rand_gal[end_sub_rand].y + 
			    sub_rand_gal[i].z*sub_rand_gal[end_sub_rand].z);
		sintheta = 1.0 - costheta*costheta;
		if ((sintheta < sinthetamax) && (sintheta > sinthetamin)) {
		  hunt(sintheta_array,sintheta,&jlo);
		  if (jlo < n_thetabins) {
		    if (sub_rand_gal[end_sub_rand].iter == 
			sub_rand_gal[i].iter) 
		      sub_wtheta[sub_rand_gal[i].iter].rand_rand->data[jlo] += 
			prob_pair;
		  }
		}
	      }
	      end_sub_rand++;
	    }
	  }
	}
	/* printf("\t\tsub_rand-sub_rand: %d %d %d\n",start_sub_rand,
	   end_sub_rand,bbox[bbox_iter].n_gal); */
      }
      
      if (stat_check == 100000) {
	StatusFile = fopen(status_file->data,"w");
	fprintf(StatusFile,"%i %i %d %d %d %d %d %d %d %d\n",i,n,
		bbox[bbox_iter].n_gal,n_rand,start_gal,end_gal,start_rand,
		end_rand,start_sub_rand,end_sub_rand);
	fclose(StatusFile);
	stat_check = 1;
      } else {
	stat_check++;
      }
    }
    StatusFile = fopen(status_file->data,"w");
    fprintf(StatusFile,"%i %i %u %u\n",i,n,bbox[bbox_iter].n_gal,n_rand);
    fclose(StatusFile);
    start_i = 0;
  }

  if (bbox_iter == n_bbox-1) {

    printf("Done iterating; calculating w(theta)...\n");
    
    MethodCompare = fopen("MethodCompare","w");
    MethodSCompare = fopen("MethodSCompare","w");

    for (k=0;k<n_bbox*iter;k++) {
      for (i=0;i<n_thetabins;i++) {
	wtheta[k].gal_rand->data[i] /= rand_iter;
	wtheta[k].rand_rand->data[i] /= rand_iter;
	sub_wtheta[k].gal_rand->data[i] /= rand_iter;
	sub_wtheta[k].rand_rand->data[i] /= rand_iter;

	if ((wtheta[k].gal_gal->data[i] <= 0.0001) || 
	    (wtheta[k].gal_rand->data[i] <= 0.0001) ||
	    (wtheta[k].rand_rand->data[i] <= 0.0001)) {
	  dd_dr = 0.0;
	  dd_ls = 0.0;
	  dd_ham = 0.0;
	  w_ratio = 0.0;
	  wtheta[k].wtheta->data[i] = 0.0;
	  wtheta[k].wtheta_error->data[i] = 0.0;
	} else {
	  dd_dr = 
	    2.0*wtheta[k].gal_gal->data[i]/wtheta[k].gal_rand->data[i] - 1.0;
	  dd_ls =
	    (wtheta[k].gal_gal->data[i]-wtheta[k].gal_rand->data[i]+
	     wtheta[k].rand_rand->data[i])/wtheta[k].rand_rand->data[i];
	  dd_ham = wtheta[k].gal_gal->data[i]*wtheta[k].rand_rand->data[i]/
	    (wtheta[k].gal_rand->data[i]*wtheta[k].gal_rand->data[i]/4.0)-1.0;
	  w_ratio = dd_dr/dd_ls;
	  wtheta[k].wtheta->data[i] = dd_ls;	
	  wtheta[k].wtheta_error->data[i] = 
	    1.0/sqrt(wtheta[k].gal_gal->data[i]);
	}
	fprintf(MethodCompare,
		"%1.2e %1.2e: %1.2e %1.2e %1.2e: %1.2e %1.2e %1.2e\n", 
		theta[i].thetamin, wtheta[k].wtheta->data[i],
		dd_dr, dd_ls, dd_ham, wtheta[k].gal_gal->data[i],
		wtheta[k].gal_rand->data[i]/2.0,wtheta[k].rand_rand->data[i]);
	
	if ((sub_wtheta[k].gal_gal->data[i] <= 0.00001) || 
	    (sub_wtheta[k].gal_rand->data[i] <= 0.00001) ||
	    (sub_wtheta[k].rand_rand->data[i] <= 0.00001)) {
	  dd_dr = 0.0;
	  dd_ls = 0.0;
	  dd_ham = 0.0;
	  w_ratio = 0.0;
	  sub_wtheta[k].wtheta->data[i] = 0.0;
	  sub_wtheta[k].wtheta_error->data[i] = 0.0;
	} else {
	  dd_dr = 
	    2.0*sub_wtheta[k].gal_gal->data[i]/sub_wtheta[k].gal_rand->data[i];
	  dd_dr -= 1.0;
	  dd_ls =
	    (sub_wtheta[k].gal_gal->data[i]-sub_wtheta[k].gal_rand->data[i]+
	     sub_wtheta[k].rand_rand->data[i])/
	    sub_wtheta[k].rand_rand->data[i];
	  dd_ham = 
	    sub_wtheta[k].gal_gal->data[i]*sub_wtheta[k].rand_rand->data[i]/
	    (sub_wtheta[k].gal_rand->data[i]*
	     sub_wtheta[k].gal_rand->data[i]/4.0) - 1.0;
	  w_ratio = dd_dr/dd_ls;
	  sub_wtheta[k].wtheta->data[i] = dd_ls;	
	  sub_wtheta[k].wtheta_error->data[i] = 
	    1.0/sqrt(sub_wtheta[k].gal_gal->data[i]);
	}
	fprintf(MethodSCompare,
		"%1.2e %1.2e: %1.2e %1.2e %1.2e: %1.2e %1.2e %1.2e\n", 
		theta[i].thetamin, sub_wtheta[k].wtheta->data[i],
		dd_dr, dd_ls, dd_ham, 
		sub_wtheta[k].gal_gal->data[i],
		sub_wtheta[k].gal_rand->data[i]/2.0,
		sub_wtheta[k].rand_rand->data[i]);
      }
    }
    
    fclose(MethodCompare);
    fclose(MethodSCompare);


    for (i=1;i<=n_thetabins;i++) {
      single_wtheta.gal_rand->data[i] /= rand_iter;
      single_wtheta.rand_rand->data[i] /= rand_iter;
      
      if ((single_wtheta.gal_gal->data[i] <= 0.0001) || 
	  (single_wtheta.gal_rand->data[i] <= 0.0001) ||
	  (single_wtheta.rand_rand->data[i] <= 0.0001)) {
	dd_dr = 0.0;
	dd_ls = 0.0;
	dd_ham = 0.0;
	w_ratio = 0.0;
	single_wtheta.wtheta->data[i] = 0.0;
      } else {
	dd_dr = 2.0*single_wtheta.gal_gal->data[i]/
	  single_wtheta.gal_rand->data[i] - 1.0;
	dd_ls =
	  (single_wtheta.gal_gal->data[i]-
	   single_wtheta.gal_rand->data[i]+single_wtheta.rand_rand->data[i])/
	  single_wtheta.rand_rand->data[i];
	dd_ham = single_wtheta.gal_gal->data[i]*
	  single_wtheta.rand_rand->data[i]/
	  (single_wtheta.gal_rand->data[i]*
	   single_wtheta.gal_rand->data[i]/4.0) - 1.0;
	w_ratio = dd_dr/dd_ls;
	single_wtheta.wtheta->data[i] = dd_ls;	
      }
    }
  }

  printf("Done.\n");


}

void CalculateMean()
{
  extern unsigned long n_thetabins,n_bbox;
  extern unsigned long iter;
  gsl_vector *mean_counter;
  double tot_iter;
  long i, j, k, n;

  tot_iter = 1.0*iter*n_bbox;

  mean_counter = gsl_vector_calloc(n_thetabins);

  printf("Calculating mean with %1.2lf total sq. degrees and %d sections\n",
	 survey_area,iter*n_bbox);

  for (j=0;j<n_bbox*iter;j++) {
    for (i=0;i<n_thetabins;i++) {      
      if (wtheta[j].wtheta->data[i] > 0.0 || wtheta[j].wtheta->data[i] < 0.0) {
        mean_wtheta.wtheta->data[i] += wtheta[j].wtheta->data[i];
        mean_counter->data[i] += 1.0;
      }      
    }
  }

  for (i=0;i<n_thetabins;i++) {

    if (mean_counter->data[i] < 0.5) mean_counter->data[i] = 1.0;

    mean_wtheta.wtheta->data[i] /= mean_counter->data[i];

    for (j=0;j<n_bbox*iter;j++) {
      mean_wtheta.wtheta_error->data[i] += 
	(mean_wtheta.wtheta->data[i]-wtheta[j].wtheta->data[i])*
        (mean_wtheta.wtheta->data[i]-wtheta[j].wtheta->data[i]);
      /* for (k=0;k<n_thetabins;k++)
	 covar_wtheta_matrix[i][k] +=
	 (mean_wtheta.wtheta->data[i] - wtheta[j].wtheta->data[i])*
	 (mean_wtheta_array[k] - wtheta[j].wtheta->data[i]); */
    }
    mean_wtheta.wtheta_error->data[i] = 
      mean_counter->data[i]*sqrt(mean_wtheta.wtheta_error->data[i])/
      (mean_counter->data[i]-1.0);
    /* for (k=0;k<n_thetabins;k++)
       covar_wtheta_matrix[i][k] *= 
       mean_counter->data[i]*mean_counter->data[k]/
       ((mean_counter->data[i] - 1.0)*(mean_counter->data[k] - 1.0)); */
  }

  for (i=0;i<n_thetabins;i++) {
    for (j=0;j<n_bbox*iter;j++) {
      single_wtheta.wtheta_error->data[i] += 
	(single_wtheta.wtheta->data[i]-wtheta[j].wtheta->data[i])*
	(single_wtheta.wtheta->data[i]-wtheta[j].wtheta->data[i]);

    }
    single_wtheta.wtheta_error->data[i] = 
      mean_counter->data[i]*sqrt(single_wtheta.wtheta_error->data[i])/
      (mean_counter->data[i]-1.0);
  }


  for (i=0;i<n_thetabins;i++) mean_counter->data[i] = 0.0;

  for (n=0;n<n_bbox;n++) {
    for (k=0;k<iter;k++) {
      j = n*iter + k;
      for (i=0;i<n_thetabins;i++) {      
	if (sub_wtheta[j].wtheta->data[i] > 0.0 || 
	    sub_wtheta[j].wtheta->data[i] < 0.0) {
	  mean_sub_wtheta.wtheta->data[i] += 
	    tot_iter*bbox[n].iter_bound[k].area*
	    sub_wtheta[j].wtheta->data[i]/survey_area;
	  mean_counter->data[i] += 
	    tot_iter*bbox[n].iter_bound[k].area/survey_area;
	}      
      }
    }
  }

  for (i=0;i<n_thetabins;i++) {

    if (mean_counter->data[i] < 0.5) mean_counter->data[i] = 1.0;

    mean_sub_wtheta.wtheta->data[i] /= mean_counter->data[i];

    for (n=0;n<n_bbox;n++) {
      for (k=0;k<iter;k++) {
	j = n*iter + k;
	mean_sub_wtheta.wtheta_error->data[i] += 
	  tot_iter*tot_iter*
	  bbox[n].iter_bound[k].area*bbox[n].iter_bound[k].area*
	  (mean_sub_wtheta.wtheta->data[i]-sub_wtheta[j].wtheta->data[i])*
	  (mean_sub_wtheta.wtheta->data[i]-sub_wtheta[j].wtheta->data[i])
	  /(survey_area*survey_area);
      }
    }
    mean_sub_wtheta.wtheta_error->data[i] = 
      sqrt(mean_sub_wtheta.wtheta_error->data[i])/mean_counter->data[i];
  }
  
}

