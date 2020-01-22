#include "..\include\QueenBee.hpp"

int Keeper::Wait() {
  cout << endl;
  while (tasks.size() != 0) {
  }
  cout << endl;
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.command.finish();
      while (h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() !=
             CL_COMPLETE) {
        cout << h.name + ":busy" << endl;
      }
    }
  }

  return 0;
}

Keeper::Keeper(string kernel_file_name) {
  SetGardens();
  SetKernel(kernel_file_name);
  Build();
}

void Keeper::Info(string mode) {
  cout << endl;

  if (mode == "ALL" || mode == "DEV") {
    for (const auto& g : gardens) {
      cout << g.platform.getInfo<CL_PLATFORM_NAME>() << endl;

      for (const auto& d : g.devices) {
        cout << d.getInfo<CL_DEVICE_TYPE>() << ": "
             << d.getInfo<CL_DEVICE_NAME>() << endl;
      }
    }
    cout << endl << kernel << endl;

    for (const auto& g : gardens) {
      for (const auto& h : g.hives) {
        cout << endl
             << h.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(h.device.back())
             << endl;
      }
    }
  }

  if (mode == "ALL" || mode == "STAT") {
    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        cout << h.name + ":" << h.completed.size() << endl;
      }
    }
  }

  cout << endl;
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

  for (int i = 0; i < gardens.size(); ++i) {
    for (int j = i + 1; j < gardens.size(); ++j) {
      for (int n = 0; n < gardens[i].devices.size(); ++n) {
        for (int m = 0; m < gardens[j].devices.size(); ++m) {
          if (gardens[i].devices[n].getInfo<CL_DEVICE_NAME>() ==
              gardens[j].devices[m].getInfo<CL_DEVICE_NAME>()) {
            gardens[j].devices.erase(gardens[j].devices.begin() + m);
            m--;
          }
        }
      }

      if (gardens[j].devices.size() == 0) {
        gardens.erase(gardens.begin() + j);
        --j;
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
    for (auto& d : g.devices) {
      Context context(d);
      Program program(context, source);
      CommandQueue command(context, d);
      Hive hive(d, command, context, program, d.getInfo<CL_DEVICE_NAME>(),
                d.getInfo<CL_DEVICE_TYPE>());
      g.hives.push_back(hive);

      if (!g.hives.back().program.build(g.hives.back().device) != CL_SUCCESS) {
        cout << g.hives.back().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d)
             << endl;
      }
    }
  }

  return 1;
}

int Keeper::SetFunction(Function& function) {
  int i = 0;
  Function tmp = function;

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      function = tmp;
      for (auto& f : h.functions) {
        if (f.id == function.id) {
          h.functions.pop_back();
        }
      }
      function = tmp;
      cl::Kernel func(h.program, function.name.c_str());
      function.kernel = func;
      i = 0;
      for (auto& a : function.arguments) {
        if (a.pointer != NULL) {
          Buffer buff(h.context, CL_MEM_READ_WRITE, a.size);
          a.buffer = buff;
          function.Write(h.command, a.buffer, CL_TRUE, 0, a.size, a.pointer);
          function.kernel.setArg(i, a.buffer);
        } else {
          function.kernel.setArg(i, a.size, a.pointer);
        }
        i++;
      }

      h.functions.push_back(function);
    }
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
    case 1:
      while (global_range[0] % steps[0] != 0) {
        steps[0] -= 1;
      }

	    if (local_range.size() == 1)
        while (steps[0] % local_range[0] != 0) {
          local_range[0] -= 1;
        }

      for (unsigned int i = 0; i < global_range[0] / steps[0]; i++) {
        unsigned int offset_i = i * steps[0];
        unsigned int global_i = steps[0] + i * steps[0];
        Task tmp(function_id, parallel_method, {offset_i}, {global_i},
                 local_range);
        tasks.push_back(tmp);
      }


      break;
    case 2:

		 while (global_range[0] % steps[0] != 0) {
        steps[0] -= 1;
      }

		  while (global_range[1] % steps[1] != 0) {
        steps[1] -= 1;
      }

		  if (local_range.size() == 1)
		  while (steps[0] % local_range[0] != 0) {
        local_range[0] -= 1;
		  }

		  if (local_range.size() == 2)
		   while (steps[1] % local_range[1] != 0) {
                    local_range[1] -= 1;
                  }


      for (unsigned int i = 0; i < global_range[0] / steps[0]; i++) {
        for (unsigned int j = 0; j < global_range[1] / steps[1]; j++) {
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

int Keeper::Read() {
  for (auto& g : gardens) {
    for (auto& hive : g.hives) {
      for (auto& task : hive.completed) {
        for (auto& f : hive.functions) {
          if (task.function_id == f.id) {
            for (auto& a : f.arguments) {
              if (a.change) {
                if (a.type == "float") Read<float>(hive, a, task, a.pointer);
                if (a.type == "double") Read<double>(hive, a, task, a.pointer);
                if (a.type == "int") Read<int>(hive, a, task, a.pointer);
                if (a.type == "unsigned int")
                  Read<unsigned int>(hive, a, task, a.pointer);
                if (a.type == "short") Read<short>(hive, a, task, a.pointer);
                if (a.type == "bool") Read<bool>(hive, a, task, a.pointer);
                if (a.type == "char") Read<char>(hive, a, task, a.pointer);
              }
            }
          }
        }
      }
    }
  }

  Wait();

  return 1;
}

int Keeper::Start() {
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.completed.clear();
    }
  }

  while (tasks.size() != 0) {
    tasks.size();
    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        for (auto& f : h.functions) {
          if (tasks.size() != 0 && f.id == tasks.back().function_id) {
            if (tasks.size() != 0) {
              h.command.flush();
              if (((tasks.back().parallel_method == "ALL" &&
                    h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                        CL_COMPLETE) ||
                   (h.name == tasks.back().parallel_method &&
                    h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                        CL_COMPLETE))) {
                int status = h.command.enqueueNDRangeKernel(
                    f.kernel, GetRange(tasks.back().offsets),
                    GetGlobalRange(tasks.back().globals, tasks.back().offsets),
                    GetRange(tasks.back().locals), NULL, &h.event);
                h.completed.push_back(tasks.back());
                if (tasks.size() != 0) tasks.pop_back();
                break;
              }
            }
          } else {
            if (tasks.size() != 0) tasks.pop_back();
          }
        }
      }
    }
  }

  return 1;
}

int Keeper::Test(string function_id, vector<unsigned int> global_range,
                 vector<unsigned int> local_range) {
  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  if (global_range.size() == 2) {
    SetTask(function_id, "CPU", {0, 0}, global_range, local_range);

    QueryPerformanceCounter(&t1);
    Start();
    Wait();
    Read();

    QueryPerformanceCounter(&t2);
    time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
    cout << "100% CPU 0% GPU: " << time << endl;

    //=====
    SetTask(function_id, "GPU", {0, 0}, global_range, local_range);

    QueryPerformanceCounter(&t1);
    Start();
    Wait();
    Read();

    QueryPerformanceCounter(&t2);
    time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
    cout << "0% CPU 100% GPU: " << time << endl;
    //========

    cout << endl;
    for (int i = 10; i <= 50; i += 10) {
      SetTasks(function_id, "ALL",
               {global_range[0] * i / 100, global_range[1] * i / 100},
               global_range, local_range);

      QueryPerformanceCounter(&t1);
      Start();
      Wait();
      Read();

      QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
      cout << "ALL " << i << "% : " << time << endl;
      Info("STAT");
    }

    cout << endl;

    cout << "X:" << endl;
    for (unsigned int i = 10; i < 100; i += 10) {
      SetTask(function_id, "GPU", {0, 0},
              {global_range[0] * i / 100, global_range[1]}, local_range);

      SetTask(function_id, "CPU", {global_range[0] * i / 100, 0},
              {global_range[0], global_range[1]}, local_range);

      QueryPerformanceCounter(&t1);
      Start();
      Wait();
      Read();

      QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
      cout << 100 - i << "% CPU " << i << "% GPU: " << time << endl;
    }

    cout << endl;

    cout << "Y:" << endl;
    for (unsigned int i = 10; i < 100; i += 10) {
      SetTask(function_id, "GPU", {0, 0},
              {global_range[1], global_range[0] * i / 100}, local_range);

      SetTask(function_id, "CPU", {0, global_range[0] * i / 100},
              {global_range[0], global_range[1]}, local_range);

      QueryPerformanceCounter(&t1);
      Start();
      Wait();
      Read();

      QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
      cout << 100 - i << "% CPU " << i << "% GPU: " << time << endl;
    }
  }
  //===========================================================
  if (global_range.size() == 1) {
    SetTask(function_id, "CPU", {0}, global_range, local_range);

    QueryPerformanceCounter(&t1);
    Start();
    Wait();
    Read();

    QueryPerformanceCounter(&t2);
    time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
    cout << "100% CPU 0% GPU: " << time << endl;

    //=====
    SetTask(function_id, "GPU", {0}, global_range, local_range);

    QueryPerformanceCounter(&t1);
    Start();
    Wait();
    Read();

    QueryPerformanceCounter(&t2);
    time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
    cout << "0% CPU 100% GPU: " << time << endl;
    //========

    cout << endl;
    for (int i = 10; i <= 50; i += 10) {
      SetTasks(function_id, "ALL",
               {global_range[0] * i / 100},
               global_range, local_range);

      QueryPerformanceCounter(&t1);
      Start();
      Wait();
      Read();

      QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
      cout << "ALL " << i << "% : " << time << endl;
      Info("STAT");
    }

    cout << endl;

    cout << "X:" << endl;
    for (unsigned int i = 10; i < 100; i += 10) {
      SetTask(function_id, "GPU", {0}, {global_range[0] * i / 100},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * i / 100},
              {global_range[0]}, local_range);

      QueryPerformanceCounter(&t1);
      Start();
      Wait();
      Read();

      QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
      cout << 100 - i << "% CPU " << i << "% GPU: " << time << endl;
    }

    cout << endl;
  }

  return 0;
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

Function::Function(string my_function_id, string kernel_function_name,
                   bool inv) {
  id = my_function_id;
  name = kernel_function_name;
  inverse = inv;
}

Hive::Hive(Device& dev, CommandQueue& comm, Context& cont, Program& prog,
           string id_name, cl_device_type device_type) {
  device.push_back(dev);
  context = cont;
  program = prog;
  command = comm;
  id = id_name;
  bool flag = true;

  if (device_type == CL_DEVICE_TYPE_CPU) name = "CPU";
  if (device_type == CL_DEVICE_TYPE_GPU) name = "GPU";
  if (device_type == CL_DEVICE_TYPE_ACCELERATOR) name = "ACC";

  cl::UserEvent tmp(cont);
  tmp.setStatus(CL_COMPLETE);
  event = tmp;
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
