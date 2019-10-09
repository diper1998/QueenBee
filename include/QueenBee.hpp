#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <Cl/cl.hpp>
#endif
#include <windows.h>
#include <fstream>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace cl;

class Argument {
 protected:
  void* pointer;
  unsigned int size;
  vector<unsigned int> dimension;
  bool change;
  Buffer buffer;
  vector<Buffer> buffers;
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
  Context context;
  vector<Device> device;
  Program program;
  CommandQueue command;
  vector<Function> functions;
  Event event;
  vector<Task> completed;

      public : Hive(Device& dev, CommandQueue& comm, Context& cont,
                    Program& prog, string id_name, cl_device_type device_type);

  friend class Keeper;
};

class Garden {  // Platform

 protected:
  Platform platform;
  vector<Device> devices;
  vector<Hive> hives;
  // Context context;
  // Program program;

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

   template <typename Type>
  int Read(Hive& hive, Argument& arg, Task task) {
    switch (task.globals.size()) {
      case 1:
        hive.command.enqueueReadBuffer(
            arg.buffer, CL_TRUE, task.offsets[0] * sizeof(Type),
            (task.globals[0] - task.offsets[0]) * sizeof(Type),
            static_cast<Type*>(arg.pointer) + task.offsets[0]);

        break;

      case 2:
        
              hive.command.enqueueReadBuffer(arg.buffer, CL_TRUE, 0, arg.size,
                                          arg.pointer);
            
          
        
        /*
        for (unsigned int i = task.offsets[0]; i < task.globals[0]; i++) {
          hive.command.enqueueReadBuffer(
              arg.buffer, CL_TRUE,
              (i * arg.dimension[0] + task.offsets[1]) * sizeof(Type),
              (task.globals[1] - task.offsets[1]) * sizeof(Type),
              static_cast<Type*>(arg.pointer) + task.offsets[1] +
                  i * arg.dimension[0]);
        }
		*/
        break;

      default:
        break;
    }

    return 0;
  };

};
