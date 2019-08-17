#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace cl;

class Garden {  // Platform

 protected:
  Platform platform;
  vector<Device> devices;
  Context context;
  Program program;

      public : Garden();

  friend class Keeper;
};

class Environment {
 protected:
  string kernel;
  Program::Sources source;

 public:
  Environment();

  friend class Keeper;
};

class Keeper {
 protected:
  vector<Garden> gardens;
  Environment env;

 public:
  Keeper();
  void Info();
  int SetGardens();
  int SetKernel(string fname);
  int Build();
};
