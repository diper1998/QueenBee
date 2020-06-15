#include "QueenBee.hpp"

template <typename Type>
Type Rand(Type low, Type high) {
  Type t = (Type)rand() / (Type)RAND_MAX;
  return (1.0 - t) * low + t * high;
}

void Pollination(unsigned int size) {
  unsigned int* ptr_size = &size;
  float* garden = new float[size * size];
  for (unsigned int i = 0; i < size * size; i++) {
    garden[i] = 1;
  }
  Keeper queen("kernel.txt");
  Function pollination("id", "Pollinate");
  pollination.SetArgument<float>(garden, {size, size}, true);
  pollination.SetArgument<unsigned int>(ptr_size, {1});

  queen.SetFunction(pollination);
  queen.SetTasks("id", "ALL", {size / 2, size / 2}, {size, size});

  queen.Start("DYNAMIC");

  queen.Info();

  for (unsigned int i = 0; i < size * size; i++) {
    if (garden[i] != 0) cout << "ERROR";
  }
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

  unsigned int block = 16;

  Type* a = NULL;
  Type* b = NULL;

  unsigned int* ptr_size = &size;

  unsigned int* ptr_block = &block;

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

  queen.Test(10, "mul", 10, {size, size}, {block, block}, {Check});

  //   queen.SetTask("mul", "GPU", {0, 0}, {size, size}, {block, block});
  //	  queen.SetTasks("mul", "ALL", {size/2, size/2}, {size, size}, {block,
  // block});
  //   queen.Start();
  //   queen.Info("TIME");

  delete[] A;
  delete[] B;
  delete[] C;
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

  Keeper queen("kernel.txt");

  Function Integration("integ", "Integration");
  Integration.SetArgument<Type>(ptr_a, {1});
  Integration.SetArgument<Type>(ptr_b, {1});
  Integration.SetArgument<Type>(ptr_c, {1});
  Integration.SetArgument<Type>(ptr_d, {1});
  Integration.SetArgument<unsigned int>(ptr_split, {1});
  Integration.SetArgument<Type>(sums, {size, size}, true);

  queen.SetFunction(Integration);

  queen.Test(10, "integ", 5, {size, size});

  // queen.SetTask("integ", "ALL", {0, 0}, {size, size});
  // queen.SetTasks("integ", "GPU", {size / 2, size / 2}, {size, size});
  // queen.Start();
  // queen.Info("TIME");

  Type I = 0;

  for (int i = 0; i < size * size; i++) {
    I += sums[i];
  }
  cout << "Integral = " << I << endl;
}

int main(int argc, char* argv[]) {
  std::string taskStr = argv[1];

  std::string sizeStr = argv[2];

  int size = atoi(sizeStr.c_str());

  int task = atoi(taskStr.c_str());

  switch (task) {
    case 1:
      Pollination(size);
      break;

    case 2:
      Integration<double>(1, 10000, 1, 10000, size);
      break;

    case 3:
      MulMatrixOpt<float>(size);
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
