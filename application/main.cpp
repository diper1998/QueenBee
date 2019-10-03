
#include <windows.h>
#include <thread>
#include "QueenBee.hpp"

int func(string name) {
  cout << name;
  return 1;
}
template <typename Type>
void MulMatrix(unsigned int size);

template <typename Type>
void SumVectors(unsigned int size);

int main(void) {

  MulMatrix<int>(6000);
 // SumVectors<int>(80);
  return 0;
}
template <typename Type>
void MulMatrix(unsigned int size) {
  Type* A = new Type[size * size];
  Type* B = new Type[size * size];
  Type* C = new Type[size * size];

  for (int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  unsigned int* ptr_size = &size;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");
  queen.Info();
  Function MulMatrix("mul", "MulMatrix");
  MulMatrix.SetArgument<Type>(A, {size, size}, false);
  MulMatrix.SetArgument<Type>(B, {size, size}, false);
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1}, false);

  queen.SetFunction(MulMatrix);

    queen.SetTask("mul", "CPU", {0, 0}, {3000, 3000});
  queen.SetTask("mul", "GPU", {0, 3000}, {3000, 6000});
    queen.SetTask("mul", "CPU", {3000, 0}, {6000, 3000});
  queen.SetTask("mul", "GPU", {3000, 3000}, {size, size});


  //queen.SetTask("mul", "CPU", {0, 0}, {5, 20});
  //queen.SetTask("mul", "GPU", {5, 0}, {10, 20});
  //queen.SetTask("mul", "GPU", {10, 0}, {15, 20});
  //queen.SetTask("mul", "CPU", {15, 0}, {20, 20});


 

  // queen.SetTask("mul", "CPU", {0, 0}, {2, 2});
  //   queen.SetTask("mul", "GPU", {2, 2}, {4, 4});
  // queen.SetTask("mul", "GPU", {4, 4}, {6, 6});
  //  queen.SetTask("mul", "CPU", {6, 6}, {8, 8});

  // queen.SetTask("mul", "GPU", {8, 8}, {16, 16});
  // queen.SetTask("mul", "CPU", {4, 0}, {4, 4});
  // queen.SetTask("mul", "CPU", {0, 4}, {4, 4});
  // queen.SetTask("mul", "CPU", {0, 4}, {4, 8});
  // queen.SetTask("mul", "CPU", {4, 4}, {4, 4});
  // queen.SetTask("mul", "CPU", {0, 0}, {4, 4});
  // queen.SetTask("mul", "ALL", {0, 2}, {2, 4});
  // queen.SetTask("mul", "ALL", {2, 0}, {4, 2});

  // queen.SetTask("mul", "CPU", {0, 0}, {2, 2});
  // queen.SetTask("mul", "CPU", {2, 2}, {4, 4});
  // queen.SetTask("mul", "CPU", {4, 4}, {8, 8});
  // queen.SetTask("mul", "CPU", {9, 9}, {16, 16});
  // queen.SetTask("mul", "CPU", {17, 17}, {32, 32});
  // queen.SetTask("mul", "GPU", {9, 9}, {16, 16});


  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  QueryPerformanceCounter(&t1);

  queen.Start();

  queen.Read();
  
  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  printf("The time: %f seconds\n", time);
  /////////////////////////////////////////////

  cout << endl << endl;

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      //  std::cout << C[i * size + j] << " ";
    }
    //cout << endl;
  }

  int n = 0;
  for (int i = 0; i < size * size; i++) {
    
	  if (C[i] != size * 2) {
      n++;

    }
  }
  cout << n << endl;



   delete[] A;
   delete[] B;
   delete[] C;
}

template <typename Type>
void SumVectors(unsigned int size) {
  Type* A = new Type[size];
  Type* B = new Type[size];
  Type* C = new Type[size];

  for (int i = 0; i < size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  unsigned int* ptr_size = &size;

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");
  queen.Info();
  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument<Type>(A, {size}, false);
  SumVectors.SetArgument<Type>(B, {size}, false);
  SumVectors.SetArgument<Type>(C, {size}, true);

  queen.SetFunction(SumVectors);

 queen.SetTask("sum", "CPU", {0}, {5});
  queen.SetTask("sum", "GPU", {5}, {10});
  queen.SetTask("sum", "GPU", {10}, {20});
  queen.SetTask("sum", "CPU", {20}, {40});
  queen.SetTask("sum", "CPU", {40}, {80});

  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  QueryPerformanceCounter(&t1);

  queen.Start();

  queen.Read();

  QueryPerformanceCounter(&t2);
  time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);

  printf("The time: %f seconds\n", time);
  /////////////////////////////////////////////

  cout << endl << endl;

  for (int i = 0; i < size; i++) {
    
      std::cout << C[i] << " ";

  }

  int n = 0;
  for (int i = 0; i < size; i++) {
    if (C[i] !=   3) {
      n++;
    }
  }
  cout << endl;
  cout << n << endl;

  delete[] A;
  delete[] B;
  delete[] C;
}