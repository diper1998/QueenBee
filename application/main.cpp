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

int main(int argc, char* argv[]) {
//  std::string sizeStr = argv[2];
//  std::string taskStr = argv[1];
//  int size = atoi(sizeStr.c_str());
//  int task = atoi(taskStr.c_str());

	int task = 3;
int size = 1600;

  cout << size << endl;
  cout << task << endl;

  switch (task) {
    case 1:
      MulMatrixOpt<float>(size);
      break;

    case 2:
      Convolution(size, 10);
      break;

    case 3:
      MonteCarlo(size);
      break;

    default:
      break;
  }

  int tmp;
  cout << "TYPE SOMETHING ";

  cin >> tmp;
  return 0;
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
  queen.Test(10, "mul", 10, {size, size}, {block, block});

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

  Keeper queen("kernel.txt");

  Function MulMatrix("mul", "MulMatrix", true);
  MulMatrix.SetArgument<Type>(A, {size, size});
  MulMatrix.SetArgument<Type>(B, {size, size});
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1});

  queen.SetFunction(MulMatrix);

  queen.Info("DEV");
  queen.Test(10, "mul", 10, {size, size});

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

  Keeper queen("kernel.txt");

  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument<Type>(A, {size});
  SumVectors.SetArgument<Type>(B, {size});
  SumVectors.SetArgument<Type>(C, {size});

  queen.SetFunction(SumVectors);

  queen.Test(10, "sum", 10, {size});

  int error = 0;
  for (unsigned int i = 0; i < size; i++) {
    if (C[i] != 3) {
      error++;
    }
  }
  cout << "ERROR = " << error << endl;

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

  Keeper queen("kernel.txt");

  Function MonteCarlo("mc", "MonteCarlo", true);
  MonteCarlo.SetArgument<int>(A, {size});
  MonteCarlo.SetArgument<unsigned int*>(ptr_size, {1});

  queen.SetFunction(MonteCarlo);

  queen.Test(10, "mc", 10, {size});

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

  Keeper queen("kernel.txt");

  Function Convolution("conv", "Convolution", true);
  Convolution.SetArgument<float>(A, {size, size});
  Convolution.SetArgument<float>(B, {size, size});
  Convolution.SetArgument<unsigned int>(ptr_size, {1});
  Convolution.SetArgument<float>(kern, {2 * radius + 1, 2 * radius + 1});
  Convolution.SetArgument<unsigned int>(ptr_radius, {1});

  queen.SetFunction(Convolution);

  queen.Test(10, "conv", 10, {size, size});

  delete[] A;
  delete[] B;
  delete[] kern;
}
