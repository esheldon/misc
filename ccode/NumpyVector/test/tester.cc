#include "tester.h"
#include "../NumpyVector.h"
#include "../NumpyVoidVector.h"
#include "../NumpyRecords.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>

using namespace std;

tester::tester() throw (const char* ) {
	import_array();
}
void tester::dotest_creation() throw (const char* ) {
    cout<<"Testing creation of new double array\n";
	NumpyVector<npy_double> dvec(10);
	cout<<"type_name: '"<<dvec.type_name()<<"'\n";
	cout<<"type_num: "<<dvec.type_num()<<"\n";
	cout<<"size: "<<dvec.size()<<"\n";

    double *d = dvec.ptr();
    d[0] = 25.0;
    d[2] = -582.21;
    dvec[5] = 3.14159;
    d[7] = 888.00;
    d[9] = 9;
    for (int i=0; i<dvec.size(); i++) {
        cout<<"d["<<i<<"] = "<<d[i]<<"\n";
        cout<<"dvec["<<i<<"] = "<<dvec[i]<<"\n";
    }

    cout<<"Test print using iterator\n";
    for (NumpyVector<npy_double>::iterator it=dvec.begin();
            it != dvec.end();
            it++) {

        cout<<"    "<<*it<<"\n";
    }
}

void tester::dotest_fromobj(PyObject* obj) throw (const char* ) {
    cout<<"Testing array from input object\n";
	NumpyVector<float> vec(obj);

    for (int i=0; i<vec.size(); i++) {
        cout<<"vec["<<i<<"] = "<<vec[i]<<"\n";
    }

    if (vec.size() > 5) {
        vec[2] = 3.14159;
    } else {
        vec[0] = 3.14159;
    }
}

double tester::test_sum(PyObject* obj) throw (const char* ) {
    // test summing using the [] notation
	NumpyVector<double> vec(obj);

    double sum=0.0;
    for (npy_intp i=0; i<vec.size(); i++) {
        sum += vec[i];
    }

    return sum;
}
double tester::test_sum_iterator(PyObject* obj) throw (const char* ) {
    // test summing using an iterator
	NumpyVector<double> vec(obj);

    double sum=0.0;
    for (NumpyVector<double>::iterator it=vec.begin(); it != vec.end(); it++ ) {
        sum += *it;
    }

    return sum;
}
double tester::test_sum_ptr(PyObject* obj) throw (const char* ) {
    // test summing using a pointer and stride
	NumpyVector<double> vec(obj);

    int stride = vec.stride();

    double sum=0.0;
    char* ptr = (char*) vec.void_ptr();
    for (npy_intp i=0; i<vec.size(); i++) {
        sum += *(double *) ptr;
        ptr += stride;
    }

    return sum;
}






PyObject* tester::dotest_output() throw (const char* ) {
    NumpyVector<float> vec(5);
    vec[3] = 25.3241;

    PyObject* output = vec.getref();
    return output;
}

void tester::testrec(PyObject* obj)  throw (const char* ) {
    NumpyRecords rec(obj);

    npy_intp size = rec.size();
    cout<<"number of rows: "<<size<<"\n";
    npy_intp rowsize = rec.rowsize();
    cout<<"size of each row is: "<<rowsize<<" bytes\n";
    npy_intp nbytes = rec.nbytes();
    cout<<"total number of bytes: "<<nbytes<<"\n";

    npy_intp nfields = rec.nfields();
    cout<<"array has "<<nfields<<" fields\n";

    fflush(stdout);

    for (npy_intp i=0; i<nfields; i++) {
        cout
            <<"    "
            <<"name: = '"<<rec.name(i)<<"'"
            <<" typecode: "<<rec.typecode(i)
            <<" offset: "<<rec.offset(rec.name(i))
            <<" size: "<<rec.elsize(i)
            <<"\n";
        fflush(stdout);

    }


    cout<<"\nTrying type safe copying\n";
    npy_int8 tint8;
    npy_int8 tuint8;
    npy_int16 tint16;
    npy_int16 tuint16;
    npy_int32 tint32;
    npy_int32 tuint32;
    npy_int64 tint64;
    npy_int64 tuint64;
    npy_float64 tfloat64;
    npy_float32 tfloat32;

    cout<<"Copying 'u1field' to various types\n";

    rec.copy("u1field", 1, tint8);
    cout<<"    u1field[1] Copied to uint8: "<<(short) tint8<<"\n";
    rec.copy("u1field", 1, tuint8);
    cout<<"    u1field[1] Copied to uint8: "<<(short)tuint8<<"\n";

    rec.copy("u1field", 1, tint16);
    cout<<"    u1field[1] Copied to int16: "<<tint16<<"\n";
    rec.copy("u1field", 1, tuint16);
    cout<<"    u1field[1] Copied to uint16: "<<tuint16<<"\n";

    rec.copy("u1field", 1, tint32);
    cout<<"    u1field[1] Copied to int32: "<<tint32<<"\n";
    rec.copy("u1field", 1, tuint32);
    cout<<"    u1field[1] Copied to uint32: "<<tuint32<<"\n";
    rec.copy("u1field", 1, tint64);
    cout<<"    u1field[1] Copied to int64: "<<tint64<<"\n";
    rec.copy("u1field", 1, tuint64);
    cout<<"    u1field[1] Copied to uint64: "<<tuint64<<"\n";
    rec.copy("u1field", 1, tfloat32);
    cout<<"    u1field[1] Copied to float32: "<<tfloat32<<"\n";
    rec.copy("u1field", 1, tfloat64);
    cout<<"    u1field[1] Copied to float64: "<<tfloat64<<"\n";

    cout<<"\nCopying elements to a string\n";
    string str;
    for (npy_intp i=0; i<rec.nfields(); i++) {
        string name=rec.name(i);
        rec.copy(name, 2, str);
        cout<<"    "<<name<<"[2] Copied to std::string: '"<<str<<"'\n";
    }

    cout<<"\nCopying each field, element 1 to float\n";
    for (npy_intp i=0; i<rec.nfields(); i++) {
        string name=rec.name(i);
        if (name != "str") {
            float tmp = rec.get<float>(i, 1);
            cout<<"  "<<rec.name(i)<<" as float: "<<tmp<<"\n";
        }
    }


    cout<<"Copying out entire i8field to an int array\n";
    vector<int> ivec;
    rec.copy("i8field", ivec);
    for (npy_intp i=0; i<rec.size(); i++) {
        tint64 = rec.get<npy_int64>("i8field", i);
        cout<<"  original: "<<tint64<<"  copy: "<<ivec[i]<<"\n";
    }

    cout<<"Copying out entire f8field to a f4 array\n";
    vector<float> fvec;
    rec.copy("f8field", fvec);
    for (npy_intp i=0; i<rec.size(); i++) {
        tfloat64 = rec.get<npy_float64>("f8field", i);
        cout<<"  original: "<<tfloat64<<"  copy: "<<fvec[i]<<"\n";
    }

    cout<<"Copying out entire f8field to a string array\n";
    vector<string> svec;
    rec.copy("f8field", svec);
    for (npy_intp i=0; i<rec.size(); i++) {
        tfloat64 = rec.get<npy_float64>("f8field", i);
        cout<<"  original: "<<tfloat64<<"  copy: '"<<svec[i]<<"'\n";
    }



    cout<<"\nType unsafe access\n";

    npy_int32 *i=NULL;
    npy_float64 *d=NULL;
    for (npy_intp row=0; row<rec.size(); row++) {
        i = (npy_int32*) rec.ptr("i4field", row);
        d = (npy_float64*) rec.ptr("f8field", row);
        cout<<"row: "<<row<<" i4field: "<<*i<<" f8field: "<<*d<<"\n";
    }

    char* ptr = rec.ptr();
    npy_intp ioffset = rec.offset("i4field");
    npy_intp doffset = rec.offset("f8field");

    cout<<"working with pointer arithmetic\n";
    for (npy_intp row=0; row<rec.size(); row++) {
        npy_int32* i = (npy_int32*) (ptr+ioffset);
        double*    d = (double*)    (ptr+doffset);

        // work with this data....
        cout<<"row: "<<row<<" i4field: "<<*i<<" f8field: "<<*d<<"\n";

        // now move to the next row
        ptr += rowsize;
    }


    cout<<"Accessing string field via ptr()\n";
    char* sptr;

    npy_intp strsize = rec.elsize("str");
    string s(strsize,' ');
    sptr = rec.ptr("str");

    for (npy_intp row=0;row<rec.size();row++) {
        for (npy_intp i=0;i<strsize;i++) {
            s[i] = sptr[i];
        }
        cout<<"str["<<row<<"]: '"<<s<<"'\n";
        sptr += rowsize;
    }


    //
    // In this section we test returning the fields as numpy arrays
    //


    cout<<"\nAccessing f8 as an f8 NumpyVoidVector\n";
    NumpyVoidVector dnpvvec;
    rec.get("f8field", dnpvvec);
    for (npy_intp i=0; i<dnpvvec.size(); i++) {
        double* d = (double*) dnpvvec.ptr(i);
        cout<<"    void vec f8["<<i<<"] = "<<*d<<"\n";
    }


    cout<<"\nAccessing f8 as NumpyVector<float> via get()\n";
    NumpyVector<float> fnpvec;
    rec.get("f8field", fnpvec);
    for (npy_intp i=0; i<fnpvec.size(); i++) {
        cout<<"    f8 as f4["<<i<<"] = "<<fnpvec[i]<<"\n";
    }



}
