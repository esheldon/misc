#include <string.h>
#include <Python.h>
#include <fitsio.h>
#include <fitsio2.h>
#include <numpy/arrayobject.h> 

struct PyFITSObject {
    PyObject_HEAD
    fitsfile* fits;
};

void set_ioerr_string_from_status(int status) {
    char status_str[FLEN_STATUS], errmsg[FLEN_ERRMSG];

    if (status) {
      fits_get_errstatus(status, status_str);  /* get the error description */

      sprintf(errmsg, "FITSIO status = %d: %s\n", status, status_str);
      PyErr_SetString(PyExc_IOError, errmsg);
    }
    return;
}

static PyObject *
PyFITSObject_close(struct PyFITSObject* self)
{
    int status=0;
    fits_close_file(self->fits, &status);
    self->fits=NULL;
    Py_RETURN_NONE;
}



static void
PyFITSObject_dealloc(struct PyFITSObject* self)
{
    int status=0;
    fits_close_file(self->fits, &status);
    self->ob_type->tp_free((PyObject*)self);
}

static int
PyFITSObject_init(struct PyFITSObject* self, PyObject *args, PyObject *kwds)
{
    char* filename;
    int mode;
    int status=0;
    int create=0;

    if (!PyArg_ParseTuple(args, (char*)"sii", &filename, &mode, &create)) {
        return -1;
    }

    if (create) {
        if (fits_create_file(&self->fits, filename, &status)) {
            //fits_report_error(stderr,status);
            set_ioerr_string_from_status(status);
            return -1;
        }
    }
    if (fits_open_file(&self->fits, filename, mode, &status)) {
        set_ioerr_string_from_status(status);
        return -1;
    }

    return 0;
}


static PyObject *
PyFITSObject_repr(struct PyFITSObject* self) {
    char repr[255];
    if (self->fits != NULL) {
        sprintf(repr, "%s", self->fits->Fptr->filename);
        return PyString_FromString(repr);
    }  else {
        return PyString_FromString("");
    }
}


// this will need to be updated for array string columns.
long get_groupsize(tcolumn* colptr) {
    long gsize=0;
    if (colptr->tdatatype == TSTRING) {
        gsize = colptr->twidth;
    } else {
        gsize = colptr->twidth*colptr->trepeat;
    }
    return gsize;
}
npy_int64* get_int64_from_array(PyObject* arr, npy_intp* ncols) {

    npy_int64* colnums;
    int npy_type=0;

    if (!PyArray_Check(arr)) {
        PyErr_SetString(PyExc_TypeError, "colnums must be an int64 array.");
        return NULL;
    }

    npy_type = PyArray_TYPE(arr);
	if (npy_type != NPY_INT64) {
        PyErr_SetString(PyExc_TypeError, "colnums must be an int64 array.");
        return NULL;
    }

    colnums = PyArray_DATA(arr);
    *ncols = PyArray_SIZE(arr);

    return colnums;
}


static PyObject *
PyFITSObject_moveabs_hdu(struct PyFITSObject* self, PyObject* args) {
    int hdunum=0, hdutype=0;
    int status=0;

    if (self->fits == NULL) {
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, (char*)"i", &hdunum)) {
        return NULL;
    }

    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        // this will give full report
        //fits_report_error(stderr, status);
        // this for now is less
        set_ioerr_string_from_status(status);
        return NULL;
    }
    Py_RETURN_NONE;
}


// get info for the specified HDU
static PyObject *
PyFITSObject_get_hdu_info(struct PyFITSObject* self, PyObject* args) {
    int hdunum=0, hdutype=0, ext=0;
    int status=0;
    PyObject* dict=NULL;

    FITSfile* hdu=NULL;

    if (self->fits == NULL) {
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, (char*)"i", &hdunum)) {
        return NULL;
    }

    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        // this will give full report
        //fits_report_error(stderr, status);
        // this for now is less
        set_ioerr_string_from_status(status);
        return NULL;
    }

    hdu = self->fits->Fptr;

    dict = PyDict_New();
    ext=hdunum-1;
    PyDict_SetItemString(dict, "hdunum", PyInt_FromLong((long)hdunum));
    PyDict_SetItemString(dict, "extnum", PyInt_FromLong((long)ext));
    PyDict_SetItemString(dict, "hdutype", PyInt_FromLong((long)hdutype));

    PyDict_SetItemString(dict, "imgdim", PyInt_FromLong((long)hdu->imgdim));
    PyDict_SetItemString(dict, "zndim", PyInt_FromLong((long)hdu->zndim));

    {
        int i;
        int imgtype;  
        PyObject* imgnaxis=PyList_New(0);
        PyObject* znaxis=PyList_New(0);

        fits_get_img_type(self->fits, &imgtype, &status);
        PyDict_SetItemString(dict, "img_type", PyInt_FromLong((long)imgtype));
        fits_get_img_equivtype(self->fits, &imgtype, &status);
        PyDict_SetItemString(dict, "img_equiv_type", PyInt_FromLong((long)imgtype));

        for (i=0; i<hdu->imgdim; i++) {
            PyList_Append(imgnaxis, PyInt_FromLong( (long)hdu->imgnaxis[i]));
        }
        PyDict_SetItemString(dict, "imgnaxis", imgnaxis);

        for (i=0; i<hdu->zndim; i++) {
            PyList_Append(znaxis, PyInt_FromLong( (long)hdu->znaxis[i]));
        }
        PyDict_SetItemString(dict, "znaxis", znaxis);

    }

    PyDict_SetItemString(dict, "numrows", PyLong_FromLongLong( (long long)hdu->numrows));
    PyDict_SetItemString(dict, "tfield", PyInt_FromLong( (long)hdu->tfield));

    {
        PyObject* colinfo = PyList_New(0);
        if (hdutype != IMAGE_HDU) {
            int i;
            tcolumn* col;
            for (i=0; i<hdu->tfield; i++) {
                PyObject* d = PyDict_New();

                col = &hdu->tableptr[i];

                PyDict_SetItemString(d, "ttype", PyString_FromString(col->ttype));
                PyDict_SetItemString(d, "tdatatype", PyInt_FromLong((long)col->tdatatype));

                PyDict_SetItemString(d, "tbcol", PyLong_FromLongLong((long long)col->tbcol));
                PyDict_SetItemString(d, "trepeat", PyLong_FromLongLong((long long)col->trepeat));

                PyDict_SetItemString(d, "twidth", PyLong_FromLong((long)col->twidth));

                PyDict_SetItemString(d, "tscale", PyFloat_FromDouble(col->tscale));
                PyDict_SetItemString(d, "tzero", PyFloat_FromDouble(col->tzero));

                PyList_Append(colinfo, d);
            }
        }
        PyDict_SetItemString(dict, "colinfo", colinfo);
    }
    return dict;
}

// this converts bitpix to a corresponding numpy dtype
// of the right type
static int 
fits_to_npy_image_type(int bitpix) {

    char mess[255];
    switch (bitpix) {
        case BYTE_IMG:
            return NPY_UINT8;
        case SBYTE_IMG:
            return NPY_INT8;

        case USHORT_IMG:
            return NPY_UINT16;
        case SHORT_IMG:
            return NPY_INT16;

        case ULONG_IMG:
            return NPY_UINT32;
        case LONG_IMG:
            return NPY_INT32;

        case LONGLONG_IMG:
            return NPY_INT64;

        case FLOAT_IMG:
            return NPY_FLOAT32;
        case DOUBLE_IMG:
            return NPY_FLOAT64;

        default:
            sprintf(mess,"Unsupported fits image type (bitpix) %d", bitpix);
            PyErr_SetString(PyExc_TypeError, mess);
            return -9999;
    }

    return 0;
}



static int 
npy_to_fits_image_types(int npy_dtype, int *fits_img_type, int *fits_datatype) {

    char mess[255];
    switch (npy_dtype) {
        case NPY_UINT8:
            *fits_img_type = BYTE_IMG;
            *fits_datatype = TBYTE;
            break;
        case NPY_INT8:
            *fits_img_type = SBYTE_IMG;
            *fits_datatype = TSBYTE;
            break;
        case NPY_UINT16:
            *fits_img_type = USHORT_IMG;
            *fits_datatype = TUSHORT;
            break;
        case NPY_INT16:
            *fits_img_type = SHORT_IMG;
            *fits_datatype = TSHORT;
            break;

        case NPY_UINT32:
            *fits_img_type = ULONG_IMG;
            if (sizeof(unsigned int) == sizeof(npy_uint32)) {
                *fits_datatype = TUINT;
            } else if (sizeof(unsigned long) == sizeof(npy_uint32)) {
                *fits_datatype = TULONG;
            } else {
                PyErr_SetString(PyExc_TypeError, "could not determine 4 byte unsigned integer type");
                *fits_datatype = -9999;
                return 1;
            }
            break;

        case NPY_INT32:
            *fits_img_type = LONG_IMG;
            if (sizeof(int) == sizeof(npy_int32)) {
                *fits_datatype = TINT;
            } else if (sizeof(long) == sizeof(npy_int32)) {
                *fits_datatype = TLONG;
            } else {
                PyErr_SetString(PyExc_TypeError, "could not determine 4 byte integer type");
                *fits_datatype = -9999;
                return 1;
            }
            break;

        case NPY_INT64:
            *fits_img_type = LONGLONG_IMG;
            if (sizeof(int) == sizeof(npy_int64)) {
                *fits_datatype = TINT;
            } else if (sizeof(long) == sizeof(npy_int64)) {
                *fits_datatype = TLONG;
            } else if (sizeof(long long) == sizeof(npy_int64)) {
                *fits_datatype = TLONGLONG;
            } else {
                PyErr_SetString(PyExc_TypeError, "could not determine 8 byte integer type");
                *fits_datatype = -9999;
                return 1;
            }
            break;


        case NPY_FLOAT32:
            *fits_img_type = FLOAT_IMG;
            *fits_datatype = TFLOAT;
            break;
        case NPY_FLOAT64:
            *fits_img_type = DOUBLE_IMG;
            *fits_datatype = TDOUBLE;
            break;

        case NPY_UINT64:
            PyErr_SetString(PyExc_TypeError, "Unsigned 8 byte integer images are not supported by the FITS standard");
            *fits_datatype = -9999;
            return 1;
            break;

        default:
            sprintf(mess,"Unsupported numpy image datatype %d", npy_dtype);
            PyErr_SetString(PyExc_TypeError, mess);
            *fits_datatype = -9999;
            return 1;
            break;
    }

    return 0;
}

int get_ndim(PyObject* obj) {
    PyArrayObject* arr;
    arr = (PyArrayObject*) obj;
    return arr->nd;
}
static PyObject *
PyFITSObject_write_image(struct PyFITSObject* self, PyObject* args) {
    // allow 10 dimensions
    int ndims=0;
    long dims[] = {0,0,0,0,0,0,0,0,0,0};
    LONGLONG nelements=1; // should be cumprod of dims
    //long firstpixels[]={1,1,1,1,1,1,1,1,1,1};
    LONGLONG firstpixel=1;
    int image_datatype=0; // fits type for image, AKA bitpix
    int datatype=0; // type for the data we entered

    PyObject* array;
    void* data=NULL;
    int npy_dtype=0;
    int i=0;
    int status=0;

    if (!PyArg_ParseTuple(args, (char*)"O", &array)) {
        return NULL;
    }

    if (!PyArray_Check(array)) {
        PyErr_SetString(PyExc_TypeError, "input must be an array.");
        return NULL;
    }

    npy_dtype = PyArray_TYPE(array);
    if (npy_to_fits_image_types(npy_dtype, &image_datatype, &datatype)) {
        return NULL;
    }

    ndims = get_ndim(array);
    // order must be reversed for FITS
    for (i=0; i<ndims; i++) {
        //dims[i] = PyArray_DIM(array, i);
        dims[ndims-i-1] = PyArray_DIM(array, i);
        nelements *= dims[ndims-i-1];
    }

    if (fits_create_img(self->fits, image_datatype, ndims, dims, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    data = PyArray_DATA(array);
    if (fits_write_img(self->fits, datatype, firstpixel, nelements, data, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    Py_RETURN_NONE;
}

struct stringlist {
    size_t size;
    char** data;
};
struct stringlist* stringlist_new(void) {
    struct stringlist* slist=NULL;

    slist = malloc(sizeof(struct stringlist));
    slist->size = 0;
    slist->data=NULL;
    return slist;
}
// push a copy of the string onto the string list
void stringlist_push(struct stringlist* slist, const char* str) {
    size_t slen=0;
    size_t i=0;

    slist->data = realloc(slist->data, slist->size+1);
    slist->size += 1;

    i = slist->size-1;
    slen = strlen(str);

    slist->data[i] = malloc(sizeof(char)*(slen+1));
    strcpy(slist->data[i], str);
}
struct stringlist* stringlist_delete(struct stringlist* slist) {
    if (slist != NULL) {
        size_t i=0;
        for (i=0; i < slist->size; i++) {
            free(slist->data[i]);
        }
        free(slist->data);
        free(slist);
    }
    return NULL;
}

int stringlist_addfrom_listobj(struct stringlist* slist, PyObject* listObj) {
    size_t size=0, i=0;

    if (!PyList_Check(listObj)) {
        return 1;
    }
    size = PyList_Size(listObj);

    for (i=0; i<size; i++) {
        PyObject* tmp = PyList_GetItem(listObj, i);
        const char* tmpstr;
        if (!PyString_Check(tmp)) {
            PyErr_SetString(PyExc_ValueError, "Expected a string in list.");
            return 1;
        }
        tmpstr = (const char*) PyString_AsString(tmp);
        stringlist_push(slist, tmpstr);
    }
    return 0;
}

void stringlist_print(struct stringlist* slist) {
    size_t i=0;
    if (slist == NULL) {
        return;
    }
    for (i=0; i<slist->size; i++) {
        printf("  slist[%ld]: %s\n", i, slist->data[i]);
    }
}


// create a new table structure.  No physical rows are added yet.
static PyObject *
PyFITSObject_create_table(struct PyFITSObject* self, PyObject* args, PyObject* kwds) {
    int status=0;
    int table_type=BINARY_TBL;

    static char *kwlist[] = {"ttyp","tform","tunit", "tdim", "extname", NULL};
    // these are all strings
    PyObject* ttypObj=NULL;
    PyObject* tformObj=NULL;
    PyObject* tunitObj=NULL;    // optional
    PyObject* tdimObj=NULL;     // optional
    PyObject* extnameObj=NULL;  // optional

    // these must be freed
    struct stringlist* ttyp=stringlist_new();
    struct stringlist* tform=stringlist_new();
    struct stringlist* tunit=stringlist_new();
    struct stringlist* tdim=stringlist_new();
    char* extname=NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|OOO", kwlist,
                          &ttypObj, &tformObj, &tunitObj, &tdimObj, &extnameObj)) {
        return NULL;
    }

    if (stringlist_addfrom_listobj(ttyp, ttypObj)) {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract ttyp list");
        goto create_table_cleanup;
    }
    stringlist_print(ttyp);
    if (stringlist_addfrom_listobj(tform, tformObj)) {
        PyErr_SetString(PyExc_RuntimeError, "failed to extract tform list");
        goto create_table_cleanup;
    }
    stringlist_print(tform);
    if (tunitObj != NULL) {
        if (stringlist_addfrom_listobj(tunit, tunitObj)) {
            PyErr_SetString(PyExc_RuntimeError, "failed to extract tunit list");
            goto create_table_cleanup;
        }
        stringlist_print(tunit);
    }


create_table_cleanup:
    ttyp = stringlist_delete(ttyp);
    tform = stringlist_delete(tform);
    tunit = stringlist_delete(tunit);
    tdim = stringlist_delete(tdim);
    free(extname);

    Py_RETURN_NONE;
}

static PyObject *
PyFITSObject_test_write_new_table(struct PyFITSObject* self, PyObject* args) {

    int status=0;
    int table_type=BINARY_TBL;

    char *ttype[] = { "Planet", "Diameter", "Density" };
    char *tform[] = { "8a",     "1J",       "1E"    };
    char *tunit[] = { "\0",      "km",       "g/cm"    };

    /* define the name diameter, and density of each planet */
    char *planet[] = {"Mercury", "Venus", "Earth", "Mars","Jupiter","Saturn"};
    int diameter[] = {4880,     12112,   12742,   6800,  143000,   121000};
    float density[]  = { 5.1,      5.3,     5.52,   3.94,   1.33,     0.69};

    long nrows    = 6;       /* table will have 6 rows    */
    int tfields   = 3;       /* table will have 3 columns */
    char* extname=NULL;           /* extension name */

    LONGLONG firstrow  = 1;  /* first row in table to write   */
    LONGLONG firstelem = 1;  /* first element in row  (ignored in ASCII tables) */

    /* append a new empty binary table onto the FITS file */
    // try with zero rows first
    if ( fits_create_tbl(self->fits, table_type, 0, tfields, ttype, tform,
                         tunit, extname, &status) ) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    fits_write_col(self->fits, TSTRING, 1, firstrow, firstelem, nrows, planet,
                   &status);
    fits_write_col(self->fits, TINT, 2, firstrow, firstelem, nrows, diameter,
                   &status);
    fits_write_col(self->fits, TFLOAT, 3, firstrow, firstelem, nrows, density,
                   &status);

    Py_RETURN_NONE;

}

 
// read a single, entire column from the current HDU into an unstrided array.
// Because of the internal fits buffering, and since we will read multiple at a
// time, this should be more efficient.  No error checking is done. No 
// scaling is performed here, that is done in python.  Byte swapping
// *is* done here.

static int read_column_bytes(fitsfile* fits, int colnum, void* data, int* status) {
    FITSfile* hdu=NULL;
    tcolumn* colptr=NULL;
    LONGLONG file_pos=0, row=0;

    // these should be LONGLONG bug arent, arg cfitsio is so inconsistent!
    long gsize=0; // number of bytes in column
    long ngroups=0; // number to read
    long offset=0; // gap between groups, not stride

    hdu = fits->Fptr;
    colptr = hdu->tableptr + (colnum-1);

    // need to deal with array string columns
    gsize = get_groupsize(colptr);
    ngroups = hdu->numrows;
    offset = hdu->rowlength-gsize;

    file_pos = hdu->datastart + row*hdu->rowlength + colptr->tbcol;

    // need to use internal file-move code because of bookkeeping
    if (ffmbyt(fits, file_pos, REPORT_EOF, status)) {
        return 1;
    }

    // Here we use the function to read everything at once
    if (ffgbytoff(fits, gsize, ngroups, offset, data, status)) {
        return 1;
    }
    return 0;
}

// there is not huge overhead reading one by one
// should convert this to the strided one
static int read_column_bytes_strided(fitsfile* fits, int colnum, void* data, npy_intp stride, int* status) {
    FITSfile* hdu=NULL;
    tcolumn* colptr=NULL;
    LONGLONG file_pos=0, row=0;

    // use char for pointer arith.  It's actually ok to use void as char but
    // this is just in case.
    char* ptr;

    // these should be LONGLONG bug arent, arg cfitsio is so inconsistent!
    long gsize=0; // number of bytes in column
    long ngroups=0; // number to read
    long offset=0; // gap between groups, not stride

    hdu = fits->Fptr;
    colptr = hdu->tableptr + (colnum-1);

    gsize = get_groupsize(colptr);
    ngroups = 1; // read one at a time
    offset = hdu->rowlength-gsize;

    ptr = (char*) data;
    for (row=0; row<hdu->numrows; row++) {
        file_pos = hdu->datastart + row*hdu->rowlength + colptr->tbcol;
        ffmbyt(fits, file_pos, REPORT_EOF, status);
        if (ffgbytoff(fits, gsize, ngroups, offset, (void*) ptr, status)) {
            return 1;
        }
        ptr += stride;
    }

    return 0;
}

// read a subset of rows for the input column
// the row array is assumed to be unique and sorted.
static int read_column_bytes_byrow(
        fitsfile* fits, 
        int colnum, 
        npy_intp nrows, 
        npy_int64* rows, 
        void* data, 
        npy_intp stride, 
        int* status) {

    FITSfile* hdu=NULL;
    tcolumn* colptr=NULL;
    LONGLONG file_pos=0, irow=0;
    npy_int64 row;

    // use char for pointer arith.  It's actually ok to use void as char but
    // this is just in case.
    char* ptr;

    // these should be LONGLONG bug arent, arg cfitsio is so inconsistent!
    long gsize=0; // number of bytes in column
    long ngroups=0; // number to read
    long offset=0; // gap between groups, not stride

    hdu = fits->Fptr;
    colptr = hdu->tableptr + (colnum-1);

    // need to deal with array string columns
    gsize = get_groupsize(colptr);

    ngroups = 1; // read one at a time
    offset = hdu->rowlength-gsize;

    ptr = (char*) data;
    for (irow=0; irow<nrows; irow++) {
        row = rows[irow];
        file_pos = hdu->datastart + row*hdu->rowlength + colptr->tbcol;
        ffmbyt(fits, file_pos, REPORT_EOF, status);
        if (ffgbytoff(fits, gsize, ngroups, offset, (void*) ptr, status)) {
            return 1;
        }
        ptr += stride;
    }

    return 0;
}



// read from a column into a contiguous array.  Don't yet
// support subset of rows.
//
// no error checking on the input array is performed!!
static PyObject *
PyFITSObject_read_column(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;
    int colnum;

    FITSfile* hdu=NULL;
    tcolumn* col;
    int status=0;

    PyObject* array;
    void* data;
    npy_intp stride=0;

    PyObject* rowsobj;

    if (!PyArg_ParseTuple(args, (char*)"iiOO", &hdunum, &colnum, &array, &rowsobj)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    hdu = self->fits->Fptr;
    if (hdu->hdutype == IMAGE_HDU) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot yet read columns from an IMAGE_HDU");
        return NULL;
    }
    if (colnum < 1 || colnum > hdu->tfield) {
        PyErr_SetString(PyExc_RuntimeError, "requested column is out of bounds");
        return NULL;
    }

    col = hdu->tableptr + (colnum-1);
    data = PyArray_DATA(array);
    
    if (rowsobj == Py_None) {
        if (PyArray_ISCONTIGUOUS(array)) {
            if (read_column_bytes(self->fits, colnum, data, &status)) {
                set_ioerr_string_from_status(status);
                return NULL;
            }
        } else {
            stride = PyArray_STRIDE(array,0);
            if (read_column_bytes_strided(self->fits, colnum, data, stride, &status)) {
                set_ioerr_string_from_status(status);
                return NULL;
            }
        }
    } else {
        npy_intp nrows=0;
        npy_int64* rows=NULL;
        rows = get_int64_from_array(rowsobj, &nrows);
        if (rows == NULL) {
            return NULL;
        }
        stride = PyArray_STRIDE(array,0);
        if (read_column_bytes_byrow(self->fits, colnum, nrows, rows,
                                    data, stride, &status)) {
            set_ioerr_string_from_status(status);
            return NULL;
        }

    }
    Py_RETURN_NONE;
}
 

// read the specified columns, and all rows, into the data array.  It is
// assumed the data match the requested columns perfectly, and that the column
// list is sorted

static int read_rec_column_bytes(fitsfile* fits, npy_intp ncols, npy_int64* colnums, void* data, int* status) {
    FITSfile* hdu=NULL;
    tcolumn* colptr=NULL;
    LONGLONG file_pos=0, row=0;
    npy_intp col=0;
    npy_int64 colnum=0;

    // use char for pointer arith.  It's actually ok to use void as char but
    // this is just in case.
    char* ptr;

    // these should be LONGLONG bug aren't, cfitsio is so inconsistent!
    long groupsize=0; // number of bytes in column
    long ngroups=1; // number to read, one for row-by-row reading
    long offset=0; // gap between groups, not stride.  zero since we aren't using it

    hdu = fits->Fptr;
    ptr = (char*) data;
    for (row=0; row<hdu->numrows; row++) {

        for (col=0; col < ncols; col++) {

            colnum = colnums[col];
            colptr = hdu->tableptr + (colnum-1);

            groupsize = get_groupsize(colptr);

            file_pos = hdu->datastart + row*hdu->rowlength + colptr->tbcol;

            // can just do one status check, since status are inherited.
            ffmbyt(fits, file_pos, REPORT_EOF, status);
            if (ffgbytoff(fits, groupsize, ngroups, offset, (void*) ptr, status)) {
                return 1;
            }
            ptr += groupsize;
        }
    }

    return 0;
}

// read specified columns and rows
static int read_rec_column_bytes_byrow(
        fitsfile* fits, 
        npy_intp ncols, npy_int64* colnums, 
        npy_intp nrows, npy_int64* rows,
        void* data, int* status) {
    FITSfile* hdu=NULL;
    tcolumn* colptr=NULL;
    LONGLONG file_pos=0;
    npy_intp col=0;
    npy_int64 colnum=0;

    npy_intp irow=0;
    npy_int64 row=0;

    // use char for pointer arith.  It's actually ok to use void as char but
    // this is just in case.
    char* ptr;

    // these should be LONGLONG bug aren't, cfitsio is so inconsistent!
    long groupsize=0; // number of bytes in column
    long ngroups=1; // number to read, one for row-by-row reading
    long offset=0; // gap between groups, not stride.  zero since we aren't using it

    hdu = fits->Fptr;
    ptr = (char*) data;
    for (irow=0; irow<nrows; irow++) {
        row = rows[irow];
        for (col=0; col < ncols; col++) {

            colnum = colnums[col];
            colptr = hdu->tableptr + (colnum-1);

            groupsize = get_groupsize(colptr);

            file_pos = hdu->datastart + row*hdu->rowlength + colptr->tbcol;

            // can just do one status check, since status are inherited.
            ffmbyt(fits, file_pos, REPORT_EOF, status);
            if (ffgbytoff(fits, groupsize, ngroups, offset, (void*) ptr, status)) {
                return 1;
            }
            ptr += groupsize;
        }
    }

    return 0;
}


// python method for reading specified columns and rows
static PyObject *
PyFITSObject_read_columns_as_rec(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;
    npy_intp ncols;
    npy_int64* colnums=NULL;

    FITSfile* hdu=NULL;
    int status=0;

    PyObject* columnsobj;
    PyObject* array;
    void* data;

    PyObject* rowsobj;

    if (!PyArg_ParseTuple(args, (char*)"iOOO", &hdunum, &columnsobj, &array, &rowsobj)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        goto recread_columns_cleanup;
    }

    hdu = self->fits->Fptr;
    if (hdu->hdutype == IMAGE_HDU) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot read IMAGE_HDU into a recarray");
        return NULL;
    }
    
    colnums = get_int64_from_array(columnsobj, &ncols);
    if (colnums == NULL) {
        return NULL;
    }

    data = PyArray_DATA(array);
    if (rowsobj == Py_None) {
        if (read_rec_column_bytes(self->fits, ncols, colnums, data, &status)) {
            goto recread_columns_cleanup;
        }
    } else {
        npy_intp nrows;
        npy_int64* rows=NULL;
        rows = get_int64_from_array(rowsobj, &nrows);
        if (rows == NULL) {
            return NULL;
        }
        if (read_rec_column_bytes_byrow(self->fits, ncols, colnums, nrows, rows, data, &status)) {
            goto recread_columns_cleanup;
        }
    }

recread_columns_cleanup:

    if (status != 0) {
        set_ioerr_string_from_status(status);
        return NULL;
    }
    Py_RETURN_NONE;
}
 
// read specified rows, all columns
static int read_rec_bytes_byrow(
        fitsfile* fits, 
        npy_intp nrows, npy_int64* rows,
        void* data, int* status) {
    FITSfile* hdu=NULL;
    LONGLONG file_pos=0;

    npy_intp irow=0;
    npy_int64 row=0;

    // use char for pointer arith.  It's actually ok to use void as char but
    // this is just in case.
    char* ptr;

    // these should be LONGLONG bug aren't, cfitsio is so inconsistent!
    long ngroups=1; // number to read, one for row-by-row reading
    long offset=0; // gap between groups, not stride.  zero since we aren't using it

    hdu = fits->Fptr;
    ptr = (char*) data;

    for (irow=0; irow<nrows; irow++) {
        row = rows[irow];
        file_pos = hdu->datastart + row*hdu->rowlength;

        // can just do one status check, since status are inherited.
        ffmbyt(fits, file_pos, REPORT_EOF, status);
        if (ffgbytoff(fits, hdu->rowlength, ngroups, offset, (void*) ptr, status)) {
            return 1;
        }
        ptr += hdu->rowlength;
    }

    return 0;
}


// python method to read all columns but subset of rows
static PyObject *
PyFITSObject_read_rows_as_rec(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;

    FITSfile* hdu=NULL;
    int status=0;
    PyObject* array;
    void* data;

    PyObject* rowsobj=NULL;
    npy_intp nrows=0;
    npy_int64* rows=NULL;

    if (!PyArg_ParseTuple(args, (char*)"iOO", &hdunum, &array, &rowsobj)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        goto recread_byrow_cleanup;
    }

    hdu = self->fits->Fptr;
    if (hdu->hdutype == IMAGE_HDU) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot read IMAGE_HDU into a recarray");
        return NULL;
    }

    data = PyArray_DATA(array);

    rows = get_int64_from_array(rowsobj, &nrows);
    if (rows == NULL) {
        return NULL;
    }
 
    if (read_rec_bytes_byrow(self->fits, nrows, rows, data, &status)) {
        //fits_report_error(stderr, status);
        goto recread_byrow_cleanup;
    }

recread_byrow_cleanup:

    if (status != 0) {
        set_ioerr_string_from_status(status);
        return NULL;
    }
    Py_RETURN_NONE;
}
 






// Read the entire table into the input rec array.  It is assumed the data
// match table perfectly.
static int read_rec_bytes(fitsfile* fits, void* data, int* status) {
    FITSfile* hdu=NULL;
    LONGLONG file_pos=0;

    long nbytes=0;

    hdu = fits->Fptr;

    file_pos = hdu->datastart;
    nbytes = hdu->numrows*hdu->rowlength;

    if (file_seek(hdu->filehandle, file_pos)) {
        *status = SEEK_ERROR;
        return 1;
    }

    if (ffread(hdu, nbytes, data, status)) {
        return 1;
    }
    // we have to return so as not to confuse the buffer system
    if (file_seek(hdu->filehandle, file_pos)) {
        *status = SEEK_ERROR;
        return 1;
    }

    return 0;
}



// read entire table at once
static PyObject *
PyFITSObject_read_as_rec(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;

    FITSfile* hdu=NULL;
    int status=0;
    PyObject* array;
    void* data;

    if (!PyArg_ParseTuple(args, (char*)"iO", &hdunum, &array)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        goto recread_cleanup;
    }

    hdu = self->fits->Fptr;
    if (hdu->hdutype == IMAGE_HDU) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot read IMAGE_HDU into a recarray");
        return NULL;
    }

    data = PyArray_DATA(array);

    if (read_rec_bytes(self->fits, data, &status)) {
        //fits_report_error(stderr, status);
        goto recread_cleanup;
    }

recread_cleanup:

    if (status != 0) {
        set_ioerr_string_from_status(status);
        return NULL;
    }
    Py_RETURN_NONE;
}
 
// read an n-dimensional "image" into the input array.  Only minimal checking
// of the input array is done.
static PyObject *
PyFITSObject_read_image_new(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;
    int status=0;
    PyObject* array=NULL;
    void* data=NULL;
    //FITSfile* hdu=NULL;

    int maxdim=10;
    int output_datatype=0;
    int datatype=0; // type info for axis
    int ndims=0; // number of axes
    int i=0;
    LONGLONG dims[]={0,0,0,0,0,0,0,0,0,0};  // size of each axis
    npy_intp npy_dims[]={0,0,0,0,0,0,0,0,0,0};
    LONGLONG firstpixels[]={1,1,1,1,1,1,1,1,1,1};
    LONGLONG size=0;
    npy_intp arrsize=0;
    int fortran=0;

    int anynul=0;

    if (!PyArg_ParseTuple(args, (char*)"i", &hdunum)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        return NULL;
    }

    /*
    if (npy_to_fits_image_types(int npy_dtype, int *fits_img_type, int *fits_datatype)) {
        return NULL;
    }
    */

    if (fits_get_img_paramll(self->fits, maxdim, &datatype, &ndims, dims, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    for (i=0; i<ndims; i++) {
        npy_dims[ndims-i-1] = dims[i];
    }


    array = PyArray_ZEROS(ndims, npy_dims, NPY_INT32, fortran);
    if (array==NULL) {
        PyErr_SetString(PyExc_MemoryError, "failed to create output array");
    }
    return array;


    // make sure dims match
    size=0;
    size = dims[0];
    for (i=1; i< ndims; i++) {
        size *= dims[i];
    }
    arrsize = PyArray_SIZE(array);
    data = PyArray_DATA(array);

    if (size != arrsize) {
        char mess[255];
        sprintf(mess,"Input array size is %ld but on disk array size is %lld", arrsize, size);
        PyErr_SetString(PyExc_RuntimeError, mess);
        return NULL;
    }

    // need to deal with incorrect types in cfitsio for long types
    if (output_datatype == TLONG) {
        if (sizeof(long) == sizeof(npy_int64) && sizeof(int) == sizeof(npy_int32)) {
            // internally read_pix uses int for TINT, so assuming int is always 32
            output_datatype = TINT;
        } else {
            PyErr_SetString(PyExc_TypeError, "don't know how to deal with TLONG on this system");
            return NULL;
        }
    }
    if (fits_read_pixll(self->fits, output_datatype, firstpixels, size,
                        0, data, &anynul, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }


    Py_RETURN_NONE;
}

// read an n-dimensional "image" into the input array.  Only minimal checking
// of the input array is done.
static PyObject *
PyFITSObject_read_image(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;
    int status=0;
    PyObject* array=NULL;
    void* data=NULL;
    int npy_dtype=0;
    int dummy=0, fits_read_dtype=0;
    //FITSfile* hdu=NULL;

    int maxdim=10;
    int datatype=0; // type info for axis
    int naxis=0; // number of axes
    int i=0;
    LONGLONG naxes[]={0,0,0,0,0,0,0,0,0,0};  // size of each axis
    LONGLONG firstpixels[]={1,1,1,1,1,1,1,1,1,1};
    LONGLONG size=0;
    npy_intp arrsize=0;

    int anynul=0;

    if (!PyArg_ParseTuple(args, (char*)"iO", &hdunum, &array)) {
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        return NULL;
    }

    if (fits_get_img_paramll(self->fits, maxdim, &datatype, &naxis, naxes, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }

    // make sure dims match
    size=0;
    size = naxes[0];
    for (i=1; i< naxis; i++) {
        size *= naxes[i];
    }
    arrsize = PyArray_SIZE(array);
    data = PyArray_DATA(array);

    if (size != arrsize) {
        char mess[255];
        sprintf(mess,"Input array size is %ld but on disk array size is %lld", arrsize, size);
        PyErr_SetString(PyExc_RuntimeError, mess);
        return NULL;
    }

    npy_dtype = PyArray_TYPE(array);
    npy_to_fits_image_types(npy_dtype, &dummy, &fits_read_dtype);

    // need to deal with incorrect types in cfitsio for long types
    /*
    if (fits_read_dtype == TLONG) {
        if (sizeof(long) == sizeof(npy_int64) && sizeof(int) == sizeof(npy_int32)) {
            // internally read_pix uses int for TINT, so assuming int is always 32
            fits_read_dtype = TINT;
        } else {
            PyErr_SetString(PyExc_TypeError, "don't know how to deal with TLONG on this system");
            return NULL;
        }
    }
    */
    if (fits_read_pixll(self->fits, fits_read_dtype, firstpixels, size,
                        0, data, &anynul, &status)) {
        set_ioerr_string_from_status(status);
        return NULL;
    }


    Py_RETURN_NONE;
}


static PyMethodDef PyFITSObject_methods[] = {
    {"moveabs_hdu",          (PyCFunction)PyFITSObject_moveabs_hdu,          METH_VARARGS, "moveabs_hdu\n\nMove to the specified HDU."},
    {"get_hdu_info",          (PyCFunction)PyFITSObject_get_hdu_info,          METH_VARARGS, "get_hdu_info\n\nReturn a dict with info about the specified HDU."},
    {"read_column",          (PyCFunction)PyFITSObject_read_column,          METH_VARARGS, "read_column\n\nRead the column into the input array.  No checking of array is done."},
    {"read_columns_as_rec",          (PyCFunction)PyFITSObject_read_columns_as_rec,          METH_VARARGS, "read_columns_as_rec\n\nRead the specified columns into the input rec array.  No checking of array is done."},
    {"read_rows_as_rec",          (PyCFunction)PyFITSObject_read_rows_as_rec,          METH_VARARGS, "read_rows_as_rec\n\nRead the subset of rows into the input rec array.  No checking of array is done."},
    {"read_as_rec",          (PyCFunction)PyFITSObject_read_as_rec,          METH_VARARGS, "read_as_rec\n\nRead the entire data set into the input rec array.  No checking of array is done."},
    {"write_image",          (PyCFunction)PyFITSObject_write_image,          METH_VARARGS, "write_image\n\nWrite the input image to a new extension."},
    {"read_image",          (PyCFunction)PyFITSObject_read_image,          METH_VARARGS, "read_image\n\nRead the entire n-dimensional image array.  No checking of array is done."},
    {"test_write_new_table",          (PyCFunction)PyFITSObject_test_write_new_table,          METH_VARARGS, "test_write_new_table\n\nWrite the input image to a new extension."},
    {"create_table",          (PyCFunction)PyFITSObject_create_table,          METH_KEYWORDS, "create_table\n\nCreate a new table with the input parameters."},
    {"close",          (PyCFunction)PyFITSObject_close,          METH_VARARGS, "close\n\nClose the fits file."},
    {NULL}  /* Sentinel */
};


static PyTypeObject PyFITSType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_fitsio.FITS",             /*tp_name*/
    sizeof(struct PyFITSObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyFITSObject_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyFITSObject_repr,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Cosmology Class",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyFITSObject_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyFITSObject_init,      /* tp_init */
    0,                         /* tp_alloc */
    //PyFITSObject_new,                 /* tp_new */
    PyType_GenericNew,                 /* tp_new */
};


static PyMethodDef fitstype_methods[] = {
    {NULL}  /* Sentinel */
};


#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_fitsio_wrap(void) 
{
    PyObject* m;

    PyFITSType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyFITSType) < 0)
        return;

    m = Py_InitModule3("_fitsio_wrap", fitstype_methods, "Define FITS type and methods.");

    Py_INCREF(&PyFITSType);
    PyModule_AddObject(m, "FITS", (PyObject *)&PyFITSType);

    import_array();
}
