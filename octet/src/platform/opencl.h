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

#if 0
namespace octet {
  /// push an argument
  template<class arg_t> void push(const arg_t &value) {
    opencl *cl = get_cl();
    if (cl->error_code) return;
    cl->error_code = clSetKernelArg(get_obj(), num_args++, sizeof(value), (void *)&value);
    if (cl->error_code) { log("push: error %s\n", cl->get_cl_error_name(cl->error_code)); }
  }

  // push a memory argument
  template<> void push<mem>(const mem &_value) {
    push(_value.get_obj());
  }

  
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
      for (cl_uint i = 0; i < numPlatforms; ++i) {
		    cl_platform_id platform = platforms[i];
        cl_int mcu;
  	    cl_uint numDevices = 0;
        cl_int devtype = CL_DEVICE_TYPE_GPU;
        size_t gms, lms;
	      error_code = clGetDeviceIDs(best_platform, devtype, 0, NULL, &numDevices);	
        devices.resize(numDevices);
  	    error_code = clGetDeviceIDs(best_platform, devtype, numDevices, devices.data(), NULL);
        clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(mcu), &mcu, NULL);
        clGetDeviceInfo(devices[0], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(gms), &gms, NULL);
        clGetDeviceInfo(devices[0], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(lms), &lms, NULL);
        char buf[512];
        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
        printf("%-40s:   %2d devs  %2d cus  %5d MB global  %5d local\n", buf, numDevices, mcu, (unsigned)gms/0x100000, (unsigned)lms);
        if (strstr(buf, prefered_platform)) {
          best_platform = platform;
        }
      }

      {
        char buf[512];
        clGetPlatformInfo(best_platform, CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
        printf("best platform %s\n", buf);
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

      cl_context_properties fullprops[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)best_platform,
        CL_GL_CONTEXT_KHR, (cl_context_properties)0,
        CL_WGL_HDC_KHR, (cl_context_properties)0,
        0
      };
      const cl_context_properties *props = NULL;
      #ifdef WIN32
        HGLRC wglcontext = wglGetCurrentContext();
        if (wglcontext) {
          fullprops[3] = (cl_context_properties)wglcontext;
          fullprops[5] = (cl_context_properties)wglGetCurrentDC();
          props = fullprops;
        }
      #else
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
    template<class obj_t> class container {
      obj_t obj;
      opencl *cl;
    public:
      container() {
        obj = 0;
        cl = 0;
      }
      container(opencl *_cl, obj_t _obj) {
        cl = _cl;
        obj = _obj;
      }
      container(const container &rhs) {
        cl = rhs.cl;
        set_obj(rhs.obj);
      }
      container &operator=(const container &rhs) {
        cl = rhs.cl;
        set_obj(rhs.obj);
        return *this;
      }
      container &operator=(obj_t _obj) {
        set_obj(_obj);
        return *this;
      }
      ~container() {
        set_obj(0);
      }
      void set_obj(obj_t _obj) {
        if (_obj) retain(_obj);
        if (obj) release(obj);
        obj = _obj;
      }
      obj_t get_obj() const {
        return obj;
      }
      opencl *get_cl() const {
        return cl;
      }
    };

    static void retain(cl_mem obj) { clRetainMemObject(obj); }
    static void release(cl_mem obj) { clReleaseMemObject(obj); }
    static void retain(cl_kernel obj) { clRetainKernel(obj); }
    static void release(cl_kernel obj) { clReleaseKernel(obj); }
    static void retain(cl_event obj) { clRetainEvent(obj); }
    static void release(cl_event obj) { clReleaseEvent(obj); }

    class mem : public container<cl_mem> {
    public:
      mem(opencl *_cl=0, cl_mem _obj=0) : container(_cl, _obj) {
      }

      mem(opencl *_cl, unsigned flags, size_t size, void *ptr) : container(_cl, 0) {
        set_obj(
          clCreateBuffer(_cl->context, flags, size, ptr, &_cl->error_code)
        );
      }

      /// queue a copy from a buffer to cpu memory
      cl_event read(size_t size, void *ptr, cl_event event=0, bool want_res_event=false) {
        cl_event res_event = 0;
        opencl *cl = get_cl();
        cl->error_code = clEnqueueReadBuffer(cl->queue, get_obj(), CL_FALSE, 0, size, ptr, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }

      /// queue a copy from cpu memory to a buffer
      cl_event write(size_t size, const void *ptr, cl_event event=0, bool want_res_event=false) {
        cl_event res_event = 0;
        opencl *cl = get_cl();
        cl->error_code = clEnqueueWriteBuffer(cl->queue, get_obj(), CL_FALSE, 0, size, ptr, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }
    };

    class gl_buffer : public container<cl_mem> {
    public:
      gl_buffer(opencl *_cl, cl_mem _obj) : container(_cl, _obj) {
      }

      /// acquire an opengl object for reading/writing
      cl_event gl_acquire(cl_event event = 0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl_mem memobj = get_obj();
        opencl *cl = get_cl();
        cl->error_code = clEnqueueAcquireGLObjects(cl->queue, 1, &memobj, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }

      /// release an opengl object for reading/writing
      cl_event gl_release(cl_event event = 0, bool want_res_event=false) {
        cl_event res_event = 0;
        cl_mem memobj = get_obj();
        opencl *cl = get_cl();
        cl->error_code = clEnqueueReleaseGLObjects(cl->queue, 1, &memobj, event != 0, &event, want_res_event ? &res_event : NULL);
        return res_event;
      }
    };

    class image : public gl_buffer {
    public:
      /// create an opencl memory from an octet image object
      image(opencl *_cl, unsigned flags, octet::scene::image *img, int mip_level=0) : gl_buffer(_cl, 0) {
        GLuint target = img->get_target();
        GLuint texture = img->get_gl_texture();
        opencl *cl = get_cl();
        set_obj(
          target == GL_TEXTURE_3D ? clCreateFromGLTexture3D(cl->context, flags, target, mip_level, texture, &cl->error_code) :
          clCreateFromGLTexture2D(cl->context, flags, target, mip_level, texture, &cl->error_code)
        );
      }
    };

    class gl_resource : public gl_buffer {
    public:
      /// create an opencl memory from an octet gl_resource
      gl_resource(opencl *_cl, unsigned flags, octet::resources::gl_resource *res) : gl_buffer(_cl, 0) {
        opencl *cl = get_cl();
        set_obj(
          clCreateFromGLBuffer(cl->context, flags, res->get_buffer(), &cl->error_code)
        );
      }
    };

    class kernel : public container<cl_kernel> {
      cl_int num_args;
      size_t local_mem_size;
      size_t prefered_workgroup_size;
      size_t max_work_items;
    public:
      kernel() : container(0, 0) {
      }

      kernel(opencl *cl, const char *kernel_name) : container(cl, 0) {
        set_obj(
          clCreateKernel(cl->program, kernel_name, NULL)
        );
        num_args = 0;
        prefered_workgroup_size = 32;
        local_mem_size = 0;

        clGetKernelWorkGroupInfo(get_obj(), cl->devices[0], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
        clGetKernelWorkGroupInfo(get_obj(), cl->devices[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(prefered_workgroup_size), &prefered_workgroup_size, NULL);
        max_work_items = prefered_workgroup_size * 32;
      }

      size_t get_local_mem_size() const {
        return local_mem_size;
      }

      size_t get_prefered_workgroup_size() const {
        return prefered_workgroup_size;
      }

      /// begin pushing arguments
      void begin() {
        num_args = 0;
      }

      /// queue a call
      cl_event call(size_t num_work_items, size_t work_group_size, cl_event event=0, bool want_res_event=false) {
        cl_event res_event;
        work_group_size = work_group_size ? work_group_size : prefered_workgroup_size;
        opencl *cl = get_cl();
        if (cl->error_code) return 0;

        size_t offset = 0;
        //printf("%08x work items\n", num_work_items);
        while (num_work_items) {
          size_t cur_work_items = num_work_items > max_work_items ? max_work_items : num_work_items;
          if (cur_work_items < work_group_size) {
            work_group_size = 1;
          } else if (cur_work_items % work_group_size) {
            cur_work_items = cur_work_items / work_group_size * work_group_size;
          }
          //printf("  %08x %08x %08x\n", offset, cur_work_items, work_group_size);

  	      size_t global_work_offset[1] = {offset};
  	      size_t global_work_size[1] = {cur_work_items};
  	      size_t local_work_size[1] = {work_group_size};
          cl->error_code = clEnqueueNDRangeKernel(
            cl->queue, get_obj(), 1, global_work_offset, global_work_size, local_work_size,
            event != 0, &event, want_res_event ? &res_event : NULL
          );
          if (cl->error_code) {
            log("call: error %s\n", cl->get_cl_error_name(cl->error_code));
          }
          cl->flush();

          offset += cur_work_items;
          num_work_items -= cur_work_items;
        }
        num_args = 0;
        return res_event;
      }
    };

    class event : public container<cl_event> {
    public:
      event(opencl *_cl, cl_event _event) : container(_cl, _event) {
      }
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
#endif

