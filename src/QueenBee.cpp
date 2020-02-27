#include "..\include\QueenBee.hpp"

int Keeper::Wait() {
  while (tasks.size() != 0) {
  }
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.command.finish();
      while (h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() !=
             CL_COMPLETE) {
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
      }
    }

    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        cout << h.name << ": " << h.work_time << endl;
      }
    }

    cout << "ALL time: " << all_time << endl;
    cout << "WORK time: " << work_time << endl;
    cout << "READ time: " << read_time << endl;
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
      //CommandQueue command(g.context, d, CL_QUEUE_PROFILING_ENABLE);
      CommandQueue command(g.context, d);

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
        Buffer buff(g.context, CL_MEM_READ_WRITE, a.size);
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

int Keeper::Read() {
  for (auto& g : gardens) {
    for (auto& hive : g.hives) {
      for (auto& f : g.functions) {
        for (auto& task : hive.completed) {
          if (task.function_id == f.id) {
            for (auto& a : f.arguments) {
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
          }
          //}
        }
      }
    }
  }

  Wait();

  return 1;
}

//int Keeper::Start() {
//  cl_ulong time_end, time_start;
//
//  LARGE_INTEGER wt1;
//  LARGE_INTEGER wt2;
//
//  LARGE_INTEGER at1;
//  LARGE_INTEGER at2;
//
//  QueryPerformanceCounter(&at1);
//  QueryPerformanceCounter(&wt1);
//  
//
//  for (auto& g : gardens) {
//    for (auto& h : g.hives) {
//      h.completed.clear();
//      h.time.clear();
//      h.busy = false;
//      h.done = false;
//    }
//  }
//  LARGE_INTEGER frequency;
//  QueryPerformanceFrequency(&frequency);
//
//  while (tasks.size() != 0) {
//    for (auto& g : gardens) {
//      for (auto& h : g.hives) {
//
//
//        if (tasks.size() != 0) {
//          for (auto& f : g.functions) {
//            if (tasks.size() != 0 && f.id == tasks.back().function_id) {
//              if (tasks.size() != 0) {
//                h.command.flush();
//                if (((tasks.back().parallel_method == "ALL" &&
//                      h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
//                          CL_COMPLETE) ||
//                     (h.name == tasks.back().parallel_method &&
//                      h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
//                          CL_COMPLETE))) {
//                  if (h.time.size() != 0 && h.busy == true) {
//                    h.busy = false;
//
//                    h.event.getProfilingInfo(CL_PROFILING_COMMAND_START,
//                                             &time_start);
//                    h.event.getProfilingInfo(CL_PROFILING_COMMAND_END,
//                                             &time_end);
//
//                    h.time.back() = double(time_end - time_start) / 1000000000;
//                  }
//
//                  h.time.push_back(0);
//                  h.busy = true;
//                  int status = h.command.enqueueNDRangeKernel(
//                      f.kernel, GetRange(tasks.back().offsets),
//                      GetGlobalRange(tasks.back().globals,
//                                     tasks.back().offsets),
//                      GetRange(tasks.back().locals), NULL, &h.event);
//
//                  h.completed.push_back(tasks.back());
//                  if (tasks.size() != 0) tasks.pop_back();
//
//
//                  break;
//                } 
//				else {
//					
//                  if (h.name == tasks.back().parallel_method) {
//                    
//					  for (auto& t : tasks) {
//                      if (t.parallel_method != h.name) {
//                        swap(t, tasks.back());
//                        break;
//                      }
//                    }
//
//				  }
//				
//				}
//              }
//            }
//          }
//        }
//      }
//    }
//  }
//
//
//
//
//
// //for (auto& g : gardens) {
// //   for (auto& h : g.hives) {
// //     
////		h.event.wait();
// //     if (h.time.size() != 0) {
// //       h.event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
// //       h.event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
// //
// //      h.time.back() = double(time_end - time_start) / 1000000000;
// //     }
// //   }
// // }
//
//
//
//
//  QueryPerformanceCounter(&wt2);
//
//  LARGE_INTEGER rt1;
//  LARGE_INTEGER rt2;
//
//  QueryPerformanceCounter(&rt1);
//  Read();
//
//  QueryPerformanceCounter(&rt2);
//
//  QueryPerformanceCounter(&at2);
//
//  work_time = (wt2.QuadPart - wt1.QuadPart) / double(frequency.QuadPart);
//
//  read_time = (rt2.QuadPart - rt1.QuadPart) / double(frequency.QuadPart);
//
//  all_time = (at2.QuadPart - at1.QuadPart) / double(frequency.QuadPart);
//
//  return 1;
//}



int Keeper::Start() {
  LARGE_INTEGER wt1;
  LARGE_INTEGER wt2;
  LARGE_INTEGER at1;
  LARGE_INTEGER at2;
  QueryPerformanceCounter(&wt1);
  QueryPerformanceCounter(&at1);
  for (auto& g : gardens) {
    for (auto& h : g.hives) {
      h.completed.clear();
      h.time.clear();
      h.done = false;
      h.busy = false;
    }
  }
  LARGE_INTEGER frequency;
  LARGE_INTEGER t2;
  LARGE_INTEGER t1;
  QueryPerformanceFrequency(&frequency);
  while (tasks.size() != 0) {
    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        h.command.flush();
        if (h.time.size() != 0 &&
            h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                CL_COMPLETE &&
            h.busy == true) {
          QueryPerformanceCounter(&t2);
          h.time.back() =
              t2.QuadPart / double(frequency.QuadPart) - h.time.back();
          h.busy = false;
        }

        for (auto& f : g.functions) {
          if (tasks.size() != 0 && f.id == tasks.back().function_id) {
            if (tasks.size() != 0) {
              h.command.flush();
              if (((tasks.back().parallel_method == "ALL" &&
                    h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                        CL_COMPLETE) ||
                   (h.name == tasks.back().parallel_method &&
                    h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                        CL_COMPLETE))) {
                if (h.time.size() != 0) {
                  QueryPerformanceCounter(&t2);
                  h.time.back() =
                      t2.QuadPart / double(frequency.QuadPart) - h.time.back();
                }

                QueryPerformanceCounter(&t1);
                h.time.push_back(t1.QuadPart / double(frequency.QuadPart));
                h.busy = true;
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
                             if (h.name == tasks.back().parallel_method) {
           
           					  for (auto& t : tasks) {
                                 if (t.parallel_method != h.name) {
                                   swap(t, tasks.back());
                                   break;
                                 }
                               }
           
           				  }
          }
        }
      }
    }
  }

  int index = 0;
  int count = 0;
  for (auto& g : gardens) {
    index += g.hives.size();
    count += g.hives.size();
  }
  while (index != 0) {
    for (auto& g : gardens) {
      for (auto& h : g.hives) {
        h.command.flush();
        if (h.event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() ==
                CL_COMPLETE &&
            h.done != true) {
          index--;
          h.done = true;
          if (h.time.size() != 0) {
            QueryPerformanceCounter(&t2);
            h.time.back() =
                t2.QuadPart / double(frequency.QuadPart) - h.time.back();
          }
        }
      }
    }
  }
  QueryPerformanceCounter(&wt2);
  LARGE_INTEGER rt1;
  LARGE_INTEGER rt2;
  QueryPerformanceCounter(&rt1);
  Read();
  QueryPerformanceCounter(&at2);
  QueryPerformanceCounter(&rt2);
  work_time = (wt2.QuadPart - wt1.QuadPart) / double(frequency.QuadPart);
  read_time = (rt2.QuadPart - rt1.QuadPart) / double(frequency.QuadPart);
  all_time = (at2.QuadPart - at1.QuadPart) / double(frequency.QuadPart);
  return 1;
}



int Keeper::Test(string function_id, vector<unsigned int> global_range,
                 vector<unsigned int> local_range) {
  std::ofstream perfomance;
  perfomance.open("performance.txt");

  if (global_range.size() == 2) {
    cout << "100% CPU 0% GPU: " << endl;
    SetTask(function_id, "CPU", {0, 0}, global_range, local_range);
    Start();
    Info("TIME");

    perfomance << "0\t" << all_time << "\t" << work_time << endl;

    //========

    cout << endl;
    for (int i = 10; i <= 50; i += 10) {
      cout << "ALL " << i << "% : " << endl;
      SetTasks(function_id, "ALL",
               {global_range[0] * i / 100, global_range[1] * i / 100},
               global_range, local_range);

      Start();
      Info("TIME");
    }

    cout << endl;

    cout << "X:" << endl;
    for (unsigned int i = 1; i < 100; i += 1) {
      cout << 100 - i << "% CPU " << i << "% GPU: " << endl;
      SetTask(function_id, "GPU", {0, 0},
              {global_range[0] * i / 100, global_range[1]}, local_range);

      SetTask(function_id, "CPU", {global_range[0] * i / 100, 0},
              {global_range[0], global_range[1]}, local_range);

      Start();
      Info("TIME");
      perfomance << i << "\t" << all_time << "\t" << work_time << endl;
    }

    //=====
    cout << "0% CPU 100% GPU: " << endl;
    SetTask(function_id, "GPU", {0, 0}, global_range, local_range);
    Start();
    Info("TIME");

    perfomance << "100\t" << all_time << "\t" << work_time << endl;

    cout << endl;

    cout << "Y:" << endl;
    for (unsigned int i = 1; i < 100; i += 1) {
      cout << 100 - i << "% CPU " << i << "% GPU: " << endl;

      SetTask(function_id, "GPU", {0, 0},
              {global_range[1], global_range[0] * i / 100}, local_range);

      SetTask(function_id, "CPU", {0, global_range[0] * i / 100},
              {global_range[0], global_range[1]}, local_range);

      Start();
      Info("TIME");
      perfomance << i << "\t" << all_time << "\t" << work_time << endl;
    }
  }
  //===========================================================
  if (global_range.size() == 1) {
    cout << "100% CPU 0% GPU: " << endl;
    SetTask(function_id, "CPU", {0}, global_range, local_range);
    Start();
    Info("TIME");
    perfomance << "0\t" << all_time << "\t" << work_time << endl;
    //=====

    cout << endl;
    for (int i = 10; i <= 50; i += 10) {
      cout << "ALL " << i << "% : " << endl;

      SetTasks(function_id, "ALL", {global_range[0] * i / 100}, global_range,
               local_range);
      Start();
      Info("TIME");
    }

    cout << endl;

    cout << "X:" << endl;
    for (unsigned int i = 1; i < 100; i += 1) {
      cout << 100 - i << "% CPU " << i << "% GPU: " << endl;
      SetTask(function_id, "GPU", {0}, {global_range[0] * i / 100},
              local_range);

      SetTask(function_id, "CPU", {global_range[0] * i / 100},
              {global_range[0]}, local_range);

      Start();
      perfomance << i << "\t" << all_time << "\t" << work_time << endl;
      Info("TIME");
    }

    cout << "0% CPU 100% GPU: " << endl;
    SetTask(function_id, "GPU", {0}, global_range, local_range);
    Start();
    Info("TIME");
    perfomance << "100\t" << all_time << "\t" << work_time << endl;
    //========

    cout << endl;
  }
  perfomance.close();
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
  done = false;

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
