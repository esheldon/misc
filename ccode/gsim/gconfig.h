#ifndef _GCONFIG_HGUARD
#define _GCONFIG_HGUARD

#define GCONFIG_STR_SIZE  20

/*
struct wcs {
    double cd1_1;
    double cd1_2;
    double cd2_1;
    double cd2_2;

    double crval1;
    double crval2;

    double crpix1;
    double crpix2;

    char ctype1[8];
    char ctype2[8];
};
*/

struct gconfig {
    long nrow;
    long ncol;
    char noise_type[GCONFIG_STR_SIZE];
    double sky;
    long nsub;
    long seed;
};

struct gconfig *gconfig_read(const char* filename);
void gconfig_write(struct gconfig *self, FILE* fobj);
int gconfig_check(struct gconfig *self);

#endif
