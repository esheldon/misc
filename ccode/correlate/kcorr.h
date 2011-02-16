#if !defined (_kcorr_h)
#define _kcorr_h

#include <cstdio>
#include <cmath>
#include <iostream>
#include "Array.h"
#include "types.h"
#include "constants.h"

using namespace std;

//////////////////////////////////////////////////
// Class to deal with k corrections
//////////////////////////////////////////////////

#define BAD_KCORR -9999.0
#define MIN_KCORR -2.0
typedef struct {

    int iz0,iz1;
    int igmr0,igmr1;
    int irmi0,irmi1;

    float zc, gmrc, rmic;

    int8 flags;

} kcorr_interp_struct;

class kcorr_table { 

    public: 
        kcorr_table(); // default constructor: do nothing
        kcorr_table(char *par_file); // read data from this file

        void read(char *par_file);

        float kcorr(float z, float gmr, float rmi, int band);
        float kflux(float z, float gmr, float rmi, int band);

        float kcorr_interp(kcorr_interp_struct &ks, int band);

        kcorr_interp_struct new_interp_struct();
        kcorr_interp_struct 
            kcorr_get_interp_info(float z, float gmr, float rmi);

        void kcorr_griflux(float z, float gmr, float rmi, 
                float &gk, float &rk, float &ik,
                int &flags);

        // convert k-corrected nanomaggies to solar luminosities (units of 10^10)
        float knmgy2lumsolar(float knmgy, float DLum, int band);
        // convert k-corrected nanomaggies to log10 solar luminosities
        float knmgy2loglumsolar(float knmgy, float DLum, int band);

        // Convert angular diameter distance in Mpc to distance modulus 
        float DA2DM(float DA, float z);
        // convert k-corrected nanomaggies to absolute magnitude 
        float knmgy2absmag(float knmgy, float DM);

        // convert absmag to 10^10 lum solar 
        float absmag2lumsolar(float absmag, int band);

        // Convert k-corrected nmgy to solar luminosities 
        //float knmgy2lumsolar(float knmgy, float DM, int band);


    protected:

        int nz;
        float zmin;
        float zmax;
        float zstep;

        vector<float> z;


        int ngmr;
        float gmrmin;
        float gmrmax;
        float gmrstep;

        vector<float> gmr;

        int nrmi;
        float rmimin;
        float rmimax;
        float rmistep;

        vector<float> rmi;

        int nband;
        vector<float> bands;


        Array::array4<float> kcorrTable;


        vector<float> sunabsmag;
        vector<float> sunnmgy;

}; // need the semicolon after class definitions


#endif // _kcorr_h
