#ifndef _CONFIG_HEADER
#define _CONFIG_HEADER

#include "defs.h"
#include "Vector.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif

#define CONFIG_KEYSZ 50

struct config {
    double H0;
    double omega_m;
    int64 npts;  // for cosmo integration

    int64 nside; // hpix

    int64 mask_style;

    int64 scstyle;

    // will be zero unless scstyle==SCSTYLE_INTERP
    int64 nzl;
    // only fill in for scstyle==SCSTYLE_INTERP
    struct f64vector* zl;  

    int64 nbin;
    double rmin; // mpc/h
    double rmax;

    // we fill these in
    double log_rmin;
    double log_rmax;
    double log_binsize;

    // optional min z lens to allow instead
    // of the full interpolation range
    double min_zlens_interp;
};

struct config* config_read(const char* url);

#ifdef HDFS
struct config* hdfs_config_read(const char* url);
#endif 

struct config* config_delete(struct config* config);
void config_print(struct config* config);

#endif
