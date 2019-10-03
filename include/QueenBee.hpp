#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <Cl/cl.hpp>
#endif
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <future>

using namespace std;
using namespace cl;

class Argument {
 protected:
  void* pointer;
  std::size_t size;
  vector<unsigned int> dimension;
  bool change;
  Buffer buffer;
  string type;

 public:
  Argument(void* arg_pointer, vector<unsigned int> dim, unsigned int data_size,
           bool flag_change);
  friend class Function;
  friend class Keeper;
};

class Task {
 protected:
  string function_id;
  string parallel_method;
  vector<unsigned int> globals;
  vector<unsigned int> locals;
  vector<unsigned int> offsets;

 public:
  Task(string my_funcion_id, string my_parallel_method,
       vector<unsigned int> my_offsets, vector<unsigned int> my_global_range,
       vector<unsigned int> my_local_range);
  friend class Keeper;
};

class Function {
 protected:
  string name;
  string id;
  Kernel kernel;
  vector<Argument> arguments;
  int Function::Write(CommandQueue& command, Buffer& buffer, bool block,
                      unsigned int offset, unsigned int size,
                      const void* data_point);

 public:
  Function(string my_function_id, string kernel_function_name);

  template <typename Type>
  int SetArgument(void* arg_pointer, vector<unsigned int> dim,
                  bool flag_change) {
    unsigned int data_size = 1;
    for (auto& d : dim) {
      data_size *= d;
    }
    data_size *= sizeof(Type);
    Argument tmp(arg_pointer, dim, data_size, flag_change);
    tmp.type = typeid(Type).name();
    arguments.push_back(tmp);
    return 1;
  }

  friend class Keeper;
};

class Hive {
 protected:
  string id;
  string name;
  CommandQueue command;
  bool busy;
  vector <future<int>*> fu;
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
  vector<thread> threads;
  vector<std::future<int>> fu;
  NDRange GetRange(vector<unsigned int> indexs);
  int Build();
  int SetGardens();
  int SetKernel(string file_name);
  NDRange GetGlobalRange(vector<unsigned int> global_range,
                         vector<unsigned int> offset);

 public:
  Keeper(string kernel_file_name);

  void Info();
  int SetFunction(Function& function);
  int SetTask(string function_id, string parallel_method,
              vector<unsigned int> offset, vector<unsigned int> global_range,
              vector<unsigned int> local_range = {});
  int SetTasks(string function_id, string parallel_method,
               vector<unsigned int> steps, vector<unsigned int> global_range,
               vector<unsigned int> local_range = {});
  int Start();
  int Execute(Hive& hive, Function& func, Task task);

  int Read();

  template <typename T>
  bool future_is_ready(std::future<T>& t) {
    return t.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }
};
