#if !defined (_binner_h)
#define _binner_h

#include <cmath>
#include "types.h"

using namespace std;

class binner {

    public:
        binner(); // default constructor: do nothing
        binner(par_struct &par);

        int kgmr_bin(float kgmr);
        int logl_bin(float lumsolar);

    protected:

        float nlum;
        float loglmin;
        float loglmax;
        float lmin;
        float lmax;
        float loglstep;

        float nkgmr;
        float kgmrmin;
        float kgmrmax;
        float kgmrstep;

}; // need the semicolon after class definitions

#endif // _binner_h
