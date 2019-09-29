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
            gardens[j].devices.erase(gardens[j].devices.begin() + m);
            m--;
          }
        }
      }

      if (gardens[j].devices.size() == 0) {
        gardens.erase(gardens.begin() + j);
        j--;
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

int Keeper::SetTasks(string function_id, string parallel_method,
                     vector<unsigned int> steps,
                     vector<unsigned int> global_range,
                     vector<unsigned int> local_range) {
  switch (global_range.size()) {
    case 2:
      for (int i = 0; i < global_range[0] / steps[0]; i++) {
        for (int j = 0; j < global_range[1] / steps[1]; j++) {
          unsigned int offset_i = i * steps[0];
          unsigned int offset_j = j * steps[1];
          unsigned int global_i = steps[0] + i * steps[0];
          unsigned int global_j = steps[1] + j * steps[1];
          Task tmp(function_id, parallel_method, {offset_i, offset_j},
                   {global_i, global_j}, local_range);
          tasks.push_back(tmp);
        }
      }

      break;
    default:
      break;
  }

  return 0;
}

int Keeper::Execute(Hive& hive, Function& func, Task task) {
  int status = hive.command.enqueueNDRangeKernel(
      func.kernel, GetRange(task.offsets),
      GetGlobalRange(task.globals, task.offsets), GetRange(task.locals));

  hive.command.finish();
  hive.busy = false;

  if (status != CL_SUCCESS) {
    cout << "Error";
  }

  return 0;
}

int Keeper::Read() {


  for (auto& f : gardens[0].functions) {
    for (auto& arg : f.arguments) {
      if (arg.change == true) {
        gardens[0].hives[0].command.enqueueReadBuffer(arg.buffer, CL_TRUE, 0,
                                                      arg.size, arg.pointer);
      }
    }
  }
  return 1;
}


int Keeper::Start() {
  while (tasks.size() != 0) {
    for (auto& g : gardens) {
      for (auto& f : g.functions) {
        if (tasks.size() != 0 && f.id == tasks.back().function_id) {
          for (auto& h : g.hives) {
            if ((tasks.size() != 0) &&
                ((tasks.back().parallel_method == "ALL" && !h.busy) ||
                 (h.name == tasks.back().parallel_method && !h.busy))) {
              //cout << h.name + " ";
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

    for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.command.finish();
      
    }
  }
	


	
	    for (auto& g : gardens) {
    for (auto& h : g.hives) {
      while (h.busy) {
        cout << "busy" + h.name << endl;
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
                    unsigned int offset, unsigned int size,
                    const void* data_point) {
  command.enqueueWriteBuffer(buffer, block, offset, size, data_point);

  return 1;
}

Argument::Argument(void* arg_pointer, vector<unsigned int> dim,
                   unsigned int data_size, bool flag_change) {
  pointer = arg_pointer;
  size = data_size;
  dimension = dim;
  change = flag_change;
}

Function::Function(string my_function_id, string kernel_function_name) {
  id = my_function_id;
  name = kernel_function_name;
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

NDRange Keeper::GetGlobalRange(vector<unsigned int> global_range,
                               vector<unsigned int> offset) {
  NDRange range;
  switch (global_range.size()) {
    case 0:
      range = cl::NullRange;
      break;

    case 1:
      range = NDRange(global_range[0] - offset[0]);
      break;
    case 2:
      range = NDRange(global_range[0] - offset[0], global_range[1] - offset[1]);
      break;
    case 3:
      range = NDRange(global_range[0] - offset[0], global_range[1] - offset[1],
                      global_range[2] - offset[2]);
      break;
    default:
      cout << endl << "ERROR:  0 <= range <= 3" << endl;
      break;
  }

  return range;
  return NDRange();
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
