#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/opencl.h>

//BEGIN KERNEL CODE
const char *kernel_source =
"__kernel void simple(                                                   \n"
"   global const float* input,                                           \n"
"   global float* output)                                                \n"
"{                                                                       \n"
"   int index = get_global_id(0);                                        \n"
"   output[index] = index;                        \n"
"}                                                                       \n";
//END KERNEL

// Storage for the arrays.
static cl_mem input;
static cl_mem output;
// OpenCL state
static cl_command_queue queue;
static cl_kernel kernel;
static cl_device_id device_ids;
static cl_context context;

static cl_platform_id platform_id;
static const size_t dataSize = 6000000;

void do_c_map(float *data, float *data_out)
{
    for (size_t i=0; i<dataSize; i++) {
        data[i] = data_out[i];
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
    //cl_float* data = new cl_float[dataSize];
    //cl_float* dataOut = new cl_float[dataSize];
    cl_float* data = calloc(dataSize, sizeof(cl_float));
    cl_float* dataOut = calloc(dataSize, sizeof(cl_float));

    cl_uint numPlatforms;
    cl_int err = CL_SUCCESS;

    clock_t t0,t1;

    //SETUP PLATFORM
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(numPlatforms > 0)
    {
        //we have at least one
        //cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        cl_platform_id* platforms = calloc(numPlatforms, sizeof(cl_platform_id));
        err = clGetPlatformIDs(numPlatforms, platforms, NULL);
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
    data[4] = 4;
    dataOut[4] = 5;

    //SETUP DEVICES
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_ids, NULL);
    //END DEVICES

    //SETUP QUEUE
    queue = clCreateCommandQueue(context, device_ids, CL_QUEUE_PROFILING_ENABLE, &err);
    //END QUEUE

    //SETUP BUFFERS
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,  sizeof(cl_float)*dataSize, data, &err);
    output = clCreateBuffer(context,  CL_MEM_WRITE_ONLY,  sizeof(cl_float)*dataSize, NULL, &err);
    //END BUFFERS

    //SETUP PROGRAM
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source , NULL, &err);
    //OPTIMIZATION OPTIONS FOUND AT http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL); // build and compile the OpenCL
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(program, device_ids, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    //printf("%s\n", buffer);
    //END PROGRAM

    //clGetDeviceInfo(devices[d], CL_DEVICE_NAME, 500, dname,NULL);
    clGetDeviceInfo(device_ids, 
                     CL_DEVICE_NAME,
                     sizeof(buffer),
                     buffer,
                     &len);
    printf("device name: '%s'\n", buffer);

    //SETUP KERNEL
    cl_kernel kernel = clCreateKernel(program, "simple", &err);

    clReleaseProgram(program); // no longer needed

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    //END KERNEL

    //RUN KERNEL
    cl_event myEvent;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &dataSize, NULL, 0, NULL, &myEvent);
    //END KERNEL

    //BEGIN PROFILING
    clWaitForEvents(1, &myEvent);
    getProfilingInformation(myEvent, "Sending the Kernel to the GPU");
    err = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(cl_float)*dataSize, dataOut, 0, NULL, &myEvent );
    clWaitForEvents(1, &myEvent);
    getProfilingInformation(myEvent, "Reading the Kernel from the GPU");
    //END PROFILING

    t0=clock();
    do_c_map(data,dataOut);
    t1=clock();
    printf("time for C loop: %lf\n", ((double)(t1-t0))/CLOCKS_PER_SEC);

    for(int i = 49; i >= 0; --i)
    {
        int out = (int) (rand()%(dataSize-1));
        printf("VALUE AT %d:\t %.16g\n", out, dataOut[out]);
        //std::cout << "VALUE AT " << out << "\t: \t" << dataOut[out] << std::endl;
    }

    //BEGIN CLEANUP
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    //END CLEANUP

    return 0;
}
