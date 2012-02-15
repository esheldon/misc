#ifndef _CONFIG_HEADER
#define _CONFIG_HEADER

#include "defs.h"
#include "Vector.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif

#define CONFIG_KEYSZ 50
#define CONFIG_STRSZ 256

struct config {
    double H0;
    double omega_m;
    int64 npts;  // for cosmo integration

    int64 nside; // hpix

    int64 sigmacrit_style;
    int64 nzl;      // will be zero unless sigmacrit_style==2
    struct f64vector* zl;  // only fill in for sigmacrit_style==2

    int64 nbin;
    double rmin; // mpc/h
    double rmax;

    // we fill these in
    double log_rmin;
    double log_rmax;
    double log_binsize;

};

struct config* config_read(const char* url);

#ifdef HDFS
struct config* hdfs_config_read(const char* url);
#endif 

struct config* config_delete(struct config* config);
void config_print(struct config* config);

#endif
