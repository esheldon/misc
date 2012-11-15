/* this one is timing.  This is more optimized than 2*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <CL/opencl.h>
#include "fmath.h"

//simulate 3 gaussians by doing the calculations 3 times
const char *kernel_source =
"__kernel void gmix(int nelem, \n"
"                   float cenrow, \n"
"                   float cencol, \n"
"                   float idet, \n"
"                   float irr, \n"
"                   float irc, \n"
"                   float icc, \n"
"                   __constant float *image, \n"
"                   __constant float *rows, \n"
"                   __constant float *cols, \n"
"                   global float *output)                            \n"
"{                                                                     \n"
"   int idx = get_global_id(0);                                        \n"
"   if (idx >= nelem)                            \n"
"       return;                                  \n"
"   float tmp=0;                                 \n"
"   float row = rows[idx];                       \n"
"   float col = cols[idx];                       \n"
"   float imval = image[idx];                    \n"
"   float u = row-cenrow;                        \n" 
"   float v = col-cencol;                        \n"
"   float chi2=icc*u*u + irr*v*v - 2.0*irc*u*v;  \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"
"\n"
"   u = row-1.1*cenrow;                              \n" 
"   v = col-1.1*cencol;                              \n"
"   chi2=1.01*icc*u*u + 0.98*irr*v*v - .999*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"
"\n"
"   u = row-0.99*cenrow;                              \n" 
"   v = col-0.99*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-0.97*cenrow;                              \n" 
"   v = col-0.93*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-1.01*cenrow;                              \n" 
"   v = col-1.03*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-.9991*cenrow;                              \n" 
"   v = col-1.0009*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-.983*cenrow;                              \n" 
"   v = col-1.31*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-0.993*cenrow;                              \n" 
"   v = col-0.99999*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-1.11*cenrow;                              \n" 
"   v = col-1.14*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"

"\n"
"   u = row-.975*cenrow;                              \n" 
"   v = col-1.00*cencol;                              \n"
"   chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;    \n"
"   chi2 *= idet;                                \n"
"   tmp += exp( -0.5*chi2 );  \n"



"\n"
"   output[idx] = tmp-imval;  \n"

"}                                                                     \n";
//END KERNEL
const char *kernel_source_old =
"__kernel void simple(                                                   \n"
"   global float* output, int nel)                                       \n"
"{                                                                       \n"
"   int index = get_global_id(0);                                        \n"
"   if (index >= nel)                                                    \n"
"       return;                                                          \n"
"   output[index] = exp( 0.5*log((float)index));                         \n"
"}                                                                       \n";


#define NWALKERS 20
//#define NWALKERS 32
//#define NWALKERS 1

// Storage for the arrays.
static cl_mem output[NWALKERS];
// OpenCL state
static cl_command_queue queues[NWALKERS];
static cl_kernel kernel;

// we expect this many
#define NDEV 4
static cl_device_id device_ids[NDEV];
static cl_context context;

static cl_platform_id platform_id;

static const int nrow=25;
static const int ncol=25;

void compare_data(int nrow, int ncol, float *gpudata, float *cpudata)
{
    int ntot=nrow*ncol;
    float maxdiff=0.;
    float diff;
    for (int i=0; i<ntot; i++) {
        diff=fabs( gpudata[i]-cpudata[i]);
        if (diff > maxdiff) {
            maxdiff=diff;
        }
    }
    printf("max diff: %.16g\n", maxdiff);
}
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

void do_c_map(int nrow, 
              int ncol, 
              float cenrow,
              float cencol,
              float idet,
              float irr,
              float irc,
              float icc,
              float *data)
{
    for (int row=0; row<nrow; row++) {
        for (int col=0; col<ncol; col++) {

            int idx=row*nrow + col;
            float u = row-cenrow;
            float v = col-cencol;
            float chi2=icc*u*u + irr*v*v - 2.0*irc*u*v;
            float tmp=0;
            chi2 *= idet;
            tmp = expd( -0.5*chi2 );

            u = row-1.1*cenrow;
            v = col-1.1*cencol;
            chi2=1.01*icc*u*u + 0.98*irr*v*v - .999*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-0.99*cenrow;
            v = col-0.99*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-0.97*cenrow;
            v = col-0.93*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

 
            u = row-1.01*cenrow;
            v = col-1.03*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );


            u = row-0.9991*cenrow;
            v = col-1.0009*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-0.983*cenrow;
            v = col-1.31*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-0.993*cenrow;
            v = col-0.99999*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-1.11*cenrow;
            v = col-1.14*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            u = row-0.975*cenrow;
            v = col-1.00*cencol;
            chi2=1.1*icc*u*u + .979*irr*v*v - 1.001*2.0*irc*u*v;
            chi2 *= idet;
            tmp += expd( -0.5*chi2 );

            data[idx] = tmp;

        }
    }
}

//END PROFILING

int main(int argc, char** argv)
{

    if (argc < 4) {
        printf("%s nrepeat docpu device\n", argv[0]);
        exit(1);
    }

    int nrepeat=atoi(argv[1]);
    int docpu=atoi(argv[2]);
    int devnum=atoi(argv[3]);

    float cenrow0=20.;
    float cencol0=20.;
    float irr0=2.;
    float irc0=0.;
    float icc0=3.;

    int nelem=nrow*ncol;

    // both 100 and 256 gave same.  Doesn't seem to matter much
    //size_t szLocalWorkSize = 10;
    //size_t szLocalWorkSize = 256;
    //size_t szLocalWorkSize = 1024;

    //size_t szLocalWorkSize = 64;
    //size_t szGlobalWorkSize = shrRoundUp((int)szLocalWorkSize, (int)(nrow*ncol));  // rounded up to the nearest multiple of the LocalWorkSize

    cl_uint numPlatforms;
    cl_int err = CL_SUCCESS;

    clock_t t0,t1;
    // burnin + steps
    int nsteps=600;
    //int nrepeat=500;
    //int nsteps=20*500*600;
    //int nrepeat=100;

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

    //int devnum=-1;
    for (int i=0; i<num_devices; i++) {
        size_t len=0;
        cl_uint avail=0;
        cl_uint id=0;
        
        clGetDeviceInfo(device_ids[i], CL_DEVICE_AVAILABLE, sizeof(cl_uint), &avail, &len);
        clGetDeviceInfo(device_ids[i], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &id, &len);
        printf("device #: %d id: %d avail: %d\n", i, id, avail);
        /*
        if (avail && devnum==-1) {
            //devnum=i;
            //break;
        }
        */
    }
    /*
    if (devnum==-1) {
        printf("no available devices\n");
        exit(1);
    }
    */
    printf("choosing device %d\n", devnum);

    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source , NULL, &err);
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
    printf("CL_DEVICE_MAX_WORK_WORK_GROUP_SIZE: %lu\n", max_work_group_size);
    //printf("CL_NV_DEVICE_WARP_SIZE:             %u\n", warp_size);

    size_t szLocalWorkSize = nrow;
    //size_t szLocalWorkSize = 512;
    // make sure multiple of 32
    szLocalWorkSize=shrRoundUp((int)32, (int)szLocalWorkSize);
    // rounded up to the nearest multiple of the LocalWorkSize
    size_t szGlobalWorkSize = shrRoundUp((int)szLocalWorkSize, (int)(nrow*ncol));

    printf("nrow: %d\n", nrow);
    printf("ncol %d\n", ncol);
    printf("setting nelem: %d\n", nelem);
    printf("setting local work size: %lu\n", szLocalWorkSize);
    printf("setting global work size: %lu\n", szGlobalWorkSize);


    cl_float *data_from_gpu = calloc(szGlobalWorkSize, sizeof(cl_float));
    cl_float *data_from_cpu = calloc(nrow*ncol, sizeof(cl_float));

    cl_float *rows=calloc(szGlobalWorkSize,sizeof(cl_float));
    cl_float *cols=calloc(szGlobalWorkSize,sizeof(cl_float));
    cl_float *image=calloc(szGlobalWorkSize,sizeof(cl_float));
    for (int row=0; row<nrow; row++) {
        for (int col=0; col<ncol; col++) {
            rows[row*nrow + col] = row;
            cols[row*nrow + col] = col;
            image[row*nrow +  col] = 3;
        }
    }



    //queue = clCreateCommandQueue(context, device_ids, 0, &err);

    for (int i=0; i<NWALKERS; i++) {
        queues[i] = clCreateCommandQueue(context, 
                                         device_ids[devnum],
                                         CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 
                                         &err);
    }
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create command queue\n");
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

    // we copy because we want the memory to be zerod
    // read-write because we will reduce it!
    for (int i=0; i<NWALKERS; i++) {
        output[i] = clCreateBuffer(context,  
            CL_MEM_READ_WRITE,  sizeof(cl_float)*szGlobalWorkSize, NULL, &err);
    }
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create buffer\n");
        exit(EXIT_FAILURE);
    }

    //OPTIMIZATION OPTIONS FOUND AT http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html

    err = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not build program\n");
        exit(EXIT_FAILURE);
    }



    //SETUP KERNEL
    cl_kernel kernel = clCreateKernel(program, "gmix", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create kernel\n");
        exit(EXIT_FAILURE);
    }


    clReleaseProgram(program); // no longer needed


    printf("processing %dx%d image %d walkers %d steps nrepeat %d\n",
           nrow,ncol,NWALKERS,nsteps,nrepeat);
    double tstandard=0;
    double topencl=0;

    srand48(10);
    err =  clSetKernelArg(kernel, 0, sizeof(cl_int), (void*)&nelem);
    err |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &rows_in);
    err |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &cols_in);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not set kernel args\n");
        exit(EXIT_FAILURE);
    }

    cl_event events[NWALKERS];

    t0=clock();
    clock_t t0wait=0,t1wait=0;
    double twait=0;
    clock_t t0enq=0,t1enq=0;
    double tenq=0;

    for (int rep=0; rep<nrepeat; rep++) {

        // each repeate represents pushing a new image in
        err=0;
        cl_mem image_in = clCreateBuffer(context,  
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*szGlobalWorkSize, image, &err);
        err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &image_in);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"could not create image buffer\n");
            exit(EXIT_FAILURE);
        }

        for (int step=0; step<nsteps; step++) {

            for (int i=0; i<NWALKERS; i++) {
                float cenrow = cenrow0 + 0.01*(drand48()-0.5);
                float cencol = cencol0 + 0.01*(drand48()-0.5);
                float irr = irr0+0.01*(drand48()-0.5);
                float irc = irc0+0.01*(drand48()-0.5);
                float icc = icc0+0.01*(drand48()-0.5);
                float det = irr*icc - irc*irc;
                float idet = 1./det;

                // a copy of the kernel is made each time, so we can add new arguments
                err |=  clSetKernelArg(kernel, 1, sizeof(cl_float), (void*)&cenrow);
                err |=  clSetKernelArg(kernel, 2, sizeof(cl_float), (void*)&cencol);
                err |=  clSetKernelArg(kernel, 3, sizeof(cl_float), (void*)&idet);
                err |=  clSetKernelArg(kernel, 4, sizeof(cl_float), (void*)&irr);
                err |=  clSetKernelArg(kernel, 5, sizeof(cl_float), (void*)&irc);
                err |=  clSetKernelArg(kernel, 6, sizeof(cl_float), (void*)&icc);
                err |=  clSetKernelArg(kernel, 10, sizeof(cl_mem), &output[i]);

                if (err != CL_SUCCESS) {
                    fprintf(stderr,"could not set kernel args\n");
                    exit(EXIT_FAILURE);
                }

                t0enq=clock();
                err = clEnqueueNDRangeKernel(queues[i], 
                        kernel, 
                        1, 
                        NULL, 
                        &szGlobalWorkSize, 
                        &szLocalWorkSize, 
                        0, 
                        NULL, 
                        &events[i]);
                t1enq += clock()-t0enq;
                if (err != CL_SUCCESS) {
                    fprintf(stderr,"error executing kernel\n");
                    exit(EXIT_FAILURE);
                }
            }

            t0wait=clock();
            err=clWaitForEvents(NWALKERS, events);
            if (err != CL_SUCCESS) {
                fprintf(stderr,"error waiting on walkers\n");
                exit(EXIT_FAILURE);
            }
            t1wait += clock()-t0wait;

            for (int i=0; i<NWALKERS; i++) {
                clReleaseEvent(events[i]);
            }

        }
        clReleaseMemObject(image_in);
    }
    t1=clock();
    topencl = ((double)(t1-t0))/CLOCKS_PER_SEC;
    twait = ((double)t1wait)/CLOCKS_PER_SEC;
    tenq = ((double)t1enq)/CLOCKS_PER_SEC;


    printf("time for GPU: %lf\n", topencl);
    printf("time for GPU per: %lf\n", topencl/nrepeat);
    printf("time for wait: %lf\n", twait);
    printf("time for enq: %lf\n", tenq);

    if (docpu) {
        printf("doing cpu\n");
        srand48(10);
        t0=clock();
        for (int rep=0; rep<nrepeat; rep++) {
            for (int step=0; step<nsteps; step++) {
                for (int i=0; i<NWALKERS; i++) {
                    float cenrow = cenrow0 + 0.01*(drand48()-0.5);
                    float cencol = cencol0 + 0.01*(drand48()-0.5);
                    float irr = irr0+0.01*(drand48()-0.5);
                    float irc = irc0+0.01*(drand48()-0.5);
                    float icc = icc0+0.01*(drand48()-0.5);
                    float det = irr*icc - irc*irc;
                    float idet = 1./det;

                    do_c_map(nrow, ncol, cenrow, cencol, idet, irr, irc, icc, data_from_cpu);
                }
            }
        }
        t1=clock();
        tstandard += ((double)(t1-t0))/CLOCKS_PER_SEC;
        printf("time for C loop: %lf\n", tstandard);
        printf("opencl was %.16g times faster\n", tstandard/topencl);
    }


    //compare_data(nrow, ncol, data_from_gpu, data_from_cpu);

    //BEGIN CLEANUP
    clReleaseMemObject(rows_in);
    clReleaseMemObject(cols_in);
    for (int i=0; i<NWALKERS;i++) {
        clReleaseMemObject(output[i]);
    }
    clReleaseKernel(kernel);
    for (int i=0; i<NWALKERS; i++) {
        clReleaseCommandQueue(queues[i]);
    }
    clReleaseContext(context);
    //END CLEANUP

    return 0;
}
