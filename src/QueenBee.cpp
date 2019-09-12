#include "..\include\QueenBee.hpp"

Keeper::Keeper(string kernel_file_name) {
  SetGardens();
  SetKernel(kernel_file_name);
  Build();
}

void Keeper::Info() {
  cout << endl;
  for (const auto& g : gardens) {
    cout << g.platform.getInfo<CL_PLATFORM_NAME>() << endl;

    for (const auto& d : g.devices) {
      cout << d.getInfo<CL_DEVICE_TYPE>() << ": " << d.getInfo<CL_DEVICE_NAME>()
           << endl;
    }
  }
  cout << endl << kernel << endl;

  for (const auto& g : gardens) {
    for (const auto& d : g.devices) {
      cout << endl << g.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d) << endl;
    }
  }
}

int Keeper::SetGardens() {  // all platforms with devices
  vector<Platform> platforms;
  Platform platform;
  Platform::get(&platforms);
  Garden garden;

  for (const auto& p : platforms) {
    garden.platform = p;
    garden.platform.getDevices(CL_DEVICE_TYPE_ALL, &garden.devices);

    gardens.push_back(garden);
  }

  
  for (int i = 0; i < gardens.size(); i++) {
    for (int j = i + 1; j < gardens.size(); j++) {
      for (int n = 0; n < gardens[i].devices.size(); n++) {
        for (int m = 0; m < gardens[j].devices.size(); m++) {
          if (gardens[i].devices[n].getInfo<CL_DEVICE_NAME>() ==
              gardens[j].devices[m].getInfo<CL_DEVICE_NAME>()) {
            gardens[i].devices.erase(gardens[i].devices.begin() + n);
            n--;
          }
        }
      }

      if (gardens[i].devices.size() == 0) {
        gardens.erase(gardens.begin() + i);
        i--;
      }
    }
  }
  
  return 1;
}

int Keeper::SetKernel(string file_name) {
  ifstream file(file_name);

  if (!file.is_open()) {
    cout << endl << "ERROR: SetKernel(string fname) fname is't found;" << endl;
    return 0;
  }

  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  string tmp(size, ' ');
  file.seekg(0);
  file.read(&tmp[0], size);
  file.close();
  kernel = tmp;

  source.push_back({kernel.c_str(), kernel.length()});

  return 1;
}

int Keeper::Build() {
  for (auto& g : gardens) {
    Context context(g.devices);
    g.context = context;

    Program program(g.context, source);
    g.program = program;

    if (g.program.build(g.devices) != CL_SUCCESS) {
      for (const auto& d : g.devices)
        cout << endl << g.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d) << endl;
    }

    for (auto& d : g.devices) {
      CommandQueue command(g.context, d);
      Hive hive(command, d.getInfo<CL_DEVICE_NAME>(),
                d.getInfo<CL_DEVICE_TYPE>());
      g.hives.push_back(hive);
    }
  }

  return 1;
}

int Keeper::SetFunction(Function& function) {
  int i = 0;
  Function tmp = function;

  for (auto& g : gardens) {
    function = tmp;

    for (auto& f : g.functions) {
      if (f.id == function.id) {
        g.functions.pop_back();
      }
    }

    cl::Kernel func(g.program, function.name.c_str());
    function.kernel = func;
    i = 0;
    for (auto& a : function.arguments) {
      Buffer buff(g.context, CL_MEM_READ_WRITE, a.size);
      function.Write(g.hives[0].command, buff, CL_TRUE, 0, a.size, a.pointer);
      a.buffer = buff;
      function.kernel.setArg(i, a.buffer);
      i++;
    }
    g.functions.push_back(function);
  }

  return 1;
}

int Keeper::SetTask(string function_id, string parallel_method,
                    vector<unsigned int> offset,
                    vector<unsigned int> global_range,
                    vector<unsigned int> local_range) {
  Task tmp(function_id, parallel_method, offset, global_range, local_range);
  tasks.push_back(tmp);

  return 1;
}

int Keeper::Read(Hive& hive, Argument& arg, Task task) {
  switch (task.globals.size()) {
    case 1:
      hive.command.enqueueReadBuffer(
          arg.buffer, true, task.offsets[0] * sizeof(double),
          (task.globals[0] - task.offsets[0]) * sizeof(double),
          arg.pointer + task.offsets[0]);

      break;

    case 2:

      for (int i = task.offsets[0]; i < task.globals[0]; i++) {
      
		  hive.command.enqueueReadBuffer(
            arg.buffer, false,
            (i*4 + task.offsets[1]) * sizeof(double),
             (task.globals[1] - task.offsets[1]) *
                sizeof(double),
            arg.pointer + task.offsets[1] + i*4);


	  }

	    hive.command.finish();

      break;

    default:
      break;
  }

  return 0;
}

int Keeper::Execute(Hive& hive, Function& func, Task task) {
  int status = hive.command.enqueueNDRangeKernel(
      func.kernel, GetRange(task.offsets), GetRange(task.globals),
      GetRange(task.locals));

  if (status != CL_SUCCESS) {
    cout << "Error";
  }

  hive.command.finish();

  for (auto& a : func.arguments) {
    if (a.change == true) {
      Read(hive, a, task);
    }
  }

  hive.busy = false;
  return 0;
}

int Keeper::Start() {
  vector<thread> threads;
  while (tasks.size() != 0) {
    for (auto& g : gardens) {
      for (auto& f : g.functions) {
        if (tasks.size() != 0 && f.id == tasks.back().function_id) {
          for (auto& h : g.hives) {
            if ((tasks.size() != 0) &&
                    ((tasks.back().parallel_method == "ALL" && !h.busy) ||
                (h.name == tasks.back().parallel_method && !h.busy))) {
              cout << h.id;
              h.busy = true;
              thread tmp(&Keeper::Execute, this, std::ref(h), f, tasks.back());
              threads.push_back(move(tmp));
              tasks.pop_back();
              break;
            }
          }
        }
      }
    }
  }

  for (auto& th : threads) {
    th.join();
  }

  return 1;
}

Garden::Garden() {}

int Function::Write(CommandQueue& command, Buffer& buffer, bool block,
                    unsigned int offset, std::size_t size,
                    const void* data_point) {
  command.enqueueWriteBuffer(buffer, block, offset, size, data_point);

  return 1;
}

Argument::Argument(double* arg_pointer, std::size_t data_size,
                   bool flag_change) {
  pointer = arg_pointer;
  size = data_size;

  change = flag_change;
}

Function::Function(string my_function_id, string kernel_function_name) {
  id = my_function_id;
  name = kernel_function_name;
}

int Function::SetArgument(double* arg_pointer, std::size_t data_size,
                          bool flag_change) {
  Argument tmp(arg_pointer, data_size, flag_change);
  arguments.push_back(tmp);
  return 1;
}

Hive::Hive(CommandQueue& comm, string id_name, cl_device_type device_type) {
  command = comm;
  id = id_name;
  bool flag = true;
  busy = false;

  if (device_type == CL_DEVICE_TYPE_CPU) name = "CPU";
  if (device_type == CL_DEVICE_TYPE_GPU) name = "GPU";
  if (device_type == CL_DEVICE_TYPE_ACCELERATOR) name = "ACC";
}

NDRange Keeper::GetRange(vector<unsigned int> indexs) {
  NDRange range;
  switch (indexs.size()) {
    case 0:
      range = cl::NullRange;
      break;

    case 1:
      range = NDRange(indexs[0]);
      break;
    case 2:
      range = NDRange(indexs[0], indexs[1]);
      break;
    case 3:
      range = NDRange(indexs[0], indexs[1], indexs[2]);
      break;
    default:
      cout << endl << "ERROR:  0 <= range <= 3" << endl;
      break;
  }

  return range;
}

Task::Task(string my_funcion_id, string my_parallel_method,
           vector<unsigned int> my_offset, vector<unsigned int> my_global_range,
           vector<unsigned int> my_local_range) {
  function_id = my_funcion_id;
  parallel_method = my_parallel_method;

  offsets = my_offset;
  globals = my_global_range;
  locals = my_local_range;
}
