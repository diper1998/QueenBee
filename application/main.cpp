#include <thread>
#include "QueenBee.hpp"

int func(string name) {
  cout << name;
  return 1;
}

void MulMatrix(int size);

int main(void) {
  MulMatrix(16);

  return 0;
}

void MulMatrix(int size) {
 
  double* A = new double[size * size];
  double* B = new double[size * size];
  double* C = new double[size * size];

  for (int i = 0; i < size * size; i++) {
    A[i] = 1;
    B[i] = 2;
    C[i] = 0;
  }

  int* ptr_size = &size;



  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");
  queen.Info();
  Function MulMatrix("mul", "MulMatrix");
  MulMatrix.SetArgument(A, {size, size}, size * size * sizeof(double), false);
  MulMatrix.SetArgument(B, {size, size}, size * size * sizeof(double), false);
  MulMatrix.SetArgument(C, {size, size}, size * size * sizeof(double), true);
  MulMatrix.SetArgument(ptr_size, {}, sizeof(unsigned int*), false);

  queen.SetFunction(MulMatrix);

  queen.SetTask("mul", "ALL", {2, 0}, {4, 2});
  queen.SetTask("mul", "ALL", {0, 2}, {2, 4});
  queen.SetTask("mul", "CPU", {0, 0}, {2, 2});
  queen.SetTask("mul", "GPU", {2, 2}, {4, 4});

  queen.Start();
  /////////////////////////////////////////////



  cout << endl << endl;
  

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      std::cout << C[i * int(size) + j] << " ";
    }
    cout << endl;
  }
    
  /*
  for (int i = 0; i < size*size; i++){
    if (C[i] != size * 2) {
    std::cout << "Error";
      break;
    }
    
  }
  */

  delete[] A;
  delete[] B;
  delete[] C;
}