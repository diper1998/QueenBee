#include "..\include\QueenBee.hpp"

int Keeper::Static() {
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.time.clear();
      h.tasks.clear();
    }
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      for (auto& t : tasks) {
        if (t.parallel_method == h.name) {
          h.tasks.push_back(t);
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

int Keeper::Dynamic() {
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.time.clear();
      h.tasks.clear();
    }
  }

  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      if (h.tasks.size() != 0) {
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

Keeper::Keeper(string kernel_file_name) {
  SetGardens();
  SetKernel(kernel_file_name);
  Build();
}

void Keeper::Info(string mode) {
  if (mode == "ALL" || mode == "DEV") {
    for (const auto& g : gardens) {
      cout << g.platform.getInfo<CL_PLATFORM_NAME>() << endl;

      for (const auto& d : g.devices) {
        cout << d.getInfo<CL_DEVICE_TYPE>() << ": "
             << d.getInfo<CL_DEVICE_NAME>() << endl
             << "MAX COMPUTE UNITS: "
             << d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl
             << "GLOBAL MEM SIZE: " << d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()
             << endl
             << "MAX CLOCK FREQUENCY: "
             << d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() / 1000.0 << "GHz"
             << endl;
      }
    }
  }

  if (mode == "ALL" || mode == "KER") {
    cout << endl << kernel << endl;

    for (const auto& g : gardens) {
      for (const auto& h : g.hives) {
        cout << endl
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
        h.work_time = 0;
        for (auto& t : h.time) {
          cout << i << ": " << t << endl;
          i++;
          h.work_time += t;
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
        cout << h.name << ": " << h.work_time << endl;
      }
    }

    cout << "ALL time: " << all_time << endl;
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

       if (g.hives.back().program.build(g.hives.back().device) != CL_SUCCESS) {
         cout << g.hives.back().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d)
              << endl;
       }
     }
   }
   */
  return 1;
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

// int Keeper::Read() {
//  for (auto& g : gardens) {
//    for (auto& hive : g.hives) {
//      for (auto& f : g.functions) {
//        for (auto& task : hive.completed) {
//          if (task.function_id == f.id) {
//            for (auto& a : f.arguments) {
//              if (a.change) {
//                if (a.type == "float") {
//                  Read<float>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "double") {
//                  Read<double>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "int") {
//                  Read<int>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "unsigned int") {
//                  Read<unsigned int>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "short") {
//                  Read<short>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "bool") {
//                  Read<bool>(hive, a, task, a.pointer);
//                  continue;
//                }
//                if (a.type == "char") {
//                  Read<char>(hive, a, task, a.pointer);
//                  continue;
//                }
//              }
//            }
//          }
//          //}
//        }
//      }
//    }
//  }
//
//  Wait();
//
//  return 1;
//}

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
      h.time.push_back(0);

      if (f->id == t.function_id) {
        int status =
            h.command.enqueueNDRangeKernel(f->kernel, GetRange(t.offsets),
                                           GetGlobalRange(t.globals, t.offsets),
                                           GetRange(t.locals), NULL, &h.event);

        h.event.wait();

        h.event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
        h.event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);

        h.time.back() = double(time_end - time_start) / 1000000000;

        Read(h, *f, t);

        h.command.finish();
      }
    }
  }
}

void Keeper::ThreadFunction(Hive& h) {
  cl_ulong time_end, time_start;
  for (auto& t : h.tasks) {
    for (auto& f : h.functions_ptrs) {
      h.time.push_back(0);

      if (f->id == t.function_id) {
        int status =
            h.command.enqueueNDRangeKernel(f->kernel, GetRange(t.offsets),
                                           GetGlobalRange(t.globals, t.offsets),
                                           GetRange(t.locals), NULL, &h.event);

        h.event.wait();

        h.event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
        h.event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);

        h.time.back() = double(time_end - time_start) / 1000000000;

        Read(h, *f, t);

        h.command.finish();
      }
    }
  }
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

int Keeper::Test(string function_id, unsigned int step,
                 vector<unsigned int> global_range,
                 vector<unsigned int> local_range, unsigned int repeat_count) {
  std::ofstream perfomance_static;
  std::ofstream perfomance_dynamic;
  std::stringstream ss;

  if (global_range.size() == 1) ss << global_range[0];

  if (global_range.size() == 2) ss << global_range[0] << "x" << global_range[1];

  std::string str;
  ss >> str;
  perfomance_static.open(string("performance_static_") + function_id + str +
                         string(".txt"));
  perfomance_dynamic.open(string("performance_dynamic_") + function_id + str +
                          string(".txt"));

  vector<double> time_x;
  vector<double> time_y;

  int count = repeat_count;
  double time_cpu = 0;
  double time_gpu = 0;
  double averege_time = 0;
  double try_time_x = 0;
  double try_time_y = 0;

  if (global_range.size() == 2) {
    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "100% CPU 0% GPU: " << endl;
      SetTask(function_id, "CPU", {0, 0}, global_range, local_range);
      Start();
      Info("TIME");

      time_cpu += all_time;
    }
    time_cpu /= count;
    perfomance_static << "0\t" << time_cpu << endl;

    time_x.push_back(time_cpu);

    cout << endl;

    cout << "X:" << endl;
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
    }

    time_cpu /= count;
    time_y.push_back(time_cpu);
    perfomance_static << "0\t" << time_cpu << endl;

    cout << "Y:" << endl;
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
    }

    time_gpu /= count;
    perfomance_static << "100\t" << time_gpu << endl;
    time_x.push_back(time_gpu);
    cout << endl;

   
    double cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    double gpu_part = 100 - cpu_part;

  

    try_time_x = 0;

    for (int n = 0; n < count; n++) {
      SetTask(function_id, "GPU", {0, 0},
              {global_range[0] * int(gpu_part) / 100, global_range[1]},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * int(gpu_part) / 100, 0},
              {global_range[0], global_range[1]}, local_range);

      Start();
     // Info("TIME");

      try_time_x += all_time;
    }

    try_time_x /= count;


    try_time_y = 0;

    for (int n = 0; n < count; n++) {
      SetTask(function_id, "GPU", {0, 0},
              {global_range[1], global_range[0] * int(gpu_part) / 100},
              local_range);

      SetTask(function_id, "CPU", {0, global_range[0] * int(gpu_part) / 100},
              {global_range[0], global_range[1]}, local_range);
      Start();
     // Info("TIME");

      try_time_y += all_time;
    }

    try_time_y /= count;

    cout << "TEST RESULTS:" << endl << endl;
    cout << "ANALYTIC:" << endl << endl;


    cout << "CPU PART: " << cpu_part << "%" << endl;
    cout << "CPU TIME: " << (cpu_part * time_cpu) / 100.0 << endl << endl;

    cout << "GPU PART: " << gpu_part << "%" << endl;
    cout << "GPU TIME: " << (gpu_part * time_gpu) / 100.0 << endl;
    cout << endl;
    cout << "TRY IT:" << endl << endl;

    cout << "TIME X: " << try_time_x << endl;

    cout << "TIME Y: " << try_time_y << endl << endl;

    int idx_min = 0;
    int idy_min = 0;
    for (int i = 0; i < time_x.size(); i++) {
      if (time_x[idx_min] > time_x[i]) idx_min = i;
    }

    for (int i = 0; i < time_y.size(); i++) {
      if (time_x[idy_min] > time_x[i]) idy_min = i;
    }

    cout << "BEST :";
    if (time_x[idx_min] < time_y[idy_min]) {
      cout << "X: " << endl;
      cout << 100 - idx_min * step << "% CPU " << idx_min * step
           << "% GPU: " << endl;
      cout << "ALL_TIME: " << time_x[idx_min] << endl;
    } else {
      cout << "Y: " << endl;
      cout << 100 - idy_min * step << "% CPU " << idy_min * step
           << "% GPU: " << endl;
      cout << "ALL_TIME: " << time_y[idy_min] << endl;
    }

    cout << endl;
  }
  //===========================================================
  if (global_range.size() == 1) {
    time_cpu = 0;
    for (int n = 0; n < count; n++) {
      cout << "________________" << endl;
      cout << "100% CPU 0% GPU: " << endl;
      SetTask(function_id, "CPU", {0}, global_range, local_range);
      Start();
      Info("TIME");
      time_cpu += all_time;
    }
    time_cpu /= count;
    time_x.push_back(time_cpu);
    perfomance_static << "0\t" << time_cpu << endl;

    //=====

    cout << endl;

    cout << "X:" << endl;
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

    double cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    double gpu_part = 100 - cpu_part;

  

    try_time_x = 0;

    for (int n = 0; n < count; n++) {
      SetTask(function_id, "GPU", {0}, {global_range[0] * int(gpu_part) / 100},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * int(gpu_part) / 100},
              {global_range[0]}, local_range);

      Start();
     // Info("TIME");
      try_time_x += all_time;
    }

    try_time_x /= count;

    cout << "TEST RESULTS:" << endl << endl;
    cout << "ANALYTIC:" << endl << endl;
    cpu_part = (1 - (time_cpu / (time_cpu + time_gpu))) * 100;
    gpu_part = 100 - cpu_part;

    cout << "CPU PART: " << cpu_part << "%" << endl;
    cout << "CPU TIME: " << (cpu_part * time_cpu) / 100.0 << endl << endl;

    cout << "GPU PART: " << gpu_part << "%" << endl;
    cout << "GPU TIME: " << (gpu_part * time_gpu) / 100.0 << endl;

    cout << endl;

    cout << "TRY IT:" << endl << endl;
    cout << "TIME X: " << try_time_x << endl << endl;

        cout
         << "BEST :";
    cout << 100 - idx_min * step << "% CPU " << idx_min * step
         << "% GPU: " << endl;

    cout << "ALL_TIME:" << time_x[idx_min] << endl;

    cout << endl;
  }
  perfomance_static.close();
  perfomance_dynamic.close();
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

Task::Task() {}
