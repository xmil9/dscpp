#include "RingBufferTests.h"
#include "SboVectorTests.h"
#include <iostream>
#include <stdlib.h>


int main()
{
   TestRingBuffer();
   TestSboVector();
   std::cout << "DsCpp tests finished.\n";
   return EXIT_SUCCESS;
}
