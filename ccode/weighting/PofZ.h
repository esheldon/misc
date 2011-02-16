#ifndef _POFZ_H
#define _POFZ_H

#include "KDTree.h"
#include "Catalog.h"
#include "util.h"
#include "whist.h"

// NDIM defined in params.h
#include "params.h"
#include <stdexcept>


struct PofZ {

    int n_near;
    int nz;
    int res; // not used
    double zmin;
    double zmax;
    Catalog wtrain;

    // variables used in calculation of p(z)

    // distance and index for n_near nearest neighbors
    vector<double> ndt;
    vector<int> nnt;
    // temporary vectors to hold the z and w from the n_near
    // nearest neighbors from the training set
    vector<double> train_z;
    vector<double> train_w;

    // vectors to hold the histogram info
    vector<double> zmin_vals;
    vector<double> zmax_vals;
    vector<double> weighted_hist;

    PofZ(string weightsfile,
         int n_near, 
         double zmin, 
         double zmax, 
         int nz): 
        wtrain(weightsfile,"train"), n_near(n_near), 
        nz(nz), zmin(zmin), zmax(zmax), 
        ndt(n_near), nnt(n_near), train_z(n_near), train_w(n_near),
        weighted_hist(nz) { 
    }

    // Calculate the p(z) for the input photometric catalog, writing
    // the results to the input files
    // this version we read the photo file one line at a time
    void calc(string photo_fname, string pzfname, string zfname) {

        Catalog photo("photo");
        size_t nphot = photo.get_nlines(photo_fname,true);
        flush(cout);

        photo.resize(1);
        photo.open(photo_fname,"r");
        cout<<"Will read line by line\n"; flush(cout);

        cout<<"Writing p(z) to file: "<<pzfname<<"\n"; flush(cout);
        ofstream pz_file(pzfname.c_str(), ios::out|ios::trunc);
        pz_file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        pz_file.precision(10); // normal 6, 10 is probably good enough

        int step=10000;

        if (nphot > step) cout<<"Each dot is "<<step<<"\n";
        try {
            KDTree<NDIM> kd_wtrain(this->wtrain.points);

            for (size_t i=0; i<nphot; i++) {
                // read a line
                photo.read_line(0);

                // get the n_near nearest neighbors from the training set
                kd_wtrain.nneigh(photo.points[0], this->ndt, this->nnt);

                // calculate the weighted histogram of the zspec
                this->do_whist();

                this->write_pofz(pz_file, 
                                 photo.id[0], 
                                 this->weighted_hist);
                if ( (nphot > step) && (i % step) == 0 ) {
                    cout<<".";flush(cout);
                }
            }
            if (nphot > step) {
                cout<<"\n"; flush(cout);
            }

        } catch (...) {
            throw_runtime("Error calculating p(z) and writing to file",
                          pzfname);
        }
        // a single column file with the centroids of the bins
        this->write_zvals(zfname);
    }



    // Calculate the p(z) for the input photometric catalog, writing
    // the results to the input files
    void calc(const Catalog& photo, string pzfname, string zfname) {

        ofstream pz_file(pzfname.c_str(), ios::out|ios::trunc);
        pz_file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        //pz_file.precision( numeric_limits<double>::digits10 );
        pz_file.precision(10); // normal 6, 10 is probably good enough
        size_t nphot = photo.points.size();

        int step=10000;

        cout<<"Writing p(z) to file: "<<pzfname<<"\n";
        if (nphot > step) cout<<"Each dot is "<<step<<"\n";
        try {
            KDTree<NDIM> kd_wtrain(this->wtrain.points);

            for (size_t i=0; i<nphot; i++) {
                // get the n_near nearest neighbors from the training set
                kd_wtrain.nneigh(photo.points[i], this->ndt, this->nnt);

                // calculate the weighted histogram of the zspec
                this->do_whist();

                this->write_pofz(pz_file, 
                                 photo.id[i], 
                                 this->weighted_hist);
                if ( (nphot > step) && (i % step) == 0 ) cout<<".";
            }
            if (nphot > step) {
                cout<<"\n";
                flush(cout);
            }

        } catch (...) {
            throw_runtime("Error calculating p(z) and writing to file",
                          pzfname);
        }
        // a single column file with the centroids of the bins
        this->write_zvals(zfname);
    }


    // call this right after you run the nneigh
    void do_whist() {
        // calculate the weighted histogram of the zspec
        for (int j=0; j<this->n_near; j++) {
            this->train_z[j] = this->wtrain.zspec[ this->nnt[j] ];
            this->train_w[j] = this->wtrain.weights[ this->nnt[j] ];
        }
        whist(this->zmin, 
              this->zmax,
              this->train_z, 
              this->train_w,
              this->zmin_vals, 
              this->zmax_vals, 
              this->weighted_hist);
    }

    // Just convenience to keep calc() shorter
    void write_pofz(
            ofstream& file, 
            int64_t id, 
            vector<double>& pofz) {

        file <<id<<" ";

        int nz = pofz.size();
        for (int zi=0; zi<nz; zi++) {
            double pz = pofz[zi];
            // Carlos had this limit
            if (pz < 1e-20) {
                pz = 0.0;
            }
            file<<pz;
            if (zi < (nz-1) ) file<<" ";
        }
        file<<"\n";
    }

    // Just convenience to keep calc() shorter
    void write_zvals(string fname) {
        cout<<"Writing z vals to file: "<<fname<<"\n";
        ofstream file;
        file.exceptions ( ifstream::failbit | ifstream::badbit );
        file.precision(10); // normal 6, 10 is probably good enough
        try {
            file.open(fname.c_str(), ios::out|ios::trunc);
            for(int j=0; j<this->zmin_vals.size(); ++j) {
                file
                    <<this->zmin_vals[j] << " "
                    <<this->zmax_vals[j] << "\n";
            }
        } catch (...) {
            throw_fileio("Error writing to file: ",fname);
        }
    }

};

#endif
