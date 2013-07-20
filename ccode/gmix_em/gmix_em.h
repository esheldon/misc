#ifndef _GMIX_EM_HEADER_GUARD
#define _GMIX_EM_HEADER_GUARD

#include "image.h"
#include "gmix.h"

#ifndef M_TWO_PI
# define M_TWO_PI   6.28318530717958647693
#endif

#ifndef wlog
#define wlog(...) fprintf(stderr, __VA_ARGS__)
#endif

#define GMIX_EM_NEGATIVE_DET 0x1
#define GMIX_EM_MAXIT 0x2
#define GMIX_EM_NEGATIVE_DET_COCENTER 0x4


struct gmix_em {
    size_t maxiter;
    double tol;
    int cocenter;
    int verbose;

    // will hold these at the end
    double fdiff;
    size_t numiter;
    int flags;
};

struct gmix_em_sums {
    // scratch on a given pixel
    double gi;
    double trowsum;
    double tcolsum;
    double tu2sum;
    double tuvsum;
    double tv2sum;

    // sums over all pixels
    double pnew;
    double rowsum;
    double colsum;
    double u2sum;
    double uvsum;
    double v2sum;
};

struct gmix_em_iter {
    size_t ngauss;

    // sums over all pixels and all gaussians
    double skysum;
    double psum;

    double psky;
    double nsky;

    struct gmix_em_sums *sums;
};

void gmix_em_run(struct gmix_em* self,
                 struct image *image, 
                 struct gmix *gmix);

void gmix_em_cocenter_run(struct gmix_em* self,
                          struct image *image, 
                          struct gmix *gmix);

int gmix_get_sums(struct gmix_em* self,
                  struct image *image,
                  struct gmix *gmix,
                  struct gmix_em_iter* iter);


struct gmix_em_iter *iter_new(size_t ngauss);
struct gmix_em_iter *iter_free(struct gmix_em_iter *self);
void gmix_em_iter_clear(struct gmix_em_iter *self);

long gmix_em_gmix_set_fromiter(struct gmix *gmix, 
                               struct gmix_em_iter* iter);


#endif
