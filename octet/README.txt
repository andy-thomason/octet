Getting started:

Windows:

To build and run on windows you must install at least Visual
Studio Express 2010 C++ edition.

http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express

Later, it might be worth downloading your Vendor's OpenCL implementation

AMD
http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/

NVidia
https://developer.nvidia.com/opencl

Intel
http://software.intel.com/en-us/vcsource/tools/opencl-sdk-2013


Xcode:

All libraries come as standard with Xcode, but you must use the
"Product -> Edit Scheme -> Options" dialogue to set the working directory
to a folder under the "xcode" folder in your project.

By default, the framework travels up two directories before looking for
files.


Example projects are in src\examples

Click on the sln files (if working with Visual studio)
Each sln file has one project and one .cpp file (main.cpp)

If you are brave, you can also build from the command line using "cl" (windows)
or "clang" (mac) and rarely these days "gcc" (linux, mac, android).


You can make a new project by using the python script in "packaging".
cd into packaging and run:

make_example.py project_name

This will put the solution, vcproj, main.cpp and header into src\examples



