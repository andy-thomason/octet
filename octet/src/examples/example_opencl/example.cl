////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014 (MIT license)
//
// Framework for OpenGLES2 rendering on multiple platforms.
//
// OpenCL example 


typedef unsigned uint32_t;

// Simple kernel:
//
// kernel using regular memory object.
//
// This kernel is run many times by OpenCL per image, once for each work item.
// each work item gets a different value of get_global_id(0)
//
// Note that the image is in __global memory.
//
__kernel void mem_kernel(__global uint32_t *image, int width, int height, int time) {
	int y = get_global_id(0);
  __global uint32_t *dest = image + y * width;
  for (int x = 0; x != width; ++x) {
    // moire fringes...
    *dest++ = 0xff000000 | (x * (y * time) >> 6 & 0xff);
  }
}

// Complex kernel:
//
// kernel using local memory object and DMA copies.
// local memory is much faster, but may restrict the number
// of workgroups that can run at the same time.
//
// many OpenCL implementations have an upper size limit on the amount of local memory.
//
// each work item in the work group gets a different value of get_local_id(0)
// up to the size of the work group.
//
__kernel void dma_kernel(__global uint32_t *image, int width, int height, int time) {
	int y = get_global_id(0);
  int ty = get_local_id(0);
  int local_size = get_local_size(0); // assume power of two!
  int first_y = (y & (-local_size)); // first y value of the workgroup

  // this local memory is shared between all the work items in the group
  __local uint32_t tmp[max_width*max_workgroup_size];

  if (width > max_width || get_local_size(0) > max_workgroup_size) {
    return;
  }

  // each work item renders one line in the local memory.
  __local uint32_t *dest = tmp + ty * width;
  for (int x = 0; x != width; ++x) {
    // moire fringes...
    *dest++ = 0xff000000 | (x * (y * time) >> 6 & 0xff);
  }

  // copy the data from local memory to global memory
  // note that this runs on all threads at the same time
  event_t event = async_work_group_copy(image + first_y * width, tmp, width * local_size, 0);

  // we could do some more work in here...

  // wait for all the threads in the workgroup to finish the copy.
  wait_group_events(1, &event);
}

