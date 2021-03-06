#include "..\include\QueenBee.hpp"

//////////////////
//// ARGUMENT ////
//////////////////

Argument::Argument() {}

Argument::Argument(void* arg_pointer, vector<unsigned int> dim,
                   unsigned int data_size, bool flag_change) {
  pointer = arg_pointer;
  size = data_size;
  dimension = dim;
  change = flag_change;
}

//////////////////
//// FUNCTION ////
//////////////////

Function::Function() {}

Function::Function(string my_function_id, string kernel_function_name,
                   bool inv) {
  id = my_function_id;
  name = kernel_function_name;
  inverse = inv;
}

int Function::Write(CommandQueue& command, Buffer& buffer, bool block,
                    unsigned int offset, unsigned int size,
                    const void* data_point) {
  command.enqueueWriteBuffer(buffer, block, offset, size, data_point);

  return 1;
}

//////////////
//// TASK ////
//////////////

Task::Task() {}

Task::Task(string my_funcion_id, string my_parallel_method,
           vector<unsigned int> my_offset, vector<unsigned int> my_global_range,
           vector<unsigned int> my_local_range) {
  function_id = my_funcion_id;
  parallel_method = my_parallel_method;

  offsets = my_offset;
  globals = my_global_range;
  locals = my_local_range;
}

//////////////
//// HIVE ////
//////////////

Hive::Hive() {}

Hive::Hive(Device& dev, CommandQueue& comm, Context& cont, Program& prog,
           string id_name, cl_device_type device_type) {
  device.push_back(dev);
  context = cont;
  program = prog;
  command = comm;
  id = id_name;

  if (device_type == CL_DEVICE_TYPE_CPU) name = "CPU";
  if (device_type == CL_DEVICE_TYPE_GPU) name = "GPU";
  if (device_type == CL_DEVICE_TYPE_ACCELERATOR) name = "ACC";

  cl::UserEvent tmp(cont);
  tmp.setStatus(CL_COMPLETE);
  event = tmp;
}

////////////////
//// GARDEN ////
////////////////

Garden::Garden() {}

////////////////
//// KEEPER ////
////////////////

Keeper::Keeper() {}

Keeper::Keeper(string kernel_file_name) {
  SetGardens();
  SetKernel(kernel_file_name);
  Build();
}

int Keeper::Build() {
  for (auto& g : gardens) {
    Context context(g.devices);
    g.context = context;
    Program program(context, source);
    g.program = program;
    for (auto& d : g.devices) {
      CommandQueue command(g.context, d, CL_QUEUE_PROFILING_ENABLE);
      // CommandQueue command(g.context, d);

      Hive hive(d, command, context, program, d.getInfo<CL_DEVICE_NAME>(),
                d.getInfo<CL_DEVICE_TYPE>());
      g.hives.push_back(hive);

      if (g.hives.back().program.build(g.hives.back().device) != CL_SUCCESS) {
        cout << g.hives.back().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d)
             << endl;
      }
    }
  }
  /* for (auto& g : gardens) {
     for (auto& d : g.devices) {
       Context context(d);
       Program program(context, source);
       CommandQueue command(context, d);
       Hive hive(d, command, context, program, d.getInfo<CL_DEVICE_NAME>(),
                 d.getInfo<CL_DEVICE_TYPE>());
       g.hives.push_back(hive);

       if (g.hives.back().program.build(g.hives.back().device) != CL_SUCCESS)
   { cout << g.hives.back().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d)
              << endl;
       }
     }
   }
   */
  return 1;
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

string Keeper::Info(string mode) {
  std::stringstream ss;
  ss << "";

  if (mode == "ALL" || mode == "DEV") {
    for (const auto& g : gardens) {
      cout << g.platform.getInfo<CL_PLATFORM_NAME>() << endl;
      ss << g.platform.getInfo<CL_PLATFORM_NAME>();
      for (const auto& d : g.devices) {
        cout << d.getInfo<CL_DEVICE_TYPE>() << ": "
             << d.getInfo<CL_DEVICE_NAME>() << endl
             << "MAX COMPUTE UNITS: "
             << d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl
             << "GLOBAL MEM SIZE: "
             << d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1000000000.0 << " GB"
             << endl
             << "MAX CLOCK FREQUENCY: "
             << d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() / 1000.0 << "GHz"
             << endl;

        ss << d.getInfo<CL_DEVICE_TYPE>() << ": "
           << d.getInfo<CL_DEVICE_NAME>();
        ss << endl;
        ss << "MAX COMPUTE UNITS: " << d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        ss << endl;
        ss << "GLOBAL MEM SIZE: ";
        ss << d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1000000000.0 << " GB";
        ss << endl;
        ss << "MAX CLOCK FREQUENCY: ";
        ss << d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() / 1000.0 << "GHz";
        ss << endl;
      }
    }
  }

  if (mode == "ALL" || mode == "KER") {
    cout << endl << kernel << endl;

    for (const auto& g : gardens) {
      for (const auto& h : g.hives) {
        cout << endl
             << h.name << endl
             << h.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(h.device.back())
             << endl;

        ss << endl
           << h.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(h.device.back())
           << endl;
      }
    }
  }

  if (mode == "ALL" || mode == "TIME") {
    LARGE_INTEGER frequency;
    int i = 0;
    QueryPerformanceFrequency(&frequency);
    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        cout << h.name << endl;
        ss << h.name << endl;

        h.all_work_time = 0;
        h.all_read_time = 0;

        for (int i = 0; i < h.work_time.size(); i++) {
          cout << i << ": WORK = " << h.work_time[i]
               << " READ = " << h.read_time[i] << endl;

          ss << i << ": WORK = " << h.work_time[i]
             << " READ = " << h.read_time[i] << endl;

          h.all_work_time += h.work_time[i];
          h.all_read_time += h.read_time[i];
        }

        // for (auto& t1 : h.time1) {
        //  cout  << "ch: " << t1 << endl;
        //  //i++;
        //  //h.work_time += t;
        //}
      }
    }

    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        cout << h.name << ": ALL_WORK = " << h.all_work_time
             << " ALL_READ = " << h.all_read_time << endl;

        ss << h.name << ": ALL_WORK = " << h.all_work_time;
        ss << " ALL_READ = " << h.all_read_time << endl;
      }
    }

    cout << "ALL time: " << all_time << endl;

    ss << "ALL time: " << all_time << endl;
  }

  cout << endl;

  // ss >> info;

  return ss.str();
}

int Keeper::SetFunction(Function& function) {
  int i = 0;
  Function tmp = function;

  for (auto& g : gardens) {
    // for (auto& h : g.hives) {
    function = tmp;
    for (auto& f : g.functions) {  //  for (auto& f : h.functions) {
      if (f.id == function.id) {
        // h.functions.pop_back();
        g.functions.pop_back();
      }
    }
    function = tmp;
    // cl::Kernel func(h.program, function.name.c_str());
    cl::Kernel func(g.program, function.name.c_str());
    function.kernel = func;
    i = 0;
    for (auto& a : function.arguments) {
      if (a.pointer != NULL) {
        // Buffer buff(h.context, CL_MEM_READ_WRITE, a.size);
        // Buffer buff(g.context, CL_MEM_USE_HOST_PTR, a.size, a.pointer);
        Buffer buff(g.context, CL_MEM_ALLOC_HOST_PTR, a.size);
        a.buffer = buff;
        // function.Write(h.command, a.buffer, CL_TRUE, 0, a.size, a.pointer);
        function.Write(g.hives[0].command, a.buffer, CL_TRUE, 0, a.size,
                       a.pointer);
        function.kernel.setArg(i, a.buffer);
      } else {
        function.kernel.setArg(i, a.size, a.pointer);
      }
      i++;
    }

    // h.functions.push_back(function);
    g.functions.push_back(function);

    //}
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      for (auto& f : g.functions) {
        h.functions_ptrs.push_back(&f);
      }
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

      for (unsigned int i = 0; i < global_range[0] / steps[0]; ++i) {
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

      for (unsigned int i = 0; i < global_range[0] / steps[0]; ++i) {
        for (unsigned int j = 0; j < global_range[1] / steps[1]; ++j) {
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

int Keeper::Start(string mode) {
  LARGE_INTEGER at_begin;
  LARGE_INTEGER at_end;

  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  QueryPerformanceCounter(&at_begin);

  if (mode == "STATIC") Static();

  if (mode == "DYNAMIC") Dynamic();

  QueryPerformanceCounter(&at_end);

  all_time = (at_end.QuadPart - at_begin.QuadPart) / double(frequency.QuadPart);

  return 1;
}

int Keeper::Static() {
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.work_time.clear();
      h.read_time.clear();
      h.tasks.clear();
    }
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      for (int i = tasks.size(); i >= 0; --i) {
        if ((tasks[i].parallel_method == h.name) ||
            (h.name == "GPU" && tasks[i].parallel_method == "ALL")) {
          h.tasks.push_back(tasks[i]);
          tasks.pop_back();
        }
      }
    }
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      if (h.tasks.size() != 0) {
        std::thread* thr =
            new thread(&Keeper::ThreadFunction, this, std::ref(h));
        threads.push_back(thr);
      }
    }
  }

  for (auto& t : threads) {
    t->join();
  }

  tasks.clear();
  threads.clear();

  return 0;
}

void Keeper::ThreadFunction(Hive& h) {
  cl_ulong time_end, time_start;
  LARGE_INTEGER start_read;
  LARGE_INTEGER end_read;
  LARGE_INTEGER frequency;

  for (auto& t : h.tasks) {
    for (auto& f : h.functions_ptrs) {
      if (f->id == t.function_id) {
        h.work_time.push_back(0);
        h.read_time.push_back(0);
        int status =
            h.command.enqueueNDRangeKernel(f->kernel, GetRange(t.offsets),
                                           GetGlobalRange(t.globals, t.offsets),
                                           GetRange(t.locals), NULL, &h.event);

        h.event.wait();

        h.event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
        h.event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);

        h.work_time.back() = double(time_end - time_start) / 1000000000;

        QueryPerformanceFrequency(&frequency);

        QueryPerformanceCounter(&start_read);

        Read(h, *f, t);

        QueryPerformanceCounter(&end_read);

        h.read_time.back() = double(end_read.QuadPart - start_read.QuadPart) /
                             double(frequency.QuadPart);

        h.command.finish();
      }
    }
  }
}

int Keeper::Dynamic() {
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.work_time.clear();
      h.read_time.clear();
      h.tasks.clear();
    }
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      if (tasks.size() != 0) {
        std::thread* thr = new thread(&Keeper::ThreadFunctionDynamic, this,
                                      std::ref(h), std::ref(tasks));

        threads.push_back(thr);
      }
    }
  }

  for (auto& t : threads) {
    t->join();
  }

  tasks.clear();
  threads.clear();

  return 0;
}

void Keeper::ThreadFunctionDynamic(Hive& h, vector<Task>& tasks) {
  cl_ulong time_end, time_start;
  Task t;
  while (tasks.size() != 0) {
    lock.lock();
    if (tasks.size() != 0) {
      t = tasks.back();
      tasks.pop_back();
    }
    lock.unlock();

    for (auto& f : h.functions_ptrs) {
      h.work_time.push_back(0);
      h.read_time.push_back(0);

      if (f->id == t.function_id) {
        int status =
            h.command.enqueueNDRangeKernel(f->kernel, GetRange(t.offsets),
                                           GetGlobalRange(t.globals, t.offsets),
                                           GetRange(t.locals), NULL, &h.event);

        h.event.wait();

        h.event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
        h.event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);

        h.work_time.back() = double(time_end - time_start) / 1000000000;

        LARGE_INTEGER time_start;
        LARGE_INTEGER time_end;
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        QueryPerformanceCounter(&time_start);

        Read(h, *f, t);

        QueryPerformanceCounter(&time_end);

        h.read_time.back() = (double(time_end.QuadPart - time_start.QuadPart) /
                              double(frequency.QuadPart));

        h.command.finish();
      }
    }
  }
}

int Keeper::Read(Hive& hive, Function& function, Task& task) {
  for (auto& a : function.arguments) {
    if (a.change) {
      if (a.type == "float") {
        Read<float>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "double") {
        Read<double>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "int") {
        Read<int>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "unsigned int") {
        Read<unsigned int>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "short") {
        Read<short>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "bool") {
        Read<bool>(hive, a, task, a.pointer);
        continue;
      }
      if (a.type == "char") {
        Read<char>(hive, a, task, a.pointer);
        continue;
      }
    }
  }

  return 1;
}

int Keeper::CompareValue(void* first, void* second, unsigned int position,
                         string type) {
  if (type == "float") {
    if (fabsf(*(static_cast<float*>(first) + position) -
              *(static_cast<float*>(second) + position)) < 0.01) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "double") {
    if (fabs(*(static_cast<double*>(first) + position) -
             *(static_cast<double*>(second) + position)) < 0.0001) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "int") {
    if (*(static_cast<int*>(first) + position) ==
        *(static_cast<int*>(second) + position)) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "unsigned int") {
    if (*(static_cast<unsigned int*>(first) + position) ==
        *(static_cast<unsigned int*>(second) + position)) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "short") {
    if (*(static_cast<short*>(first) + position) ==
        *(static_cast<short*>(second) + position)) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "bool") {
    if (*(static_cast<bool*>(first) + position) ==
        *(static_cast<bool*>(second) + position)) {
      return 1;
    } else {
      return 0;
    }
  }

  if (type == "char") {
    if (*(static_cast<char*>(first) + position) ==
        *(static_cast<char*>(second) + position)) {
      return 1;
    } else {
      return 0;
    }
  }

  return 0;
}

int Keeper::Compare(vector<Argument> arguments, vector<void*> compare_result) {
  int j = 0;
  int error = 0;

  for (int i = 0; i < arguments.size(); ++i) {
    if (arguments[i].change == true) {
      if (arguments[i].dimension.size() == 2) {
        for (unsigned int k = 0;
             k < arguments[i].dimension[0] * arguments[i].dimension[1]; ++k) {
          if (!CompareValue(arguments[i].pointer, compare_result[j], k,
                            arguments[i].type)) {
            ++error;
          }
        }
      }

      if (arguments[i].dimension.size() == 1) {
        for (unsigned int k = 0; k < arguments[i].dimension[0]; ++k) {
          if (!CompareValue(arguments[i].pointer, compare_result[j], k,
                            arguments[i].type)) {
            ++error;
          }
        }
      }

      ++j;
    }
  }

  if (error == 0) {
    return 0;
  }

  return 1;
}

int Keeper::Test(unsigned int repeat_count, string function_id,
                 unsigned int step, vector<unsigned int> global_range,
                 vector<unsigned int> local_range,
                 vector<void*> compare_result) {
  std::ofstream perfomance_static;
  std::ofstream perfomance_dynamic;
  std::ofstream perfomance_results;
  std::stringstream ss;

  if (global_range.size() == 1) ss << global_range[0];

  if (global_range.size() == 2) ss << global_range[0] << "x" << global_range[1];

  std::string str;
  ss >> str;
  perfomance_static.open(function_id + str + string("_static") +
                         string(".txt"));
  perfomance_dynamic.open(function_id + str + string("_dynamic") +
                          string(".txt"));
  perfomance_results.open(function_id + str + string("_results") +
                          string(".txt"));

  vector<double> time_x;
  vector<double> time_y;

  int count = repeat_count;
  double time_cpu = 0;
  double time_gpu = 0;

  double time_x50gpu_work = 0;
  double time_x50cpu_work = 0;

  double time_y50gpu_work = 0;
  double time_y50cpu_work = 0;

  double time_x50gpu_read = 0;
  double time_x50cpu_read = 0;
  double time_y50gpu_read = 0;
  double time_y50cpu_read = 0;

  double time_x50 = 0;
  double time_y50 = 0;

  double time_100gpu_work = 0;
  double time_100cpu_work = 0;

  double time_100gpu_read = 0;
  double time_100cpu_read = 0;

  double try_time_xcpu_work = 0;
  double try_time_xgpu_work = 0;
  double try_time_xcpu_read = 0;
  double try_time_xgpu_read = 0;

  double try_time_ycpu_work = 0;
  double try_time_ygpu_work = 0;
  double try_time_ycpu_read = 0;
  double try_time_ygpu_read = 0;

  double averege_time = 0;
  double try_time_x = 0;
  double try_time_y = 0;
  double time_dynamic = 0;
  double best_time_dynamic = 0;
  double best_percent = 0;

  unsigned int percent = 0;
  unsigned int split_x = 0;
  unsigned int split_y = 0;

  bool error_flag = false;

  if (global_range.size() == 2) {
    percent = 10;
    split_x = global_range[0] * percent / 100;
    split_y = global_range[1] * percent / 100;

    // TEST DYNAMIC

    percent = 10;
    split_x = global_range[0] * percent / 100;
    split_y = global_range[1] * percent / 100;

    cout << "DYNAMIC" << endl;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;
      SetTasks(function_id, "CPU", {split_x, split_y}, global_range,
               local_range);

      Start("DYNAMIC");
      Info("TIME");
      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    best_time_dynamic = time_dynamic;
    best_percent = percent;

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    time_dynamic = 0;

    percent = 20;
    split_x = global_range[0] * percent / 100;
    split_y = global_range[1] * percent / 100;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;

      SetTasks(function_id, "CPU", {split_x, split_y}, global_range,
               local_range);

      Start("DYNAMIC");
      Info("TIME");
      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    if (time_dynamic < best_time_dynamic) {
      best_time_dynamic = time_dynamic;
      best_percent = percent;
    }

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    time_dynamic = 0;

    percent = 50;
    split_x = global_range[0] * percent / 100;
    split_y = global_range[1] * percent / 100;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;

      SetTasks(function_id, "CPU", {split_x, split_y}, global_range,
               local_range);

      Start("DYNAMIC");
      Info("TIME");
      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    if (time_dynamic < best_time_dynamic) {
      best_time_dynamic = time_dynamic;
      best_percent = percent;
    }

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    // TEST STATIC
    cout << "STATIC" << endl;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "100% CPU 0% GPU: " << endl;
      SetTask(function_id, "CPU", {0, 0}, global_range, local_range);
      Start();
      Info("TIME");

      time_cpu += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }
    time_cpu /= count;
    perfomance_static << "0\t" << time_cpu << endl;

    time_x.push_back(time_cpu);

    cout << endl;

    cout << "X" << endl;
    for (unsigned int i = step; i < 100; i += step) {
      averege_time = 0;
      for (int n = 0; n < count; n++) {
        cout << "________________" << endl;
        cout << 100 - i << "% CPU " << i << "% GPU: " << endl;
        SetTask(function_id, "GPU", {0, 0},
                {global_range[0] * i / 100, global_range[1]}, local_range);

        SetTask(function_id, "CPU", {global_range[0] * i / 100, 0},
                {global_range[0], global_range[1]}, local_range);

        Start();
        Info("TIME");

        averege_time += all_time;

        if (i == 50) {
          for (auto& g : gardens) {
            for (auto& h : g.hives) {
              if (h.name == "CPU") {
                time_x50cpu_work += h.all_work_time;
                time_x50cpu_read += h.all_read_time;
              }

              if (h.name == "GPU") {
                time_x50gpu_work += h.all_work_time;
                time_x50gpu_read += h.all_read_time;
              }
            }
          }

          time_x50 += all_time;
        }

        if (compare_result.size() != 0) {
          for (auto& g : gardens) {
            for (auto& f : g.functions) {
              if (Compare(f.arguments, compare_result)) {
                cout << "ERROR" << endl;
                error_flag = true;
                break;
              }
            }

            if (error_flag) break;
          }
        }
      }
      averege_time /= count;
      perfomance_static << i << "\t" << averege_time << endl;
      time_x.push_back(averege_time);
    }

    time_gpu = 0;
    averege_time = 0;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "0% CPU 100% GPU: " << endl;
      SetTask(function_id, "GPU", {0, 0}, global_range, local_range);
      Start();
      Info("TIME");

      time_gpu += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_gpu /= count;
    perfomance_static << "100\t" << time_gpu << endl;
    time_x.push_back(time_gpu);
    cout << endl;

    //=====
    time_cpu = 0;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "100% CPU 0% GPU: " << endl;
      SetTask(function_id, "CPU", {0, 0}, global_range, local_range);
      Start();
      Info("TIME");
      time_cpu += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "CPU") {
            time_100cpu_work += h.all_work_time;
            time_100cpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_cpu /= count;
    time_y.push_back(time_cpu);
    perfomance_static << "0\t" << time_cpu << endl;

    cout << "Y" << endl;
    for (unsigned int i = step; i < 100; i += step) {
      averege_time = 0;
      for (int n = 0; n < count; n++) {
        cout << "________________" << endl;
        cout << 100 - i << "% CPU " << i << "% GPU: " << endl;

        SetTask(function_id, "GPU", {0, 0},
                {global_range[1], global_range[0] * i / 100}, local_range);

        SetTask(function_id, "CPU", {0, global_range[0] * i / 100},
                {global_range[0], global_range[1]}, local_range);

        Start();
        Info("TIME");
        averege_time += all_time;

        if (i == 50) {
          for (auto& g : gardens) {
            for (auto& h : g.hives) {
              if (h.name == "CPU") {
                time_y50cpu_work += h.all_work_time;
                time_y50cpu_read += h.all_read_time;
              }

              if (h.name == "GPU") {
                time_y50gpu_work += h.all_work_time;
                time_y50gpu_read += h.all_read_time;
              }
            }
          }

          time_y50 += all_time;
        }

        if (compare_result.size() != 0) {
          for (auto& g : gardens) {
            for (auto& f : g.functions) {
              if (Compare(f.arguments, compare_result)) {
                cout << "ERROR" << endl;
                error_flag = true;
                break;
              }
            }

            if (error_flag) break;
          }
        }
      }

      averege_time /= count;
      perfomance_static << i << "\t" << averege_time << "\t" << endl;
      time_y.push_back(averege_time);
    }

    time_gpu = 0;
    averege_time = 0;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "0% CPU 100% GPU: " << endl;
      SetTask(function_id, "GPU", {0, 0}, global_range, local_range);
      Start();
      Info("TIME");

      time_gpu += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "GPU") {
            time_100gpu_work += h.all_work_time;
            time_100gpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_gpu /= count;
    perfomance_static << "100\t" << time_gpu << endl;
    time_y.push_back(time_gpu);
    cout << endl;

    int cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    int gpu_part = 100 - cpu_part;

    try_time_x = 0;

    cout << "ANALYTIC" << endl << endl;

    cout << "X:" << endl << endl;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;
      SetTask(function_id, "GPU", {0, 0},
              {global_range[0] * int(gpu_part) / 100, global_range[1]},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * int(gpu_part) / 100, 0},
              {global_range[0], global_range[1]}, local_range);

      Start();
      Info("TIME");

      try_time_x += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "CPU") {
            try_time_xcpu_work += h.all_work_time;
            try_time_xcpu_read += h.all_read_time;
          }

          if (h.name == "GPU") {
            try_time_xgpu_work += h.all_work_time;
            try_time_xgpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    try_time_x /= count;

    try_time_y = 0;

    cout << "Y: " << endl << endl;
    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;

      SetTask(function_id, "GPU", {0, 0},
              {global_range[1], global_range[0] * int(gpu_part) / 100},
              local_range);

      SetTask(function_id, "CPU", {0, global_range[0] * int(gpu_part) / 100},
              {global_range[0], global_range[1]}, local_range);
      Start();
      Info("TIME");

      try_time_y += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "CPU") {
            try_time_ycpu_work += h.all_work_time;
            try_time_ycpu_read += h.all_read_time;
          }

          if (h.name == "GPU") {
            try_time_ygpu_work += h.all_work_time;
            try_time_ygpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    try_time_y /= count;

    perfomance_results << Info("DEV");

    cout << "TEST RESULTS:" << endl << endl;

    cout << "DYNAMIC:" << endl << endl;
    cout << "PERCENT: " << best_percent << "%" << endl;
    cout << "TIME: " << best_time_dynamic << endl;
    cout << endl;

    cout << "ANALYTIC:" << endl << endl;

    cout << "100% CPU" << endl;
    cout << "CPU: WORK = " << time_100cpu_work / count
         << " READ = " << time_100cpu_read / count << endl;
    cout << "ALL_WORK = " << time_cpu << endl;
    cout << endl;
    cout << "100% GPU" << endl;
    cout << "GPU: WORK = " << time_100gpu_work / count
         << " READ = " << time_100gpu_read / count << endl;
    cout << "ALL_WORK = " << time_gpu << endl;

    cout << endl;

    cout << "X" << endl;
    cout << "50% CPU 50% GPU" << endl;
    cout << "CPU: WORK = " << time_x50cpu_work / count
         << " READ = " << time_x50cpu_read / count << endl;
    cout << "GPU: WORK = " << time_y50gpu_work / count
         << " READ = " << time_x50gpu_read / count << endl;

    cout << "ALL_WORK = " << time_x50 / count << endl;
    cout << endl;

    cout << "Y" << endl;
    cout << "50% CPU 50% GPU" << endl;
    cout << "CPU: WORK = " << time_y50cpu_work / count
         << " READ = " << time_y50cpu_read / count << endl;
    cout << "GPU: WORK = " << time_y50gpu_work / count
         << " READ = " << time_y50gpu_read / count << endl;

    cout << "ALL_WORK = " << time_y50 / count << endl;

    cout << endl;

    cout << "BEST:" << endl;
    cout << "CPU PART = " << cpu_part << "%" << endl;
    cout << "CPU TIME = " << (cpu_part * time_cpu) / 100.0 << endl << endl;

    cout << "GPU PART = " << gpu_part << "%" << endl;
    cout << "GPU TIME = " << (gpu_part * time_gpu) / 100.0 << endl;
    cout << endl;
    cout << "TRY IT:" << endl << endl;

    cout << "X " << endl;
    cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;
    cout << "CPU: WORK = " << try_time_xcpu_work / count
         << " READ = " << try_time_xcpu_read / count << endl;

    cout << "GPU: WORK = " << try_time_xgpu_work / count
         << " READ = " << try_time_xgpu_read / count << endl;

    cout << "ALL_WORK = " << try_time_x << endl;

    cout << "Y " << endl;
    cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;
    cout << "CPU: WORK = " << try_time_ycpu_work / count
         << " READ = " << try_time_ycpu_read / count << endl;

    cout << "GPU: WORK = " << try_time_ygpu_work / count
         << " READ = " << try_time_ygpu_read / count << endl;

    cout << "ALL_WORK = " << try_time_y << endl << endl;

    int idx_min = 0;
    int idy_min = 0;
    for (int i = 0; i < time_x.size(); i++) {
      if (time_x[idx_min] > time_x[i]) idx_min = i;
    }

    for (int i = 0; i < time_y.size(); i++) {
      if (time_y[idy_min] > time_y[i]) idy_min = i;
    }

    cout << "STATIC:" << endl << endl;

    cout << "BEST:" << endl;

    stringstream best;

    cout << "X " << endl;
    cout << 100 - idx_min * step << "% CPU " << idx_min * step
         << "% GPU: " << endl;
    cout << "TIME: " << time_x[idx_min] << endl;

    cout << "Y " << endl;
    cout << 100 - idy_min * step << "% CPU " << idy_min * step
         << "% GPU: " << endl;
    cout << "TIME: " << time_y[idy_min] << endl;

    if (error_flag) cout << endl << "ERROR" << endl;

    perfomance_results << endl;
    perfomance_results << "TEST RESULTS:" << endl << endl;
    perfomance_results << "DYNAMIC:" << endl << endl;
    perfomance_results << "PERCENT: " << best_percent << "%" << endl;
    perfomance_results << "TIME: " << best_time_dynamic << endl << endl;
    perfomance_results << "ANALYTIC:" << endl << endl;
    perfomance_results << "100% CPU" << endl;
    perfomance_results << "CPU: WORK = " << time_100cpu_work / count;
    perfomance_results << " READ = " << time_100cpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_cpu << endl << endl;
    perfomance_results << "100% GPU" << endl;
    perfomance_results << "GPU: WORK = " << time_100gpu_work / count;
    perfomance_results << " READ = " << time_100gpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_gpu << endl << endl;
    perfomance_results << "X" << endl;
    perfomance_results << "50% CPU 50% GPU" << endl;
    perfomance_results << "CPU: WORK = " << time_x50cpu_work / count;
    perfomance_results << " READ = " << time_x50cpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << time_x50gpu_work / count;
    perfomance_results << " READ = " << time_x50gpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_x50 / count << endl << endl;

    perfomance_results << "Y" << endl;
    perfomance_results << "50% CPU 50% GPU" << endl;
    perfomance_results << "CPU: WORK = " << time_y50cpu_work / count;
    perfomance_results << " READ = " << time_y50cpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << time_y50gpu_work / count;
    perfomance_results << " READ = " << time_y50gpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_y50 / count << endl;
    perfomance_results << endl;
    perfomance_results << "BEST: " << endl;
    perfomance_results << "CPU PART: " << cpu_part << "%" << endl;
    perfomance_results << "CPU TIME: " << (cpu_part * time_cpu) / 100.0 << endl
                       << endl;
    perfomance_results << "GPU PART: " << gpu_part << "%" << endl;
    perfomance_results << "GPU TIME: " << (gpu_part * time_gpu) / 100.0 << endl
                       << endl;
    perfomance_results << "TRY IT:" << endl << endl;
    perfomance_results << "X" << endl;
    perfomance_results << cpu_part << "% CPU " << gpu_part
                       << "%  GPU: " << endl;
    perfomance_results << "CPU: WORK = " << try_time_xcpu_work / count;
    perfomance_results << " READ = " << try_time_xcpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << try_time_xgpu_work / count;
    perfomance_results << " READ = " << try_time_xgpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << try_time_x << endl << endl;

    perfomance_results << "Y" << endl;
    perfomance_results << cpu_part << "% CPU " << gpu_part
                       << "%  GPU: " << endl;
    perfomance_results << "CPU: WORK = " << try_time_ycpu_work / count;
    perfomance_results << " READ = " << try_time_ycpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << try_time_ygpu_work / count;
    perfomance_results << " READ = " << try_time_ygpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << try_time_y << endl << endl;

    perfomance_results << "STATIC:" << endl << endl;

    perfomance_results << "BEST:" << endl;
    perfomance_results << "X" << endl;
    perfomance_results << 100 - idx_min * step << "% CPU " << idx_min * step;
    perfomance_results << "% GPU: " << endl;
    perfomance_results << "TIME: " << time_x[idx_min] << endl;
    perfomance_results << "Y" << endl;
    perfomance_results << 100 - idy_min * step << "% CPU " << idy_min * step;
    perfomance_results << "% GPU: " << endl;
    perfomance_results << "TIME: " << time_y[idy_min] << endl;

    cout << endl;
  }
  //===========================================================
  if (global_range.size() == 1) {
    percent = 10;
    split_x = global_range[0] * percent / 100;

    // TEST DYNAMIC
    time_dynamic = 0;

    percent = 10;
    split_x = global_range[0] * percent / 100;

    cout << "DYNAMIC" << endl;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;

      SetTasks(function_id, "CPU", {split_x}, global_range, local_range);

      Start("DYNAMIC");
      Info("TIME");
      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    best_time_dynamic = time_dynamic;
    best_percent = percent;

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    time_dynamic = 0;

    percent = 20;
    split_x = global_range[0] * percent / 100;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;

      SetTasks(function_id, "CPU", {split_x}, global_range, local_range);

      Start("DYNAMIC");
      Info("TIME");

      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    if (time_dynamic < best_time_dynamic) {
      best_time_dynamic = time_dynamic;
      best_percent = percent;
    }

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    time_dynamic = 0;

    percent = 50;
    split_x = global_range[0] * percent / 100;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << percent << "% SPLIT" << endl;

      SetTasks(function_id, "CPU", {split_x}, global_range, local_range);

      Start("DYNAMIC");
      Info("TIME");
      time_dynamic += all_time;

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    time_dynamic /= count;

    if (time_dynamic < best_time_dynamic) {
      best_time_dynamic = time_dynamic;
      best_percent = percent;
    }

    perfomance_dynamic << percent << "\t" << time_dynamic << endl;

    // TEST STATIC

    time_cpu = 0;
    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "100% CPU 0% GPU: " << endl;
      SetTask(function_id, "CPU", {0}, global_range, local_range);
      Start();
      Info("TIME");
      time_cpu += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "CPU") {
            time_100cpu_work += h.all_work_time;
            time_100cpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }
    time_cpu /= count;
    time_x.push_back(time_cpu);
    perfomance_static << "0\t" << time_cpu << endl;

    //=====

    cout << endl;

    cout << "X" << endl;
    for (unsigned int i = 10; i < 100; i += 10) {
      averege_time = 0;
      for (int n = 0; n < count; n++) {
        cout << "________________" << endl;
        cout << 100 - i << "% CPU " << i << "% GPU: " << endl;
        SetTask(function_id, "GPU", {0}, {global_range[0] * i / 100},
                local_range);

        SetTask(function_id, "CPU", {global_range[0] * i / 100},
                {global_range[0]}, local_range);

        Start();
        Info("TIME");
        averege_time += all_time;

        if (i == 50) {
          for (auto& g : gardens) {
            for (auto& h : g.hives) {
              if (h.name == "CPU") {
                time_x50cpu_work += h.all_work_time;
                time_x50cpu_read += h.all_read_time;
              }

              if (h.name == "GPU") {
                time_x50gpu_work += h.all_work_time;
                time_x50gpu_read += h.all_read_time;
              }
            }
          }

          time_x50 += all_time;
        }

        if (compare_result.size() != 0) {
          for (auto& g : gardens) {
            for (auto& f : g.functions) {
              if (Compare(f.arguments, compare_result)) {
                cout << "ERROR" << endl;
                error_flag = true;
                break;
              }
            }

            if (error_flag) break;
          }
        }
      }
      averege_time /= count;
      perfomance_static << i << "\t" << averege_time << endl;
      time_x.push_back(averege_time);
    }
    time_gpu = 0;
    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "0% CPU 100% GPU: " << endl;
      SetTask(function_id, "GPU", {0}, global_range, local_range);
      Start();
      Info("TIME");
      time_gpu += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "GPU") {
            time_100gpu_work += h.all_work_time;
            time_100gpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }
    time_gpu /= count;

    perfomance_static << "100\t" << time_gpu << endl;
    time_x.push_back(time_gpu);
    //========

    cout << endl;

    int idx_min = 0;

    for (int i = 0; i < time_x.size(); i++) {
      if (time_x[idx_min] > time_x[i]) idx_min = i;
    }

    int cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    int gpu_part = 100 - cpu_part;

    try_time_x = 0;

    cout << "ANALYTIC" << endl << endl;

    cout << "X:" << endl << endl;

    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;

      SetTask(function_id, "GPU", {0}, {global_range[0] * int(gpu_part) / 100},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * int(gpu_part) / 100},
              {global_range[0]}, local_range);

      Start();
      Info("TIME");
      try_time_x += all_time;

      for (auto& g : gardens) {
        for (auto& h : g.hives) {
          if (h.name == "CPU") {
            try_time_xcpu_work += h.all_work_time;
            try_time_xcpu_read += h.all_read_time;
          }

          if (h.name == "GPU") {
            try_time_xgpu_work += h.all_work_time;
            try_time_xgpu_read += h.all_read_time;
          }
        }
      }

      if (compare_result.size() != 0) {
        for (auto& g : gardens) {
          for (auto& f : g.functions) {
            if (Compare(f.arguments, compare_result)) {
              cout << "ERROR" << endl;
              error_flag = true;
              break;
            }
          }

          if (error_flag) break;
        }
      }
    }

    try_time_x /= count;

    perfomance_results << Info("DEV");

    cout << "TEST RESULTS:" << endl << endl;

    cout << "DYNAMIC:" << endl << endl;

    cout << "PERCENT: " << best_percent << "%" << endl;

    cout << "TIME: " << best_time_dynamic << endl << endl;

    cout << "ANALYTIC:" << endl << endl;

    cout << "100% CPU" << endl;
    cout << "CPU: WORK = " << time_100cpu_work / count
         << " READ = " << time_100cpu_read / count << endl;
    cout << "ALL_WORK = " << time_cpu << endl;
    cout << endl;
    cout << "100% GPU" << endl;
    cout << "GPU: WORK = " << time_100gpu_work / count
         << " READ = " << time_100gpu_read / count << endl;
    cout << "ALL_WORK = " << time_gpu << endl;

    cout << endl;

    cout << "X" << endl;
    cout << "50% CPU 50% GPU" << endl;
    cout << "CPU: WORK = " << time_x50cpu_work / count
         << " READ = " << time_x50cpu_read / count << endl;
    cout << "GPU: WORK = " << time_x50gpu_work / count
         << " READ = " << time_x50gpu_read / count << endl;

    cout << "ALL_WORK = " << time_x50 / count << endl;
    cout << endl;

    cout << "STATIC:" << endl << endl;

    cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    gpu_part = 100 - cpu_part;

    cout << "CPU PART: " << cpu_part << "%" << endl;
    cout << "CPU TIME: " << (cpu_part * time_cpu) / 100.0 << endl << endl;

    cout << "GPU PART: " << gpu_part << "%" << endl;
    cout << "GPU TIME: " << (gpu_part * time_gpu) / 100.0 << endl;

    cout << endl;

    cout << "TRY IT:" << endl << endl;

    cout << "X" << endl;
    cout << cpu_part << "% CPU " << gpu_part << "%  GPU: " << endl;
    cout << "CPU: WORK = " << try_time_xcpu_work / count
         << " READ = " << try_time_xcpu_read / count << endl;

    cout << "GPU: WORK = " << try_time_xgpu_work / count
         << " READ = " << try_time_xgpu_read / count << endl;

    cout << "ALL_WORK = " << try_time_x << endl << endl;

    cout << "BEST:" << endl;
    cout << 100 - idx_min * step << "% CPU " << idx_min * step
         << "% GPU: " << endl;

    cout << "TIME:" << time_x[idx_min] << endl;

    if (error_flag) cout << endl << "ERROR" << endl;

    perfomance_results << endl;
    perfomance_results << "TEST RESULTS:" << endl << endl;
    perfomance_results << "DYNAMIC:" << endl << endl;
    perfomance_results << "PERCENT: " << best_percent << "%" << endl;
    perfomance_results << "TIME: " << best_time_dynamic << endl << endl;
    perfomance_results << "ANALYTIC:" << endl << endl;
    perfomance_results << "100% CPU" << endl;
    perfomance_results << "CPU: WORK = " << time_100cpu_work / count;
    perfomance_results << " READ = " << time_100cpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_cpu << endl << endl;
    perfomance_results << "100% GPU" << endl;
    perfomance_results << "GPU: WORK = " << time_100gpu_work / count;
    perfomance_results << " READ = " << time_100gpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_gpu << endl << endl;
    perfomance_results << "X" << endl;
    perfomance_results << "50% CPU 50% GPU" << endl;
    perfomance_results << "CPU: WORK = " << time_x50cpu_work / count;
    perfomance_results << " READ = " << time_x50cpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << time_x50gpu_work / count;
    perfomance_results << " READ = " << time_x50gpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << time_x50 << endl << endl;
    perfomance_results << "STATIC:" << endl << endl;
    perfomance_results << "CPU PART: " << cpu_part << "%" << endl;
    perfomance_results << "CPU TIME: " << (cpu_part * time_cpu) / 100.0 << endl
                       << endl;
    perfomance_results << "GPU PART: " << gpu_part << "%" << endl;
    perfomance_results << "GPU TIME: " << (gpu_part * time_gpu) / 100.0 << endl
                       << endl;
    perfomance_results << "TRY IT:" << endl << endl;
    perfomance_results << "X " << endl;
    perfomance_results << cpu_part << "% CPU " << gpu_part
                       << "%  GPU: " << endl;
    perfomance_results << "CPU: WORK = " << try_time_xcpu_work / count;
    perfomance_results << " READ = " << try_time_xcpu_read / count << endl;
    perfomance_results << "GPU: WORK = " << try_time_xgpu_work / count;
    perfomance_results << " READ = " << try_time_xgpu_read / count << endl;
    perfomance_results << "ALL_WORK = " << try_time_x << endl << endl;
    perfomance_results << "BEST:" << endl;
    perfomance_results << 100 - idx_min * step << "% CPU " << idx_min * step;
    perfomance_results << "% GPU: " << endl;
    perfomance_results << "TIME:" << time_x[idx_min] << endl;

    cout << endl;
  }
  perfomance_static.close();
  perfomance_dynamic.close();
  perfomance_results.close();
  return 0;
}
