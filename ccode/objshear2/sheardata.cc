#include <stdio.h>
#include "sheardata.h"

void read_sheardata(const char* config_file, struct sheardata& data) {

    read_config(config_file, data.pars);
    print_config(data.pars);

    printf("\n");
    read_lens(data.pars.lens_file, data.lcat);
    add_lens_distances(data.pars.H0, data.pars.omega_m, data.lcat);
    print_lens_firstlast(data.lcat);

    printf("\n");
    read_source(data.pars.source_file, data.scat);
    add_source_distances(data.pars.H0, data.pars.omega_m, data.scat);
    print_source_firstlast(data.scat);

    printf("\n");
    read_rev(data.pars.rev_file, data.rev);
    print_rev_sample(data.rev);

}
