//
// Michael Lindner
// MIT license
//
#include "HeapTests.h"
#include "LinearAlgebraTests.h"
#include "MathAlgTests.h"
#include "MatrixViewTests.h"
#include "PriorityQueueTests.h"
#include "RandomTests.h"
#include "RingBufferTests.h"
#include "SboVectorPerformanceTests.h"
#include "SboVectorTests.h"
#include "SortTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   testHeapView();
   testLinearAlgebra();
   testMathAlg();
   testMatrixView();
   testPriorityQueue();
   testRandom();
   testRingBuffer();
   testSboVector();
   testSboVectorPerformance();
   testSort();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
