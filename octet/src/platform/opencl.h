////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014 (MIT license)
//
// Framework for OpenGLES2 rendering on multiple platforms.
//
// OpenCL context support
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "CL/cl.h"

namespace octet {
  class opencl {
    cl_int error_code;
    cl_int num_args;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device_id;
    cl_mem mem;
    cl_program program;

    dynarray<cl_kernel> kernels;
    dynarray<cl_mem> mems;
    dynarray<char> source;

    static const char *cl_error_name(cl_int error) {
      switch (error) {
        case CL_SUCCESS: return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

        case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
        default: return "UNKNOWN";
      }
    }
  public:
    opencl(const char *cl_filename) {
      error_code = 0;
      context = 0;
      queue = 0;
      program = 0;
      num_args = 0;

	    cl_uint numPlatforms;	//the NO. of platforms
	    error_code = clGetPlatformIDs(0, NULL, &numPlatforms);
	    if (error_code != CL_SUCCESS) {
        return;
	    }

		  dynarray<cl_platform_id> platforms(numPlatforms);
		  error_code = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
	    if (error_code != CL_SUCCESS) {
        return;
	    }

      dynarray<cl_device_id> devices;

      cl_platform_id best_platform = platforms[0];
      for (cl_int i = 0; i != numPlatforms; ++i) {
		    cl_platform_id platform = platforms[i];
        char buf[512];
        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
        printf("platform %d: %s\n", i, buf);
        if (strstr("NVIDIA", buf)) {
          best_platform = platform;
        }
      }

	    cl_uint numDevices = 0;
      for (cl_uint devtype = CL_DEVICE_TYPE_GPU; devtype != CL_DEVICE_TYPE_CPU; devtype = CL_DEVICE_TYPE_CPU) {
	      error_code = clGetDeviceIDs(best_platform, devtype, 0, NULL, &numDevices);	
        if (numDevices != 0) {
          devices.resize(numDevices);
	        error_code = clGetDeviceIDs(best_platform, devtype, numDevices, &devices[0], NULL);	
	        if (error_code != CL_SUCCESS) {
            return;
	        }
          break;
        }
      }

      printf("platform %d devices\n", numDevices);

      // create a compute context with GPU device
      context = clCreateContext(NULL, devices.size(), &devices[0], NULL, NULL, &error_code);
      if (!context) return;
 
      queue = clCreateCommandQueue(context, devices[0], 0, NULL);
      if (!queue) return;
 
      FILE *src = fopen(cl_filename, "rb");
      if (!src) {
        char cwd[256];
        _getcwd(cwd, 256);
        fprintf(stderr, "could not open kernel %s/%s\n", cwd, cl_filename);
        return;
      }

      fseek(src, 0, SEEK_END);
      size_t source_size = ftell(src);
      printf("size=%d\n", source_size);
      source.resize(source_size+1);
      fseek(src, 0, SEEK_SET);
      fread(&source[0], 1, source_size, src);
      source[source_size] = 0;
      fclose(src);

      // create the compute program
      const char *source_ptr = (const char*)&source[0];
      puts(source_ptr);

      program = clCreateProgramWithSource(context, 1, &source_ptr, &source_size, &error_code);
      if (!program || error_code) {
        return;
      }
 
      // build the compute program executable
      //printf("program=%p\n", program);
      error_code = clBuildProgram(program, devices.size(), &devices[0], NULL, NULL, NULL);
      if (error_code) {
        char log[2048];
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(log), &log, NULL);
        fputs(log, stderr);
        return;
      }

      num_args = 0;
      printf("done\n");
    }
  };
}

