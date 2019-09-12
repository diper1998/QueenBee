#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace cl;


class Argument {
 protected:
  double* pointer;
  std::size_t size;
  bool change;
  Buffer buffer;

 public:
  Argument(double* arg_pointer, std::size_t data_size, bool flag_change);
  friend class Keeper;
};

class Task {
 protected:
  string function_id;
  string parallel_method;
  vector<unsigned int> globals;
  vector<unsigned int> locals;
  vector<unsigned int> offsets;

  //unsigned int global_id;
  //unsigned int offset;

 public:
  Task(string my_funcion_id, string my_parallel_method, vector<unsigned int> my_offsets,
       vector<unsigned int> my_global_range, vector<unsigned int> my_local_range);
  friend class Keeper;
};


class Function {
 protected:
  string name;
  string id;
  Kernel kernel;
  vector<Argument> arguments;
  int Function::Write(CommandQueue& command, Buffer& buffer, bool block,
                            unsigned int offset, std::size_t size,
                            const void* data_point);

 public:
  Function(string my_function_id, string kernel_function_name);
  int SetArgument(double* arg_pointer, std::size_t data_size, bool flag_change);

  friend class Keeper;
};

class Hive {
 protected:
  string id;
  string name;
  CommandQueue command;
  bool busy;
  

 public:
  Hive(CommandQueue& comm, string id_name, cl_device_type device_type);

  friend class Keeper;
};

class Garden {  // Platform

 protected:
  Platform platform;
  vector<Device> devices;
  vector<Hive> hives;
  Context context;
  Program program;
  vector<Function> functions;

 public:
  Garden();

  friend class Keeper;
};

class Keeper {
 protected:
  vector<Garden> gardens;
  vector<Task> tasks;
  string kernel;
  Program::Sources source;
  int Read(Hive& hive, Argument& arg, Task task);
  NDRange GetRange(vector<unsigned int> indexs);
 public:
  Keeper(string kernel_file_name);
  void Info();
  int SetGardens();
  int SetKernel(string file_name);
  int Build();
  int SetFunction(Function& function);
  int SetTask(string function_id, string parallel_method, vector<unsigned int> offset, vector<unsigned int> global_range,
              vector<unsigned int> local_range = {});
  int Start();

  int Execute(Hive& hive, Function& func, Task task);
};

