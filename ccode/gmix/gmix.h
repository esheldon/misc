#ifndef _GMIX_HEADER_GUARD
#define _GMIX_HEADER_GUARD

struct gauss {
    double p;
    double row;
    double col;
    double irr;
    double irc;
    double icc;
    double det;
};

struct gmix {
    size_t size;
    struct gauss* data;

    // these only make sense for same-center gaussians
    // e.g. the psf
    double total_irr;
    double total_irc;
    double total_icc;
};

enum gapprox {
    GAPPROX_EXP,
    GAPPROX_DEV
};
 
struct gmix *gmix_new(size_t n);
struct gmix *gmix_free(struct gmix *self);
void gmix_set_dets(struct gmix *self);

// make sure pointer not null and det>0 for all gauss
int gmix_verify(struct gmix *self);

// only makes sense for same center, e.g. psf
void gmix_set_total_moms(struct gmix *self);

// this is actually kind of unclear to use in practice since it is easy to
// screw up which parameters go where
void gauss_set(struct gauss* self, 
               double p, 
               double row, 
               double col,
               double irr,
               double irc,
               double icc);

int gmix_copy(struct gmix *self, struct gmix* dest);
void gmix_print(struct gmix *self, FILE* fptr);

// calculate the weighted sum of the moments
//  sum_gi( p*(irr + icc )
double gmix_wmomsum(struct gmix* gmix);

// 0 returned if a zero determinant is found somewhere, else 1
//int gmix_wmean_center(const struct gmix* gmix, struct vec2* mu_new);

//void gmix_wmean_covar(const struct gmix* gmix, struct mtx2 *cov);

/* convolution results in an nobj*npsf total gaussians */
struct gmix *gmix_convolve(struct gmix *obj_gmix, 
                           struct gmix *psf_gmix);


/* full parameters list
   [pi,rowi,coli,irri,irci,icci,...]
*/
struct gmix *gmix_from_pars(double *pars, int size);
/* coellip list
   [row,col,e1,e2,Tmax,f2,f3,...,p1,p2,p3..]
 */
struct gmix *gmix_from_coellip(double *pars, int size);

/* 
   Generate a gmix from the inputs pars assuming an appoximate
   3-gaussian representation of an exponential disk. It's only
   a good approx when convolved with a substantial psf.

   One component is nearly a delta function

   pars should be [row,col,e1,e2,T,p]

   T = sum(pi*Ti)/sum(pi)

   The p and F values are chosen to make this so
*/
struct gmix *gmix_from_pars_exp(double *pars, int size);
/* 
   Generate a gmix from the inputs pars assuming an appoximate
   3-gaussian representation of a devauc profile. It's only
   a good approx when convolved with a substantial psf.

   One component is nearly a delta function

   pars should be [row,col,e1,e2,T,p]

   T = sum(pi*Ti)/sum(pi)

   The p and F values are chosen to make this so
*/
struct gmix *gmix_from_pars_dev(double *pars, int size);

/* similar to above but for a turbulent psf */
struct gmix *gmix_from_pars_turb(double *pars, int size);

#endif
