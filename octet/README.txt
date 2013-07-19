Getting started:

Windows:

To build and run on windows you must install at least Visual
Studio Express 2010 C++ edition.

http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express

At present, you must download your Vendor's OpenCL implementation

AMD
http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/

NVidia
https://developer.nvidia.com/opencl

Intel
http://software.intel.com/en-us/vcsource/tools/opencl-sdk-2013

You may also need to install OpenAL

I may get the time to make this optional.


Xcode:

All libraries come as standard with Xcode, but you must use the
"Product -> Edit Scheme -> Options" dialogue to set the working directory
to the "xcode" folder in your project.
