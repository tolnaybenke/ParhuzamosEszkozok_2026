#include "ocl_utils.h"
#include "kernel_loader.h" // EZT ADTUK HOZZÁ

void check_error(cl_int err, const char* operation) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "OpenCL Hiba (%s): %d\n", operation, err);
        exit(1);
    }
}

OCLResources init_opencl(const char* kernel_path, const char* kernel_name) {
    OCLResources res;
    cl_int err;

    cl_platform_id platform;
    check_error(clGetPlatformIDs(1, &platform, NULL), "Platform");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &res.device, NULL);
    if (err != CL_SUCCESS) {
        check_error(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &res.device, NULL), "Eszkoz");
    }

    res.context = clCreateContext(NULL, 1, &res.device, NULL, NULL, &err);
    check_error(err, "Context");

    // OpenCL 2.0 kompatibilis queue létrehozás
    cl_queue_properties props[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
    res.queue = clCreateCommandQueueWithProperties(res.context, res.device, props, &err);
    check_error(err, "Queue");

    // Itt használjuk a külső kernel_loader modult
    char* source = load_kernel_source(kernel_path);
    res.program = clCreateProgramWithSource(res.context, 1, (const char**)&source, NULL, &err);
    check_error(err, "Program source");
    free(source);

    err = clBuildProgram(res.program, 1, &res.device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(res.program, res.device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = malloc(log_size);
        clGetProgramBuildInfo(res.program, res.device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Kernel hiba:\n%s\n", log);
        free(log); exit(1);
    }

    res.kernel = clCreateKernel(res.program, kernel_name, &err);
    check_error(err, "Kernel creation");

    return res;
}

void cleanup_opencl(OCLResources* res) {
    clReleaseKernel(res->kernel);
    clReleaseProgram(res->program);
    clReleaseCommandQueue(res->queue);
    clReleaseContext(res->context);
}