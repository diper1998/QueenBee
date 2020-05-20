#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define _CL_ENABLE_EXCEPTIONS

#include <Cl/cl.hpp>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>
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
  Argument();
  Argument(void* arg_pointer, vector<unsigned int> dim, unsigned int data_size,
           bool flag_change = false);
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
  Function();
  Function(string my_function_id, string kernel_function_name,
           bool inverse = false);

  template <typename Type>
  int SetArgument(void* arg_pointer, vector<unsigned int> dim,
                  bool flag_chage = false) {
    unsigned int data_size = 1;
    for (auto& d : dim) {
      data_size *= d;
    }
    data_size *= sizeof(Type);
    Argument tmp(arg_pointer, dim, data_size, flag_chage);
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
  Task();
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
  vector<Function*> functions_ptrs;
  Event event;
  vector<Task> tasks;
  vector<double> work_time;
  double all_work_time;
  vector<double> read_time;
  double all_read_time;

 public:
  Hive();
  Hive(Device& dev, CommandQueue& comm, Context& cont, Program& prog,
       string id_name, cl_device_type device_type);

  friend class Keeper;
};

class Garden {  // Platform

 protected:
  Platform platform;
  vector<Device> devices;
  vector<Hive> hives;

  vector<Function> functions;
  Context context;
  Program program;

 public:
  Garden();

  friend class Keeper;
};

class Keeper {
 protected:
  double all_time;

  vector<Garden> gardens;
  vector<Task> tasks;
  string kernel;
  Program::Sources source;

  vector<std::thread*> threads;
  std::mutex lock;

  int Build();
  int SetGardens();
  int SetKernel(string file_name);
  NDRange GetRange(vector<unsigned int> indexs);
  NDRange GetGlobalRange(vector<unsigned int> global_range,
                         vector<unsigned int> offset);

  int Static();
  int Dynamic();
  void Keeper::ThreadFunction(Hive& h);
  void Keeper::ThreadFunctionDynamic(Hive& h, vector<Task>& tasks);
  int CompareValue(void* first, void* second, unsigned int position, string type);
  int Compare(vector<Argument> arguments,
              vector<void*> compare_result);


 public:
  Keeper();
  Keeper(string kernel_file_name);
  string Info(string mode = "ALL");
  int SetFunction(Function& function);
  int SetTask(string function_id, string parallel_method,
              vector<unsigned int> offset, vector<unsigned int> global_range,
              vector<unsigned int> local_range = {});
  int SetTasks(string function_id, string parallel_method,
               vector<unsigned int> steps, vector<unsigned int> global_range,
               vector<unsigned int> local_range = {});

  int Start(string mode = "STATIC");

  int Read(Hive& h, Function& function, Task& task);

  template <typename Type>
  int Read(Hive& hive, Argument& arg, Task task, void* pointer) {
    switch (task.globals.size()) {
      case 1:

        if (arg.dimension.size() == 1 && arg.dimension[0] == 1) {
          hive.command.enqueueReadBuffer(arg.buffer, CL_FALSE, 0, sizeof(Type),
                                         static_cast<Type*>(pointer));

        } else {
          hive.command.enqueueReadBuffer(
              arg.buffer, CL_FALSE, task.offsets[0] * sizeof(Type),
              (task.globals[0] - task.offsets[0]) * sizeof(Type),
              static_cast<Type*>(pointer) + task.offsets[0]);
        }
        break;

      case 2:

        if (arg.inverse) {
          for (unsigned int i = task.offsets[1]; i < task.globals[1]; ++i) {
            hive.command.enqueueReadBuffer(
                arg.buffer, CL_FALSE,
                (i * arg.dimension[1] + task.offsets[0]) * sizeof(Type),
                (task.globals[0] - task.offsets[0]) * sizeof(Type),
                static_cast<Type*>(pointer) + task.offsets[0] +
                    i * arg.dimension[1]);
          }
        }

        if (!arg.inverse) {
          for (unsigned int i = task.offsets[0]; i < task.globals[0]; ++i) {
            hive.command.enqueueReadBuffer(
                arg.buffer, CL_FALSE,
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

  int Test(unsigned int repeat_count, string function_id, unsigned int step,
           vector<unsigned int> global_range,
           vector<unsigned int> local_range = {},
           vector<void*> compare_result = {});

  

};
