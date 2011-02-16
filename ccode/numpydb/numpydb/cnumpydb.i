
%module cnumpydb
%{
#include "cnumpydb.h"
%}
//%feature("kwargs");

// must you declare with throw (const char *)?
%typemap(throws) const char * %{
    PyErr_SetString(PyExc_RuntimeError, $1);
    SWIG_fail;
%}

enum NUMPYDB_QUERY_TYPE {
    NUMPYDB_EQ=0,
    NUMPYDB_GE,
    NUMPYDB_GT,

    // everything below here requires a high value
    NUMPYDB_GE_LE,
    NUMPYDB_GE_LT,
    NUMPYDB_GT_LE,
    NUMPYDB_GT_LT,
    // all above here will require low value

    // only require high value
    NUMPYDB_LE,
    NUMPYDB_LT
};


//
// Return type codes
//

enum NUMPYDB_RETURN_TYPE {
    NUMPYDB_GETDATA,
    NUMPYDB_GETKEYS,
    NUMPYDB_GETBOTH,
    NUMPYDB_GETCOUNT
};


class NumpyDB {
    public:
        NumpyDB() throw (const char*);
        NumpyDB(PyObject* pyobj_dbfile, 
                PyObject* pyobj_db_open_flags) throw (const char*);

        // This must be defined fully or linking will fail.
        ~NumpyDB() {
            if (dbstruct.dbp != NULL) {
                dbstruct.dbp->close();
            }
        };
        void open(
                PyObject* pyobj_dbfile, 
                PyObject* pyobj_db_open_flags) throw (const char*);

        // this must be called to create a database from scratch
        void create(
                PyObject* pyobj_dbfile, 
                PyObject* pyobj_key_descr, 
                PyObject* pyobj_data_descr) throw (const char*);

        // close the database
        void close();


        // must be called before open
        void set_cachesize(int gbytes, int bytes, int ncache) throw (const char*);

        // add records to the database
        void put(
                PyObject* key_obj, 
                PyObject* data_obj) throw (const char*);

        // get records in a range of keys.  By default just returns
        // the data part.  Depending on the action variable only return the
        // keys, both data and keys, or simply the count.
        //     1: return data only
        //     2: return keys only
        //     3: return both
        //     4: return just the count
        
        PyObject* between(
                PyObject* pyobj_low, 
                PyObject* pyobj_high, 
                PyObject* pyobj_action) throw (const char*);

        PyObject* range_generic(
                PyObject* pyobj_low, 
                PyObject* pyobj_high, 
                PyObject* pyobj_range_type,
                PyObject* pyobj_return) throw (const char*);


        // get records that match the input values.  By default just returns
        // the data part.  Depending on the action variable only return the
        // keys, both data and keys, or simply the count.
        //     1: return data only
        //     2: return keys only
        //     3: return both
        //     4: return just the count
        
        PyObject* match(
                PyObject* pyobj_value, 
                PyObject* pyobj_action) throw (const char*);


        // print out the top n records in a column  key value
        void print_nrecords(PyObject* obj) throw (const char*);


        // Return python string for file name
        PyObject* file_name();

        // Return python strings for dtypes
        // note I just put the prototypes here
        PyObject* key_dtype();
        PyObject* data_dtype();


        // shall we print info?
        void set_verbosity(int verbosity);

        // print some info about the database
        void print_info();
        PyObject* test(PyObject* obj);
};


