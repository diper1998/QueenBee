
#include <windows.h>
#include <thread>
#include "QueenBee.hpp"

template <typename Type>
void MulMatrix(unsigned int size);

template <typename Type>
void MulMatrixOpt(unsigned int size);

template <typename Type>
void SumVectors(unsigned int size);

void MonteCarlo(unsigned int size);
void Convolution(unsigned int size, unsigned int radius);


int main(void) {
// MulMatrixOpt<float>(1600);
// SumVectors<float>(10000000);
// MonteCarlo(10000);
  Convolution(8000, 50);
  return 0;
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

  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

 
  Function MulMatrix("mul", "MulMatrix", true);
  MulMatrix.SetArgument<Type>(A, {size, size}, false);
  MulMatrix.SetArgument<Type>(B, {size, size}, false);
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1}, false);

  queen.SetFunction(MulMatrix);

  queen.Info();

  queen.SetTasks("mul", "GPU", {size, size/2}, {size, size});

 QueryPerformanceCounter(&t1);

 queen.Start();
 queen.Wait();
 queen.Read();

 QueryPerformanceCounter(&t2);
 time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  cout << "The time: seconds\n"<< time << endl;

  /////////////////////////////////////////////

  /*
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
   std::cout << C[i * size + j] << " ";
    }
      cout << endl;
  }
  */

  int error = 0;
  for (unsigned int i = 0; i < size * size; i++) {
    if (C[i] != size * 2) {
      error++;
    }
  }

  cout << "ERROR = " << error << endl;


  delete[] A;
  delete[] B;
  delete[] C;


}


template <typename Type>
void MulMatrixOpt(unsigned int size) {
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
  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function MulMatrixOpt("mul", "MulMatrixOpt", true);
  MulMatrixOpt.SetArgument<Type>(A, {size, size}, false);
  MulMatrixOpt.SetArgument<Type>(B, {size, size}, false);
  MulMatrixOpt.SetArgument<Type>(C, {size, size}, true);
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_size, {1}, false);
  MulMatrixOpt.SetArgument<Type>(a, {block, block}, false);
  MulMatrixOpt.SetArgument<Type>(b, {block, block}, false);
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_block, {1}, false);

  queen.SetFunction(MulMatrixOpt);

  queen.Info();

  queen.SetTasks("mul", "GPU", {size/10, size/10}, {size, size}, {block, block});

  QueryPerformanceCounter(&t1);

  queen.Start();
  queen.Wait();
  queen.Read();

  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  cout << "The time: seconds\n" << time << endl;

  queen.Info("STAT");
  /////////////////////////////////////////////

  /*
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
   std::cout << C[i * size + j] << " ";
    }
      cout << endl;
  }
  */

  int error = 0;
  for (unsigned int i = 0; i < size * size; i++) {
    if (C[i] != size * 2) {
      error++;
    }
  }

  cout << "ERROR = " << error << endl;

  delete[] A;
  delete[] B;
  delete[] C;
  delete[] a;
  delete[] b;
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
  SumVectors.SetArgument<Type>(A, {size}, false);
  SumVectors.SetArgument<Type>(B, {size}, false);
  SumVectors.SetArgument<Type>(C, {size}, true);

  queen.SetFunction(SumVectors);
  queen.SetTasks("sum", "CPU", {size}, {size});

  QueryPerformanceCounter(&t1);
  queen.Start();
  queen.Wait();
  queen.Read();
  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  cout << "The time: seconds\n" << time << endl;

  queen.Info("STAT");
  /////////////////////////////////////////////

  /*
  for (unsigned int i = 0; i < size; i++) {
    std::cout << C[i] << " ";
  }
  */

  int error = 0;
  for (unsigned int i = 0; i < size; i++) {
    if (C[i] != 3) {
      error++;
    }
  }
  cout <<"ERROR = "<<error << endl;

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

  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function MonteCarlo("mc", "MonteCarlo", true);
  MonteCarlo.SetArgument<int>(A, {size}, true);
  MonteCarlo.SetArgument<unsigned int*>(ptr_size, {1}, false);

  queen.SetFunction(MonteCarlo);

  queen.Info();

  queen.SetTasks("mc", "CPU", {size}, {size});

  QueryPerformanceCounter(&t1);

  //queen.Start();
  queen.Wait();
  queen.Read();

  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  cout << "The time: " << time << "seconds"<< endl;

  /////////////////////////////////////////////

  double sum = 0;
  for (unsigned int i = 0; i < size ; i++) {
    sum += double(A[i])/size;

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
    A[i] = 1 ;
    B[i] = 0;    
  }

  for (unsigned int i = 0; i < (2 * radius + 1) * (2 * radius + 1); i++) {
    kern[i] = 1.0/9;
  }

  unsigned int* ptr_size = &size;
  unsigned int* ptr_radius = &radius;

  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function Convolution("conv", "Convolution", false);
  Convolution.SetArgument<float>(A, {size, size}, false);
  Convolution.SetArgument<float>(B, {size, size}, true);
  Convolution.SetArgument<unsigned int>(ptr_size, {1}, false);
  Convolution.SetArgument<float>(kern, {2*radius+1, 2*radius+1}, false);
  Convolution.SetArgument<unsigned int>(ptr_radius, {1}, false);

  queen.SetFunction(Convolution);

  queen.Info();

  queen.SetTasks("conv", "ALL", {size, size/8}, {size, size});

  QueryPerformanceCounter(&t1);

  queen.Start();
  queen.Wait();
  queen.Read();
  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  cout << "The time: seconds\n" << time << endl;

  /////////////////////////////////////////////

  
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
   //std::cout << B[i * size + j] << " ";
    }
    //  cout << endl;
  }
  

  

  delete[] A;
  delete[] B;
  delete[] kern; 
 

}
