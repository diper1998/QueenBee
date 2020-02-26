
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
void Convolution(unsigned int size, unsigned int radius, string mode,
                 int peace);


void ReadFunct(float* out, float*in) {
  *out = *out + *in;
  cout << out;
}


int main(void) {
  //MulMatrixOpt<float>(3200);
  MulMatrix<float>(1600);
 //   SumVectors<float>(10000);
// MonteCarlo(100000);
	//Convolution(2125, 50, "ALL", 10);
  
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
  //queen.Test("mul", {size, size});

  // queen.Info("DEV");

   queen.Test("mul", {size, size});

//  queen.SetTasks("mul", "ALL", {size/10 , size/10}, {size, size});
 // queen.SetTask("mul", "ALL", {0, 0}, {size, size});
    QueryPerformanceCounter(&t1);

 // queen.Start();
//  queen.Wait();
//  void (*pt2Func)(Type*, Type*) = NULL;
//  pt2Func = &ReadFunct;
  //queen.Read<Type>((*pt2Func));

 
//  queen.Info("STAT");
//  queen.Info("TIME");

  /////////////////////////////////////////////
 
  
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
  // std::cout << C[i * size + j] << " ";
    }
    //  cout << endl;
  }
  
 
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
  //Type* A = new Type[size * size];
  //Type* B = new Type[size * size];
  //Type* C = new Type[size * size];

  Type* A_1 = new Type[1600*1600];
  Type* B_1 = new Type[1600*1600];
  Type* C_1 = new Type[1600*1600];

  Type* A_2 = new Type[2400*2400];
  Type* B_2 = new Type[2400*2400];
  Type* C_2 = new Type[2400*2400];

  Type* A_3 = new Type[3200*3200];
  Type* B_3 = new Type[3200*3200];
  Type* C_3 = new Type[3200*3200];


  unsigned int block = 16;

  Type* a = NULL;
  Type* b = NULL;

  for (unsigned int i = 0; i < size * size; i++) {
   // A[i] = 1;
   // B[i] = 2;
   // C[i] = 0;

  }

   for (unsigned int i = 0; i < 1600 * 1600; i++) {
    A_1[i] = 1;
    B_1[i] = 2;
    C_1[i] = 0;
  }


    for (unsigned int i = 0; i < 2400 * 2400; i++) {
    A_2[i]= 1;
    B_2[i]= 2;
    C_2[i]= 0;
  }

	  for (unsigned int i = 0; i < 3200 * 3200; i++) {
    A_3[i] = 1;
    B_3[i] = 2;
    C_3[i] = 0;
  }



 // unsigned int* ptr_size = &size;
  unsigned int size_1 = 1600;
  unsigned int size_2 = 2400;
  unsigned int size_3 = 3200;


  unsigned int* ptr_size_1 = &size_1;
  unsigned int* ptr_size_2 = &size_2;
  unsigned int* ptr_size_3 = &size_3;


  unsigned int* ptr_block = &block;
  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

 // Function MulMatrixOpt("mul", "MulMatrixOpt", true);
 // MulMatrixOpt.SetArgument<Type>(A, {size, size}, false);
 // MulMatrixOpt.SetArgument<Type>(B, {size, size}, false);
 // MulMatrixOpt.SetArgument<Type>(C, {size, size}, true);
 // MulMatrixOpt.SetArgument<unsigned int*>(ptr_size, {1}, false);
 // MulMatrixOpt.SetArgument<Type>(a, {block, block}, false);
 // MulMatrixOpt.SetArgument<Type>(b, {block, block}, false);
 // MulMatrixOpt.SetArgument<unsigned int*>(ptr_block, {1}, false);


   Function MulMatrixOpt_1("mul_1", "MulMatrixOpt", true);
  MulMatrixOpt_1.SetArgument<Type>(A_1, {1600,1600}, false);
  MulMatrixOpt_1.SetArgument<Type>(B_1, {1600,1600}, false);
  MulMatrixOpt_1.SetArgument<Type>(C_1, {1600,1600}, true);
  MulMatrixOpt_1.SetArgument<unsigned int*>(ptr_size_1, {1}, false);
  MulMatrixOpt_1.SetArgument<Type>(a, {block, block}, false);
  MulMatrixOpt_1.SetArgument<Type>(b, {block, block}, false);
  MulMatrixOpt_1.SetArgument<unsigned int*>(ptr_block, {1}, false);	
   queen.SetFunction(MulMatrixOpt_1);

   Function MulMatrixOpt_2("mul_2", "MulMatrixOpt", true);
  MulMatrixOpt_2.SetArgument<Type>(A_2, {2400,2400}, false);
  MulMatrixOpt_2.SetArgument<Type>(B_2, {2400,2400}, false);
  MulMatrixOpt_2.SetArgument<Type>(C_2, {2400,2400}, true);
  MulMatrixOpt_2.SetArgument<unsigned int*>(ptr_size_2, {1}, false);
  MulMatrixOpt_2.SetArgument<Type>(a, {block, block}, false);
  MulMatrixOpt_2.SetArgument<Type>(b, {block, block}, false);
  MulMatrixOpt_2.SetArgument<unsigned int*>(ptr_block, {1}, false);

    queen.SetFunction(MulMatrixOpt_2);

   Function MulMatrixOpt_3("mul_3", "MulMatrixOpt", true);
  MulMatrixOpt_3.SetArgument<Type>(A_3, {3200,3200}, false);
  MulMatrixOpt_3.SetArgument<Type>(B_3, {3200,3200}, false);
  MulMatrixOpt_3.SetArgument<Type>(C_3, {3200,3200}, true);
  MulMatrixOpt_3.SetArgument<unsigned int*>(ptr_size_3, {1}, false);
  MulMatrixOpt_3.SetArgument<Type>(a, {block, block}, false);
  MulMatrixOpt_3.SetArgument<Type>(b, {block, block}, false);
  MulMatrixOpt_3.SetArgument<unsigned int*>(ptr_block, {1}, false);

  queen.SetFunction(MulMatrixOpt_3);

  queen.Info("STAT");

 


//    queen.SetTask("mul_3", "GPU", {0, 0}, {3200, 3200}, {block, block});
//
	
//    queen.SetTask("mul_3", "GPU", {0, 0}, {3200, 3200}, {block, block});
//
//	
	 queen.SetTask("mul_2", "GPU", {0, 0}, {2400, 2400}, {block, block});
//    
//	  queen.SetTask("mul_2", "GPU", {0, 0}, {2400, 2400}, {block, block});
     

	  queen.SetTask("mul_1", "CPU", {0, 0}, {1600, 1600}, {block, block});

	      queen.SetTask("mul_2", "CPU", {0, 0}, {2400, 2400}, {block, block});


	  queen.SetTask("mul_2", "GPU", {0, 0}, {2400, 2400}, {block, block});


	    queen.SetTask("mul_1", "GPU", {0, 0}, {1600, 1600}, {block, block});

		  

		    queen.SetTask("mul_3", "GPU", {0, 0}, {3200, 3200},
                                {block, block});

   queen.Start();

   queen.Info("TIME");

 // queen.Test("mul", {size, size}, {block, block});

 // queen.SetTasks("mul", "ALL", {100, 100}, {1000, 1000}, {block, block});

 // queen.SetTask("mul", "CPU", {8500, 8500}, {10000, 10000}, {block, block});
 // queen.SetTask("mul", "GPU", {0, 0}, {8500, 8500}, {block, block});
 // queen.SetTask("mul", "GPU", {0, 8500}, {8500, 10000}, {block, block});
 // queen.SetTask("mul", "GPU", {8500, 0}, {10000, 8500}, {block, block});
 //
 //
 // QueryPerformanceCounter(&t1);
 //
 // queen.Start();
 // queen.Wait();
 //// queen.Read();
 //
 // QueryPerformanceCounter(&t2);
 // time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
 //
 // cout << "The time: seconds\n" << time << endl;
 //
 // queen.Info("STAT");
 // /////////////////////////////////////////////
 //
 // /*
 // for (int i = 0; i < size; i++) {
 //   for (int j = 0; j < size; j++) {
 //  std::cout << C[i * size + j] << " ";
 //   }
 //     cout << endl;
 // }
 // */
 //
 // int error = 0;
 // for (unsigned int i = 0; i < size * size; i++) {
 //   if (C[i] != size * 2) {
 //     error++;
 //   }
 // }
 //
 // cout << "ERROR = " << error << endl;

  delete[] A_1;
  delete[] B_1;
  delete[] C_1;

  delete[] A_2;
  delete[] B_2;
  delete[] C_2;

  delete[] A_3;
  delete[] B_3;
  delete[] C_3;

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

  queen.Test("sum", {size});

  //queen.SetTasks("sum", "ALL", {size / 10}, {size});
  //
  //QueryPerformanceCounter(&t1);
  //queen.Start();
  //queen.Wait();
  //queen.Read();
  //QueryPerformanceCounter(&t2);
  //time = (t2.QuadPart - t1.QuadPart) / double(frequency.QuadPart);
  //
  //cout << "The time: seconds\n" << time << endl;
  //
  //queen.Info("STAT");
  ///////////////////////////////////////////////
  //
  ///*
  //for (unsigned int i = 0; i < size; i++) {
  //  std::cout << C[i] << " ";
  //}
  //*/
  //
  //int error = 0;
  //for (unsigned int i = 0; i < size; i++) {
  //  if (C[i] != 3) {
  //    error++;
  //  }
  //}
  //cout << "ERROR = " << error << endl;

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

//  queen.Info("STAT");

  queen.Test("mc", {size});

 //  queen.SetTasks("mc", "ALL", {size / 10}, {size});
 //
 // QueryPerformanceCounter(&t1);
 //
 //  queen.Start();
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

void Convolution(unsigned int size, unsigned int radius, string mode,
                 int peace) {
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

  LARGE_INTEGER frequency;
  LARGE_INTEGER t1, t2;
  double time;
  QueryPerformanceFrequency(&frequency);

  ///////////////////////////////////////////////
  Keeper queen("kernel.txt");

  Function Convolution("conv", "Convolution", true);
  Convolution.SetArgument<float>(A, {size, size}, false);
  Convolution.SetArgument<float>(B, {size, size}, true);
  Convolution.SetArgument<unsigned int>(ptr_size, {1}, false);
  Convolution.SetArgument<float>(kern, {2 * radius + 1, 2 * radius + 1}, false);
  Convolution.SetArgument<unsigned int>(ptr_radius, {1}, false);

  queen.SetFunction(Convolution);

  queen.Info("STAT");

  queen.Test("conv", {size, size});

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

  //QueryPerformanceCounter(&t2);
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
