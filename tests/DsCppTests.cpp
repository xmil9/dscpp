//
// Michael Lindner
// MIT license
//
#include "LinearAlgebraTests.h"
#include "MathAlgTests.h"
#include "RingBufferTests.h"
#include "SboVectorPerformanceTests.h"
#include "SboVectorTests.h"
#include "SortTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   TestLinearAlgebra();
   TestMathAlg();
   TestRingBuffer();
   TestSboVector();
   TestSboVectorPerformance();
   TestSort();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
