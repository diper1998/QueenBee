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
  void* res;
  unsigned int size;
  vector<unsigned int> dimension;
  bool change;
  Buffer buffer;
  string type;
  bool inverse;

 public:
  Argument(void* arg_pointer, vector<unsigned int> dim, unsigned int data_size,
           bool flag_change);
  friend class Function;
  friend class Keeper;
};

class Function {
 protected:
  string name;
  string id;
  Kernel kernel;
  vector<Argument> arguments;
  bool inverse;
  int Function::Write(CommandQueue& command, Buffer& buffer, bool block,
                      unsigned int offset, unsigned int size,
                      const void* data_point);

 public:
  Function(string my_function_id, string kernel_function_name,
           bool inverse = false);

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
    tmp.inverse = inverse;
    arguments.push_back(tmp);
    return 1;
  }

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
 public:
  Hive(Device& dev, CommandQueue& comm, Context& cont, Program& prog,
       string id_name, cl_device_type device_type);

  friend class Keeper;
};

class Garden {  // Platform

 protected:
  Platform platform;
  vector<Device> devices;
  vector<Hive> hives;

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
  NDRange GetRange(vector<unsigned int> indexs);
  int Build();
  int SetGardens();
  int SetKernel(string file_name);
  NDRange GetGlobalRange(vector<unsigned int> global_range,
                         vector<unsigned int> offset);

  int Launch();

 public:
  Keeper(string kernel_file_name);
  int Wait();
  void Info(string mode = "ALL");
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
  int Read(void (*pt2Func)(Type* x, Type* y), string function) {
    vector<Type*> results;
    Type* pointer;
    for (auto& g : gardens) {
      for (auto& hive : g.hives) {
        for (auto& task : hive.completed) {
          for (auto& f : hive.functions) {
            if (task.function_id == f.id) {
              for (auto& a : f.arguments) {
                if (a.change) {
                  pointer = static_cast<Type*>(a.pointer);
                  a.res = new Type[a.size / sizeof(Type)];
                  Read<Type>(hive, a, task, a.res);
                  results.push_back(static_cast<Type*>(a.res));
                }
              }
            }
          }
        }
      }
    }

    for (auto r : results) {
      pt2Func(pointer, r);
    }

    return 1;
  };

  template <typename Type>
  int Read(Hive& hive, Argument& arg, Task task, void* pointer) {
    switch (task.globals.size()) {
      case 1:

        if (arg.dimension.size() == 1 && arg.dimension[0] == 1) {
          hive.command.enqueueReadBuffer(arg.buffer, CL_TRUE, 0, sizeof(Type),
                                         static_cast<Type*>(pointer));

        } else {
          hive.command.enqueueReadBuffer(
              arg.buffer, CL_TRUE, task.offsets[0] * sizeof(Type),
              (task.globals[0] - task.offsets[0]) * sizeof(Type),
              static_cast<Type*>(pointer) + task.offsets[0]);
        }
        break;

      case 2:       

        if (arg.inverse) {
          for (unsigned int i = task.offsets[1]; i < task.globals[1]; i++) {
            hive.command.enqueueReadBuffer(
                arg.buffer, CL_TRUE,
                (i * arg.dimension[1] + task.offsets[0]) * sizeof(Type),
                (task.globals[0] - task.offsets[0]) * sizeof(Type),
                static_cast<Type*>(pointer) + task.offsets[0] +
                    i * arg.dimension[1]);
          }
        }

        if (!arg.inverse) {
          for (unsigned int i = task.offsets[0]; i < task.globals[0]; i++) {
            hive.command.enqueueReadBuffer(
                arg.buffer, CL_TRUE,
                (i * arg.dimension[0] + task.offsets[1]) * sizeof(Type),
                (task.globals[1] - task.offsets[1]) * sizeof(Type),
                static_cast<Type*>(pointer) + task.offsets[1] +
                    i * arg.dimension[0]);
          }
        }

        break;

      default:
        break;
    }

    return 0;
  };


};
