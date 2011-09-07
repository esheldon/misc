#include <Python.h>
#include <fitsio.h>
#include <fitsio2.h>
#include <numpy/arrayobject.h> 

struct PyFITSObject {
    PyObject_HEAD
    fitsfile* fits;
};



void set_err_string_from_status(int status) {
    char status_str[FLEN_STATUS], errmsg[FLEN_ERRMSG];

    if (status) {
      fits_get_errstatus(status, status_str);  /* get the error description */

      sprintf(errmsg, "FITSIO status = %d: %s\n", status, status_str);
      PyErr_SetString(PyExc_IOError, errmsg);

      /*
      while ( fits_read_errmsg(errmsg) )  // get error stack messages
      fprintf(stream, "%s\n", errmsg);
      */
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
        fits_report_error(stderr, status);
        // this for now is less
        set_err_string_from_status(status);
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

static PyObject *
PyFITSObject_get_hdu_info(struct PyFITSObject* self) {
    int hdunum;
    int hdutype;
    int status=0;
    PyObject* list;

    if (self->fits == NULL) {
        Py_RETURN_NONE;
    }
    list = PyList_New(0);
    for (hdunum=1; hdunum<1000; hdunum++) {
        if (fits_movabs_hdu(self->fits, hdunum, &hdutype, &status)) {
            break;
        }
        PyList_Append(list, PyInt_FromLong( (long)hdutype ));
    }

    return list;
}



static PyMethodDef PyFITSObject_methods[] = {
    {"get_hdu_info",          (PyCFunction)PyFITSObject_get_hdu_info,          METH_VARARGS, "get_hdu_info\n\nGet a list of the hdu types for all hdus."},
    {"close",          (PyCFunction)PyFITSObject_close,          METH_VARARGS, "close\n\nClose the fits file."},
    {NULL}  /* Sentinel */
};


/*

static PyMethodDef PyFITSObject_methods[] = {
    {"DH",          (PyCFunction)PyFITSObject_DH,          METH_VARARGS, "DH\n\nGet the Hubble distance"},
    {"flat",          (PyCFunction)PyFITSObject_flat,          METH_VARARGS, "flat\n\nReturn if universe if flat"},
    {"omega_m",          (PyCFunction)PyFITSObject_omega_m,          METH_VARARGS, "omega_m\n\nGet omega matter"},
    {"omega_l",          (PyCFunction)PyFITSObject_omega_l,          METH_VARARGS, "omega_m\n\nGet omega lambda"},
    {"omega_k",          (PyCFunction)PyFITSObject_omega_k,          METH_VARARGS, "omega_m\n\nGet omega curvature"},
    {"ez_inverse",          (PyCFunction)PyFITSObject_ez_inverse,          METH_VARARGS, "ez_inverse(z)\n\nGet 1/E(z)"},
    {"ez_inverse_vec",          (PyCFunction)PyFITSObject_ez_inverse_vec,          METH_VARARGS, "ez_inverse_vec(z)\n\nGet 1/E(z) for z an array"},
    {"ez_inverse_integral", (PyCFunction)PyFITSObject_ez_inverse_integral, METH_VARARGS, "ez_inverse_integral(zmin, zmax)\n\nGet integral of 1/E(z) from zmin to zmax"},
    {"Dc",               (PyCFunction)PyFITSObject_Dc,               METH_VARARGS, "Dc(zmin,zmax)\n\nComoving distance between zmin and zmax"},
    {"Dc_vec1",          (PyCFunction)PyFITSObject_Dc_vec1,          METH_VARARGS, "Dc_vec1(zmin,zmax)\n\nComoving distance between zmin(array) and zmax"},
    {"Dc_vec2",          (PyCFunction)PyFITSObject_Dc_vec2,          METH_VARARGS, "Dc_vec2(zmin,zmax)\n\nComoving distance between zmin and zmax(array)"},
    {"Dc_2vec",          (PyCFunction)PyFITSObject_Dc_2vec,          METH_VARARGS, "Dc_2vec(zmin,zmax)\n\nComoving distance between zmin and zmax both arrays"},
    {"Dm",              (PyCFunction)PyFITSObject_Dm,              METH_VARARGS, "Dm(zmin,zmax)\n\nTransverse comoving distance between zmin and zmax"},
    {"Dm_vec1",         (PyCFunction)PyFITSObject_Dm_vec1,         METH_VARARGS, "Dm_vec1(zmin,zmax)\n\nTransverse Comoving distance between zmin(array) and zmax"},
    {"Dm_vec2",         (PyCFunction)PyFITSObject_Dm_vec2,         METH_VARARGS, "Dm_vec2(zmin,zmax)\n\nTransverse Comoving distance between zmin and zmax(array)"},
    {"Dm_2vec",         (PyCFunction)PyFITSObject_Dm_2vec,         METH_VARARGS, "Dm_2vec(zmin,zmax)\n\nTransverse Comoving distance between zmin and zmax both arrays"},
    {"Da",             (PyCFunction)PyFITSObject_Da,             METH_VARARGS, "Da(zmin,zmax)\n\nAngular diameter distance distance between zmin and zmax"},
    {"Da_vec1",        (PyCFunction)PyFITSObject_Da_vec1,        METH_VARARGS, "Da_vec1(zmin,zmax)\n\nAngular diameter distance distance between zmin(array) and zmax"},
    {"Da_vec2",        (PyCFunction)PyFITSObject_Da_vec2,        METH_VARARGS, "Da_vec2(zmin,zmax)\n\nAngular diameter distance distance between zmin and zmax(array)"},
    {"Da_2vec",        (PyCFunction)PyFITSObject_Da_2vec,        METH_VARARGS, "Da_2vec(zmin,zmax)\n\nAngular diameter distance distance between zmin and zmax both arrays"},
    {"Dl",             (PyCFunction)PyFITSObject_Dl,             METH_VARARGS, "Dl(zmin,zmax)\n\nLuminosity distance distance between zmin and zmax"},
    {"Dl_vec1",        (PyCFunction)PyFITSObject_Dl_vec1,        METH_VARARGS, "Dl_vec1(zmin,zmax)\n\nLuminosity distance distance between zmin(array) and zmax"},
    {"Dl_vec2",        (PyCFunction)PyFITSObject_Dl_vec2,        METH_VARARGS, "Dl_vec2(zmin,zmax)\n\nLuminosity distance distance between zmin and zmax(array)"},
    {"Dl_2vec",        (PyCFunction)PyFITSObject_Dl_2vec,        METH_VARARGS, "Dl_2vec(zmin,zmax)\n\nLuminosity distance distance between zmin and zmax both arrays"},
    {"dV",                  (PyCFunction)PyFITSObject_dV,                  METH_VARARGS, "dV(z)\n\nComoving volume element at redshift z"},
    {"dV_vec",              (PyCFunction)PyFITSObject_dV_vec,              METH_VARARGS, "dV(z)\n\nComoving volume element at redshift z(array)"},
    {"V",                   (PyCFunction)PyFITSObject_V,                   METH_VARARGS, "V(z)\n\nComoving volume between zmin and zmax"},
    {"scinv",               (PyCFunction)PyFITSObject_scinv,               METH_VARARGS, "scinv(zl,zs)\n\nInverse critical density distance between zl and zs"},
    {"scinv_vec1",          (PyCFunction)PyFITSObject_scinv_vec1,          METH_VARARGS, "scinv_vec1(zl,zs)\n\nInverse critical density distance between zl(array) and zs"},
    {"scinv_vec2",          (PyCFunction)PyFITSObject_scinv_vec2,          METH_VARARGS, "scinv_vec2(zl,zs)\n\nInverse critical density distance between zl and zs(array)"},
    {"scinv_2vec",          (PyCFunction)PyFITSObject_scinv_2vec,          METH_VARARGS, "scinv_2vec(zl,zs)\n\nInverse critical density distance between zl and zs both arrays"},

    {NULL}
};


*/


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
