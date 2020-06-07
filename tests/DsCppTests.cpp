#include "RingBufferTests.h"
#include "SboVectorPerformanceTests.h"
#include "SboVectorTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   TestRingBuffer();
   TestSboVector();
   TestSboVectorPerformance();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
