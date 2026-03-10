#ifndef OCL_UTILS_H
#define OCL_UTILS_H

#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_device_id device;
} OCLResources;

OCLResources init_opencl(const char* kernel_path, const char* kernel_name);
void cleanup_opencl(OCLResources* res);
void check_error(cl_int err, const char* operation);

#endif