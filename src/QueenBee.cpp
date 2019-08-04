#include "..\include\QueenBee.hpp"

Keeper::Keeper() {
  SetGardens();
  SetKernel("kernel.txt");
}

void Keeper::Info() {
  cout << endl;
  for (auto g : gardens) {
    cout << g.name << endl;
    for (auto h : g.hives) {
      cout << h.type << ": " << h.name << endl;
    }
  }
  cout << endl << kernel << endl;
}

int Keeper::SetGardens() {
  vector<Platform> platforms;
  vector<Device> devices;
  Hive hive;
  Platform platform;

  Platform::get(&platforms);
  
  Garden garden;

  for (auto p : platforms) {
    garden.platform = p;
    garden.name = p.getInfo<CL_PLATFORM_NAME>();

    p.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    for (auto d : devices) {
      hive.device = d;
      hive.name = d.getInfo<CL_DEVICE_NAME>();

      switch (d.getInfo<CL_DEVICE_TYPE>()) {
        case CL_DEVICE_TYPE_CPU:
          hive.type = "CPU";
          break;
        case CL_DEVICE_TYPE_GPU:
          hive.type = "GPU";
          break;
        case CL_DEVICE_TYPE_ACCELERATOR:
          hive.type = "ACCELERATOR";
          break;
      }

      garden.hives.push_back(hive);
    }

    gardens.push_back(garden);
  }

  return 1;
}

int Keeper::SetKernel(string fname) {
  ifstream file(fname);

  if (!file.is_open()) {
    cout << endl <<"ERROR: SetKernel(string fname) fname is't found;" << endl;
    return 0;
  }

  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  string tmp(size, ' ');
  file.seekg(0);
  file.read(&tmp[0], size);
  file.close();
  kernel = tmp;

  return 1;
}

Garden::Garden() {}

Hive::Hive() {}
