#include <string>
#include "types.h"

using std::string;

struct config {
    char lens_file[255];
    char source_file[255];
    char rev_file[255];

    char output_file[255];

    float32 H0;
    float32 omega_m;

    /*
     * 1: treat scat.z as truth
     * 2: interpolate scinv(zl) for each source
     */

    int sigmacrit_style;

    // logarithmic binning
    int nbin;
    float32 rmin;
    float32 rmax;
    float32 log_rmin;
    float32 log_rmax;
    float32 log_binsize;


};

void read_config(const char* filename, struct config& pars);
void print_config(struct config& pars);
