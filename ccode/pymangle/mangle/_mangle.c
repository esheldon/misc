#define NPY_NO_DEPRECATED_API

#include <string.h>
#include <Python.h>
#include "numpy/arrayobject.h" 

#define _MANGLE_SMALL_BUFFSIZE 12

struct Cap {
    double x;
    double y;
    double z;
    double cm;
};

struct CapVec {
    npy_intp size;
    struct Cap* data;
};

struct Polygon {

    npy_intp poly_id;
    npy_intp pixel_id; // optional
    double weight;
    double area; // in str

    struct CapVec* cap_vec;

};

struct PolygonVec {
    npy_intp size;
    struct Polygon* data;
};

struct NpyIntpVec {
    npy_intp size;
    npy_intp* data;
};

struct PyMangleMask {
    PyObject_HEAD

    char* filename;
    struct PolygonVec* poly_vec;

    npy_intp pixelres;
    char pixeltype;
    struct NpyIntpVec* pixel_vec;

    int snapped;
    int balkanized;

    int verbose;
};


static struct CapVec* 
CapVec_new(npy_intp n) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Cap vector");
        return NULL;
    }
    self->data = calloc(n, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Cap vector");
        return NULL;
    }
    self->size = n;
    return self;
}

static struct CapVec* CapVec_free(struct CapVec* self)
{
    if (self != NULL) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}

static struct PolygonVec* 
PolygonVec_new(npy_intp n) 
{
    struct PolygonVec* self=NULL;
    struct Polygon* ply=NULL;
    npy_intp i=0;

    self=calloc(1, sizeof(struct PolygonVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Polygon vector");
        return NULL;
    }
    self->data = calloc(n, sizeof(struct Polygon));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Polygon vector");
        return NULL;
    }

    self->size = n;
    ply = self->data;
    for (i=0; i<self->size; i++) {
        ply->cap_vec=NULL;
    }
    return self;
}

static struct PolygonVec*
PolygonVec_free(struct PolygonVec* self)
{
    struct Polygon* ply=NULL;
    npy_intp i=0;
    if (self != NULL) {
        if (self->data!= NULL) {

            ply=self->data;
            for (i=0; i<self->size; i++) {
                ply->cap_vec = CapVec_free(ply->cap_vec);
                ply++;
            }
            free(self->data);

        }
        free(self);
        self=NULL;
    }
    return self;
}



static struct NpyIntpVec* 
NpyIntpVec_new(npy_intp n) 
{
    struct NpyIntpVec* self=NULL;

    self=calloc(1, sizeof(struct NpyIntpVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp vector");
        return NULL;
    }
    self->data = calloc(n, sizeof(npy_intp));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp vector");
        return NULL;
    }
    self->size = n;
    return self;
}


static struct NpyIntpVec* 
NpyIntpVec_free(struct NpyIntpVec* self) 
{
    if (self != NULL) {
        free(self->data);
        free(self);
    }
    return self;
}

static int
get_pix_scheme(char buff[_MANGLE_SMALL_BUFFSIZE], npy_intp* res, char* pixeltype) {
    int status=1;
    char pixres_buff[_MANGLE_SMALL_BUFFSIZE];
    char* ptr=NULL;
    npy_intp res_bytes=0;

    memset(pixres_buff, 0, _MANGLE_SMALL_BUFFSIZE);

    ptr = strchr(buff, 's');
    if (ptr == NULL) {
        status=0;
        PyErr_Format(PyExc_IOError, "Only support pix scheme s, got: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    *pixeltype = 's';

    // extract the numerical prefactor, which is the resolution
    res_bytes = (ptr-buff);
    if (res_bytes > 9) {
        status=0;
        PyErr_Format(PyExc_IOError, "pix scheme designation too big: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    strncpy(pixres_buff, buff, res_bytes);

    if (1 != sscanf(pixres_buff, "%ld", res)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Could not extract resolution from pix scheme: '%s'", buff);
        goto _get_pix_scheme_errout;
    }

_get_pix_scheme_errout:
    return status;
}

static int
scan_expected_value(FILE* fptr, 
                    char buff[_MANGLE_SMALL_BUFFSIZE], 
                    const char* expected_value, 
                    npy_intp poly_index)
{
    int status=1, res=0;

    res = fscanf(fptr, "%s", buff);
    if (1 != res || strcmp(buff,expected_value) != 0) {
        status=0;
        PyErr_Format(PyExc_IOError, 
                     "Failed to read expected string '%s' for polygon %ld", expected_value, poly_index);
    }
    return status;
}

/* 
 * parse the polygon "header"
 *
 * this is after reading the initial 'polygon' token
 */
static int
read_polygon_header(struct PyMangleMask* self, 
                    FILE* fptr, 
                    npy_intp poly_index, 
                    char buff[_MANGLE_SMALL_BUFFSIZE],
                    npy_intp* ncaps)
{
    int status=1;
    struct Polygon* ply=NULL;
    ply = &self->poly_vec->data[poly_index];

    if (1 != fscanf(fptr, "%ld", &ply->poly_id)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read polygon id for polygon %ld", poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, buff, "(", poly_index)) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%ld",ncaps)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read ncaps for polygon %ld", poly_index);
        goto _read_polygon_header_errout;
    }


    if (!scan_expected_value(fptr, buff, "caps,", poly_index)) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%lf",&ply->weight)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read weight for polygon %ld", poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, buff, "weight,", poly_index)) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%ld",&ply->pixel_id)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read pixel id for polygon %ld", poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, buff, "pixel,", poly_index)) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%lf",&ply->area)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read area for polygon %ld", poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, buff, "str):", poly_index)) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (self->verbose > 1) {
        fprintf(stderr,
          "polygon %ld: poly_id %ld ncaps: %ld weight: %g pixel: %ld area: %g\n", 
          poly_index, ply->poly_id, *ncaps, ply->weight, ply->pixel_id, ply->area);
    }

_read_polygon_header_errout:
    return status;
}
/*
 * this is after reading the initial 'polygon' token
 *
 * poly_index is the index into the PolygonVec
 */
static int
read_polygon(struct PyMangleMask* self, FILE* fptr, npy_intp poly_index, char buff[_MANGLE_SMALL_BUFFSIZE]) {
    int status=1;
    struct Polygon* ply=NULL;
    npy_intp ncaps=0;
    ply = &self->poly_vec->data[poly_index];

    if (!read_polygon_header(self, fptr, poly_index, buff, &ncaps)) {
        status=0;
        goto _read_single_polygon_errout;
    }

_read_single_polygon_errout:
    return status;
}
/*
 * Should be passed FILE* right after reading the first
 * 'polygon' token, which should be stored in buff
 *
 * poly_vec should be allocated now
 */
static int
_read_polygons(struct PyMangleMask* self, FILE* fptr, char buff[_MANGLE_SMALL_BUFFSIZE])
{
    int status=1;

    npy_intp npoly=0, i=0;

    npoly=self->poly_vec->size;
    // buff comes in with 'polygon'

    for (i=0; i<npoly; i++) {
        if (strcmp(buff,"polygon") != 0) {
            status=0;
            PyErr_Format(PyExc_IOError, "Expected first token in poly to read 'polygon', got '%s'", buff);
            goto _read_some_polygons_errout;
        }
        status = read_polygon(self, fptr, i, buff);
        if (status != 1) {
            break;
        }
        i++;

        // stop reading where expected
        if (i == npoly) {
            break;
        }

        if (i != (npoly-1)) {
            if (1 != fscanf(fptr,"%s",buff)) {
                status=0;
                PyErr_Format(PyExc_IOError, "Error reading token for polygon %ld", i);
                goto _read_some_polygons_errout;
            }
        }
    }

_read_some_polygons_errout:
    return status;
}

/* 
 * read polygons from the file.  first parse the header and
 * then call the lower level routine _read_polygons to actuall
 * process the polygon specifications
 */
static int
read_polygons(struct PyMangleMask* self)
{
    int status=1;
    FILE* fptr=NULL;
    npy_intp npoly;
    char buff[_MANGLE_SMALL_BUFFSIZE];

    memset(buff,0,_MANGLE_SMALL_BUFFSIZE);
    
    if (self->verbose)
        fprintf(stderr,"reading polygon file: %s\n", self->filename);

    fptr = fopen(self->filename,"r");
    if (fptr==NULL) {
        status=0;
        PyErr_Format(PyExc_IOError, "Could open file: %s", self->filename);
        goto _read_polygons_errout;
    }

    if (2 != fscanf(fptr,"%ld %s", &npoly, buff)) {
        status = 0;
        PyErr_Format(PyExc_IOError, "Could not read number of polygons");
        goto _read_polygons_errout;
    }
    if (strcmp(buff,"polygons") != 0) {
        status = 0;
        PyErr_Format(PyExc_IOError, "Expected keyword 'polygons' but got '%s'", buff);
        goto _read_polygons_errout;
    }

    if (self->verbose)
        fprintf(stderr,"Expect %ld polygons\n", npoly);

    // get some metadata
    if (1 != fscanf(fptr,"%s", buff) ) {
        status=0;
        PyErr_Format(PyExc_IOError, "Error reading header keyword");
        goto _read_polygons_errout;
    }
    while (strcmp(buff,"polygon") != 0) {
        if (strcmp(buff,"snapped") == 0) {
            if (self->verbose) 
                fprintf(stderr,"\tpolygons are snapped\n");
            self->snapped=1;
        } else if (strcmp(buff,"balkanized") == 0) {
            if (self->verbose) 
                fprintf(stderr,"\tpolygons are balkanized\n");
            self->balkanized=1;
        } else if (strcmp(buff,"pixelization")==0) {
            // read the pixelization description, e.g. 9s
            if (1 != fscanf(fptr,"%s", buff)) {
                status=0;
                PyErr_Format(PyExc_IOError, "Error reading pixelization scheme");
                goto _read_polygons_errout;
            }
            if (self->verbose) 
                fprintf(stderr,"\tpixelization scheme: '%s'\n", buff);


            if (!get_pix_scheme(buff, &self->pixelres, &self->pixeltype)) {
                goto _read_polygons_errout;
            }
            if (self->verbose) {
                fprintf(stderr,"\t\tscheme: '%c'\n", self->pixeltype);
                fprintf(stderr,"\t\tres:     %ld\n", self->pixelres);
            }
        } else {
            status=0;
            PyErr_Format(PyExc_IOError, "Got unexpected header keyword: '%s'", buff);
            goto _read_polygons_errout;
        }
        if (1 != fscanf(fptr,"%s", buff) ) {
            status=0;
            PyErr_Format(PyExc_IOError, "Error reading header keyword");
            goto _read_polygons_errout;
        }
    }

    if (self->verbose)
        fprintf(stderr,"Allocating %ld polygons\n", npoly);
    self->poly_vec = PolygonVec_new(npoly);
    if (self->poly_vec == NULL) {
        status=0;
        goto _read_polygons_errout;
    }
    status = _read_polygons(self, fptr, buff);

_read_polygons_errout:
    fclose(fptr);
    return status;
}

static void
cleanup(struct PyMangleMask* self)
{
    self->poly_vec  = PolygonVec_free(self->poly_vec);
    self->pixel_vec = NpyIntpVec_free(self->pixel_vec);
    free(self->filename);
    self->filename=NULL;
}
static void
set_defaults(struct PyMangleMask* self)
{
    self->filename=NULL;
    self->poly_vec=NULL;
    self->pixelres=-1;
    self->pixeltype='\0';
    self->pixel_vec=NULL;

    self->snapped=0;
    self->balkanized=0;

    self->verbose=0;
}


static int
PyMangleMask_init(struct PyMangleMask* self, PyObject *args, PyObject *kwds)
{
    set_defaults(self);
    char* tmp_filename=NULL;
    if (!PyArg_ParseTuple(args, (char*)"si", &tmp_filename, &self->verbose)) {
        return -1;
    }
    self->filename = strdup(tmp_filename);

    if (!read_polygons(self)) {
        cleanup(self);
        return -1;
    }
    return 0;
}

static PyObject *
PyMangleMask_repr(struct PyMangleMask* self) {
    npy_intp npoly;
    npy_intp npix;

    npoly = (self->poly_vec != NULL) ? self->poly_vec->size : 0;
    npix = (self->pixel_vec != NULL) ? self->pixel_vec->size : 0;
    return PyString_FromFormat(
     "Mangle\n\tfile: %s\n\tnpoly: %ld\n\tpixeltype: '%c'\n\tpixelres: %ld\n\tnpix: %ld\n\tverbose: %d\n", 
            self->filename, npoly, self->pixeltype, self->pixelres, npix, self->verbose);
}




static void
PyMangleMask_dealloc(struct PyMangleMask* self)
{

    cleanup(self);

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    // old way, removed in python 3
    self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyMethodDef PyMangleMask_methods[] = {
    {NULL}  /* Sentinel */
};


static PyTypeObject PyMangleMaskType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_mangle.Mangle",             /*tp_name*/
    sizeof(struct PyMangleMask), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyMangleMask_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyMangleMask_repr,                         /*tp_repr*/
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
    "Mangle Class",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyMangleMask_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyMangleMask_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};

static PyMethodDef mangle_methods[] = {
    {NULL}  /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_mangle",      /* m_name */
        "Defines the Mangle class and some methods",  /* m_doc */
        -1,                  /* m_size */
        mangle_methods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_mangle(void) 
{
    PyObject* m;


    PyMangleMaskType.tp_new = PyType_GenericNew;

#if PY_MAJOR_VERSION >= 3
    if (PyType_Ready(&PyMangleMaskType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&moduledef);
    if (m==NULL) {
        return NULL;
    }

#else
    if (PyType_Ready(&PyMangleMaskType) < 0) {
        return;
    }
    m = Py_InitModule3("_mangle", mangle_methods, "Define Mangle type and methods.");
    if (m==NULL) {
        return;
    }
#endif

    Py_INCREF(&PyMangleMaskType);
    PyModule_AddObject(m, "Mangle", (PyObject *)&PyMangleMaskType);

    import_array();
}
