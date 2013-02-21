#ifndef _GCONFIG_HGUARD
#define _GCONFIG_HGUARD

#define GCONFIG_STR_SIZE  20
struct gconfig {
    long nrow;
    long ncol;
    char noise_type[GCONFIG_STR_SIZE];
    char ellip_type[GCONFIG_STR_SIZE];
    double sky;
    long nsub;
    long seed;
};

struct gconfig *gconfig_read(const char* filename);
void gconfig_write(struct gconfig *self, FILE* fobj);

#endif
