#include <thread>
#include "QueenBee.hpp"

int func(string name) {
  cout << name;
  return 1;
}

void MulMatrix(double size);

int main(void) {
  /*Keeper queen("kernel.txt");
  queen.Info();

  std::size_t size = 20;

  double* A = new double[size];
  double* B = new double[size];
  double* C = new double[size];

  for (int i = 0; i < size; i++) {
    A[i] = size;
    B[i] = size;
    C[i] = size;
  }

  Function SumVectors("sum", "SumVectors");
  SumVectors.SetArgument(A, size * sizeof(double), false);
  SumVectors.SetArgument(B, size * sizeof(double), false);
  SumVectors.SetArgument(C, size * sizeof(double), true);

  queen.SetFunction(SumVectors);
  queen.SetTask("sum", "ALL", {10}, {20});
  queen.SetTask("sum", "ALL", {0}, {5});
  queen.SetTask("sum", "ALL", {15}, {20});
  // queen.SetTask("sum", "ALL", {5}, {10});

  // queen.SetTask("sum", "CPU", 66, 75);

  // queen.SetTask("sum", "CPU", 1501, 2000);

  queen.Start();

  for (int i = 0; i < size; i++) {
    cout << C[i] << ", ";
  }
  */

  MulMatrix(4);

  return 0;
}

void MulMatrix(double size) {
  Keeper queen("kernel.txt");
  queen.Info();

  double* A = new double[size * size];
  double* B = new double[size * size];
  double* C = new double[size * size];

  for (int i = 0; i < size * size; i++) {
    A[i] = i;
    B[i] = i;
    C[i] = 0;
  }

  ///////////////////////////////////////////////
 double* ptr_size = &size;

  Function MulMatrix("mul", "MulMatrix");
  MulMatrix.SetArgument(A, size * size * sizeof(double), false);
  MulMatrix.SetArgument(B, size * size * sizeof(double), false);
  MulMatrix.SetArgument(C, size * size * sizeof(double), true);
  MulMatrix.SetArgument(ptr_size, sizeof(double*), false);

  queen.SetFunction(MulMatrix);
 
 queen.SetTask("mul", "ALL", {0, 2}, {2, 4});
  queen.SetTask("mul", "ALL", {0, 0}, {2, 2});
  queen.SetTask("mul", "ALL", {0, 2}, {2, 4});
  queen.SetTask("mul", "ALL", {2, 0}, {4, 2});
 // queen.SetTask("mul", "ALL", {2, 2}, {4, 4});
 // queen.SetTask("mul", "ALL", {0, 2}, {2, 4});
 // queen.SetTask("mul", "GPU", {0, 0}, {4, 4});
  //queen.SetTask("mul", "GPU", {0, 2}, {4, 4});
 // queen.SetTask("mul", "GPU", {3, 3}, {4, 4});
  // queen.SetTask("sum", "ALL", {0}, {5});
  // queen.SetTask("sum", "ALL", {15}, {20});

  queen.Start();
      /////////////////////////////////////////////
  cout << endl;
      for (int i = 0; i < size ; i++) {
    for (int j = 0; j < size; j++) {
          std::cout << C[i*int(size)+j] << " "; 
	}
   
	/*
    if (C[i] != size * 2) {
     std::cout << "Error";
    }
	*/
        cout << endl;
  }

  delete[] A;
  delete[] B;
  delete[] C;
}