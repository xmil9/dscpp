//
// Michael Lindner
// MIT license
//
#include "MathAlgTests.h"
#include "RingBufferTests.h"
#include "SboVectorPerformanceTests.h"
#include "SboVectorTests.h"
#include "SortTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   TestMathAlg();
   TestRingBuffer();
   TestSboVector();
   TestSboVectorPerformance();
   TestSort();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
