/*
  Read in the info from the catalog, call Hirata's correction routine, 
  and output the results to standard out
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "export.h"
#include "CompEA4.h"

main(int argc, char *argv[])
{

  int i;
  char *input_file;

  int number;
  float mag_iso, magerr_iso, mag_aper, magerr_aper, flux_auto, fluxerr_auto;
  float mag_auto, magerr_auto, mag, magerr, kron_rasius, background;
  float x_image, y_image;
  double alpha_j2000, delta_j2000;
  float x2_image, y2_image, xy_image, cxx, cyy, cixy, a, b, theta, ellip, fwhm;
  int flags;
  float class_star, e1_sex, e2_sex, e1, e2, ixx, iyy, ixy, rho4, a4, r;
  float ellip_err, psf_fwhm, s2n;
  int whyflag;

  float ixx_psf, ixy_psf, iyy_psf, rho4_psf;

  float e1_psf, e2_psf, a4_psf;
  int   compea4_flags;

  double Tratio;
  double rsend;
  double e1_corr, e2_corr;

  /*---Read-Arguments---------------------------------------------------*/

  if (argc < 1) 
    {
      printf("-Syntax: ./jenny_CompEA4 input file > output file\n");
      exit(1);
    }

  input_file = argv[1];

  i=0;
  /*
  while(fscanf(stdin, "%i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %lf %lf %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f", &number, &mag_iso, &magerr_iso, &mag_aper, &magerr_aper, &flux_auto, &fluxerr_auto, &mag_auto, &magerr_auto, &mag, &magerr, &kron_rasius, &background, &x_image, &y_image, &alpha_j2000, &delta_j2000, &x2_image, &y2_image, &xy_image, &cxx, &cyy, &cixy, &a, &b, &theta, &ellip, &fwhm, &flags, &class_star, &e1_sex, &e2_sex, &e1, &e2, &ixx, &iyy, &ixy, &rho4, &a4, &r, &ellip_err, &psf_fwhm, &s2n, &whyflag, &ixx_psf, &ixy_psf, &iyy_psf, &rho4_psf) != EOF)
  */
  while(fscanf(stdin, "%i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %lf %lf %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f", &number, &mag_iso, &magerr_iso, &mag_aper, &magerr_aper, &flux_auto, &fluxerr_auto, &mag_auto, &magerr_auto, &mag, &magerr, &kron_rasius, &background, &x_image, &y_image, &alpha_j2000, &delta_j2000, &x2_image, &y2_image, &xy_image, &cxx, &cyy, &cixy, &a, &b, &theta, &ellip, &fwhm, &flags, &class_star, &e1_sex, &e2_sex, &e1, &e2, &ixx, &iyy, &ixy, &rho4, &a4, &r, &ellip_err, &psf_fwhm, &s2n, &whyflag) != EOF)
    {

      /* for testing */
      ixx_psf = ixx/2.0;
      iyy_psf = iyy/2.0;
      ixy_psf = ixy/2.0;
      rho4_psf = rho4/2.0;

      Tratio = (ixx + iyy)/(ixx_psf + iyy_psf);

      e1_psf = (ixx_psf - iyy_psf)/(ixx_psf + iyy_psf);
      e1_psf = 2.*ixy_psf/(ixx_psf + iyy_psf);
      a4_psf = rho4_psf/2 - 1;

      /* r will be copied over */
      compea4_flags = CompEA4(Tratio, 
			      e1_psf, e2_psf, a4_psf, 
			      e1, e2, a4, 
			      &e1_corr, &e2_corr, &rsend);
      r = 1. - rsend;

      /* two new outputs: e1_corr, e2_corr */
      fprintf(stdout, "%i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %lf %lf %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %i %f %f %f %f %f %f", number, mag_iso, magerr_iso, mag_aper, magerr_aper, flux_auto, fluxerr_auto, mag_auto, magerr_auto, mag, magerr, kron_rasius, background, x_image, y_image, alpha_j2000, delta_j2000, x2_image, y2_image, xy_image, cxx, cyy, cixy, a, b, theta, ellip, fwhm, flags, class_star, e1_sex, e2_sex, e1, e2, ixx, iyy, ixy, rho4, a4, r, ellip_err, psf_fwhm, s2n, whyflag, ixx_psf, ixy_psf, iyy_psf, rho4_psf, e1_corr, e2_corr);

    }
  
}

/* 

   NUMBER          LONG              
   MAG_ISO         FLOAT         
   MAGERR_ISO      FLOAT        
   MAG_APER        FLOAT         
   MAGERR_APER     FLOAT        
   FLUX_AUTO       FLOAT          
   FLUXERR_AUTO    FLOAT        
   MAG_AUTO        FLOAT        
   MAGERR_AUTO     FLOAT        
   MAG             FLOAT        
   MAGERR          FLOAT        
   KRON_RADIUS     FLOAT         
   BACKGROUND      FLOAT        
   X_IMAGE         FLOAT        
   Y_IMAGE         FLOAT        
   ALPHA_J2000     DOUBLE       
   DELTA_J2000     DOUBLE      
   X2_IMAGE        FLOAT       
   Y2_IMAGE        FLOAT       
   XY_IMAGE        FLOAT       
   CXX             FLOAT        
   CYY             FLOAT        
   CIXY            FLOAT       
   A               FLOAT         
   B               FLOAT       
   THETA           FLOAT       
   ELLIPTICITY     FLOAT        
   FWHM            FLOAT         
   FLAGS           LONG       
   CLASS_STAR      FLOAT      
   E1_SEX          FLOAT       
   E2_SEX          FLOAT       
   E1              FLOAT       
   E2              FLOAT       
   IXX             FLOAT        
   IYY             FLOAT      
   IXY             FLOAT       
   RHO4            FLOAT      
   A4              FLOAT      
   R               FLOAT       
   ELLIP_ERR       FLOAT        
   PSF_FWHM        FLOAT       
   S2N             FLOAT       
   WHYFLAG         LONG         
*/
