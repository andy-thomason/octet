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
  /// wrapper for building opencl programs and executing them.
  class opencl {
    cl_int error_code;
    cl_context context;
    cl_command_queue queue;
    dynarray<cl_device_id> devices;
    
    cl_program program;

    static const char *get_cl_error_name(cl_int error) {
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

    // If we have many platforms, choose just one.
    cl_platform_id get_prefered_platform(const char *prefered_platform) {
	    cl_uint numPlatforms;	//the NO. of platforms
	    error_code = clGetPlatformIDs(0, NULL, &numPlatforms);
	    if (error_code != CL_SUCCESS) {
        return 0;
	    }

		  dynarray<cl_platform_id> platforms(numPlatforms);
		  error_code = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
	    if (error_code != CL_SUCCESS) {
        return 0;
	    }

      cl_platform_id best_platform = platforms[0];
      for (cl_uint i = 1; i < numPlatforms; ++i) {
		    cl_platform_id platform = platforms[i];
        char buf[512];
        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
        if (strstr(buf, prefered_platform)) {
          best_platform = platform;
        }
      }

      return best_platform;
    }

    void init_devices_and_queues(cl_platform_id best_platform) {
      // check GPU before CPU
	    cl_uint numDevices = 0;
      cl_uint devtype = 0;
      for (devtype = CL_DEVICE_TYPE_GPU; devtype != CL_DEVICE_TYPE_CPU; devtype = CL_DEVICE_TYPE_CPU) {
	      error_code = clGetDeviceIDs(best_platform, devtype, 0, NULL, &numDevices);	
        if (numDevices != 0) {
          break;
        }
      }

      #ifdef WIN32
        const cl_context_properties props[] = {
          CL_CONTEXT_PLATFORM, (cl_context_properties)best_platform,
          CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
          CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
          0
        };
      #else
        const cl_context_properties *props = NULL;
      #endif

      // for now, assume all devices are gl/cl capable.
      devices.resize(numDevices);
  	  error_code = clGetDeviceIDs(best_platform, devtype, numDevices, devices.data(), NULL);	

      /*size_t size = 0;
      error_code = clGetGLContextInfoKHR(props, CL_DEVICES_FOR_GL_CONTEXT_KHR, 0, NULL, &size);
      devices.resize(size / sizeof(cl_device_id));
      error_code = clGetGLContextInfoKHR(props, CL_DEVICES_FOR_GL_CONTEXT_KHR, size, devices.data(), NULL);
      */

      context = clCreateContext(props, devices.size(), devices.data(), NULL, NULL, &error_code);
      if (!context) return;
 
      queue = clCreateCommandQueue(context, devices[0], 0, NULL);
      if (!queue) return;
    }

    void build_program(const char *cl_filename, const char *defines = "") {
      dynarray<uint8_t> source;
      app_utils::get_url(source, cl_filename);
      source.push_back(0);

      // create the compute program
      const char *source_ptr[] = { defines, (const char*)&source[0] };
      size_t source_sizes[] = { strlen(defines), (size_t)source.size() };
      //fputs(source_ptr, log("opencl source\n"));

      program = clCreateProgramWithSource(context, 2, source_ptr, source_sizes, &error_code);
      if (!program || error_code) {
        log("unable to create program\n");
        return;
      }
 
      // build the compute program executable
      //printf("program=%p\n", program);
      error_code = clBuildProgram(program, devices.size(), &devices[0], NULL, NULL, NULL);
      if (error_code) {
        char build_log[2048];
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(build_log), &build_log, NULL);
        fputs(build_log, log("build errors:\n"));
        return;
      }

      //printf("done\n");
      log("done creating opencl context\n");
    }

  public:

    class mem {
    protected:
      cl_mem obj;
      int ref_count;
      opencl &cl;
    public:
      mem(opencl &_cl) : cl(_cl) {
        ref_count = 0;
        obj = 0;
      }

      /// create an opencl memory from cpu memory
      mem(opencl &_cl, unsigned flags, const void *ptr, size_t size) : cl(_cl) {
        obj = clCreateBuffer(cl.context, flags, size, (void*)ptr, &cl.error_code);
      }

      /// queue a copy from a buffer to cpu memory
      cl_event read(size_t size, void *ptr, cl_event event=0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl.error_code = clEnqueueReadBuffer(cl.queue, obj, CL_FALSE, 0, size, ptr, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }

      /// queue a copy from cpu memory to a buffer
      cl_event write(size_t size, const void *ptr, cl_event event=0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl.error_code = clEnqueueWriteBuffer(cl.queue, obj, CL_FALSE, 0, size, ptr, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }

      void add_ref() { ++ref_count; }
      void release() { if (!--ref_count) { clReleaseMemObject(obj); obj = 0; } }
      cl_mem get_obj() const { return obj; }
    };

    class gl_object : public mem {
    public:
      gl_object(opencl &_cl) : mem(cl) {
      }

      /// acquire an opengl object for reading/writing
      cl_event gl_acquire(cl_event event = 0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl.error_code = clEnqueueAcquireGLObjects(cl.queue, 1, &obj, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }

      /// release an opengl object for reading/writing
      cl_event gl_release(cl_event event = 0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl.error_code = clEnqueueReleaseGLObjects(cl.queue, 1, &obj, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }
    };

    class image : public gl_object {
    public:
      /// create an opencl memory from an octet image object
      image(opencl &_cl, unsigned flags, octet::scene::image *img, int mip_level=0) : gl_object(_cl) {
        GLuint target = img->get_target();
        GLuint texture = img->get_gl_texture();
        obj =
          target == GL_TEXTURE_3D ? clCreateFromGLTexture3D(cl.context, flags, target, mip_level, texture, &cl.error_code) :
          clCreateFromGLTexture2D(cl.context, flags, target, mip_level, texture, &cl.error_code)
        ;
      }
    };

    class gl_resource : public gl_object {
    public:
      /// create an opencl memory from an octet gl_resource
      gl_resource(opencl &_cl, unsigned flags, octet::resources::gl_resource *res) : gl_object(_cl) {
        obj = clCreateFromGLBuffer(cl.context, flags, res->get_buffer(), &cl.error_code);
      }
    };

    class kernel {
      cl_kernel obj;
      int ref_count;
      cl_int num_args;
      opencl &cl;
    public:
      kernel(opencl &_cl, const char *kernel_name) : cl(_cl) {
        obj = clCreateKernel(cl.program, kernel_name, NULL);
        ref_count = 0;
        num_args = 0;
      }

      /// begin pushing arguments
      void begin() {
        num_args = 0;
      }

      /// push an argument
      template<class arg_t> void push(arg_t value) {
        cl.error_code = clSetKernelArg(obj, num_args++, sizeof(value), (void *)&value);
        if (cl.error_code) { log("push: error %s\n", cl.get_cl_error_name(cl.error_code)); }
      }

      /// queue a call
      cl_event call(size_t num_work_items, size_t work_group_size, cl_event event=0, bool want_res_event=false) {
  	    size_t global_work_size[1] = {num_work_items};
  	    size_t local_work_size[1] = {work_group_size};
        cl_event res_event;
        cl.error_code = clEnqueueNDRangeKernel(cl.queue, obj, 1, NULL, global_work_size, local_work_size, event != 0, &event, want_res_event ? &res_event : NULL);
        num_args = 0;
        if (cl.error_code) { log("call: error %s\n", cl.get_cl_error_name(cl.error_code)); }
        return res_event;
      }

      void add_ref() { ++ref_count; }
      void release() { if (!--ref_count) { clReleaseKernel(obj); obj = 0; } }
      cl_kernel get_obj() const { return obj; }
    };

  public:
    opencl() {
      init(NULL, NULL);
    }

    /// Create a context, queue and program for opencl.
    /// The program may contain multiple kernels.
    void init(const char *cl_filename, const char *prefered_platform = "NVIDIA", const char *defines = "") {
      error_code = 0;
      context = 0;
      queue = 0;
      program = 0;

      if (!cl_filename || !prefered_platform) {
        return;
      }

      cl_platform_id best_platform = get_prefered_platform(prefered_platform);
	    if (error_code != CL_SUCCESS) {
        return;
	    }

      init_devices_and_queues(best_platform);
	    if (error_code != CL_SUCCESS) {
        return;
	    }

      if (cl_filename) {
        build_program(cl_filename, defines);
      }
    }

    /// clean up resources
    ~opencl() {
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
    }

    /// start any opencl activities in the queue.
    void flush() {
      clFlush(queue);
    }

    /// wait until all opencl activities have finished. (inefficient!)
    void finish() {
      clFinish(queue);
    }

    /// wait (stopping the host cpu) for an event
    void wait(cl_event event) {
      clWaitForEvents(1, &event);
    }

    /// return a string representing any error that has occured or NULL.
    const char *get_error() {
      if (error_code) {
        return get_cl_error_name(error_code);
      }
      if (!context) return "no context";
      if (!queue) return "no queue";
      if (!program) return "no program";
      return 0;
    }
  };
}

