#include "cnumpydb.h"

NumpyDB::NumpyDB() throw (const char*) {
    import_array();

    // don't set this in set_defaults, we want it to persist
    mVerbosity=0;

    set_defaults();
}
NumpyDB::NumpyDB(
        PyObject* pyobj_dbfile, 
        PyObject* pyobj_db_open_flags) throw (const char*) {

    // DONT FORGET THIS!!!!
    import_array();

    // don't set this in set_defaults, we want it to persist
    mVerbosity=0;

    set_defaults();
    open(pyobj_dbfile, pyobj_db_open_flags);
}

// clean up the database memory and close
void NumpyDB::close() {
    cleanup();
}


void NumpyDB::open(
        PyObject* pyobj_dbfile, 
        PyObject* pyobj_db_open_flags) throw (const char*) {

    cleanup();
    set_defaults();
  
    // Process the arguments to a more usable form.
    extract_args(pyobj_dbfile, pyobj_db_open_flags);

    // read the metadata from the database. This actually opens
    // the "metadata" table and reads the key and data types.
    // Note the create() method must have been called prior to
    // this method
    get_meta_data();

    // Initialize the database structure.
    initialize_db();

    // Configure the database for sorted duplicates
    set_dupsort();

    if (mVerbosity > 0) {
        print_info();
    }

    // Set our comparison function (or not for strings)
    set_comparators();
    set_printers();

    // open the database as btree
    if (mVerbosity > 0) {
        cout<<"Opening database\n";
    }
    open_main_database();

    if (mpDB == NULL) {
        throw "db was not initialized!";
    }
}

// Clean up our db connection and our references to the input data types
void NumpyDB::cleanup() {
    if (mpDB != NULL) {
        mpDB->close(mpDB, 0);
        mpDB=NULL;
    }
}

void NumpyDB::set_defaults() {
    mpDB=NULL;
    mDBType = DB_BTREE;

    mKeyDtypeStr="";
    mKeyTypeNum=0;
    mKeyItemSize=0;

    mDataDtypeStr="";
    mDataTypeNum=0;
    mDataItemSize=0;

    mKeyComparator=NULL;
    mKeyPrinter=NULL;
    mDataPrinter=NULL;

    // default to 512Mb of cache
    mCacheSizeGBytes = 0;
    mCacheSizeBytes = 512*1024*1024;
    mNCache = 1;

}



// this must be called to create a database from scratch
// This method is a monstrosity
void NumpyDB::create(
        PyObject* pyobj_dbfile, 
        PyObject* pyobj_key_descr, 
        PyObject* pyobj_data_descr) throw (const char*) {


    stringstream ss;
	int ret=0;

    string fname = extract_string(pyobj_dbfile, "file_name");
    if (file_exists(fname.c_str())) {
        stringstream ss;
        ss<<"Database file already exists: '"<<fname<<"'";
        throw ss.str().c_str();
    }

    // make sure this is a good dtype, fully endianness specified
    // and meets our qualifications (currently native is required)
    string key_dtype = extract_dtype(pyobj_key_descr, "key_dtype");
    string data_dtype = extract_dtype(pyobj_data_descr, "data_dtype");

    DB* dbp=NULL;
    ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        throw("Error creating database structure");
    }


    // create the metadata database
    if (mVerbosity > 0) { 
        cout<<"Creating metadata database in file: '"<<fname<<"'\n";
    }
    ret = dbp->open(
            dbp,                // DB structure pointer
            NULL,               // Transaction pointer
            fname.c_str(),      // On-disk file that holds the database. 
            "metadata",         // Optional logical database name 
            mDBType,            // Database access method 
            DB_CREATE,          // Open flags 
            0);                 // File mode (using defaults) 

	if (ret != 0) {
        if (dbp != NULL) {
            dbp->close(dbp,0);
        }
        cleanup();
		throw("Error creating metadata database");
	}

    DBT key_dbt, data_dbt;
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));


    // Fill in the key datatype in the metadata database
    if (mVerbosity > 0) { 
        cout<<"Writing key dtype as '"<<key_dtype<<"'\n";
    }
    key_dbt.data = (void*)"key_dtype";
    key_dbt.size = strlen((char *)key_dbt.data);
    data_dbt.data = (void*)key_dtype.c_str();
    data_dbt.size = key_dtype.size();

    ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
    if (ret != 0) {
        dbp->err(dbp, ret, "Putting key dtype failed because:");
        throw "Halting";
    }

    // Fill in the data datatype in the metadata database
    if (mVerbosity > 0) { 
        cout<<"Writing data dtype as '"<<data_dtype<<"'\n";
    }
    key_dbt.data = (void*)"data_dtype";
    key_dbt.size = strlen((char *)key_dbt.data);
    data_dbt.data = (void*)data_dtype.c_str();
    data_dbt.size = data_dtype.size();

    ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
    if (ret != 0) {
        dbp->err(dbp, ret, "Putting data dtype failed because:");
        throw "Halting";
    }

    if (dbp != NULL) {
        dbp->close(dbp,0);
    }







    // create an empty main
    if (mVerbosity > 0) { 
        cout<<"Creating main database in file: '"<<fname<<"'\n";
    }

    ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        throw("Error creating database structure");
    }

    dbp->set_flags(dbp, DB_DUPSORT);

    ret = dbp->open(
            dbp,                // DB structure pointer
            NULL,               // Transaction pointer
            fname.c_str(),      // On-disk file that holds the database. 
            "data",            // Optional logical database name 
            mDBType,            // Database access method 
            DB_CREATE,          // Open flags 
            0);                 // File mode (using defaults) 

	if (ret != 0) {
        if (dbp != NULL) {
            dbp->close(dbp,0);
        }
        cleanup();
		throw("Error creating main database");
	}

    if (dbp != NULL) {
        dbp->close(dbp,0);
    }




}


string NumpyDB::extract_dtype(
        PyObject* pyobj_dtype, const char* name) throw (const char*) {
    // make sure this is a fully byte-order specified dtype

    string dtype = extract_string(pyobj_dtype,name);

    char this_order='<';
    if (IS_BIG_ENDIAN) {
        this_order = '>';
    }

    // Make sure explicit endianness shown
    if (dtype[0] != '<' 
            && dtype[0] != '>' 
            && dtype[0] != '|') {

        if (dtype[0] == 'S') {
            dtype = '|' + dtype;

        } else if (dtype[0] == '=') {
            dtype[0] = this_order;

        } else {
            dtype = this_order + dtype;
        }
    }

    // now make sure this is what we want
    verify_string_dtype(dtype, name);

    return dtype;
}


void NumpyDB::verify_string_dtype(
        string& dtype, const char* name) throw (const char*) {
    stringstream err;
    if (dtype[0] == '>' && IS_LITTLE_ENDIAN) {
        goto ENDIAN_FAIL;
    }
    if (dtype[0] == '<' && IS_BIG_ENDIAN) {
        goto ENDIAN_FAIL;
    }

    if (dtype[0] == 'V') {
        goto STRUCTURED_FAIL;
    }

    if (dtype.size() > 1){
        if (dtype[1] == 'V') {
            goto STRUCTURED_FAIL;
        }
    }
    return;

ENDIAN_FAIL:
    err<<"only support native byte order types in order to facilitate "
        <<"comparisons. "
        <<"You requested "<<name<<" dtype = '"<<dtype<<"'";
    throw err.str().c_str();
    return;

STRUCTURED_FAIL:
    err<<"only support simple types, "
        <<"requested "<<name<<" as structured dtype = '"<<dtype<<"'";
    throw err.str().c_str();
    return;

}



void NumpyDB::get_meta_data() throw (const char*) {
    // Try to get metadata.  If it doesn't exist, the user
    // must enter it.

    if (!file_exists(mDBFile.c_str())) {
        stringstream ss;
        ss<<"DB does not exist: "<<mDBFile<<". Use the create() method "
            <<"to create the database";
        throw ss.str().c_str();
    }

    DB* dbp=NULL;
    int ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        throw("Error creating database structure");
    }


    // create the metadata database
    if (mVerbosity > 0) { 
        cout<<"Opening metadata database as read only:\n";
    }
    ret = dbp->open(
            dbp,                // DB structure pointer
            NULL,               // Transaction pointer
            mDBFile.c_str(),      // On-disk file that holds the database. 
            "metadata",         // Optional logical database name 
            mDBType,            // Database access method 
            DB_RDONLY,          // Open flags 
            0);                 // File mode (using defaults) 

	if (ret != 0) {
        if (dbp != NULL) {
            dbp->close(dbp,0);
        }
        cleanup();
		throw("Error opening metadata database");
	}


    DBT key_dbt, data_dbt;
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));


    // Get the key data type
    key_dbt.data = (void*)"key_dtype";
    key_dbt.size = strlen((char *)key_dbt.data);

    ret = dbp->get(dbp, NULL, &key_dbt, &data_dbt, 0);
    if (ret == DB_NOTFOUND) {
        throw "key_dtype not in meta database";
    }
    if (ret != 0) {
        dbp->err(dbp, ret, "reading key dtype failed because:");
        throw "Halting";
    }

    mKeyDtypeStr.assign((char*)data_dbt.data, data_dbt.size);



    // Get the values data type
    key_dbt.data = (void*)"data_dtype";
    key_dbt.size = strlen((char *)key_dbt.data);

    ret = dbp->get(dbp, NULL, &key_dbt, &data_dbt, 0);
    if (ret == DB_NOTFOUND) {
        throw "data_dtype not in meta database";
    }
    if (ret != 0) {
        dbp->err(dbp, ret, "reading data dtype failed because:");
        throw "Halting";
    }

    mDataDtypeStr.assign((char*)data_dbt.data, data_dbt.size);

    if (mVerbosity > 0) { 
        cout<<"    Found key dtype: '"<<mKeyDtypeStr<<"'\n";
        cout<<"    Found data dtype: '"<<mDataDtypeStr<<"'\n";
    }


    // instantiate some small vector examples in order to get more info
    NumpyVoidVector tmp(mKeyDtypeStr.c_str(),1);
    mKeyTypeNum = tmp.type_num();
    mKeyItemSize = tmp.item_size();

    tmp.init(mDataDtypeStr.c_str(),1);
    mDataTypeNum = tmp.type_num();
    mDataItemSize = tmp.item_size();


    if (dbp != NULL) {
        dbp->close(dbp,0);
    }


}

void NumpyDB::set_cachesize(int gbytes, int bytes, int ncache) throw (const char*) {
    mCacheSizeGBytes = (u_int32_t) gbytes;
    mCacheSizeBytes = (u_int32_t) bytes;
    mNCache = ncache;
    std::cout<<"Setting cache: \n"
        <<"    gbytes: "<<mCacheSizeGBytes<<"\n"
        <<"    bytes: "<<mCacheSizeBytes<<"\n"
        <<"    ncache: "<<mNCache<<"\n";
}

void NumpyDB::open_main_database() throw (const char*) {

    int ret=0;
    // disable for now
    /*int ret = mpDB->set_cachesize(mpDB,mCacheSizeGBytes,mCacheSizeBytes,mNCache);
    if (ret != 0) {
        stringstream err;
        err<<"could net set cache:\n"
            <<"    gbytes: "<<mCacheSizeGBytes<<"\n"
            <<"    bytes: "<<mCacheSizeBytes<<"\n"
            <<"    ncache: "<<mNCache;
        throw err.str().c_str();
    }
    */
    ret = mpDB->open(
            mpDB,               // DB structure pointer
            NULL,               // Transaction pointer
            mDBFile.c_str(),    // On-disk file that holds the database. 
            "data",             // Optional logical database name 
            mDBType,            // Database access method 
            mDBOpenFlags,       // Open flags 
            0);                 // File mode (using defaults) 

	if (ret != 0) {
        cleanup();
		throw("Error opening main database");
	}

}



// This one adds key-value data.  The type of the key and
// data are checked
void NumpyDB::put(PyObject* key_obj, PyObject* data_obj) throw (const char*) {

    if (mpDB == NULL) {
        throw "You must open a database";
    }

    // get array versions of the inputs.  Might make copy
    NumpyVoidVector key_array(mKeyDtypeStr.c_str(), key_obj);
    NumpyVoidVector data_array(mDataDtypeStr.c_str(), data_obj);

	if (key_array.size() == 0 || data_array.size() == 0) {
		throw "keys and values must be non-empty";
	}
	if (key_array.size() != data_array.size()) {
		throw "keys and values must be the same length";
	}

    // add some int key, float data to the database DBT stands for data base
    // thang
    DBT key_dbt, data_dbt;

	int ret;

    // for printing
	int step = 100000;

	// Zero out the DBTs before using them.
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));

    // sizes for the items we are placing
	key_dbt.size = key_array.item_size();
	data_dbt.size = data_array.item_size();

	npy_intp nel = key_array.size();
	for (npy_intp index=0; index<nel; index++) {

        key_dbt.data = key_array.ptr(index);
        data_dbt.data = data_array.ptr(index);

		ret = mpDB->put(mpDB, NULL, &key_dbt, &data_dbt, 0);
		if (ret != 0) {
			mpDB->err(mpDB, ret, "Put failed because:");
            throw "Halting";
		}

        if (mVerbosity > 0) { 
            if ( ((index+1) % step) == 0) {
                cout<<"Added row number: "<<index+1<<"/"<<nel<<"\n";
            }
        }
	}

}


// This is our workhorse for generic range queries.  This covers all normal
// selection cases: equality (including possibly multiple matches) and one or
// two sided range queries, inclusive and not inclusive

// 2.5% slower than between()
// 5% slower than match()
PyObject* NumpyDB::range_generic(
        PyObject* pyobj_low, 
        PyObject* pyobj_high, 
        PyObject* pyobj_query_type,
        PyObject* pyobj_return_type) throw (const char*) {

    if (mpDB == NULL) {
        throw "You must open a database";
    }

    PyObject* result=NULL;

	DBC *cursorp;
    int ret=0;
	if((ret = mpDB->cursor(mpDB, NULL, &cursorp, 0)) != 0){
		mpDB->err(mpDB, ret, "DB->cursor failed.");
        throw "Failed to create cursor";
	}

    NUMPYDB_RANGE_STRUCT rs;
    extract_range_generic_inputs(
            pyobj_low, 
            pyobj_high, 
            pyobj_query_type,
            pyobj_return_type, 
            rs);


    // get pointer to the lower and upper values
    // note we don't care if they entered long arrays, only look
    // at the first element
    void* low=NULL;
    void* high=NULL;
    if (rs.low.size() > 0) {
        low = rs.low.ptr();
    }
    if (rs.high.size() > 0) {
        high = rs.high.ptr();
    }

	// DBT stands for data base thang
	DBT key_dbt, data_dbt;


	// Zero out the DBTs before using them.
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));


    // make some space to put our search and return values
    void* key = malloc( mKeyItemSize );
    memset(key, 0, mDataItemSize);
    void* data = malloc( mDataItemSize );
    memset(data, 0, mDataItemSize);


    // can set this out here
	data_dbt.data = data;
	data_dbt.size = mDataItemSize;
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = mDataItemSize;

    // note for key_dbt, setting the value happens in the main loop below
    key_dbt.size = mKeyItemSize;
    key_dbt.flags = DB_DBT_USERMEM;
    key_dbt.ulen = mKeyItemSize;


    // these might not get used.  Used char* so we can do pointer arith
    NumpyVoidVector key_vec;
    char* key_ptr=NULL;
    NumpyVoidVector data_vec;
    char* data_ptr=NULL;


    // set up sizes and the bit saying we are managing this memory

    // Do a first pass counting matches, maybe only return that
    npy_intp count = 0;
    int npass=2;
    for (int pass=0; pass<npass; pass++) {

        // Note we are using the memory of "key" for memory in
        // key_dbt


        // If this is an equals query, we can loop over multiple
        // "low" values to match.  for all other query types nlow=1
        for (npy_intp ilow=0; ilow<rs.nlow; ilow++) {


            // set to low if low was sent.  We won't always use it, for example
            // if doing a "<" search, but this simplifies the code
            if (rs.low.size() > 0) {
                low=rs.low.ptr(ilow);
                memcpy(key, low, mKeyItemSize);
            }
            key_dbt.data = key;

            // note use of DB_RANGE and DB_NEXT since we are doing ranges
            for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, 
                        rs.cursor_start_flags);
                    ret == 0;
                    ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, 
                        rs.cursor_step_flags)) {

                if (range_generic_break(key_dbt.data, 
                            low, 
                            high, 
                            rs.break_cond)) {
                    break;
                }

                if (range_generic_keep(key_dbt.data,
                            low,
                            high,
                            rs.keep_cond)) {

                    // we are keeping this row
                    if (pass == 0) {
                        count++;
                    } else {
                        // copy data
                        if (rs.return_type == NUMPYDB_GETDATA 
                                || rs.return_type==NUMPYDB_GETBOTH) {
                            memcpy(data_ptr, data, mDataItemSize);
                            // can do this because we know it's contiguous
                            data_ptr += mDataItemSize;
                        }
                        if (rs.return_type == NUMPYDB_GETKEYS 
                                || rs.return_type==NUMPYDB_GETBOTH) {
                            memcpy(key_ptr, key, mKeyItemSize);
                            // can do this because we know it's contiguous
                            key_ptr += mKeyItemSize;
                        }
                    }
                }
            } // loop over records

        } // loop over low values (1 for all but "equals" queries)

        // If we were only counting, return the count
        if (rs.return_type == NUMPYDB_GETCOUNT) {
            // don't need a second pass
            break;
        }

        // If we get here, then we want to create output, even if it is
        // empty
        if (pass == 0) {
            if (rs.return_type == NUMPYDB_GETDATA 
                    || rs.return_type == NUMPYDB_GETBOTH) {
                data_vec.init(mDataDtypeStr.c_str(), count);
                data_ptr = (char* ) data_vec.ptr();
            } 
            if (rs.return_type == NUMPYDB_GETKEYS 
                    || rs.return_type == NUMPYDB_GETBOTH) {
                key_vec.init(mKeyDtypeStr.c_str(), count);
                key_ptr = (char* ) key_vec.ptr();
            }
        }

        if (count == 0) {
            // we will return empty
            break;
        }


    } // loop over two passes

    // Cursors must be closed
    if (cursorp != NULL)  {
        cursorp->c_close(cursorp); 
    }

    // free our temporary work space
    free(key);
    free(data);

    // 
    // Return the results
    //
    
    if (rs.return_type == NUMPYDB_GETCOUNT) {
        // we will just return the count
        result = PyLong_FromLongLong( count );
    } else {

        // If we found no results, return None
        if (rs.return_type == NUMPYDB_GETDATA) {
            result = data_vec.getref();
        } else if (rs.return_type == NUMPYDB_GETKEYS) {
            result = key_vec.getref();
        } else if (rs.return_type == NUMPYDB_GETBOTH) {

            result= PyTuple_New(2);
            PyTuple_SetItem(result, 0, key_vec.getref());
            PyTuple_SetItem(result, 1, data_vec.getref());

        }
    } // not just returning the count


    return result;
}


void NumpyDB::extract_range_generic_inputs(
        PyObject* pyobj_low, 
        PyObject* pyobj_high, 
        PyObject* pyobj_query_type,
        PyObject* pyobj_return_type, 
        NUMPYDB_RANGE_STRUCT& rs) throw (const char*) {

    stringstream err;

    // Get the type of query we are running
    rs.query_type = 
        (NUMPYDB_QUERY_TYPE) extract_longlong(pyobj_query_type, "query_type");
    if (rs.query_type < NUMPYDB_EQ || rs.query_type > NUMPYDB_LT) {
        err<<"query_type must be within ["<<NUMPYDB_EQ<<","<<NUMPYDB_LT<<"]";
        throw err.str().c_str();
    }
    if (mKeyTypeNum == NPY_STRING) {
        if (rs.query_type != NUMPYDB_EQ) {
            throw "string range selection is not implemented.";
        }
    }

    // will we return data,keys,both or just a count?
    rs.return_type = 
        (NUMPYDB_RETURN_TYPE) extract_longlong(pyobj_return_type, 
                                               "return_type");
    if (rs.return_type < NUMPYDB_GETDATA || rs.return_type > NUMPYDB_GETCOUNT) {
        err<<"return_type must be within ["
            <<NUMPYDB_GETDATA<<","<<NUMPYDB_GETCOUNT<<"]";
        throw err.str().c_str();
    }

    // Use numpy's awesome converters to convert the entered values into our
    // key type

    // do we need the low value?
    if (rs.query_type <= NUMPYDB_GT_LT) {
        rs.low.init(mKeyDtypeStr.c_str(), pyobj_low);
        if (rs.low.size() == 0) {
            throw "Lower bound must not be empty";
        }
    }
    // do we need the high value
    if (rs.query_type >= NUMPYDB_GE_LE) {
        rs.high.init(mKeyDtypeStr.c_str(), pyobj_high);
        if (rs.high.size() == 0) {
            throw "Upper bound must not be empty";
        }
    }

    // for equals type queries, we allow multiple values to match and these are
    // drawn from the "low" variable otherwise just 1 low value
    rs.nlow=1;

    // set up some simple variables that will guide our query execution
    switch (rs.query_type) {
        case NUMPYDB_EQ:
            // simplest: only look at keys that exactly match
            rs.cursor_start_flags = DB_SET;
            rs.cursor_step_flags = DB_NEXT_DUP;
            rs.break_cond = NUMPYDB_BREAK_NONE;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            // For "equals" queries, we can match multiple values
            rs.nlow=rs.low.size();
            break;

        case NUMPYDB_GE:
            // next easiest: Start range search, which goes to smallest
            // key <= our low value and read until we are out of rows
            rs.cursor_start_flags = DB_SET_RANGE;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_NONE;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;
        case NUMPYDB_GT:
            // Start at the end and work our way backward.  We keep everything
            // and break when (key-low) <= 0.
            rs.cursor_start_flags = DB_LAST;
            rs.cursor_step_flags = DB_PREV;
            rs.break_cond = NUMPYDB_BREAK_LELOW;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;


        case NUMPYDB_LE:
            // start at first and scan keeping all until key > high
            rs.cursor_start_flags = DB_FIRST;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GTHIGH;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;
        case NUMPYDB_LT:
            // start at first and scan keeping all until key >= high
            rs.cursor_start_flags = DB_FIRST;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GEHIGH;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;



        case NUMPYDB_GE_LE:
            // Just seach for low and move upward, keeping all until 
            // key strictly > high
            rs.cursor_start_flags = DB_SET_RANGE;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GTHIGH;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;
        case NUMPYDB_GE_LT:
            // Just seach for low and move upward, keeping all until 
            // key >= high
            rs.cursor_start_flags = DB_SET_RANGE;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GEHIGH;
            rs.keep_cond = NUMPYDB_KEEP_ANY;
            break;

        case NUMPYDB_GT_LE:
            // Seach for low and move upward until key > high, but
            // only keep those with key > low
            rs.cursor_start_flags = DB_SET_RANGE;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GTHIGH;
            rs.keep_cond = NUMPYDB_KEEP_GTLOW;
            break;
        case NUMPYDB_GT_LT:
            // Seach for low and move upward until key >= high, but
            // only keep those with key > low
            rs.cursor_start_flags = DB_SET_RANGE;
            rs.cursor_step_flags = DB_NEXT;
            rs.break_cond = NUMPYDB_BREAK_GEHIGH;
            rs.keep_cond = NUMPYDB_KEEP_GTLOW;
            break;





    };

}

// Determine if we should break the loop over rows
bool NumpyDB::range_generic_break(
        void* data, 
        void* low, 
        void* high, 
        NUMPYDB_BREAK_TYPE break_cond) throw (const char*) {

    bool dobreak=false;
    switch (break_cond) {
        case NUMPYDB_BREAK_NONE:
            break;
        case NUMPYDB_BREAK_LELOW:
            if (mKeyComparator(data, low) <= 0) {
                dobreak=true;
            }
            break;
        case NUMPYDB_BREAK_GTHIGH:
            if (mKeyComparator(data, high) > 0) {
                dobreak=true;
            }
            break;
        case NUMPYDB_BREAK_GEHIGH:
            if (mKeyComparator(data, high) >= 0) {
                dobreak=true;
            }
            break;
    }
    return dobreak;
}

// Determine if we should keep this row.  Turns out for the most part we are in
// keep everything mode in almost all cases, and it is intead looking for a
// break condition (see range_generic_break) that kicks us out of the loop

bool NumpyDB::range_generic_keep(
        void* data, 
        void* low, 
        void* high, 
        NUMPYDB_KEEP_TYPE keep_cond) throw (const char*) {

    bool dokeep=false;
    switch (keep_cond) {
        case NUMPYDB_KEEP_ANY:
            dokeep=true;
            break;
        case NUMPYDB_KEEP_GTLOW:
            if (mKeyComparator(data, low) > 0) {
                dokeep=true;
            }
            break;
    }
    return dokeep;
}



PyObject* NumpyDB::between(
        PyObject* pyobj_low, 
        PyObject* pyobj_high, 
        PyObject* pyobj_return_type) throw (const char*) {

    stringstream err;
    if (mpDB == NULL) {
        throw "You must open a database";
    }

    if (mKeyTypeNum == NPY_STRING) {
        throw "string range selection is not implemented.";
    }

	DBC *cursorp;
    int ret=0;
	if((ret = mpDB->cursor(mpDB, NULL, &cursorp, 0)) != 0){
		mpDB->err(mpDB, ret, "DB->cursor failed.");
        throw "Failed to create cursor";
	}


    // 1: return data only
    // 2: return keys only
    // 3: return both
    // 4: return just the count
    int return_type = extract_longlong(pyobj_return_type, "return_type");
    if (return_type < NUMPYDB_GETDATA || return_type > NUMPYDB_GETCOUNT) {
        err<<"return_type must be within ["
            <<NUMPYDB_GETDATA<<","<<NUMPYDB_GETCOUNT<<"]";
        throw err.str().c_str();
    }

    // Use numpy's awesome converters to convert the entered values into our
    // key type
    NumpyVoidVector low_vec(mKeyDtypeStr.c_str(), pyobj_low);
    NumpyVoidVector high_vec(mKeyDtypeStr.c_str(), pyobj_high);

    if ( (low_vec.size() == 0) || (high_vec.size()== 0) ) {
        throw "Lower and upper bounds must not be empty";
    }

    // get pointer to the lower and upper values
    // note we don't care if they entered long arrays, only look
    // at the first element
    void* low = low_vec.ptr();
    void* high = high_vec.ptr();


	// DBT stands for data base thang
	DBT key_dbt, data_dbt;



	// Zero out the DBTs before using them.
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));


    // make some space to put our search and return values
    void* key = malloc( mKeyItemSize );
    memset(key, 0, mDataItemSize);
    void* data = malloc( mDataItemSize );
    memset(data, 0, mDataItemSize);



    // can set this out here
	data_dbt.data = data;
	data_dbt.size = mDataItemSize;
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = mDataItemSize;

    PyObject* result=NULL;

    // these might not get used.  Used char* so we
    // can do pointer arith
    NumpyVoidVector key_vec;
    char* key_ptr=NULL;
    NumpyVoidVector data_vec;
    char* data_ptr=NULL;


    // Do a first pass counting matches, maybe only return that
    npy_intp count = 0;
    int npass=2;
    for (int pass=0; pass<npass; pass++) {

        // Note we are using the memory of "key" for memory in
        // key_dbt
        memcpy(key, low, mKeyItemSize);
        key_dbt.data = key;
        key_dbt.size = mKeyItemSize;
        key_dbt.flags = DB_DBT_USERMEM;
        key_dbt.ulen = mKeyItemSize;

        // note use of DB_RANGE and DB_NEXT since we are doing ranges
        for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_SET_RANGE);
                ret == 0;
                ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT)) {

            // See if past high end of range.  Note strict >
            if (mKeyComparator(key_dbt.data, high) > 0) {
                break;
            }
            if (pass == 0) {
                count++;
            } else {
                // copy data
                if (return_type == NUMPYDB_GETDATA 
                        || return_type==NUMPYDB_GETBOTH) {
                    memcpy(data_ptr, data, mDataItemSize);
                    // can do this because we know it's contiguous
                    data_ptr += mDataItemSize;
                }
                if (return_type == NUMPYDB_GETKEYS 
                        || return_type==NUMPYDB_GETBOTH) {
                    memcpy(key_ptr, key, mKeyItemSize);
                    // can do this because we know it's contiguous
                    key_ptr += mKeyItemSize;
                }
            } // return_types if
        } // loop over records


        // If we were only counting, return the count
        if (return_type == NUMPYDB_GETCOUNT) {
            // don't need a second pass
            break;
        }

        // If we get here, then we want to create output, even if it is
        // empty
        if (pass == 0) {
            if (return_type == NUMPYDB_GETDATA 
                    || return_type == NUMPYDB_GETBOTH) {
                data_vec.init(mDataDtypeStr.c_str(), count);
                data_ptr = (char* ) data_vec.ptr();
            } 
            if (return_type == NUMPYDB_GETKEYS 
                    || return_type == NUMPYDB_GETBOTH) {
                key_vec.init(mKeyDtypeStr.c_str(), count);
                key_ptr = (char* ) key_vec.ptr();
            }
        }

        if (count == 0) {
            // we will return empty
            break;
        }


    } // loop over two passes

    // Cursors must be closed
    if (cursorp != NULL)  {
        cursorp->c_close(cursorp); 
    }

    // free our temporary work space
    free(key);
    free(data);

    // 
    // Return the results
    //
    
    if (return_type == NUMPYDB_GETCOUNT) {
        // we will just return the count
        result = PyLong_FromLongLong( count );
    } else {

        // If we found no results, return None
        if (return_type == NUMPYDB_GETDATA) {
            result = data_vec.getref();
        } else if (return_type == NUMPYDB_GETKEYS) {
            result = key_vec.getref();
        } else if (return_type == NUMPYDB_GETBOTH) {

            result= PyTuple_New(2);
            PyTuple_SetItem(result, 0, key_vec.getref());
            PyTuple_SetItem(result, 1, data_vec.getref());

        }
    } // not just returning the count


    return result;
}

PyObject* NumpyDB::match(
        PyObject* pyobj_values, 
        PyObject* pyobj_return_type) throw (const char*) {

    stringstream err;
    if (mpDB == NULL) {
        throw "You must open a database";
    }

	DBC *cursorp;
    int ret=0;
	if((ret = mpDB->cursor(mpDB, NULL, &cursorp, 0)) != 0){
		mpDB->err(mpDB, ret, "DB->cursor failed.");
        throw "Failed to create cursor";
	}


    int return_type = extract_longlong(pyobj_return_type, "return_type");
    if (return_type < NUMPYDB_GETDATA || return_type > NUMPYDB_GETCOUNT) {
        err<<"return_type must be within ["
            <<NUMPYDB_GETDATA<<","<<NUMPYDB_GETCOUNT<<"]";
        throw err.str().c_str();
    }



    // Use numpy's awesome converters to convert the entered value into our key
    // type

    NumpyVoidVector values_vec(mKeyDtypeStr.c_str(), pyobj_values);
    if (values_vec.size() == 0) {
        throw "value array must be non-empty";
    }


	// DBT stands for data base thang
	DBT key_dbt, data_dbt;


	// Zero out the DBTs before using them.
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));


    // make some space to put our search and return values
    void* key = malloc( mKeyItemSize );
    memset(key, 0, mDataItemSize);
    void* data = malloc( mDataItemSize );
    memset(data, 0, mDataItemSize);



    // .data will be set in the loop below
	key_dbt.size = mKeyItemSize;
	key_dbt.flags = DB_DBT_USERMEM;
	key_dbt.ulen = mKeyItemSize;


    // can set this out here
	data_dbt.data = data;
	data_dbt.size = mDataItemSize;
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = mDataItemSize;

    PyObject* result=NULL;

    // these might not get used, depends on the value of return_type
    NumpyVoidVector key_vec;
    char* key_ptr=NULL;
    NumpyVoidVector data_vec;
    char* data_ptr=NULL;

    // Do a first pass counting matches, maybe only return that
    npy_intp count = 0;
    int npass=2;
    for (int pass=0; pass<npass; pass++) {

        for (npy_intp ivalue=0; ivalue<values_vec.size(); ivalue++) {

            // get pointer to the value of interest
            void* value = values_vec.ptr(ivalue);

            // Note we are using the memory of "key" for memory in
            // key_dbt
            memcpy(key, value, mKeyItemSize);
            key_dbt.data = key;

            // note use of DB_SET and DB_NEXT_DUP since we are matching
            for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_SET);
                 ret == 0;
                 ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT_DUP)) {

                if (pass == 0) {
                    count++;
                } else {
                    // copy data
                    if (return_type == NUMPYDB_GETDATA 
                            || return_type==NUMPYDB_GETBOTH) {
                        memcpy(data_ptr, data, mDataItemSize);
                        // can do this because we know it's contiguous
                        data_ptr += mDataItemSize;
                    }
                    if (return_type == NUMPYDB_GETKEYS 
                            || return_type==NUMPYDB_GETBOTH) {
                        memcpy(key_ptr, key, mKeyItemSize);
                        // can do this because we know it's contiguous
                        key_ptr += mKeyItemSize;
                    }
                } // return_types if
            } // loop over records

        } // loop over values to match

        // If we were only counting, return the count
        if (return_type == NUMPYDB_GETCOUNT) {
            // don't need a second pass
            break;
        }

        // If we get here, then we want to create output, even if it is
        // empty
        if (pass == 0) {
            if (return_type == NUMPYDB_GETDATA 
                    || return_type == NUMPYDB_GETBOTH) {
                data_vec.init(mDataDtypeStr.c_str(), count);
                data_ptr = (char* ) data_vec.ptr();
            } 
            if (return_type == NUMPYDB_GETKEYS 
                    || return_type == NUMPYDB_GETBOTH) {
                key_vec.init(mKeyDtypeStr.c_str(), count);
                key_ptr = (char* ) key_vec.ptr();
            }
        }

        if (count == 0) {
            // we will return empty
            break;
        }

    } // loop over two passes

    // Cursors must be closed
    if (cursorp != NULL)  {
        cursorp->c_close(cursorp); 
    }

    // free our temporary work space
    free(key);
    free(data);

    // 
    // Return the results
    //
    
    if (return_type == NUMPYDB_GETCOUNT) {
        // we will just return the count
        result = PyLong_FromLongLong( count );
    } else {

        // If we found no results, return None
        if (return_type == NUMPYDB_GETDATA) {
            result = data_vec.getref();
        } else if (return_type == NUMPYDB_GETKEYS) {
            result = key_vec.getref();
        } else if (return_type == NUMPYDB_GETBOTH) {

            result= PyTuple_New(2);
            PyTuple_SetItem(result, 0, key_vec.getref());
            PyTuple_SetItem(result, 1, data_vec.getref());

        }
    } // not just returning the count

    return result;
}



// decreffing seems to be an issue
PyObject* NumpyDB::test(PyObject* obj) {
    // try making zero size
    // get around stupid bug in gcc 4.1

    NumpyVoidVector tmp("i8",(npy_intp)0);

    return tmp.getref();
}


// print out the first n recorsd
void NumpyDB::print_nrecords(PyObject* obj) throw (const char*) {

    if (mpDB == NULL) {
        throw "You must open a database";
    }

    int nrecords = extract_longlong(obj, "nrecords");

	DBC *cursorp;
	// DBT stands for data base thang
	DBT key_dbt, data_dbt;


    int ret=0;
	if((ret = mpDB->cursor(mpDB, NULL, &cursorp, 0)) != 0){
		mpDB->err(mpDB, ret, "DB->cursor failed.");
        throw "Failed to create cursor";
	}

	// Zero out the DBTs before using them.
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));

    int i=0;
	while ( (i < nrecords) 
            && ( (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT)) == 0)) {

        mKeyPrinter(key_dbt.data);
        cout<<" ";
        mDataPrinter(data_dbt.data);
        cout<<"\n";
        i+=1;
	}


    // Cursors must be closed
    if (cursorp != NULL)  {
        cursorp->c_close(cursorp); 
    }

}


// Extract the filename, data type string, and open flags
void NumpyDB::extract_args(
        PyObject* pyobj_dbfile, 
        PyObject* pyobj_db_open_flags) throw (const char*) {

    mDBFile = extract_string(pyobj_dbfile, "file_name");
    mDBOpenFlags = extract_longlong(pyobj_db_open_flags,"db_open_flags");


}


string NumpyDB::extract_string(PyObject* pyobj, const char* name) throw (const char*) {

    if (!PyString_Check(pyobj)) {
        stringstream err;
        err<<name<<" must be string";
        throw err.str().c_str();
    }
    string s = PyString_AsString(pyobj);
    return s;
}

long long NumpyDB::extract_longlong(
        PyObject* pyobj, const char* name) throw (const char*) {
    long long val=0;
    if (PyInt_Check(pyobj)) {
        val = (long long) PyInt_AsLong(pyobj);
    } else if (PyLong_Check(pyobj)) {
        val = (long long) PyLong_AsLongLong(pyobj);
    } else {
        stringstream err;
        err<<name<<" must be an int or long";
        throw err.str().c_str();
    }
    return val;
}

// Configure the database for sorted duplicates
// Must call this before the open command.
void NumpyDB::set_dupsort() throw (const char*) {

    if (mpDB == NULL) {
        throw "You must open a database";
    }

    int ret = mpDB->set_flags(mpDB, DB_DUPSORT);
	if (ret != 0) {
		mpDB->err(mpDB, ret, "Attempt to set DUPSORT flag failed.");
        cleanup();
        throw("Halting");
	}
}


void NumpyDB::initialize_db()  throw (const char*) {
    // Initialize the database structure. This database is not opened in an
    // environment, so the environment pointer is NULL.

    int ret = db_create(&mpDB, NULL, 0);
    if (ret != 0) {
        throw("Error creating database structure");
    }
}



void NumpyDB::set_comparators()  throw (const char*) {

    if (mpDB == NULL) {
        throw "You must open a database";
    }

    switch (mKeyTypeNum) {
        case NPY_INT8:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int8>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_int8>;
            break;
        case NPY_UINT8:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint8>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_uint8>;
            break;

        case NPY_INT16:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int16>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_int16>;
            break;
        case NPY_UINT16:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint16>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_uint16>;
            break;

        case NPY_INT32:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int32>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_int32>;
            break;
        case NPY_UINT32:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint32>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_uint32>;
            break;

        case NPY_INT64:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int64>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_int64>;
            break;
        case NPY_UINT64:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint64>);
            mKeyComparator= &NumpyDB::compare_fixed<npy_uint64>;
            break;

        case NPY_FLOAT32:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_float_dbt<npy_float32>);
            mKeyComparator= &NumpyDB::compare_float<npy_float32>;
            break;
        case NPY_FLOAT64:
            mpDB->set_bt_compare(
                    mpDB, 
                    &NumpyDB::compare_float_dbt<npy_float64>);
            mKeyComparator= &NumpyDB::compare_float<npy_float64>;
            break;

        case NPY_STRING:
            // We use the default lexical comparison
            break;
        default: 
            stringstream err;
            err<<"Unsupported numpy type num: "<<mKeyTypeNum;
            throw err.str().c_str();
    }


    // data comparison functions.
    switch (mDataTypeNum) {
        case NPY_INT8:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int8>);
            break;
        case NPY_UINT8:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint8>);
            break;

        case NPY_INT16:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int16>);
            break;
        case NPY_UINT16:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint16>);
            break;

        case NPY_INT32:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int32>);
            break;
        case NPY_UINT32:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint32>);
            break;

        case NPY_INT64:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_int64>);
            break;
        case NPY_UINT64:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_fixed_dbt<npy_uint64>);
            break;

        case NPY_FLOAT32:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_float_dbt<npy_float32>);
            break;
        case NPY_FLOAT64:
            mpDB->set_dup_compare(
                    mpDB, 
                    &NumpyDB::compare_float_dbt<npy_float64>);
            break;

        default: 
            // anything else we use default lexical comparison
            break;
    }


}

void NumpyDB::set_printers()  throw (const char*) {

    switch (mKeyTypeNum) {
        case NPY_INT8:
            mKeyPrinter = &NumpyDB::print_num<npy_int8>;
            break;
        case NPY_UINT8:
            mKeyPrinter = &NumpyDB::print_num<npy_uint8>;
            break;

        case NPY_INT16:
            mKeyPrinter = &NumpyDB::print_num<npy_int16>;
            break;
        case NPY_UINT16:
            mKeyPrinter = &NumpyDB::print_num<npy_uint16>;
            break;

        case NPY_INT32:
            mKeyPrinter = &NumpyDB::print_num<npy_int32>;
            break;
        case NPY_UINT32:
            mKeyPrinter = &NumpyDB::print_num<npy_uint32>;
            break;

        case NPY_INT64:
            mKeyPrinter = &NumpyDB::print_num<npy_int64>;
            break;
        case NPY_UINT64:
            mKeyPrinter = &NumpyDB::print_num<npy_uint64>;
            break;

        case NPY_FLOAT32:
            mKeyPrinter = &NumpyDB::print_num<npy_float32>;
            break;
        case NPY_FLOAT64:
            mKeyPrinter = &NumpyDB::print_num<npy_float64>;
            break;

        case NPY_STRING:
            mKeyPrinter = &NumpyDB::print_string;
            break;
        default: 
            stringstream err;
            err<<"Unsupported numpy type num: "<<mKeyTypeNum;
            throw err.str().c_str();
    }
    switch (mDataTypeNum) {
        case NPY_INT8:
            mDataPrinter = &NumpyDB::print_num<npy_int8>;
            break;
        case NPY_UINT8:
            mDataPrinter = &NumpyDB::print_num<npy_uint8>;
            break;

        case NPY_INT16:
            mDataPrinter = &NumpyDB::print_num<npy_int16>;
            break;
        case NPY_UINT16:
            mDataPrinter = &NumpyDB::print_num<npy_uint16>;
            break;

        case NPY_INT32:
            mDataPrinter = &NumpyDB::print_num<npy_int32>;
            break;
        case NPY_UINT32:
            mDataPrinter = &NumpyDB::print_num<npy_uint32>;
            break;

        case NPY_INT64:
            mDataPrinter = &NumpyDB::print_num<npy_int64>;
            break;
        case NPY_UINT64:
            mDataPrinter = &NumpyDB::print_num<npy_uint64>;
            break;

        case NPY_FLOAT32:
            mDataPrinter = &NumpyDB::print_num<npy_float32>;
            break;
        case NPY_FLOAT64:
            mDataPrinter = &NumpyDB::print_num<npy_float64>;
            break;

        case NPY_STRING:
            mDataPrinter = &NumpyDB::print_string;
            break;
        default: 
            stringstream err;
            err<<"Unsupported numpy type num: "<<mDataTypeNum;
            throw err.str().c_str();
    }

}





/* 
 * these comparison functions return: 
 * < 0 if a < b 
 * = 0 if a = b 
 * > 0 if a > b 
 */ 

template <class T>
int
NumpyDB::compare_fixed_dbt(DB *dbp, const DBT *a, const DBT *b)
{
    T ai, bi;
    memcpy(&ai, a->data, sizeof(T)); 
    memcpy(&bi, b->data, sizeof(T)); 
    return int(ai - bi); 
} 

template <class T>
int
NumpyDB::compare_fixed(const void *a, const void *b)
{
    T ai, bi;
    memcpy(&ai, a, sizeof(T)); 
    memcpy(&bi, b, sizeof(T)); 
    return int(ai - bi); 
} 



template <class T>
int
NumpyDB::compare_float_dbt(DB *dbp, const DBT *a, const DBT *b)
{
    T ai, bi;
    memcpy(&ai, a->data, sizeof(T)); 
    memcpy(&bi, b->data, sizeof(T)); 
	return int(  (ai > bi) - (ai < bi)  );
} 
template <class T>
int
NumpyDB::compare_float(const void *a, const void *b)
{
    T ai, bi;
    memcpy(&ai, a, sizeof(T)); 
    memcpy(&bi, b, sizeof(T)); 
	return int(  (ai > bi) - (ai < bi)  );
} 

