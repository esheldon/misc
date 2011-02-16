#ifndef _numpydb_h
#define _numpydb_h

#include <Python.h>
#include <sstream>
#include <iostream>
#include <string>
#include <sys/stat.h>

// berkeley db
#include <db.h>

#include "numpy/arrayobject.h"

// to handle references and conversions we need more flexibility than we get
// with ordinary typed NumpyVector

#include "NumpyVoidVector.h"


// NPY_BYTE_ORDER is new in 1.3.0, so implement here
#ifndef NPY_BYTE_ORDER
    #include <endian.h>
    #define NPY_BYTE_ORDER __BYTE_ORDER
    #if (__BYTE_ORDER == __LITTLE_ENDIAN)
        #define NPY_LITTLE_ENDIAN
    #elif (__BYTE_ORDER == __BIG_ENDIAN)
        #define NPY_BIG_ENDIAN
    #else
        #error Unknown machine endianness detected.
    #endif
#endif

// This is my thing
#ifdef NPY_LITTLE_ENDIAN
    #define IS_LITTLE_ENDIAN 1
    #define IS_BIG_ENDIAN 0
#else
    #define IS_LITTLE_ENDIAN 0
    #define IS_BIG_ENDIAN 1
#endif

// max rows defined by this for current implementation
// of the berkeley db
#define BDB_ROW_TYPE uint32_t

//
// Query codes
//

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

// How we break out of the loop for different
// range queries
enum NUMPYDB_BREAK_TYPE {
    NUMPYDB_BREAK_NONE,
    NUMPYDB_BREAK_LELOW,
    NUMPYDB_BREAK_GTHIGH,
    NUMPYDB_BREAK_GEHIGH,
};
// Under what situations do we keep keys in range searches
enum NUMPYDB_KEEP_TYPE {
    NUMPYDB_KEEP_ANY,
    NUMPYDB_KEEP_GTLOW,
};


typedef struct {
    // hold the low/high inputs
    NumpyVoidVector low;
    NumpyVoidVector high;
    // See above NUMPYDB_ range types
    NUMPYDB_QUERY_TYPE query_type;
    // 1-4 if returning data,keys,both,count
    NUMPYDB_RETURN_TYPE return_type;
    // e.g. DB_SET_RANGE
    u_int32_t cursor_start_flags;
    // e.g. DB_NEXT
    u_int32_t cursor_step_flags;
    // if not 0, we range check against low or high and on failure we
    // break the loop
    NUMPYDB_BREAK_TYPE break_cond;
    // if not 0, we require a comparison for keeping
    NUMPYDB_KEEP_TYPE keep_cond;

    // for equals type queries, we allow multiple values to match
    // This will be equal to 1 for all other types of queries
    npy_intp nlow;
} NUMPYDB_RANGE_STRUCT;

using namespace std;


class NumpyDB {
    public:
        NumpyDB() throw (const char*);
        NumpyDB(PyObject* pyobj_dbfile, 
                PyObject* pyobj_db_open_flags) throw (const char*);

        // This must be defined fully or linking will fail.
        ~NumpyDB() {
            cleanup();
        };
        void open(
                PyObject* pyobj_dbfile, 
                PyObject* pyobj_db_open_flags) throw (const char*);

        // this must be called to create a database from scratch
        void create(
                PyObject* pyobj_dbfile, 
                PyObject* pyobj_key_descr, 
                PyObject* pyobj_data_descr) throw (const char*);

        // clean up the database memory and close
        void close();

        // add records from a numpy array
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
                PyObject* pyobj_query_type,
                PyObject* pyobj_return_type) throw (const char*);



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
        PyObject* file_name() {
            PyObject* fname = PyString_FromString(mDBFile.c_str());
            return fname;
        }
        // Return python strings for dtypes
        PyObject* key_dtype() {
            PyObject* dtype = PyString_FromString(mKeyDtypeStr.c_str());
            return dtype;
        }
        PyObject* data_dtype() {
            PyObject* dtype = PyString_FromString(mDataDtypeStr.c_str());
            return dtype;
        }

        // must be called before open
        void set_cachesize(int gbytes, int bytes, int ncache) throw (const char*);

        // shall we print info?
        void set_verbosity(int verbosity) {
            mVerbosity = verbosity;
        }

        // print some info about the database
        void print_info() {
            cout<<"Database info: \n";
            cout<<"  File name:    '"<<mDBFile<<"'\n";
            cout<<"  key type:     '"<<mKeyDtypeStr<<"'\n";
            cout<<"  data type:    '"<<mDataDtypeStr<<"'\n";
            cout<<"  db open flags: "<<mDBOpenFlags<<"\n";
        }

        PyObject* test(PyObject* obj);
    private:
        //
        // private methods
        //

        // Clean up our db connection and our references to the input data
        // types
        void cleanup();

        void set_defaults();

        // Extract the key and value data types from the metadata
        // table
        void get_meta_data() throw (const char*);

        // open the main database
        void open_main_database() throw (const char*);


        // make sure the dtype is fully specified and is simple native
        string extract_dtype(
                PyObject* pyobj_dtype, const char* name) throw (const char*);

        void verify_string_dtype(
                string& dtype, 
                const char* name) throw (const char*);

        // Extract needed info from the inputs and place into
        // a NUMPYDB_RANGE_STRUCT
        void extract_range_generic_inputs(
                PyObject* pyobj_low, 
                PyObject* pyobj_high, 
                PyObject* pyobj_query_type,
                PyObject* pyobj_return_type, 
                NUMPYDB_RANGE_STRUCT& rs) throw (const char*);

        // Determine if we should break the loop over rows
        bool range_generic_break(
                void* data, 
                void* low, 
                void* high, 
                NUMPYDB_BREAK_TYPE break_cond) throw (const char*);
        // Determine if we should keep this row.
        bool range_generic_keep(
                void* data, 
                void* low, 
                void* high, 
                NUMPYDB_KEEP_TYPE keep_cond) throw (const char*);



        // Configure the database for sorted duplicates
        // Must call this before the open command.
        void set_dupsort() throw (const char*);

        // Initialize the database structure. This database is not opened in an
        // environment, so the environment pointer is NULL.
        void initialize_db() throw (const char*);

        // Extract the filename, data type string, and open flags
        void extract_args(
                PyObject* pyobj_dbfile, 
                PyObject* pyobj_db_open_flags) throw (const char*);


        // extract a stl string from the py object
        string extract_string(
                PyObject* pyobj, 
                const char* name) throw (const char*);
        // Extract an int from a py int or py long
        long long extract_longlong(
                PyObject* pyobj, const char* name) throw (const char *);

        // comparision functions
        void set_comparators()  throw (const char*);

        template <class T>
            static int compare_fixed_dbt(DB *dbp, const DBT *a, const DBT *b);
        template <class T>
            static int compare_fixed(const void *a, const void *b);

        template <class T>
            static int compare_float_dbt(DB *dbp, const DBT *a, const DBT *b);
        template <class T>
            static int compare_float(const void *a, const void *b);

        // printers
        void set_printers()  throw (const char*);
        static void print_string(void* data) {
            cout<<(char* )data;
        }
        template <class T>
            static void print_num(void* data) {
                T* tmp;
                tmp = (T* ) data;
                cout<<(*tmp);
            }

        bool file_exists(string filename) {
            struct stat stFileInfo;

            // Attempt to get the file attributes
            int intStat = stat(filename.c_str(),&stFileInfo);
            if(intStat == 0) {
                return true;
            } else {
                return false;
            }
        }



        //
        // Data Members
        //

        // The database pointer.  We must close this in the destructor
        DB *mpDB;

        // Database file name
        string mDBFile;

        // this will always be btree
        DBTYPE mDBType;

        /*
           database opening flags.

           DB_CREATE
               If the database does not currently exist, create it. By default,
               the database open fails if the database does not already exist.  

           DB_EXCL
               Exclusive database creation. Causes the database open to fail if
               the database already exists. This flag is only meaningful when
               used with DB_CREATE.

           DB_RDONLY
               Open the database for read operations only. Causes any
               subsequent database write operations to fail.

           DB_TRUNCATE
               Physically truncate (empty) the on-disk file that contains the
               database. Causes DB to delete all databases physically contained
               in that file.

           */ 

        int mDBOpenFlags;

        string mKeyDtypeStr;
        int mKeyTypeNum;
        int mKeyItemSize;

        string mDataDtypeStr;
        int mDataTypeNum;
        int mDataItemSize;

        u_int32_t mCacheSizeGBytes;
        u_int32_t mCacheSizeBytes;
        int mNCache;

        // len in bytes of this data type
        int mDataLen;

        // What we plan to do with the data,might not need this
        int mAction;

        // our comparison function
        int (*mKeyComparator)(const void *a, const void *b);

        // our printer function
        void (*mKeyPrinter)(void* data);
        void (*mDataPrinter)(void* data);

        int mVerbosity;


};


#endif  // _numpydb_h
