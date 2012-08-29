#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_vector_ulong.h>
#include <gsl/gsl_matrix_long.h>
#include <gsl/gsl_vector_char.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_rng.h>
#include "sdsspix.c"

typedef struct {
    double lammin, lammax, etamin, etamax;
    int stripe;
} stripe_struct;

typedef struct {
    long n_stripe;
    stripe_struct *stripe_bound;
    double lammin, lammax, etamin, etamax;
    long n_obj, n_keep;
} bbox_struct;

long n_masks, n_bbox, n_superpix, n_bbox, bbox_iter;
superpixnum_struct *mask_struct;
bbox_struct *bbox;
int superpix_resolution;

int main(int argc, char *argv[])
{
    extern long n_masks, n_bbox, n_superpix, n_bbox, bbox_iter;
    extern superpixnum_struct *mask_struct;
    extern bbox_struct *bbox;
    extern int superpix_resolution;
    extern gsl_rng *mt19937_rand;
    double LAM, ETA,dLAM,mag,LAMMIN,ETAMAX,LAMMAX,ETAMIN;
    double ra, dec, temp_lam, temp_eta, temp_r, temp_abs_r, temp_z, temp_type;
    double temp_covar_zz, temp_covar_tz, temp_covar_tt, temp_prob;
    double tmp_lammin, tmp_lammax, tmp_etamin, tmp_etamax;
    double upper_mag, lower_mag, prob,LAM_length,ETA_length;
    double z_min, z_max, z_length, z, temp_u, temp_g, temp_i;
    double temp_redshift, temp_redshifterr, temp_red, max_seg, x, y;
    double lammin, lammax, etamin, etamax;
    long run, col, fld, id, bit, i, j, k, c, not_masked, nkeep, n;
    gsl_vector_int *stripe_array, *mask_resolution_array;
    gsl_vector_ulong *mask_pixnum_array, *mask_superpixnum_array;
    long idum1, idum2, southern_stripe, n_stripe, pixnum;
    long bbox_finder, n_masks_old, jlo, ilo, stripe_iter;
    int resolution;
    gsl_vector_char *output_type;
    FILE *MaskFile;

    assign_parameters();

    superpix_resolution = 4;

    if (argc < 2) {
        fprintf(stderr,"example usage:\n");
        fprintf(stderr,"    ./filter maskfile < radec_file > output\n");
        fprintf(stderr,"    cat radec_file | ./filter maskfile > output\n");
        fprintf(stderr,"radec_file should be columns of ra dec\n");
        fprintf(stderr,"output columns are ra dec flags\n");
        fprintf(stderr,"flags is 1 if not masked 0 if masked\n");
        exit(1);
    }
    MaskFile = fopen(argv[1],"r");

    n_masks = 0;

    while ((c = getc(MaskFile)) != EOF) {
        if (c == '\n') n_masks++;
    }
    rewind(MaskFile);

    n_stripe = 0;
    bbox_finder = 1;
    n_masks_old = n_masks;
    while ((bbox_finder == 1) && (n_stripe < n_masks_old)) {
        fscanf(MaskFile,"%ld %i\n", &pixnum, &resolution);
        if (resolution < 0) {
            n_stripe++;
            n_masks--;
        } else {
            bbox_finder = 0;
        }
    }

    rewind(MaskFile);

    stripe_array = gsl_vector_int_alloc(n_stripe);

    for (i=0;i<n_stripe;i++)
        fscanf(MaskFile,"%i %i\n",&stripe_array->data[i],&resolution);

    gsl_sort_vector_int(stripe_array);

    n_bbox = 1;

    for (i=1;i<n_stripe;i++) {
        if ((stripe_array->data[i] < 50) || (stripe_array->data[i-1] < 50)) {
            if (stripe_array->data[i] > stripe_array->data[i-1]+1) n_bbox++;
        }
    }

    if (!(bbox=malloc(n_bbox*sizeof(bbox_struct)))) {
        fprintf(stderr,"Couldn't allocate bbox_struct memory...\n");
        exit(1);
    }

    fprintf(stderr,"Found %ld bounding regions...\n",n_bbox);

    for (i=0;i<n_bbox;i++) bbox[i].n_stripe = 1;

    j = 0;
    for (i=1;i<n_stripe;i++) {
        if ((stripe_array->data[i] < 50) || (stripe_array->data[i-1] < 50)) {
            if (stripe_array->data[i] == stripe_array->data[i-1]+1) {
                bbox[j].n_stripe++;
            } else {
                j++;
            }
        } else {
            bbox[j].n_stripe++;
        }
    }

    for (i=0;i<n_bbox;i++) {
        if (!(bbox[i].stripe_bound=
                    malloc(bbox[i].n_stripe*sizeof(stripe_struct)))) {
            fprintf(stderr,"Couldn't allocate stripe_struct memory...\n");
            exit(1);
        }
        bbox[i].n_obj = bbox[i].n_keep = 0;
    }

    j = k = 0;
    bbox[0].stripe_bound[0].stripe = stripe_array->data[0];
    for (i=1;i<n_stripe;i++) {
        if ((stripe_array->data[i] < 50) || (stripe_array->data[i-1] < 50)) {
            if (stripe_array->data[i] == stripe_array->data[i-1]+1) {
                k++;
                bbox[j].stripe_bound[k].stripe = stripe_array->data[i];
            } else {
                j++;
                k = 0;
                bbox[j].stripe_bound[k].stripe = stripe_array->data[i];
            }
        } else {
            k++;
            bbox[j].stripe_bound[k].stripe = stripe_array->data[i];
        }
    }

    for (i=0;i<n_bbox;i++) {
        fprintf(stderr,"BBOX %ld:\n\t",i+1);
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
            fprintf(stderr,"%i ",bbox[i].stripe_bound[j].stripe);
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
        fprintf(stderr,"\n");
    }

    fprintf(stderr,"There are %ld masks\n",n_masks);

    mask_pixnum_array = gsl_vector_ulong_alloc(n_masks);
    mask_resolution_array = gsl_vector_int_alloc(n_masks);

    for (i=0;i<n_masks;i++) 
        fscanf(MaskFile,"%lu %i\n",&mask_pixnum_array->data[i],
                &mask_resolution_array->data[i]);

    fclose(MaskFile);

    n_superpix = find_n_superpix(superpix_resolution, mask_pixnum_array, 
            mask_resolution_array, n_masks);

    if (!(mask_struct=malloc(n_superpix*sizeof(superpixnum_struct)))) {
        fprintf(stderr,"Couldn't allocate superpixnum_struct memory...\n");
        exit(1);
    }

    mask_superpixnum_array = gsl_vector_ulong_alloc(n_superpix);

    make_superpix_struct(superpix_resolution,mask_pixnum_array,
            mask_resolution_array,n_masks,mask_struct,n_superpix);

    for (i=0;i<n_superpix;i++) 
        mask_superpixnum_array->data[i] = mask_struct[i].superpixnum;

    gsl_vector_ulong_free(mask_pixnum_array);
    gsl_vector_int_free(mask_resolution_array);

    nkeep = 0;

    while (2==fscanf(stdin,"%lf %lf\n",&ra,&dec)) {

        eq2csurvey(ra, dec, &temp_lam, &temp_eta);
        not_masked = 0;
        bbox_iter = -1;
        stripe_iter = -1;
        for (j=0;j<n_bbox;j++) {
            if ((temp_lam <= bbox[j].lammax) && 
                    (temp_lam >= bbox[j].lammin) &&
                    (temp_eta <= bbox[j].etamax) && 
                    (temp_eta >= bbox[j].etamin)) {
                bbox_iter = j;
                j = n_bbox;
            }
        }

        if (bbox_iter >= 0) {
            bbox[bbox_iter].n_obj++;
            for (k=0;k<bbox[bbox_iter].n_stripe;k++) {
                if ((temp_eta <= bbox[bbox_iter].stripe_bound[k].etamax) && 
                        (temp_eta >= bbox[bbox_iter].stripe_bound[k].etamin)) {
                    stripe_iter = k;
                    k = bbox[bbox_iter].n_stripe;
                }
            }

            if (stripe_iter >= 0) {
                if ((temp_lam <= 
                            bbox[bbox_iter].stripe_bound[stripe_iter].lammax) && 
                        (temp_lam >= bbox[bbox_iter].stripe_bound[stripe_iter].lammin)) 
                    not_masked = 1;
            }
        }

        if (not_masked == 1) {
            ang2pix(superpix_resolution,temp_lam,temp_eta,&pixnum);

            lhunt(mask_superpixnum_array,pixnum,&jlo);

            if (jlo <= n_superpix-1) {
                if (pixnum == mask_superpixnum_array->data[jlo]) { 
                    for (k=0;k<mask_struct[jlo].n_res;k++) {
                        ang2pix(mask_struct[jlo].res_struct[k].resolution,
                                temp_lam,temp_eta,&pixnum);
                        if (mask_struct[jlo].res_struct[k].n_pixel == 1) {
                            ilo = 0;
                        } else {
                            lhunt(mask_struct[jlo].res_struct[k].pixnum,pixnum,&ilo);
                        }
                        if (ilo < mask_struct[jlo].res_struct[k].n_pixel) {
                            if (mask_struct[jlo].res_struct[k].pixnum->data[ilo] ==
                                    pixnum) not_masked = 0;
                        }
                    }
                }
            }
        }


        if (not_masked == 1) {
            bbox[bbox_iter].n_keep++;
            nkeep++;
        }
        printf("%.16g %.16g %d\n", ra, dec, not_masked);  
    }

    fprintf(stderr,"Kept %ld points\n",nkeep);
    return 0;

}


