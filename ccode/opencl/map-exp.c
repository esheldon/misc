#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include <CL/opencl.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _EXPNUM(X) 1.193e-##X

// slightly above 1.19209289551e−7
#define MIN_ERROR       _EXPNUM(7)
#define MAX_GROUPS      (64)
#define MAX_WORK_ITEMS  (64)
#define SEPARATOR       ("----------------------------------------------------------------------\n")

static int iterations = 4000;
static int count      = 1024 * 1024;
static int channels   = 1;
static bool integer   = true;

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

float reduce_validate_float(float *data, int size)
{
    int i;
    float sum = data[0];
    float c = (float)0.0f;              
    for (i = 1; i < size; i++)
    {
        float y = data[i] - c;  
        float t = sum + y;      
        c = (t - sum) - y;  
        sum = t;            
    }
    return sum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void create_reduction_pass_counts(
    int count, 
    int max_group_size,    
    int max_groups,
    int max_work_items, 
    int *pass_count, 
    size_t **group_counts, 
    size_t **work_item_counts,
    int **operation_counts,
    int **entry_counts)
{
    int work_items = (count < max_work_items * 2) ? count / 2 : max_work_items;
    if(count < 1)
        work_items = 1;
        
    int groups = count / (work_items * 2);
    printf("********groups1 %d\n", groups);
    groups = max_groups < groups ? max_groups : groups;
    printf("********groups2 %d\n", groups);

    int max_levels = 1;
    int s = groups;

    while(s > 1) 
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        s = s / (work_items*2);
        max_levels++;
    }
 
    *group_counts = (size_t*)malloc(max_levels * sizeof(size_t));
    *work_item_counts = (size_t*)malloc(max_levels * sizeof(size_t));
    *operation_counts = (int*)malloc(max_levels * sizeof(int));
    *entry_counts = (int*)malloc(max_levels * sizeof(int));

    (*pass_count) = max_levels;
    (*group_counts)[0] = groups;
    (*work_item_counts)[0] = work_items;
    (*operation_counts)[0] = 1;
    (*entry_counts)[0] = count;
    if(max_group_size < work_items)
    {
        (*operation_counts)[0] = work_items;
        (*work_item_counts)[0] = max_group_size;
    }
    printf("********groups3 %d\n", groups);
    printf("********work items: %d\n", work_items);
    
    s = groups;
    int level = 1;
   
    while(s > 1) 
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        int groups = s / (work_items * 2);
        groups = (max_groups < groups) ? max_groups : groups;

        (*group_counts)[level] = groups;
        (*work_item_counts)[level] = work_items;
        (*operation_counts)[level] = 1;
        (*entry_counts)[level] = s;
        if(max_group_size < work_items)
        {
            (*operation_counts)[level] = work_items;
            (*work_item_counts)[level] = max_group_size;
        }
        
        s = s / (work_items*2);
        level++;
    }
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    int              err;
    cl_device_id     device_id;
    cl_command_queue commands;
    cl_context       context;
    cl_mem			 output_buffer;
    cl_mem           input_buffer;
    cl_mem           partials_buffer;
    size_t           typesize;
    int              pass_count = 0;
    size_t*          group_counts = 0;
    size_t*          work_item_counts = 0;
    int*             operation_counts = 0;
    int*             entry_counts = 0;
    int              use_gpu = 1;
    
    int i;
    int c;
    
    // Parse command line options
    //
    for( i = 0; i < argc && argv; i++)
    {
        if(!argv[i])
            continue;
            
        if(strstr(argv[i], "cpu"))
        {
            use_gpu = 0;        
        }
        else if(strstr(argv[i], "gpu"))
        {
            use_gpu = 1;
        }
    }

    channels=1;
    
    // Create some random input data on the host 
    //
    time_t tstart=0;
    (void) time(&tstart);
    srand48((long) tstart);
    float *float_data = (float*)malloc(count * channels * sizeof(float));
    for (i = 0; i < count * channels; i++)
    {
        float_data[i] = drand48();
    }


    //SETUP PLATFORM
    cl_uint numPlatforms;
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr,"could not get platform\n");
        exit(EXIT_FAILURE);
    }

    cl_platform_id platform_id;
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


    // Connect to a compute device
    //
    err = clGetDeviceIDs(platform_id, use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to locate a compute device!\n");
        return EXIT_FAILURE;
    }

    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve device info!\n");
        return EXIT_FAILURE;
    }

    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
    err|= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve device info!\n");
        return EXIT_FAILURE;
    }

    printf(SEPARATOR);
    printf("Connecting to %s %s...\n", vendor_name, device_name);

    // Load the compute program from disk into a cstring buffer
    //
    typesize = (sizeof(float));    
    const char* filename = 0;
    filename = "apple-reduce-kernel-float.cl";

    printf(SEPARATOR);
    printf("Loading program '%s'...\n", filename);
    printf(SEPARATOR);

    char *source = load_program_source(filename);
    if(!source)
    {
        printf("Error: Failed to load compute program from file!\n");
        return EXIT_FAILURE;    
    }
    
    // Create a compute context 
    //
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command queue
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }

    // Create the input buffer on the device
    //
    size_t buffer_size = typesize * count * channels;
    input_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
    if (!input_buffer)
    {
        printf("Error: Failed to allocate input buffer on device!\n");
        return EXIT_FAILURE;
    }

    // Fill the input buffer with the host allocated random data
    //
    void *input_data = (void*)float_data;
    err = clEnqueueWriteBuffer(commands, input_buffer, CL_TRUE, 0, buffer_size, input_data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        return EXIT_FAILURE;
    }

    // Create an intermediate data buffer for intra-level results
    //
    partials_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
    if (!partials_buffer)
    {
        printf("Error: Failed to allocate partial sum buffer on device!\n");
        return EXIT_FAILURE;
    }

    // Create the output buffer on the device
    //
    output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
    if (!output_buffer)
    {
        printf("Error: Failed to allocate result buffer on device!\n");
        return EXIT_FAILURE;
    }

    // Determine the reduction pass configuration for each level in the pyramid
    //
    create_reduction_pass_counts(
        count, max_workgroup_size, 
        MAX_GROUPS, MAX_WORK_ITEMS, 
        &pass_count, &group_counts, 
        &work_item_counts, &operation_counts,
        &entry_counts);

    // Create specialized programs and kernels for each level of the reduction
    //
    cl_program *programs = (cl_program*)malloc(pass_count * sizeof(cl_program));
    memset(programs, 0, pass_count * sizeof(cl_program));

    cl_kernel *kernels = (cl_kernel*)malloc(pass_count * sizeof(cl_kernel));
    memset(kernels, 0, pass_count * sizeof(cl_kernel));

    for(i = 0; i < pass_count; i++)
    {
        char *block_source = malloc(strlen(source) + 1024);
        size_t source_length = strlen(source) + 1024;
        memset(block_source, 0, source_length);
        
        // Insert macro definitions to specialize the kernel to a particular group size
        //
        const char group_size_macro[] = "#define GROUP_SIZE";
        const char operations_macro[] = "#define OPERATIONS";
        sprintf(block_source, "%s (%d) \n%s (%d)\n\n%s\n", 
            group_size_macro, (int)group_counts[i], 
            operations_macro, (int)operation_counts[i], 
            source);
        
        // Create the compute program from the source buffer
        //
        programs[i] = clCreateProgramWithSource(context, 1, (const char **) & block_source, NULL, &err);
        if (!programs[i] || err != CL_SUCCESS)
        {
            printf("%s\n", block_source);
            printf("Error: Failed to create compute program!\n");
            return EXIT_FAILURE;
        }
    
        // Build the program executable
        //
        err = clBuildProgram(programs[i], 0, NULL, NULL, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            size_t length;
            char build_log[2048];
            printf("%s\n", block_source);
            printf("Error: Failed to build program executable!\n");
            clGetProgramBuildInfo(programs[i], device_id, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
            printf("%s\n", build_log);
            return EXIT_FAILURE;
        }
    
        // Create the compute kernel from within the program
        //
        kernels[i] = clCreateKernel(programs[i], "reduce", &err);
        if (!kernels[i] || err != CL_SUCCESS)
        {
            printf("Error: Failed to create compute kernel!\n");
            return EXIT_FAILURE;
        }

        free(block_source);
    }
    
    // Do the reduction for each level  
    // this is one pass over it to establish the kernel args and such, so
    // it is negligible time
    //
    cl_mem pass_swap;
    cl_mem pass_input = output_buffer;
    cl_mem pass_output = input_buffer;

    for(i = 0; i < pass_count; i++)
    {
        size_t global = group_counts[i] * work_item_counts[i];        
        size_t local = work_item_counts[i];
        unsigned int operations = operation_counts[i];
        unsigned int entries = entry_counts[i];
        size_t shared_size = typesize * channels * local * operations;

        printf("Pass[%4d] Global[%4d] Local[%4d] Groups[%4d] WorkItems[%4d] Operations[%d] Entries[%d]\n",  i, 
            (int)global, (int)local, (int)group_counts[i], (int)work_item_counts[i], operations, entries);

        // Swap the inputs and outputs for each pass
        //
        pass_swap = pass_input;
        pass_input = pass_output;
        pass_output = pass_swap;
        
        err = CL_SUCCESS;
        err |= clSetKernelArg(kernels[i],  0, sizeof(cl_mem), &pass_output);  
        err |= clSetKernelArg(kernels[i],  1, sizeof(cl_mem), &pass_input);
        err |= clSetKernelArg(kernels[i],  2, shared_size,    NULL);
        err |= clSetKernelArg(kernels[i],  3, sizeof(int),    &entries);
        if (err != CL_SUCCESS)
        {
            printf("Error: Failed to set kernel arguments!\n");
            return EXIT_FAILURE;
        }
        
        // After the first pass, use the partial sums for the next input values
        //
        if(pass_input == input_buffer)
            pass_input = partials_buffer;
            
        err = CL_SUCCESS;
        err |= clEnqueueNDRangeKernel(commands, kernels[i], 1, NULL, &global, &local, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("Error: Failed to execute kernel!\n");
            return EXIT_FAILURE;
        }
    }
    
    err = clFinish(commands);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to wait for command queue to finish! %d\n", err);
        return EXIT_FAILURE;
    }

    // Start the timing loop and execute the kernel over several iterations  
    //
    printf(SEPARATOR);
    printf("Timing %d iterations of reduction with %d elements of type %s%s...\n", 
        iterations, count, "float", 
        (channels <= 1) ? (" ") : (channels == 2) ? "2" : "4");
    printf(SEPARATOR);

    int k;
    err = CL_SUCCESS;
    time_t t1 = clock();
    for (k = 0 ; k < iterations; k++)
    {    
        for(i = 0; i < pass_count; i++)
        {
            size_t global = group_counts[i] * work_item_counts[i];        
            size_t local = work_item_counts[i];

            err = clEnqueueNDRangeKernel(commands, kernels[i], 1, NULL, &global, &local, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("Error: Failed to execute kernel!\n");
                return EXIT_FAILURE;
            }
        }
    }
    err = clFinish(commands);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to wait for command queue to finish! %d\n", err);
        return EXIT_FAILURE;
    }
    time_t t2 = clock();
    
    // Calculate the statistics for execution time and throughput
    //
    double t = (t2-t1)/( (double)CLOCKS_PER_SEC );
    printf("Exec Time:  %.2f ms\n", t);
    printf("Throughput: %.2f GB/sec\n", 1e-9 * buffer_size * iterations / t);
    printf(SEPARATOR);

    // Read back the results that were computed on the device
    //
    void *computed_result = malloc(typesize * channels);
    memset(computed_result, 0, typesize * channels);
    err = clEnqueueReadBuffer(commands, pass_output, CL_TRUE, 0, typesize * channels, computed_result, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to read back results from the device!\n");
        return EXIT_FAILURE;
    }

    // now do the speed test on standard

    float reference=0;
    t1 = clock();
    for (k=0; k<iterations; k++) {
        reference = reduce_validate_float(float_data, count);
    }
    t2 = clock();
    double tcpu = (t2-t1)/( (double)CLOCKS_PER_SEC );
    printf("CPU Exec Time:  %.2f ms\n", tcpu);
    printf("CPU Throughput: %.2f GB/sec\n", 1e-9 * buffer_size * iterations / tcpu);
    printf("GPU is faster by %.16g\n", tcpu/t);
    printf(SEPARATOR);


    float result= ( (float *)computed_result )[0];

    float ferror = fabs(reference - result)/reference;
   
    if (ferror > MIN_ERROR)
    {
        printf("Result %.16g != %.16g\n", reference, result);

        printf("Error:  Incorrect results obtained! Rel error %.16g > Max allowed = %.16g\n", ferror, MIN_ERROR);
        return EXIT_FAILURE;
    }
    else
    {
        printf("Results Validated!\n");
        printf(SEPARATOR);
    }

    // Shutdown and cleanup
    //
    for(i = 0; i < pass_count; i++)
    {
        clReleaseKernel(kernels[i]);
        clReleaseProgram(programs[i]);
    }
    
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseMemObject(partials_buffer);        
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
    
    free(group_counts);
    free(work_item_counts);
    free(operation_counts);
    free(entry_counts);
    free(computed_result);
    free(kernels);
    free(float_data);
    
        
    return 0;
}

