#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <iostream>
#include <vector>

using namespace std;
using namespace cl;

class Hive {  // Device
 protected:
  Device device;
  string name;
  string type;

 public:
  Hive();

  friend class Keeper;
};

class Garden {  // Platform
 protected:
  Platform platform;
  vector<Hive> hives;
  string name;

 public:
  Garden();

  friend class Keeper;
};

class Keeper {
 protected:
  vector<Garden> gardens;

 public:
  Keeper();
  void Info();
};
