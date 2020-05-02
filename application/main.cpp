#include <windows.h>
#include <thread>
#include "QueenBee.hpp"

template <typename Type>
void MulMatrix(unsigned int size);

template <typename Type>
void MulMatrixOpt(unsigned int size, string device);

template <typename Type>
void MulMatrixOpt1(unsigned int size, string device);


template <typename Type>
void SumVectors(unsigned int size);

void MonteCarlo(unsigned int size);
void Convolution(unsigned int size, unsigned int radius);

// void ReadFunct(float* out, float*in) {
//  *out = *out + *in;
//  cout << out;
//}



int main(int argc, char* argv[]) {



  // std::string sizeStr = argv[2];
  // std::string taskStr = argv[1];
  // int size = atoi(sizeStr.c_str());
  // int task = atoi(taskStr.c_str());
  //
  // cout << size <<endl;
  // cout << task << endl;

  int size = 1600000;
  int task = 1;

  //MulMatrixOpt<float>(1600, "GPU");
  //MulMatrixOpt<float>(1600, "CPU");
  //MulMatrixOpt1<float>(1600, "CPU");

 // MulMatrixOpt<float>(1600);
  //MonteCarlo(size);

  //switch (task) {
  //  case 1:
  //    MulMatrixOpt<float>(size);
  //    break;
  //
  //  case 2:
  //    MulMatrix<float>(size);
  //    break;
  //
  //  case 3:
  //    MonteCarlo(size);
  //    break;
  //  case 4:
  //    Convolution(size, 10);
  //    break;
  //  default:
  //    break;
  //}

   MulMatrixOpt<float>(1600, "GPU");
  //  MulMatrixOpt<float>(3200, "CPU");

  // MulMatrixOpt<float>(1600,  "GPU");
  // MulMatrix<float>(1600);
  // SumVectors<float>(size);
  // MonteCarlo(100000);
  //	Convolution(15000, 3, "GPU", 10);
   int tmp;
   cin >> tmp;
  return 0;
}

template <typename Type>
void MulMatrixOpt(unsigned int size, string device) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* C = new Type[size * size];

  unsigned int block = 16;

  Type* a = NULL;
  Type* b = NULL;

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  unsigned int* ptr_size = &size;

  unsigned int* ptr_block = &block;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function MulMatrixOpt("mul", "MulMatrixOpt", true);
  MulMatrixOpt.SetArgument<Type>(A, {size, size});
  MulMatrixOpt.SetArgument<Type>(B, {size, size});
  MulMatrixOpt.SetArgument<Type>(C, {size, size}, true);
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_size, {1});
  MulMatrixOpt.SetArgument<Type>(a, {block, block});
  MulMatrixOpt.SetArgument<Type>(b, {block, block});
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_block, {1});
  queen.SetFunction(MulMatrixOpt);

  queen.Info("DEV");
 // queen.SetTask("mul", device, {0, 0}, {size, size}, {block, block});
  
  // queen.Info("TIME");
  // queen.Start();

   //queen.Info("TIME");

  queen.Test("mul", 10, {size, size}, {block, block});

  // for (int i = 0; i < size; i++) {
  // for (int j = 0; j < size; j++) {
  // std::cout << C[i * size + j] << " ";
  // }
  // cout << endl;
  // }

  int error = 0;
  for (unsigned int i = 0; i < size * size; i++) {
    if (C[i] != size * 2) {
      error++;
    }
  }

  cout << "ERROR = " << error << endl;
  //  int tmp;
  //  cin >> tmp;
  delete[] A;
  delete[] B;
  delete[] C;

  // delete a;
  // delete b;
}


template <typename Type>
void MulMatrix(unsigned int size) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* C = new Type[size * size];

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  unsigned int* ptr_size = &size;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function MulMatrix("mul", "MulMatrix", true);
  MulMatrix.SetArgument<Type>(A, {size, size});
  MulMatrix.SetArgument<Type>(B, {size, size});
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1});

  queen.SetFunction(MulMatrix);
  // queen.Test("mul", {size, size});

  // queen.Info("DEV");

  queen.Test("mul", 10, {size, size});

  //  queen.SetTasks("mul", "ALL", {size/10 , size/10}, {size, size});
  // queen.SetTask("mul", "GPU", {0, 0}, {size, size});
  // QueryPerformanceCounter(&t1);
  //
  // queen.Start();
  //  queen.Wait();
  //  void (*pt2Func)(Type*, Type*) = NULL;
  //  pt2Func = &ReadFunct;
  // queen.Read<Type>((*pt2Func));

  //  queen.Info("STAT");
  // queen.Info("TIME");

  /////////////////////////////////////////////

  // for (int i = 0; i < size; i++) {
  //   for (int j = 0; j < size; j++) {
  //     // std::cout << C[i * size + j] << " ";
  //   }
  //   //  cout << endl;
  // }
  //
  // int error = 0;
  // for (unsigned int i = 0; i < size * size; i++) {
  //   if (C[i] != size * 2) {
  //     error++;
  //   }
  // }

  // cout << "ERROR = " << error << endl;

  delete[] A;
  delete[] B;
  delete[] C;
}

template <typename Type>
void SumVectors(unsigned int size) {
  Type* A = new Type[size];
  Type* B = new Type[size];
  Type* C = new Type[size];

  for (unsigned int i = 0; i < size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  unsigned int* ptr_size = &size;
  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");
  // queen.Info();
  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument<Type>(A, {size});
  SumVectors.SetArgument<Type>(B, {size});
  SumVectors.SetArgument<Type>(C, {size});

  queen.SetFunction(SumVectors);

  queen.Test("sum", 10, {size});

  // queen.SetTasks("sum", "ALL", {size / 10}, {size});
  //
  // QueryPerformanceCounter(&t1);
  // queen.Start();
  // queen.Wait();
  // queen.Read();
  // QueryPerformanceCounter(&t2);
  // time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
  //
  // cout << "The time: seconds\n" << time << endl;
  //
  // queen.Info("STAT");
  ///////////////////////////////////////////////
  //
  ///*
  // for (unsigned int i = 0; i < size; i++) {
  //  std::cout << C[i] << " ";
  //}
  //*/
  //
  // int error = 0;
  // for (unsigned int i = 0; i < size; i++) {
  //  if (C[i] != 3) {
  //    error++;
  //  }
  //}
  // cout << "ERROR = " << error << endl;

  delete[] A;
  delete[] B;
  delete[] C;
}

void MonteCarlo(unsigned int size) {
  int* A = new int[size];

  for (unsigned int i = 0; i < size; i++) {
    A[i] = 0;
  }

  unsigned int* ptr_size = &size;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function MonteCarlo("mc", "MonteCarlo", true);
  MonteCarlo.SetArgument<int>(A, {size});
  MonteCarlo.SetArgument<unsigned int*>(ptr_size, {1});

  queen.SetFunction(MonteCarlo);

  //  queen.Info("STAT");

  // queen.SetTask("mul", "GPU", {0, 0}, {size, size}, {block, block});

  // queen.SetTask("mc", "CPU", {0}, {size/2});
  // queen.SetTask("mc", "GPU", {size/2}, {size });
  queen.Test("mc", 10, {size});

  //  queen.SetTasks("mc", "ALL", {size / 10}, {size});
  //
  // QueryPerformanceCounter(&t1);
  //
  // queen.StartT();
  // queen.Info("TIME");
  // queen.Wait();
  //  void (*pt2Func)(float*, float*) = NULL;

  // pt2Func = &ReadFunct;
  //  int sum = 0;

  //  queen.Read(*pt2Func);

  //
  // QueryPerformanceCounter(&t2);
  // time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
  //
  // cout << "The time: " << time << "seconds" << endl;
  //
  // queen.Info("STAT");
  // /////////////////////////////////////////////
  //
  double sum = 0;
  for (unsigned int i = 0; i < size; i++) {
    sum += double(A[i]) / size;
  }
  double fraction_in_circle = sum / size,
         pi = 3 * sqrt(3) / 2 + fraction_in_circle * 6.0 * (1.0 - sqrt(3) / 2);
  cout << "PI = " << pi << endl;

  delete[] A;
}

void Convolution(unsigned int size, unsigned int radius) {
  float* A = new float[size * size];
  float* B = new float[size * size];
  float* kern = new float[(2 * radius + 1) * (2 * radius + 1)];

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 0;
  }

  for (unsigned int i = 0; i < (2 * radius + 1) * (2 * radius + 1); i++) {
    kern[i] = 2;
  }

  unsigned int* ptr_size = &size;
  unsigned int* ptr_radius = &radius;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function Convolution("conv", "Convolution", true);
  Convolution.SetArgument<float>(A, {size, size});
  Convolution.SetArgument<float>(B, {size, size});
  Convolution.SetArgument<unsigned int>(ptr_size, {1});
  Convolution.SetArgument<float>(kern, {2 * radius + 1, 2 * radius + 1});
  Convolution.SetArgument<unsigned int>(ptr_radius, {1});

  queen.SetFunction(Convolution);

  // queen.Info("STAT");

  queen.Test("conv", 10, {size, size});

  /*

    for (unsigned int i = 1000; i <= 10000; i+=1000) {
      QueryPerformanceCounter(&t1);
      queen.SetTask( "conv", "GPU", {0, 0}, {size, i} );
      queen.SetTask("conv", "CPU", {0,  i}, {size, size});
          queen.Start();
      queen.Wait();
          cout << "CPU: " << size -  i << endl;
                  cout << "GPU: " <<  i << endl;
          QueryPerformanceCounter(&t2);
      time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
          cout << time << endl;

    }
    */

  //  queen.SetTasks("conv", mode, {size / peace, size / peace}, {size, size});

  //  QueryPerformanceCounter(&t1);

  // queen.Start();
  //  queen.Wait();
  // queen.Read();
  // queen.Info("STAT");

  // QueryPerformanceCounter(&t2);
  // time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  // cout << "The time: seconds\n" << time << endl;

  /////////////////////////////////////////////
  /*
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
       std::cout << B[i * size + j] << " ";
    }
      cout << endl;
  }
  */
  delete[] A;
  delete[] B;
  delete[] kern;
}
