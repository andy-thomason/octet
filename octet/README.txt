Getting started:


Windows:

To build and run on windows you must install at least Visual
Studio Express 2013 C++ edition.

http://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-desktop

Download git for windows (or some other variant) avoid GUI-based git clients.

Get the latest revision:

git clone git://git.code.sf.net/p/octetframework/code octet



Xcode:

Get the latest revision:

git clone git://git.code.sf.net/p/octetframework/code octet


You will need to make some xcode projects for the examples. Only add "src/main.cpp"
do not add *all* the source files.

You will need to add the OpenAL, OpenGL and OpenCL frameworks.

All libraries come as standard with Xcode, but you must use the
"Product -> Edit Scheme -> Options" dialogue to set the working directory
to a folder under the "xcode" folder in your project.

By default, the framework travels up two directories before looking for
files.



Make (on any system):

The simplet way of building is to use "make". But you have to have installed
some unix utilities (such as unxutils on sourceforge) if you are using Windows.

Just typing

$ make

will build all the examples in bin/


Example projects are in src\examples

Click on the sln files (if working with Visual studio)
Each sln file has one project and one .cpp file (main.cpp)

If you are brave, you can also build from the command line using "cl" (windows)
or "clang" (mac) and rarely these days "gcc" (linux, mac, android).


You can make a new project by using the python script in "packaging".
From the project root run:

packaging\make_example.py project_name

This will put the solution, vcproj, main.cpp and header into src\examples


Later, it might be worth downloading your Vendor's OpenCL implementation

AMD
http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/

NVidia
https://developer.nvidia.com/opencl

Intel
http://software.intel.com/en-us/vcsource/tools/opencl-sdk-2013


