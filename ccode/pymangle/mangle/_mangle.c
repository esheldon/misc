#define NPY_NO_DEPRECATED_API

#include <string.h>
#include <math.h>
#include <Python.h>
#include "numpy/arrayobject.h" 

#define _MANGLE_SMALL_BUFFSIZE 25
#define D2R  0.017453292519943295
#define R2D  57.295779513082323

struct Point {
    double theta;
    double phi;
    double x;
    double y;
    double z;
};
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

struct NpyIntpStack {
    npy_intp size;
    npy_intp allocated_size;
    npy_intp* data;
};

struct PixelListVec {
    npy_intp size;
    struct NpyIntpStack** data;
};

struct PyMangleMask {
    PyObject_HEAD

    char* filename;
    struct PolygonVec* poly_vec;

    double total_area;
    npy_intp pixelres;
    npy_intp maxpix;
    char pixeltype;
    struct PixelListVec* pixel_list_vec;

    int snapped;
    int balkanized;

    int verbose;

    char buff[_MANGLE_SMALL_BUFFSIZE];

    FILE* fptr;

    // for error messages
    npy_intp current_poly_index;
};


static void set_point_from_radec(struct Point* pt, double ra, double dec) {

    double stheta=0;

    if (pt != NULL) {
        pt->phi = ra*D2R;
        pt->theta = (90.0-dec)*D2R;

        stheta = sin(pt->theta);
        pt->x = stheta*cos(pt->phi);
        pt->y = stheta*sin(pt->phi);
        pt->z = cos(pt->theta); 
    }
}

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

    self=calloc(1, sizeof(struct PolygonVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Polygon vector");
        return NULL;
    }
    // pointers will be NULL (0)
    self->data = calloc(n, sizeof(struct Polygon));
    if (self->data == NULL) {
        free(self);
        PyErr_Format(PyExc_MemoryError, "Could not allocate Polygon vector %ld", n);
        return NULL;
    }

    self->size = n;
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



static struct NpyIntpStack* 
NpyIntpStack_new(void) 
{
    struct NpyIntpStack* self=NULL;
    npy_intp start_size=1;

    self=calloc(1, sizeof(struct NpyIntpStack));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp stack");
        return NULL;
    }
    self->data = calloc(start_size, sizeof(npy_intp));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp stack");
        return NULL;
    }
    self->size = 0;
    self->allocated_size=start_size;
    return self;
}

static void
NpyIntpStack_realloc(struct NpyIntpStack* self, npy_intp newsize)
{
    npy_intp oldsize=0;
    npy_intp* newdata=NULL;
    npy_intp elsize=0;
    npy_intp num_new_bytes=0;

    oldsize = self->allocated_size;
    if (newsize > oldsize) {
        elsize = sizeof(npy_intp);

        newdata = realloc(self->data, newsize*elsize);
        if (newdata == NULL) {
            printf("failed to reallocate\n");
            exit(EXIT_FAILURE);
        }

        // the allocated size is larger.  make sure to initialize the new
        // memory region.  This is the area starting from index [oldsize]
        num_new_bytes = (newsize-oldsize)*elsize;
        memset(&newdata[oldsize], 0, num_new_bytes);

        self->data = newdata;
        self->allocated_size = newsize;
    }


}

void NpyIntpStack_push(struct NpyIntpStack* self, npy_intp val) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (self->size == self->allocated_size) {
        NpyIntpStack_realloc(self, self->size*2);
    }

    self->size++;
    self->data[self->size-1] = val;
}

static struct NpyIntpStack* 
NpyIntpStack_free(struct NpyIntpStack* self) 
{
    if (self != NULL) {
        free(self->data);
        free(self);
    }
    return self;
}

static struct PixelListVec* 
PixelListVec_new(npy_intp n)
{
    struct PixelListVec* self=NULL;
    npy_intp i=0;

    if (n <= 0) {
        PyErr_Format(PyExc_MemoryError, "Vectors must be size > 0, got %ld", n);
        return NULL;
    }
    self=calloc(1, sizeof(struct PixelListVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate pixel list vector");
        return NULL;
    }
    // array of pointers. The pointers will be NULL
    self->data = calloc(n, sizeof(struct NpyIntpStack*));
    if (self->data == NULL) {
        free(self);
        PyErr_Format(PyExc_MemoryError, "Could not allocate %ld pixel list pointers", n);
        return NULL;
    }

    for (i=0; i<n; i++) {
        self->data[i] = NpyIntpStack_new();
    }
    self->size=n;
    return self;
}

static struct PixelListVec* 
PixelListVec_free(struct PixelListVec* self)
{
    npy_intp i=0;
    struct NpyIntpStack* s=NULL;
    if (self != NULL) {
        for (i=0; i<self->size; i++) {
            s = self->data[i];
            if (s != NULL) {
                s=NpyIntpStack_free(s);
            }
        }
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
scan_expected_value(struct PyMangleMask* self, const char* expected_value)
{
    int status=1, res=0;

    res = fscanf(self->fptr, "%s", self->buff);
    if (1 != res || 0 != strcmp(self->buff,expected_value)) {
        status=0;
        PyErr_Format(PyExc_IOError, 
                "Failed to read expected string '%s' for polygon %ld", 
                expected_value, self->current_poly_index);
    }
    return status;
}

/* 
 * parse the polygon "header" for the index poly_index
 *
 * this is after reading the initial 'polygon' token
 */
static int
read_polygon_header(struct PyMangleMask* self, struct Polygon* ply, npy_intp* ncaps)
{
    int status=1;
    int got_pixel=0;
    char kwbuff[20];

    if (1 != fscanf(self->fptr, "%ld", &ply->poly_id)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read polygon id for polygon %ld", self->current_poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(self, "(")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(self->fptr,"%ld",ncaps)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read ncaps for polygon %ld", self->current_poly_index);
        goto _read_polygon_header_errout;
    }


    if (!scan_expected_value(self, "caps,")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(self->fptr,"%lf",&ply->weight)) {
        status=0;
        PyErr_Format(PyExc_IOError, "Failed to read weight for polygon %ld", self->current_poly_index);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(self, "weight,")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    // pull in the value and keyword
    if (2 != fscanf(self->fptr,"%s %s",self->buff, kwbuff)) {
        status=0;
        PyErr_Format(PyExc_IOError, 
                "Failed to read value and keyword (pixel,str) for polygon %ld", 
                self->current_poly_index);
        goto _read_polygon_header_errout;
    }

    if (0 == strcmp(kwbuff,"pixel,")) {
        // we read a pixel value into self->buff
        got_pixel=1;
        sscanf(self->buff, "%ld", &ply->pixel_id);
    } else {
        // we probably read the area
        if (0 != strcmp(kwbuff,"str):")) {
            status=0;
            PyErr_Format(PyExc_IOError, "Expected str): keyword at polygon %ld, got %s", 
                    self->current_poly_index, kwbuff);
            goto _read_polygon_header_errout;
        }
        sscanf(self->buff, "%lf", &ply->area);
    }
    if (got_pixel) {
        if (1 != fscanf(self->fptr,"%lf",&ply->area)) {
            status=0;
            PyErr_Format(PyExc_IOError, "Failed to read area for polygon %ld", self->current_poly_index);
            goto _read_polygon_header_errout;
        }
        if (!scan_expected_value(self, "str):")) {
            status=0;
            goto _read_polygon_header_errout;
        }
    }

    if (ply->pixel_id > self->maxpix) {
        self->maxpix = ply->pixel_id;
    }
    self->total_area += ply->area;

    if (self->verbose > 1) {
        fprintf(stderr,
          "polygon %ld: poly_id %ld ncaps: %ld weight: %g pixel: %ld area: %g\n", 
          self->current_poly_index, ply->poly_id, *ncaps, ply->weight, ply->pixel_id, ply->area);
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
read_polygon(struct PyMangleMask* self, struct Polygon* ply) {
    int status=1;
    struct Cap* cap=NULL;

    npy_intp ncaps=0, i=0, nres=0;

    if (!read_polygon_header(self, ply, &ncaps)) {
        status=0;
        goto _read_single_polygon_errout;
    }

    ply->cap_vec = CapVec_new(ncaps);
    if (ply->cap_vec == NULL) {
        status=0;
        goto _read_single_polygon_errout;
    }

    cap = &ply->cap_vec->data[0];
    for (i=0; i<ncaps; i++) {
        nres=0;
        nres += fscanf(self->fptr,"%lf", &cap->x);
        nres += fscanf(self->fptr,"%lf", &cap->y);
        nres += fscanf(self->fptr,"%lf", &cap->z);
        nres += fscanf(self->fptr,"%lf", &cap->cm);

        if (nres != 4) {
            status=0;
            PyErr_Format(PyExc_IOError, 
                         "Failed to read cap number %ld for polygon %ld", i, self->current_poly_index);
            goto _read_single_polygon_errout;
        }
        if (self->verbose > 2) {
            fprintf(stderr, 
               "    %.16g %.16g %.16g %.16g\n", cap->x, cap->y, cap->z, cap->cm);
        }

        cap++;
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
_read_polygons(struct PyMangleMask* self)
{
    int status=1;

    npy_intp npoly=0, i=0;

    npoly=self->poly_vec->size;

    if (self->verbose)
        fprintf(stderr,"reading %ld polygons\n", npoly);
    for (i=0; i<npoly; i++) {
        // buff comes in with 'polygon'
        if (0 != strcmp(self->buff,"polygon")) {
            status=0;
            PyErr_Format(PyExc_IOError, 
                    "Expected first token in poly to read 'polygon', got '%s'", 
                    self->buff);
            goto _read_some_polygons_errout;
        }

        // just for error messages and verbosity
        self->current_poly_index = i;

        status = read_polygon(self, &self->poly_vec->data[i]);
        if (status != 1) {
            break;
        }

        if (i != (npoly-1)) {
            if (1 != fscanf(self->fptr,"%s",self->buff)) {
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
    npy_intp npoly;

    
    if (self->verbose)
        fprintf(stderr,"reading polygon file: %s\n", self->filename);

    self->fptr = fopen(self->filename,"r");
    if (self->fptr==NULL) {
        status=0;
        PyErr_Format(PyExc_IOError, "Could open file: %s", self->filename);
        goto _read_polygons_errout;
    }

    if (2 != fscanf(self->fptr,"%ld %s", &npoly, self->buff)) {
        status = 0;
        PyErr_Format(PyExc_IOError, "Could not read number of polygons");
        goto _read_polygons_errout;
    }
    if (0 != strcmp(self->buff,"polygons")) {
        status = 0;
        PyErr_Format(PyExc_IOError, "Expected keyword 'polygons' but got '%s'", self->buff);
        goto _read_polygons_errout;
    }

    if (self->verbose)
        fprintf(stderr,"Expect %ld polygons\n", npoly);

    // get some metadata
    if (1 != fscanf(self->fptr,"%s", self->buff) ) {
        status=0;
        PyErr_Format(PyExc_IOError, "Error reading header keyword");
        goto _read_polygons_errout;
    }
    while (0 != strcmp(self->buff,"polygon")) {
        if (0 == strcmp(self->buff,"snapped")) {
            if (self->verbose) 
                fprintf(stderr,"\tpolygons are snapped\n");
            self->snapped=1;
        } else if (0 == strcmp(self->buff,"balkanized")) {
            if (self->verbose) 
                fprintf(stderr,"\tpolygons are balkanized\n");
            self->balkanized=1;
        } else if (0 == strcmp(self->buff,"pixelization")) {
            // read the pixelization description, e.g. 9s
            if (1 != fscanf(self->fptr,"%s", self->buff)) {
                status=0;
                PyErr_Format(PyExc_IOError, "Error reading pixelization scheme");
                goto _read_polygons_errout;
            }
            if (self->verbose) 
                fprintf(stderr,"\tpixelization scheme: '%s'\n", self->buff);


            if (!get_pix_scheme(self->buff, &self->pixelres, &self->pixeltype)) {
                goto _read_polygons_errout;
            }
            if (self->verbose) {
                fprintf(stderr,"\t\tscheme: '%c'\n", self->pixeltype);
                fprintf(stderr,"\t\tres:     %ld\n", self->pixelres);
            }
        } else {
            status=0;
            PyErr_Format(PyExc_IOError, "Got unexpected header keyword: '%s'", self->buff);
            goto _read_polygons_errout;
        }
        if (1 != fscanf(self->fptr,"%s", self->buff) ) {
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

    status = _read_polygons(self);

_read_polygons_errout:
    fclose(self->fptr);
    return status;
}

static void
cleanup(struct PyMangleMask* self)
{
    self->poly_vec  = PolygonVec_free(self->poly_vec);
    self->pixel_list_vec = PixelListVec_free(self->pixel_list_vec);
    free(self->filename);
    self->filename=NULL;
}
static void
set_defaults(struct PyMangleMask* self)
{
    self->filename=NULL;
    self->poly_vec=NULL;

    self->total_area=0.0;
    self->pixelres=-1;
    self->maxpix=-1;
    self->pixeltype='u';
    self->pixel_list_vec=NULL;

    self->snapped=0;
    self->balkanized=0;

    self->verbose=0;

    memset(self->buff, 0, _MANGLE_SMALL_BUFFSIZE);

    self->fptr=NULL;
}

static int
set_pixel_map(struct PyMangleMask* self)
{
    int status=1;
    struct Polygon* ply=NULL;
    npy_intp ipoly=0;

    if (self->pixelres >= 0) {
        if (self->verbose) {
            fprintf(stderr,"Allocating %ld in PixelListVec\n", 
                    self->maxpix+1);
        }
        self->pixel_list_vec = PixelListVec_new(self->maxpix+1);
        if (self->pixel_list_vec == NULL) {
            status = 0;
            goto _set_pixel_map_errout;
        } else {
            if (self->verbose)
                fprintf(stderr,"Filling pixel map\n");
            ply=&self->poly_vec->data[0];
            for (ipoly=0; ipoly<self->poly_vec->size; ipoly++) {
                NpyIntpStack_push(self->pixel_list_vec->data[ply->pixel_id], ipoly);
                if (self->verbose > 2)
                    fprintf(stderr,"Adding poly %ld to pixel map at %ld (%ld)\n",
                            ipoly,ply->pixel_id,self->pixel_list_vec->data[ply->pixel_id]->size);
                ply++;
            }
        }
    }
_set_pixel_map_errout:

    return status;
}

/*
 * Initalize the mangle mask.  Read the file and, if pixelized, 
 * set the pixel mask
 */

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

    if (!set_pixel_map(self)) {
        cleanup(self);
        return -1;
    }
    return 0;
}

/*
 * we use sprintf since PyString_FromFormat doesn't accept floating point types
 */

static PyObject *
PyMangleMask_repr(struct PyMangleMask* self) {
    npy_intp npoly;
    npy_intp npix;
    char buff[255];

    npoly = (self->poly_vec != NULL) ? self->poly_vec->size : 0;
    npix = (self->pixel_list_vec != NULL) ? self->pixel_list_vec->size : 0;

    sprintf(buff,
            "Mangle\n\tfile: %s\n\tarea: %g sqdeg\n\tnpoly: %ld\n\t"
            "pixeltype: '%c'\n\tpixelres: %ld\n\tnpix: %ld\n\tverbose: %d\n", 
            self->filename, self->total_area*R2D*R2D, 
            npoly, self->pixeltype, self->pixelres, npix, self->verbose);
    return PyString_FromString(buff);
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

static int
is_in_cap(struct Cap* cap, struct Point* pt)
{
    int incap=0;
    double cdot;

    cdot = 1.0 - cap->x*pt->x - cap->y*pt->y - cap->z*pt->z;
    if (cap->cm < 0.0) {
        incap = cdot > (-cap->cm);
    } else {
        incap = cdot < cap->cm;
    }

    return incap;
}
static int
is_in_poly(struct Polygon* ply, struct Point* pt)
{
    npy_intp i=0;
    struct Cap* cap=NULL;

    int inpoly=1;


    cap = &ply->cap_vec->data[0];
    for (i=0; i<ply->cap_vec->size; i++) {
        inpoly = inpoly && is_in_cap(cap, pt);
        if (!inpoly) {
            break;
        }
        cap++;
    }
    return inpoly;
}


/*
 * Get the pixel number of the input point for the
 * simple scheme
 */

static npy_intp
get_pixel_simple(struct PyMangleMask* self, struct Point* pt)
{
    npy_intp pix=0;

    npy_intp i=0;
    npy_intp ps=0, p2=1;
    double cth=0;
    npy_intp n=0, m=0;
    if (self->pixelres > 0) {
        for (i=0; i<self->pixelres; i++) { // Work out # pixels/dim and start pix.
            p2  = p2<<1;
            ps += (p2/2)*(p2/2);
        }
      cth = cos(pt->theta);
      n   = (cth==1.0) ? 0: (npy_intp) ( ceil( (1.0-cth)/2 * p2 )-1 );
      m   = (npy_intp) ( floor( (pt->phi/2./M_PI)*p2 ) );
      pix = p2*n+m + ps;

    }
    return pix;
}
/*
 * check the point against a pixelized mask.  If found, will return the
 * id and weight.  These default to -1 and 0
 *
 */
static int
check_point_pixelized(struct PyMangleMask* self, 
                      struct Point* pt, npy_intp* poly_id, double* weight)
{
    int status=1;
    npy_intp pix=0, i=0, ipoly=0;
    npy_intp* iptr=NULL;
    struct NpyIntpStack* pstack=NULL;
    struct Polygon* ply=NULL;

    *poly_id=-1;
    *weight=0.0;

    if (self->pixeltype == 's') {
        fprintf(stderr,"getting pixel id\n");
        pix = get_pixel_simple(self, pt);

        fprintf(stderr,"setting stack\n");
        // this is a stack holding indices into the polygon vector
        pstack = self->pixel_list_vec->data[pix];

        fprintf(stderr,"looping stack\n");
        for (i=0; i<pstack->size; i++) {
            ipoly = pstack->data[i];
            ply = &self->poly_vec->data[ipoly];

            if (is_in_poly(ply, pt)) {
                *poly_id=ply->poly_id;
                *weight=ply->weight;
                break;
            }
        }
    } else {
        status=0;
        PyErr_Format(PyExc_IOError, "Unsupported pixelization scheme: '%c'",self->pixeltype);
    }

    return status;
}

/*
 * check the point against the mask.  If found, will return the
 * id and weight.  These default to -1 and 0
 *
 * this version does not use pixelization
 */
static int
check_point(struct PyMangleMask* self, 
            struct Point* pt, npy_intp* poly_id, double* weight)
{
    int status=1;
    npy_intp i=0;
    struct Polygon* ply=NULL;

    *poly_id=-1;
    *weight=0.0;

    // check every pixel until a match is found
    // assuming snapped so no overlapping polygons.
    ply = &self->poly_vec->data[0];
    for (i=0; i<self->poly_vec->size; i++) {
        if (is_in_poly(ply, pt)) {
            *poly_id=ply->poly_id;
            *weight=ply->weight;
            break;
        }
        ply++;
    }

    return status;
}



static PyObject*
PyMangleMask_test(struct PyMangleMask* self)
{
    int status=1;
    struct Point pt;
    double ra=200;
    double dec=0;

    npy_intp poly_id=0;
    double weight=0;

    set_point_from_radec(&pt, ra, dec);

    if (self->pixelres == -1) {
        status=check_point(self, &pt, &poly_id, &weight);
    } else {
        status=check_point_pixelized(self, &pt, &poly_id, &weight);
    }

    if (status != 1) {
        return NULL;
    }

    fprintf(stderr,"ra: %g dec: %g\n",ra,dec);
    fprintf(stderr,"x: %g y: %g z: %g\n",pt.x, pt.y, pt.z);
    fprintf(stderr,"poly_id: %ld weight: %g\n", poly_id, weight);

    Py_RETURN_NONE;
}

static PyMethodDef PyMangleMask_methods[] = {
    {"test",             (PyCFunction)PyMangleMask_test,             METH_NOARGS,  "run a test."},
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
