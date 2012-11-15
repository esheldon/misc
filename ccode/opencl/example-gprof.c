/* try to add some realistic overhead */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include "fmath.h"

void check_err(int err, const char* mess)
{
    if (err != CL_SUCCESS) {
        fprintf(stderr,"%s: ",mess);
        if (err==CL_OUT_OF_RESOURCES) {
            fprintf(stderr,"CL_OUT_OF_RESOURCES\n");
        } else if (err== CL_INVALID_WORK_GROUP_SIZE) {
            fprintf(stderr,"CL_INVALID_WORK_GROUP_SIZE\n");
        } else if (err==CL_INVALID_WORK_ITEM_SIZE) {
            fprintf(stderr,"CL_INVALID_WORK_ITEM_SIZE\n");
        } else if (err==CL_INVALID_GLOBAL_OFFSET) {
            fprintf(stderr,"CL_INVALID_GLOBAL_OFFSET\n");
        } else if (err==CL_OUT_OF_RESOURCES) {
            fprintf(stderr,"CL_OUT_OF_RESOURCES\n");
        } else if (err==CL_MEM_OBJECT_ALLOCATION_FAILURE) {
            fprintf(stderr,"CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
        } else if (err==CL_INVALID_EVENT_WAIT_LIST) {
            fprintf(stderr,"CL_INVALID_EVENT_WAIT_LIST\n");
        } else if (err==CL_OUT_OF_HOST_MEMORY) {
            fprintf(stderr,"CL_OUT_OF_HOST_MEMORY\n");
        } else if (err==CL_INVALID_PROGRAM_EXECUTABLE) {
            fprintf(stderr,"CL_INVALID_PROGRAM_EXECUTABLE\n");
        } else if (err==CL_INVALID_COMMAND_QUEUE) {
            fprintf(stderr,"CL_INVALID_COMMAND_QUEUE\n");
        } else if (err==CL_INVALID_KERNEL) {
            fprintf(stderr,"CL_INVALID_KERNEL\n");
        } else if (err==CL_INVALID_CONTEXT) {
            fprintf(stderr,"CL_INVALID_CONTEXT\n");
        } else if (err==CL_INVALID_KERNEL_ARGS) {
            fprintf(stderr,"CL_INVALID_KERNEL_ARGS\n");
        } else if (err==CL_INVALID_WORK_DIMENSION) {
            fprintf(stderr,"CL_INVALID_WORK_DIMENSION\n");
        } else {
            fprintf(stderr,"unknown: %d\n", err);
        }
        exit(EXIT_FAILURE);
    }
}

static char *
load_program_source(const char *filename)
{
    struct stat statbuf;
    FILE        *fh;
    char        *source;
    size_t nread=0;

    fh = fopen(filename, "r");
    if (fh == 0)
        return 0;

    stat(filename, &statbuf);
    source = (char *) malloc(statbuf.st_size + 1);
    nread=fread(source, statbuf.st_size, 1, fh);
    source[statbuf.st_size] = '\0';

    return source;
}

// we expect this many
#define NDEV 4

// Round Up Division function
size_t shrRoundUp(int group_size, int global_size) 
{
    int r = global_size % group_size;
    if(r == 0) 
    {
        return (size_t)global_size;
    } else 
    {
        return (size_t)(global_size + group_size - r);
    }
}


/* write and read the data to add some overhead */
cl_float *get_new_image(int nrow, int ncol)
{
    char fname[256];
    int irand=(int)(1000*drand48());

    sprintf(fname,"data/test-image-%d.dat",irand);
    FILE *fobj=fopen(fname,"w");

    fprintf(fobj,"%d %d\n", nrow, ncol);
    for (int row=0; row<nrow; row++) {
        for (int col=0; col<ncol; col++) {
            float val = 3 + 0.01*drand48();
            fprintf(fobj,"%.7g ", val);
        }
        fprintf(fobj,"\n");
    }
    fclose(fobj);


    fobj=fopen(fname,"r");
    int tnrow=0, tncol=0;
    int nread=0;
    nread += fscanf(fobj, "%d %d", &tnrow, &tncol);
    cl_float *image=calloc(tnrow*tncol,sizeof(cl_float));
    for (int row=0; row<tnrow; row++) {
        for (int col=0; col<tncol; col++) {
            nread += fscanf(fobj,"%f", &image[row*ncol + col]);
        }
    }
    fclose(fobj);

    remove(fname);
    return image;
}

void fill_rows_cols(int nwalkers, int nrow, int ncol, cl_float *rows, cl_float *cols)
{
    for (int iwalk=0; iwalk<nwalkers; iwalk++) {
        for (int row=0; row<nrow; row++) {
            for (int col=0; col<ncol; col++) {
                int idx = iwalk*(nrow*ncol) + row*ncol + col;
                rows[idx] = row;
                cols[idx] = col;
            }
        }
    }
}

// three gaussians
#define NPARS_PSF 18

int main(int argc, char** argv)
{

    if (argc < 3) {
        printf("exmaple-gprof ngauss nrepeat devnum\n");
        exit(1);
    }

    int ngauss=atoi(argv[1]);
    int nrepeat=atoi(argv[2]);

    int devnum=0;
    if (argc > 3) {
        devnum=atoi(argv[3]);
    }


    // Storage for the arrays.
    static cl_mem output;
    // OpenCL state
    static cl_command_queue queue;
    //static cl_kernel kernel;

    static cl_device_id device_ids[NDEV];
    static cl_context context;

    static cl_platform_id platform_id;


    cl_int nrow=25;
    cl_int ncol=25;
    float cenrow0=12.;
    float cencol0=12.;
    float e10 = 0.2;
    float e20 = -0.3;
    float T0 = 7.8;
    float counts0 = 1.0;

    int nelem=nrow*ncol;

    //int nwalkers=20;
    //int nsteps=600;
    int nwalkers=20;
    int nsteps=600;
    int npars=6;
    int npars_tot=nwalkers*npars;

    int ntot=nrow*ncol*nwalkers;

    cl_uint numPlatforms;
    cl_int err = CL_SUCCESS;

    clock_t t0,t1;

    int device_type=0;
    if (1) {
        device_type=CL_DEVICE_TYPE_GPU;
    } else {
        device_type=CL_DEVICE_TYPE_CPU;
    }

    //SETUP PLATFORM
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not get platform\n");
        exit(EXIT_FAILURE);
    }
    if(numPlatforms > 0)
    {
        //we have at least one
        //cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        cl_platform_id* platforms = calloc(numPlatforms, sizeof(cl_platform_id));
        err = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not get platform id\n");
            exit(EXIT_FAILURE);
        }

        fprintf(stderr,"Found %d platforms\n", numPlatforms);
        platform_id = platforms[0];
        //delete[] platforms;
        free(platforms);
    }
    else
        exit(0);
    //END PLATFORM

    //SETUP CONTEXT
    cl_context_properties cps[3] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform_id,
        0
    };

    context = clCreateContextFromType(
            cps,
            device_type,
            NULL,
            NULL,
            &err);
    //END CONTEXT

    int num_devices=0;
    err = clGetDeviceIDs(platform_id, device_type, NDEV, device_ids, &num_devices);
    fprintf(stderr,"found %d devices\n", num_devices);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not get device ids\n");
        exit(EXIT_FAILURE);
    }
    if (NDEV != num_devices) {
        printf("expected %d devices\n", NDEV);
        exit(1);
    }

    for (int i=0; i<num_devices; i++) {
        size_t len=0;
        cl_uint avail=0;
        cl_uint id=0;
        
        clGetDeviceInfo(device_ids[i], CL_DEVICE_AVAILABLE, sizeof(cl_uint), &avail, &len);
        clGetDeviceInfo(device_ids[i], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &id, &len);
        printf("device #: %d id: %d avail: %d\n", i, id, avail);
    }
    printf("choosing device %d\n", devnum);

    const char *kernel_file=NULL;
    char *kernel_source=NULL;
    if (ngauss==3) {
        kernel_file="kern3gauss.c";
    } else if (ngauss==6) {
        kernel_file="kern6gauss.c";
    } else if (ngauss==10) {
        kernel_file="kern10gauss.c";
    } else {
        printf("ngauss 3,6,10\n");
        exit(1);
    }
    printf("loading kernel: %s\n", kernel_file);
    kernel_source = load_program_source(kernel_file);
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernel_source , NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create program\n");
        exit(EXIT_FAILURE);
    }




    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(program, device_ids[devnum], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

    clGetDeviceInfo(device_ids[devnum], CL_DEVICE_NAME, sizeof(buffer), buffer, &len);
    cl_ulong memsize;
    clGetDeviceInfo(device_ids[devnum], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &memsize, &len);
    cl_uint nunits;
    clGetDeviceInfo(device_ids[devnum], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &nunits, &len);
    cl_ulong max_work_group_size;
    clGetDeviceInfo(device_ids[devnum], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), &max_work_group_size, &len);
    //cl_uint warp_size;
    //clGetDeviceInfo(device_ids[devnum], CL_NV_DEVICE_WARP_SIZE, sizeof(cl_uint), &warp_size, &len);
     
    printf("CL_DEVICE_NAME:                    '%s'\n", buffer);
    printf("CL_DEVICE_GLOBAL_MEM_SIZE:          %lu\n", memsize);
    // compute unit is a lump of hardware that executes 'work groups'
    printf("CL_DEVICE_MAX_COMPUTE_UNITS:        %u\n", nunits);
    // max number of items per work group
    printf("CL_DEVICE_MAX_WORK_GROUP_SIZE: %lu\n", max_work_group_size);
    //printf("CL_NV_DEVICE_WARP_SIZE:             %u\n", warp_size);

    size_t szLocalWorkSize = nrow;
    //size_t szLocalWorkSize = 512;
    // make sure multiple of 32
    int szLocalWorkSize0=256;
    szLocalWorkSize=shrRoundUp(szLocalWorkSize0, (int)szLocalWorkSize);
    // rounded up to the nearest multiple of the LocalWorkSize
    size_t szGlobalWorkSize = shrRoundUp((int)szLocalWorkSize, (int)ntot);

    printf("nrow: %d\n", nrow);
    printf("ncol %d\n", ncol);
    printf("setting nelem: %d\n", nelem);
    printf("setting ntot: %d\n", ntot);
    printf("setting local work size: %lu\n", szLocalWorkSize);
    printf("setting global work size: %lu\n", szGlobalWorkSize);
    printf("bytes global work size: %lu\n", szGlobalWorkSize*sizeof(float));



    //queue = clCreateCommandQueue(context, device_ids, 0, &err);
    queue = clCreateCommandQueue(context, 
                                 device_ids[devnum],
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 
                                 //0,
                                 &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create command queue\n");
        exit(EXIT_FAILURE);
    }



    //OPTIMIZATION OPTIONS FOUND AT http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html

    err = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    /*
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not build program\n");
        exit(EXIT_FAILURE);
    }
    */
    if (err != CL_SUCCESS)
    {
        size_t length;
        char build_log[50000];
        //char build_log[256]={0};
        //printf("%s\n", block_source);
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_ids[devnum], CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
        printf("%lu\n", length);
        printf("%s\n", build_log);
        return EXIT_FAILURE;
    }

 



    //SETUP KERNEL
    cl_kernel kernel = clCreateKernel(program, "gmix", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create kernel\n");
        exit(EXIT_FAILURE);
    }


    clReleaseProgram(program); // no longer needed


    double tstandard=0;
    double topencl=0;



    cl_float *image=NULL;

    srand48(10);
    t0=clock();

    /*
    if (1) {
        for (int iwalk=0; iwalk<nwalkers; iwalk++) {
            int widx=iwalk*6;
            pars[widx+0] = cenrow0 + 0.01*(drand48()-0.5);
            pars[widx+1] = cencol0 + 0.01*(drand48()-0.5);
            pars[widx+2] = e10 + 0.01*(drand48()-0.5);
            pars[widx+3] = e20 + 0.01*(drand48()-0.5);
            pars[widx+4] = T0 + 0.01*(drand48()-0.5);
            pars[widx+5] = counts0 + 0.01*(drand48()-0.5);
        }
    }
    */

    cl_float *pars=NULL;
    cl_mem pars_in=NULL;

    pars=calloc(npars_tot,sizeof(cl_float));

    // values don't matter
    cl_float psf_pars[NPARS_PSF] = {
        0.33,-1., -1., 1.0, 0.0, 1.0,
        0.33,-1., -1., 1.0, 0.0, 1.0,
        0.33,-1., -1., 1.0, 0.0, 1.0};

    pars_in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  
            sizeof(cl_float)*npars_tot, pars, &err);
    check_err(err, "could not create pars buffer");

    printf("processing %dx%d image %d walkers %d steps nrepeat %d\n",
           nrow,ncol,nwalkers,nsteps,nrepeat);
    for (int rep=0; rep<nrepeat; rep++) {

        // we can probably instead re-use rows so this
        // is overkill simulating overhead
        image=get_new_image(nrow,ncol);

        cl_float *data_from_gpu = calloc(szGlobalWorkSize, sizeof(cl_float));
        cl_float *rows=calloc(szGlobalWorkSize,sizeof(cl_float));
        cl_float *cols=calloc(szGlobalWorkSize,sizeof(cl_float));

        fill_rows_cols(nwalkers, nrow, ncol, rows, cols);

        cl_mem psf_pars_in = clCreateBuffer(context,  
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*NPARS_PSF, psf_pars, &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create psf buffer\n");
            exit(EXIT_FAILURE);
        }


        err=0;
        cl_mem image_in = clCreateBuffer(context,  
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*nrow*ncol, image, &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create image buffer\n");
            exit(EXIT_FAILURE);
        }



        cl_mem rows_in = clCreateBuffer(context,  
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*szGlobalWorkSize, rows, &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create rows buffer\n");
            exit(EXIT_FAILURE);
        }
        cl_mem cols_in = clCreateBuffer(context,  
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*szGlobalWorkSize, cols, &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create cols buffer\n");
            exit(EXIT_FAILURE);
        }

        output = clCreateBuffer(context,  
                CL_MEM_WRITE_ONLY,  sizeof(cl_float)*szGlobalWorkSize, NULL, &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create buffer\n");
            exit(EXIT_FAILURE);
        }

        err =  clSetKernelArg(kernel, 0, sizeof(cl_int), &ntot);
        err |= clSetKernelArg(kernel, 1, sizeof(cl_int), &nrow);
        err |= clSetKernelArg(kernel, 2, sizeof(cl_int), &ncol);
        err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &image_in);
        err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &rows_in);
        err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &cols_in);
        err |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &output);
        err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &psf_pars_in);
        err |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &pars_in); // the buffer values will change as we go
        check_err(err, "could not set kernel args");

        for (int step=0; step<nsteps; step++) {

            for (int iwalk=0; iwalk<nwalkers; iwalk++) {
                int widx=iwalk*6;
                pars[widx+0] = cenrow0 + 0.01*(drand48()-0.5);
                pars[widx+1] = cencol0 + 0.01*(drand48()-0.5);
                pars[widx+2] = e10 + 0.01*(drand48()-0.5);
                pars[widx+3] = e20 + 0.01*(drand48()-0.5);
                pars[widx+4] = T0 + 0.01*(drand48()-0.5);
                pars[widx+5] = counts0 + 0.01*(drand48()-0.5);
            }
            err= clEnqueueWriteBuffer(queue,
                    pars_in,
                    CL_TRUE, // blocking write
                    0, // zero offset
                    sizeof(cl_float)*npars_tot,
                    pars,
                    0, // no events to wait for
                    NULL, // no events to wait for
                    NULL);  // no even associated with this call

            err = clEnqueueNDRangeKernel(queue, 
                    kernel, 
                    1, 
                    NULL, 
                    &szGlobalWorkSize, 
                    &szLocalWorkSize, 
                    0, 
                    NULL, 
                    NULL);

            check_err(err,"error executing kernel");

        }
        clReleaseMemObject(psf_pars_in);
        clReleaseMemObject(image_in);
        clReleaseMemObject(rows_in);
        clReleaseMemObject(cols_in);
        clReleaseMemObject(output);

        free(image);
        free(rows);
        free(cols);
        free(data_from_gpu);
    }

    clReleaseMemObject(pars_in);

    t1=clock();
    topencl = ((double)(t1-t0))/CLOCKS_PER_SEC;


    printf("\n%d gaussians\n", ngauss);
    printf("-----------------------------------\n");
    printf("time for GPU: %lf\n", topencl);
    printf("time per repeat: %lf\n", topencl/nrepeat);

    free(pars);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
