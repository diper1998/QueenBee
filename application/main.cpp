#include <windows.h>
#include <thread>
#include "QueenBee.hpp"

template <typename Type>
Type Rand(Type low, Type high) {
  Type t = (Type)rand() / (Type)RAND_MAX;
  return (1.0 - t) * low + t * high;
}

template <typename Type>
void DotProduct(unsigned int size) {
  Type* A = new Type[size];
  Type* B = new Type[size];
  Type* C = new Type[size];
  Type* Check = new Type[size];

  for (int i = 0; i < size; i++) {
    A[i] = Rand<Type>(-100, 100);
    B[i] = Rand<Type>(-100, 100);
    C[i] = 0;
    Check[i] = A[i] * B[i];
  }

  unsigned int* ptr_size = &size;

  Keeper keeper("kernel.txt");

  Function DotProduct("dot", "DotProduct");
  DotProduct.SetArgument<Type>(A, {size});
  DotProduct.SetArgument<Type>(B, {size});
  DotProduct.SetArgument<Type>(C, {size}, true);
  DotProduct.SetArgument<unsigned int*>(ptr_size, {1});

  keeper.SetFunction(DotProduct);
  keeper.Test(10, "dot", 10, {size}, {}, {Check});

  Type dot = 0;

  for (int i = 0; i < size; i++) {
    dot += C[i];
  }
  cout << "Dot product = " << dot << endl;
}

template <typename Type>
void MulMatrix(unsigned int size) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* C = new Type[size * size];
  Type* Check = new Type[size * size];

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = Rand<Type>(-100, 100);
    B[i] = Rand<Type>(-100, 100);
    C[i] = 0;
    Check[i] = 0;
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      for (int k = 0; k < size; ++k) {
        Check[i * size + j] += A[i * size + k] * B[k * size + j];
      }
    }
  }

  unsigned int* ptr_size = &size;

  Keeper keeper("kernel.txt");

  Function MulMatrix("mul", "MulMatrix", false);
  MulMatrix.SetArgument<Type>(A, {size, size});
  MulMatrix.SetArgument<Type>(B, {size, size});
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1});

  keeper.SetFunction(MulMatrix);

  // keeper.SetTask("mul", "GPU", {0, size / 2}, {size, size});
  // keeper.SetTask("mul", "CPU", {0, 0}, {size, size / 2});
  // keeper.Start();

  //  keeper.Info("DEV");

  keeper.Test(10, "mul", 10, {size, size}, {}, {Check});

  delete[] A;
  delete[] B;
  delete[] C;
}

template <typename Type>
void MulMatrixOpt(unsigned int size) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* C = new Type[size * size];
  Type* Check = new Type[size * size];

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = Rand<Type>(-100, 100);
    B[i] = Rand<Type>(-100, 100);
    C[i] = 0;
    Check[i] = 0;
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      for (int k = 0; k < size; ++k) {
        Check[i * size + j] += A[i * size + k] * B[k * size + j];
      }
    }
  }

  unsigned int block = 10;

  Type* a = NULL;
  Type* b = NULL;

  unsigned int* ptr_size = &size;

  unsigned int* ptr_block = &block;

  ///////////////////////////////////////////////
  Keeper keeper("kernel.txt");

  Function MulMatrixOpt("mul", "MulMatrixOpt", true);
  MulMatrixOpt.SetArgument<Type>(A, {size, size});
  MulMatrixOpt.SetArgument<Type>(B, {size, size});
  MulMatrixOpt.SetArgument<Type>(C, {size, size}, true);
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_size, {1});
  MulMatrixOpt.SetArgument<Type>(a, {block, block});
  MulMatrixOpt.SetArgument<Type>(b, {block, block});
  MulMatrixOpt.SetArgument<unsigned int*>(ptr_block, {1});
  keeper.SetFunction(MulMatrixOpt);

  keeper.Test(10, "mul", 10, {size, size}, {block, block}, {Check});

  //  keeper.SetTask("mul", "GPU", {0, 0}, {size, size}, {block, block});

  //	queen.SetTasks("mul", "CPU", {32, 32}, {size, size}, {block, block});

  //   keeper.Start();

  for (int i = 0; i < size * size; i++) cout << C[i] << " " << Check[i] << endl;

  delete[] A;
  delete[] B;
  delete[] C;
}

template <typename Type>
void Convolution(unsigned int size, unsigned int radius) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* kern = new Type[(2 * radius + 1) * (2 * radius + 1)];

  for (unsigned int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 0;
  }

  for (unsigned int i = 0; i < (2 * radius + 1) * (2 * radius + 1); i++) {
    kern[i] = 2;
  }

  unsigned int* ptr_size = &size;
  unsigned int* ptr_radius = &radius;

  Keeper keeper("kernel.txt");

  Function Convolution("conv", "Convolution", true);
  Convolution.SetArgument<Type>(A, {size, size});
  Convolution.SetArgument<Type>(B, {size, size}, true);
  Convolution.SetArgument<unsigned int>(ptr_size, {1});
  Convolution.SetArgument<Type>(kern, {2 * radius + 1, 2 * radius + 1});
  Convolution.SetArgument<unsigned int>(ptr_radius, {1});

  keeper.SetFunction(Convolution);

  keeper.Test(10, "conv", 10, {size, size});

  delete[] A;
  delete[] B;
  delete[] kern;
}

template <typename Type>
void Integration(Type a, Type b, Type c, Type d, unsigned int size) {
  Type* sums = new Type[size * size];
  for (int i = 0; i < size * size; i++) {
    sums[i] = 0;
  }

  Type* ptr_a = &a;
  Type* ptr_b = &b;
  Type* ptr_c = &c;
  Type* ptr_d = &d;
  unsigned int* ptr_split = &size;

  Keeper keeper("kernel.txt");

  Function Integration("integ", "Integration");
  Integration.SetArgument<Type>(ptr_a, {1});
  Integration.SetArgument<Type>(ptr_b, {1});
  Integration.SetArgument<Type>(ptr_c, {1});
  Integration.SetArgument<Type>(ptr_d, {1});
  Integration.SetArgument<unsigned int>(ptr_split, {1});
  Integration.SetArgument<Type>(sums, {size, size}, true);

  keeper.SetFunction(Integration);
  // keeper.Test(10, "integ", 10, {size, size});

 // keeper.SetTask("integ", "ALL", {0, 0}, {size, size});

  keeper.SetTasks("integ", "ALL", {size / 2, size / 2}, {size, size});

  keeper.Start("DYNAMIC");
  keeper.Info("TIME");
  Type I = 0;

  for (int i = 0; i < size * size; i++) {
    I += sums[i];
  }
  cout << "Integral = " << I << endl;
}

int main(int argc, char* argv[]) {
  // std::string taskStr = argv[1];
  //
  // std::string sizeStr = argv[2];
  // int size = atoi(sizeStr.c_str());
  //
  // int task = atoi(taskStr.c_str());

  unsigned int size = 4000;

  int task = 5;

  switch (task) {
    case 1:
      DotProduct<float>(size);
      break;

    case 2:
      MulMatrix<int>(size);
      break;

    case 3:
      MulMatrixOpt<float>(size);
      break;

    case 4:
      Convolution<float>(size, size / 4);
      break;

    case 5:
      Integration<double>(1, 1000, 1, 1000, size);
      break;

    default:
      break;
  }

  int tmp;

  cout << endl;
  cout << "TYPE SOMETHING: ";

  cin >> tmp;
  return 0;
}
