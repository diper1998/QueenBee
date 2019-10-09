
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


  MulMatrix<int>(16000);



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


   Type* A1 = new Type[size];
  Type* B1 = new Type[size];
  Type* C1 = new Type[size];

  for (unsigned int i = 0; i < size; i++) {
    A1[i] = 1;
    B1[i] = 2;
    C1[i] = 0;
  }

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument<Type>(A1, {size}, false);
  SumVectors.SetArgument<Type>(B1, {size}, false);
  SumVectors.SetArgument<Type>(C1, {size},  true);

  queen.SetFunction(SumVectors);



  ///////////////////////////////////////////////

  Function MulMatrix("mul", "MulMatrix");
  MulMatrix.SetArgument<Type>(A, {size, size}, false);
  MulMatrix.SetArgument<Type>(B, {size, size}, false);
  MulMatrix.SetArgument<Type>(C, {size, size}, true);
  MulMatrix.SetArgument<unsigned int>(ptr_size, {1}, false);

  queen.SetFunction(MulMatrix);

    queen.Info();



 queen.SetTasks("sum", "ALL", {size/16}, {size});

 //  queen.SetTasks("mul", "ALL", {4, 4}, {16, 16});
 //   queen.SetTask("mul", "CPU", {0, 0}, {4, 4});
 //   queen.SetTask("mul", "GPU", {4, 4}, {8, 8});
 //   queen.SetTask("mul", "GPU", {8, 8}, {12, 12});
 //   queen.SetTask("mul", "CPU", {12, 12}, {16, 16});

   // queen.SetTask("sum", "GPU", {0}, {1});
  // queen.SetTask("sum", "ALL", {1000}, {2000});
   





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
       // std::cout << C[i * size + j] << " ";
    }
    //cout << endl;
  }
  
  int n = 0;
  for (unsigned int i = 0; i < size * size; i++) {
    
	  if (C[i] != size * 2) {
      n++;

    }
  }

  cout <<"!!!!!!!!!!!!!!!!!n = "<< n << endl;



  for (unsigned int i = 0; i < size; i++) {
    std::cout << C1[i] << " ";
  }

  n = 0;
  for (unsigned int i = 0; i < size; i++) {
    if (C1[i] != 3) {
      n++;
    }
  }
  cout << endl;
  cout << n << endl;


   delete[] A;
   delete[] B;
   delete[] C;

   delete[] A1;
   delete[] B1;
   delete[] C1;
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

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");
  queen.Info();
  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument<Type>(A, {size}, false);
  SumVectors.SetArgument<Type>(B, {size}, false);
  SumVectors.SetArgument<Type>(C, {size}, true);

  queen.SetFunction(SumVectors);
  queen.SetTask("sum", "CPU", {40}, {80});
  queen.SetTask("sum", "CPU", {10}, {20});
 queen.SetTask("sum", "CPU", {0}, {5});
  queen.SetTask("sum", "GPU", {5}, {10});
 queen.SetTask("sum", "GPU", {20}, {40});





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

  for (unsigned int i = 0; i < size; i++) {
    
      std::cout << C[i] << " ";

  }

  int n = 0;
  for (unsigned int i = 0; i < size; i++) {
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