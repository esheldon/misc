/* this one is timing.  This is more optimized than 2*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <CL/opencl.h>

//BEGIN KERNEL CODE
const char *kernel_source =
"__kernel void simple(int nrow,                                        \n"
"                     int ncol,                                        \n"
"                     global float* output)                            \n"
"{                                                                     \n"
"   int row = get_global_id(0);                                        \n"
"   int col = get_global_id(1);                                        \n"
"   if ( (row >= nrow) || (col >= ncol) ) \n"
"       return;                           \n"
"   int idx=row*nrow + col;                                            \n"
"   output[idx] = idx;                                                 \n"
"}                                                                     \n";
//END KERNEL

// Storage for the arrays.
static cl_mem output;
// OpenCL state
static cl_command_queue queue;
static cl_kernel kernel;
static cl_device_id device_ids;
static cl_context context;

static cl_platform_id platform_id;

static const int nrow=40;
static const int ncol=40;

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

void do_c_map(int nrow, int ncol, float *data)
{
    int idx=0;
    for (int row=0; row<nrow; row++) {
        for (int col=0; col<ncol; col++) {
            idx=row*nrow + col;
            data[idx] = idx;
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

    // working with local work sizes gives a big speedup, at least 2.5
    // what other optimizations can we do?
    cl_float* data_from_gpu = calloc(nrow*ncol, sizeof(cl_float));
    cl_float* data_from_cpu = calloc(nrow*ncol, sizeof(cl_float));

    cl_uint numPlatforms;
    cl_int err = CL_SUCCESS;

    clock_t t0,t1;
    size_t numiter=400000;

    int nelem=nrow*ncol;

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


    output = clCreateBuffer(context,  CL_MEM_WRITE_ONLY,  sizeof(cl_float)*nrow*ncol, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create buffer\n");
        exit(EXIT_FAILURE);
    }


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

    clGetDeviceInfo(device_ids, 
                     CL_DEVICE_NAME,
                     sizeof(buffer),
                     buffer,
                     &len);
    printf("device name: '%s'\n", buffer);

    //SETUP KERNEL
    cl_kernel kernel = clCreateKernel(program, "simple", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not create kernel\n");
        exit(EXIT_FAILURE);
    }


    clReleaseProgram(program); // no longer needed

    t0=clock();
    err = clSetKernelArg(kernel, 0, sizeof(cl_int), (void*)&nrow);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&ncol);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not set kernel args\n");
        exit(EXIT_FAILURE);
    }


    //END KERNEL

    cl_uint nd=2;
    size_t global_work_sizes[2] = {0};
    global_work_sizes[0] = (size_t)nrow;
    global_work_sizes[1] = (size_t)ncol;
    // leaving local work size alone
    for (size_t iter=0; iter<numiter; iter++) {
        err = clEnqueueNDRangeKernel(queue, kernel, nd, NULL, global_work_sizes, NULL, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr,"error executing kernel\n");
            exit(EXIT_FAILURE);
        }
    }

    err = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(cl_float)*nrow*ncol, data_from_gpu, 0, NULL, NULL );
    if (err != CL_SUCCESS) {
        fprintf(stderr,"error reading buffer into local\n");
        exit(EXIT_FAILURE);
    }

    t1=clock();
    printf("time for GPU: %lf\n", ((double)(t1-t0))/CLOCKS_PER_SEC);

    for(int i = 10; i < 20; i++)
    {
        printf("VALUE AT %d:\t %.16g\n", i, data_from_gpu[i]);
        //std::cout << "VALUE AT " << out << "\t: \t" << dataOut[out] << std::endl;
    }

    t0=clock();
    for (size_t iter=0; iter<numiter; iter++) {
        do_c_map(nrow, ncol, data_from_cpu);
    }
    t1=clock();
    printf("time for C loop: %lf\n", ((double)(t1-t0))/CLOCKS_PER_SEC);
    for(int i = 10; i < 20; i++)
    {
        printf("VALUE AT %d:\t %.16g\n", i, data_from_cpu[i]);
        //std::cout << "VALUE AT " << out << "\t: \t" << dataOut[out] << std::endl;
    }


    compare_data(nrow, ncol, data_from_gpu, data_from_cpu);

    //BEGIN CLEANUP
    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    //END CLEANUP

    return 0;
}
