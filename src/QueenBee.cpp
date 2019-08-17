#include "..\include\QueenBee.hpp"

Keeper::Keeper() {
  SetGardens();
  SetKernel("kernel.txt");
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
  cout << endl << env.kernel << endl;
  
   for (const auto& g : gardens) {

      for (const auto& d : g.devices){
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

  return 1;
}

int Keeper::SetKernel(string fname) {
  ifstream file(fname);

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
  env.kernel = tmp;

  env.source.push_back({env.kernel.c_str(), env.kernel.length()});

  return 1;
}

int Keeper::Build() {
  for (auto& g : gardens) {
    Context context(g.devices);
    g.context = context;

    Program program(g.context, env.source);
    g.program = program;

    if (g.program.build(g.devices) != CL_SUCCESS) {
      for (const auto& d : g.devices)
        cout << endl << g.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(d) << endl;
    }

  }

  

  return 1;
}

Garden::Garden() {}

Environment::Environment() {}
