//
// Michael Lindner
// MIT license
//
#include "LinearAlgebraTests.h"
#include "MathAlgTests.h"
#include "MatrixViewTests.h"
#include "RandomTests.h"
#include "RingBufferTests.h"
#include "SboVectorPerformanceTests.h"
#include "SboVectorTests.h"
#include "SortTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   testLinearAlgebra();
   testMathAlg();
   testMatrixView();
   testRandom();
   testRingBuffer();
   testSboVector();
   testSboVectorPerformance();
   testSort();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
