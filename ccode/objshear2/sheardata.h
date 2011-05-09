#ifndef _SHEARDATA_H
#define _SHEARDATA_H

#include <vector>
#include "lcat.h"
#include "scat.h"
#include "rev.h"
//#include "output.h"
#include "config.h"

using std::vector;

struct sheardata {

    struct config pars;

    vector<struct lens> lcat;
    vector<struct source> scat;
    struct revhtm rev;

    //struct lensout output;

    //htmInterface *mHTM;  // The htm interface
    //const SpatialIndex *mSpatialIndex;

};

void read_sheardata(const char* config_file, struct sheardata& data);

#endif
