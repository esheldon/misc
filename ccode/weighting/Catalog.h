#ifndef _CATALOG_H
#define _CATALOG_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <stdint.h>
#include <limits>
#include <stdexcept>

#include <stdint.h>

#include "util.h"

#include "Point.h"

// NDIM defined in params.h
#include "params.h"

using namespace std;

struct Catalog {

    ifstream infile;
    string type;
    int typenum;

    // points are u,g,r,i,z
    // or maybe r,u-g,g-r,r-i,i-z etc.
    vector<Point<NDIM> > points;

    // only in the photometric catalog
    vector<int64_t> id;

    // only in the training set
    vector<double> zspec;
    vector<double> extra;    // this is not generally used
    vector<double> weights;


    // a counter for use when finding neighbors
    vector<size_t> num;


    Catalog() {
        // default to training file
        this->init("train");
    };

    Catalog(string type) {
        // default to training file
        this->init(type);
    };

    Catalog(string fname, string type) {
        this->init(fname, type);
    }

    // just set the tupe and make all vectors zero
    void init(string type) {
        this->check_type(type);
        this->type = type;
        this->set_typenum();
        this->resize(0);
    }
    void init(string file, string type) {
        this->check_type(type);
        this->type = type;
        this->set_typenum();
        this->read(file);
    }
    void set_typenum() {
        if (this->type == "train") {
            this->typenum = 0;
        } else {
            this->typenum = 1;
        }
    }

    void resize(size_t size) {
        points.resize(size);

        if (this->type == "train") {
            id.clear();
            zspec.resize(size);
            extra.resize(size);
            weights.resize(size);
        } else {
            id.resize(size);
            zspec.clear();
            extra.clear();
            weights.clear();
        }
    }

    void check_type(string type) {
        if ( (type != "train") && (type != "photo") ) {
            throw_runtime("Catalog type must be 'train' or 'photo'");
        }
    }

    void open(string fname, string mode) {

        if (mode == "r") {
            cout<<"Opening '"<<this->type<<"' catalog file: '"<<fname<<"' to read\n";
            flush(cout);
            this->infile.exceptions( 
                    ifstream::eofbit | ifstream::failbit | ifstream::badbit );
            this->infile.open(fname.c_str());
        } else {
            throw_runtime("need to implement open for write mode");
        }
    }
    // this version can reset the type
    void read(string fname, string type) {
        this->init(fname, type);
    }

    void read(string fname) {

        cout<<"Getting lines for file: '"<<fname<<"'\n";flush(cout);
        size_t count = this->get_nlines(fname);
        cout<<"    found "<<count<<"\n";flush(cout);

        this->resize(count);
        try {
            this->open(fname,"r");
            for (size_t i=0; i<this->points.size(); i++) {
                this->read_line(i);
            }
        } catch (...) {
            throw_fileio("Error reading file:",fname); 
        }

    }


    // read next line into the specified row
    // can be useful if you are reading one row at a time!
    void read_line(size_t i) {
        if (this->typenum == 0) {
            // training set
            this->infile 
                >> this->zspec[i]
                >> this->extra[i]
                >> this->weights[i];
        } else {
            // photometric set
            this->infile >> this->id[i];
        }

        for (int j=0; j<this->points[i].ndim(); j++) {
            this->infile >> this->points[i].x[j];
        }

    }

    // run nlines and throw and exception for zero lines
    size_t get_nlines(string fname, bool verbose=false) {
        if (verbose) {
            cout<<"Getting nlines for file: "<<fname<<"\n";flush(cout);
        }
        size_t count = nlines(fname);
        if (count == 0) {
            throw_fileio("Counted zero lines in file:",fname);
        }
        if (verbose) {
            cout<<"    found: "<<count<<"\n";flush(cout);
        }
        return count;
    }

    void write(string outfile) {
        cout<<"Writing '"<<this->type<<"' catalog to file: "<<outfile<<"\n";
        ofstream file;
        file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int itype;
        if (this->type == "train") {
            itype = 0;
        } else {
            itype = 1;
        }

        try {
            file.open(outfile.c_str());
            //file.precision( numeric_limits<double>::digits10 );
            file.precision(10); // normal 6, 10 is probably good enough

            for (size_t i=0;i<this->points.size(); i++) {
                if (itype == 0) {
                    // training set
                    file
                        << this->zspec[i]   <<" "
                        << this->extra[i]   <<" "
                        << this->weights[i] <<" ";
                } else {
                    // photometric set
                    file << this->id[i]<<" ";
                }

                for (int j=0;j<NDIM;j++) {
                    file << this->points[i].x[j];
                    if (j < (NDIM-1)) {
                        file<<" ";
                    }
                }
                file<<"\n";
            }
        } catch (...) {
            throw_fileio("Error writing to file:",outfile);
        }

    }


    void write_num(string outfile) {
        cout<<"Writing num to file: "<<outfile<<"\n";
        if (this->num.size() != this->points.size()) {
            throw_runtime("num is the wrong size");
        }
        ofstream file;
        file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        try {
            file.open(outfile.c_str());
            for(int i=0; i<this->points.size(); ++i) {
                file
                    <<this->id[i]<<" "
                    <<this->num[i]<<"\n";
            }
        } catch (...) {
            throw_fileio("Error writing to file:",outfile);
        }
    }


};


#endif
