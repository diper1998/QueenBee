#include "QueenBee.hpp"

Keeper::Keeper() {
  vector<Platform> platforms;
  Platform::get(&platforms);

  Hive hive;
  Platform platform;
  vector<Device> devices;

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
}

void Keeper::Info() {
  for (auto g : gardens) {
    cout << g.name << endl;
    for (auto h : g.hives) {
      cout << h.type << ": " << h.name << endl;
    }
  }
}

Garden::Garden() {}

Hive::Hive() {}
