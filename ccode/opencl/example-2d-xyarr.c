/* this one is timing.  This is more optimized than 2*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <CL/opencl.h>

//BEGIN KERNEL CODE
const char *kernel_source =
"__kernel void gmix(int nelem, \n"
"                   float cenrow, \n"
"                   float cencol, \n"
"                   float idet, \n"
"                   float irr, \n"
"                   float irc, \n"
"                   float icc, \n"
"                   global float *rows, \n"
"                   global float *cols, \n"
"                   global float *output)                            \n"
"{                                                                     \n"
"   int idx = get_global_id(0);                                        \n"
"   if (idx >= nelem)                            \n"
"       return;                                  \n"
"   float row = rows[idx];                       \n"
"   float col = cols[idx];                       \n"
"   float u = row-cenrow;                        \n" 
"   float v = col-cencol;                        \n"
"   float chi2=icc*u*u + irr*v*v - 2.0*irc*u*v;  \n"
"   chi2 *= idet;                                \n"
"   output[idx] += exp( -0.5*chi2 );  \n"
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

// Storage for the arrays.
static cl_mem output;
// OpenCL state
static cl_command_queue queue;
static cl_kernel kernel;
static cl_device_id device_ids;
static cl_context context;

static cl_platform_id platform_id;

static const int nrow=64;
static const int ncol=64;

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
            chi2 *= idet;
            data[idx] += exp( -0.5*chi2 );

        }
    }
}

//BEGIN PROFILING FUNCTION
void getProfilingInformation(cl_event eventIn, char* buffer)
{
    cl_ulong start, end;
    clGetEventProfilingInfo(eventIn, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, 0);
    clGetEventProfilingInfo(eventIn, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, 0);

    printf("%s %.16g (ms)\n", buffer, (end-start)*1.0e-6f);
    //std::cout<< buffer << " " << (end-start)*1.0e-6f << "(ms)" << std::endl;
}

//END PROFILING

int main(int argc, char** argv)
{

    float cenrow=20.;
    float cencol=20.;
    float irr=2.;
    float irc=0.;
    float icc=3.;
    float det = irr*icc - irc*irc;
    float idet = 1./det;

    int nelem=nrow*ncol;

    // both 100 and 256 gave same.  Doesn't seem to matter much
    //size_t szLocalWorkSize = 10;
    //size_t szLocalWorkSize = 256;
    size_t szLocalWorkSize = nrow;
    size_t szGlobalWorkSize = shrRoundUp((int)szLocalWorkSize, (int)(nrow*ncol));  // rounded up to the nearest multiple of the LocalWorkSize
    printf("nelem: %d\n", nelem);
    printf("local work size: %lu\n", szLocalWorkSize);
    printf("global work size: %lu\n", szGlobalWorkSize);


    cl_float *data_from_gpu = calloc(szGlobalWorkSize, sizeof(cl_float));
    cl_float *data_from_cpu = calloc(nrow*ncol, sizeof(cl_float));

    cl_float *rows=calloc(szGlobalWorkSize,sizeof(cl_float));
    cl_float *cols=calloc(szGlobalWorkSize,sizeof(cl_float));
    for (int row=0; row<nrow; row++) {
        for (int col=0; col<ncol; col++) {
            rows[row*nrow + col] = row;
            cols[row*nrow + col] = col;
        }
    }



    cl_uint numPlatforms;
    cl_int err = CL_SUCCESS;

    clock_t t0,t1;
    size_t numiter=100000;


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
            CL_DEVICE_TYPE_GPU,
            NULL,
            NULL,
            &err);
    //END CONTEXT

    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_ids, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not get device ids\n");
        exit(EXIT_FAILURE);
    }


    queue = clCreateCommandQueue(context, device_ids, CL_QUEUE_PROFILING_ENABLE, &err);
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
    //output = clCreateBuffer(context,  
    //        CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*szGlobalWorkSize, data_from_gpu, &err);
    output = clCreateBuffer(context,  
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,  sizeof(cl_float)*szGlobalWorkSize, data_from_gpu, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create buffer\n");
        exit(EXIT_FAILURE);
    }

    // true here is for blocking write,0 for 0 offset. End stuff is event oriented
    /*
    err=clEnqueueWriteBuffer(queue,rows_in,CL_TRUE,0,(size_t)(sizeof(float)*nrow*ncol),rows,0,NULL,NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not copy rows\n");
        exit(EXIT_FAILURE);
    }
    err=clEnqueueWriteBuffer(queue,cols_in,CL_TRUE,0,(size_t)(sizeof(float)*nrow*ncol),cols,0,NULL,NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not copy cols\n");
        exit(EXIT_FAILURE);
    }
    */
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source , NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create program\n");
        exit(EXIT_FAILURE);
    }


    //OPTIMIZATION OPTIONS FOUND AT http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html

    err = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not build program\n");
        exit(EXIT_FAILURE);
    }


    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(program, device_ids, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

    clGetDeviceInfo(device_ids, CL_DEVICE_NAME, sizeof(buffer), buffer, &len);
    cl_ulong memsize;
    clGetDeviceInfo(device_ids, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &memsize, &len);
    cl_uint nunits;
    clGetDeviceInfo(device_ids, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &nunits, &len);
     
    printf("device name:  '%s'\n", buffer);
    printf("memory:        %lu\n", memsize);
    printf("compute units: %u\n", nunits);

    //SETUP KERNEL
    cl_kernel kernel = clCreateKernel(program, "gmix", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create kernel\n");
        exit(EXIT_FAILURE);
    }


    clReleaseProgram(program); // no longer needed

    t0=clock();
    err =  clSetKernelArg(kernel, 0, sizeof(cl_int), (void*)&nelem);
    err |=  clSetKernelArg(kernel, 1, sizeof(cl_float), (void*)&cenrow);
    err |=  clSetKernelArg(kernel, 2, sizeof(cl_float), (void*)&cencol);
    err |=  clSetKernelArg(kernel, 3, sizeof(cl_float), (void*)&idet);
    err |=  clSetKernelArg(kernel, 4, sizeof(cl_float), (void*)&irr);
    err |=  clSetKernelArg(kernel, 5, sizeof(cl_float), (void*)&irc);
    err |=  clSetKernelArg(kernel, 6, sizeof(cl_float), (void*)&icc);

    err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &rows_in);
    err |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &cols_in);
    err |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &output);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not set kernel args\n");
        exit(EXIT_FAILURE);
    }
    /*
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &output);
    err |=  clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&nelem);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not set kernel args\n");
        exit(EXIT_FAILURE);
    }
    */


    //END KERNEL

    // leaving local work size alone
    for (size_t iter=0; iter<numiter; iter++) {
        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &szGlobalWorkSize, &szLocalWorkSize, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"error executing kernel\n");
            exit(EXIT_FAILURE);
        }
    }

    // not not reading all elements
    err = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(cl_float)*nrow*ncol, data_from_gpu, 0, NULL, NULL );
    if (err != CL_SUCCESS) {
        fprintf(stderr,"error reading buffer into local\n");
        exit(EXIT_FAILURE);
    }

    t1=clock();
    double topencl = ((double)(t1-t0))/CLOCKS_PER_SEC;
    printf("time for GPU: %lf\n", topencl);

    /*
    for (int row=cenrow-2; row<cenrow+2; row++) {
        for (int col=cencol-2; col<cencol+2; col++) {
            printf("VALUE AT [%d,%d]:\t %.16g\n", row, col, data_from_gpu[row*nrow + col]);
        }
    }
    */

    t0=clock();
    for (size_t iter=0; iter<numiter; iter++) {
        do_c_map(nrow, ncol, cenrow, cencol, idet, irr, irc, icc, data_from_cpu);
    }
    t1=clock();
    double tstandard = ((double)(t1-t0))/CLOCKS_PER_SEC;
    printf("time for C loop: %lf\n", tstandard);
    printf("opencl was %.16g times faster\n", tstandard/topencl);
    /*
    for (int row=cenrow-2; row<cenrow+2; row++) {
        for (int col=cencol-2; col<cencol+2; col++) {
            printf("VALUE AT [%d,%d]:\t %.16g\n", row, col, data_from_cpu[row*nrow + col]);
        }
    }
    */



    compare_data(nrow, ncol, data_from_gpu, data_from_cpu);

    //BEGIN CLEANUP
    clReleaseMemObject(rows_in);
    clReleaseMemObject(cols_in);
    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    //END CLEANUP

    return 0;
}
