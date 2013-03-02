#ifndef _GCONFIG_HGUARD
#define _GCONFIG_HGUARD

#define GCONFIG_STR_SIZE  20

#define CTYPE_STRLEN 20

struct simple_wcs {
    double cd1_1;
    double cd1_2;
    double cd2_1;
    double cd2_2;

    double crval1;
    double crval2;

    double crpix1;
    double crpix2;

    char ctype1[CTYPE_STRLEN];
    char ctype2[CTYPE_STRLEN];
};

struct gconfig {
    long nrow;
    long ncol;
    char noise_type[GCONFIG_STR_SIZE];
    double sky;
    long nsub;
    long seed;

    int has_wcs;
    struct simple_wcs wcs;
};


struct gconfig *gconfig_read(const char* filename);
int gconfig_check(const struct gconfig *self);

void gconfig_write(const struct gconfig *self, FILE* fobj);
void gconfig_wcs_write(const struct gconfig *self, FILE* fobj);

void gconfg_write2fits(const struct gconfig *self, 
                       const char* filename);

#endif
