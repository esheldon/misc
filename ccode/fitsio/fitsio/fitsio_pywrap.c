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

    if (!PyArg_ParseTuple(args, (char*)"si", &filename, &mode)) {
        printf("failed to Parse init");
        return -1;
    }

    if (fits_open_file(&self->fits, filename, mode, &status) != 0) {
        // this will give full report
        //fits_report_error(stderr, status);
        // this for now is less
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

npy_int64* get_int64_from_array(PyObject* arr, npy_intp* ncols) {

    npy_int64* colnums;

	PyArray_Descr* descr;
    if (!PyArray_Check(arr)) {
        PyErr_SetString(PyExc_TypeError, "colnums must be an int64 array.");
        return NULL;
    }

    descr = PyArray_DESCR(arr);
	if (descr->type_num != NPY_INT64) {
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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number");
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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number");
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

    {
        int i;
        int imgtype;  
        PyObject* imgnaxis=PyList_New(0);

        fits_get_img_type(self->fits, &imgtype, &status);
        PyDict_SetItemString(dict, "img_type", PyInt_FromLong((long)imgtype));
        fits_get_img_equivtype(self->fits, &imgtype, &status);
        PyDict_SetItemString(dict, "img_equiv_type", PyInt_FromLong((long)imgtype));

        for (i=0; i<hdu->imgdim; i++) {
            PyList_Append(imgnaxis, PyInt_FromLong( (long)hdu->imgnaxis[i]));
        }
        PyDict_SetItemString(dict, "imgnaxis", imgnaxis);
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

    gsize = colptr->twidth*colptr->trepeat;
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

    gsize = colptr->twidth*colptr->trepeat;
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

    gsize = colptr->twidth*colptr->trepeat;
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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse column number, array");
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

            groupsize = colptr->twidth*colptr->trepeat;

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

            groupsize = colptr->twidth*colptr->trepeat;

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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number, column list, array, rows");
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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number, array");
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
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number, array");
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
PyFITSObject_read_image(struct PyFITSObject* self, PyObject* args) {
    int hdunum;
    int hdutype;
    int status=0;
    PyObject* array=NULL;
    void* data=NULL;
    FITSfile* hdu=NULL;

    int maxdim=10;
    int output_datatype=0;
    int datatype=0; // type info for axis
    int naxis=0; // number of axes
    int i=0;
    LONGLONG naxes[]={0,0,0,0,0,0,0,0,0,0};  // size of each axis
    LONGLONG firstpixels[]={1,1,1,1,1,1,1,1,1,1};
    LONGLONG size=0;
    npy_intp arrsize=0;

    int anynul=0;

    if (!PyArg_ParseTuple(args, (char*)"iiO", &hdunum, &output_datatype, &array)) {
        PyErr_SetString(PyExc_RuntimeError, "failed to parse hdu number, array");
        return NULL;
    }

    if (self->fits == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "FITS file is NULL");
        return NULL;
    }
    if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
        return NULL;
    }

    hdu = self->fits->Fptr;
    if (hdu->hdutype != IMAGE_HDU) {
        PyErr_SetString(PyExc_RuntimeError, "HDU is not an IMAGE_HDU");
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


static PyMethodDef PyFITSObject_methods[] = {
    {"moveabs_hdu",          (PyCFunction)PyFITSObject_moveabs_hdu,          METH_VARARGS, "moveabs_hdu\n\nMove to the specified HDU."},
    {"get_hdu_info",          (PyCFunction)PyFITSObject_get_hdu_info,          METH_VARARGS, "get_hdu_info\n\nReturn a dict with info about the specified HDU."},
    {"read_column",          (PyCFunction)PyFITSObject_read_column,          METH_VARARGS, "read_column\n\nRead the column into the input array.  No checking of array is done."},
    {"read_columns_as_rec",          (PyCFunction)PyFITSObject_read_columns_as_rec,          METH_VARARGS, "read_columns_as_rec\n\nRead the specified columns into the input rec array.  No checking of array is done."},
    {"read_rows_as_rec",          (PyCFunction)PyFITSObject_read_rows_as_rec,          METH_VARARGS, "read_rows_as_rec\n\nRead the subset of rows into the input rec array.  No checking of array is done."},
    {"read_as_rec",          (PyCFunction)PyFITSObject_read_as_rec,          METH_VARARGS, "read_as_rec\n\nRead the entire data set into the input rec array.  No checking of array is done."},
    {"read_image",          (PyCFunction)PyFITSObject_read_image,          METH_VARARGS, "read_image\n\nRead the entire n-dimensional image array.  No checking of array is done."},
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
