#include "SboVectorPerformanceTests.h"
#include "SboVector.h"
#include "TestUtil.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <list>
#include <vector>


namespace
{

///////////////////

void printTestResult(const std::string& caseLabel, int64_t svTime, int64_t stdTime)
{
   std::cout << caseLabel << '\n';
   std::cout << "SboVector     :" << std::setw(15) << std::right << svTime << '\n';
   std::cout << "std::vector   :" << std::setw(15) << std::right << stdTime << '\n';
   std::cout << "Ratio (SV/std):" << std::setw(15) << std::right
             << static_cast<float>(stdTime) / svTime << '\n';
}


template <typename TRes> struct MicroBenchmark
{
   using Clock = std::chrono::high_resolution_clock;

   MicroBenchmark(TRes& res) : result{res} { start = Clock::now(); }
   ~MicroBenchmark()
   {
      const auto end = Clock::now();
      const auto duration =
         std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
      result = duration.count();
   }

   TRes& result;
   Clock::time_point start;
};


///////////////////

void TestPushBackLValue()
{
   {
      const std::string caseLabel{"push_back: SboVector in buffer vs std::vector"};

      constexpr std::size_t numRuns = 1000;
      constexpr std::size_t BufCap = 100;
      constexpr std::size_t numElems = BufCap;
      int64_t svTime = 0;
      int64_t stdTime = 0;

      {
         MicroBenchmark measure{stdTime};

         for (int k = 0; k < numRuns; ++k)
         {
            std::vector<int> v;
            for (int i = 0; i < numElems; ++i)
               v.push_back(i);
         }
      }
      {
         MicroBenchmark measure{svTime};

         for (int k = 0; k < numRuns; ++k)
         {
            SboVector<int, BufCap> sv;
            for (int i = 0; i < numElems; ++i)
               sv.push_back(i);
         }
      }

      printTestResult(caseLabel, svTime, stdTime);
      VERIFY(svTime < stdTime, caseLabel);
   }
   {
      const std::string caseLabel{"push_back - SboVector on heap vs std::vector"};

      constexpr std::size_t numRuns = 1000;
      constexpr std::size_t BufCap = 10;
      constexpr std::size_t numElems = 100;
      int64_t svTime = 0;
      int64_t stdTime = 0;

      {
         MicroBenchmark measure{stdTime};

         for (int k = 0; k < numRuns; ++k)
         {
            std::vector<int> v;
            for (int i = 0; i < numElems; ++i)
               v.push_back(i);
         }
      }
      {
         MicroBenchmark measure{svTime};

         for (int k = 0; k < 1000; ++k)
         {
            SboVector<int, numRuns> sv;
            for (int i = 0; i < numElems; ++i)
               sv.push_back(i);
         }
      }

      printTestResult(caseLabel, svTime, stdTime);
      VERIFY(svTime < stdTime, caseLabel);
   }
}


void TestCopyIntoContainer()
{
   {
      const std::string caseLabel{"std::copy: SboVector in buffer vs std::vector"};

      constexpr std::size_t numRuns = 1000;
      constexpr std::size_t BufCap = 100;
      constexpr std::size_t numElems = BufCap;
      std::list<int> src;
      for (int i = 0; i < numElems; ++i)
         src.push_back(i);
      int64_t svTime = 0;
      int64_t stdTime = 0;

      {
         MicroBenchmark measure{stdTime};

         for (int k = 0; k < numRuns; ++k)
         {
            std::vector<int> v;
            std::copy(src.begin(), src.end(), std::back_inserter(v));
         }
      }
      {
         MicroBenchmark measure{svTime};

         for (int k = 0; k < numRuns; ++k)
         {
            SboVector<int, BufCap> sv;
            std::copy(src.begin(), src.end(), std::back_inserter(sv));
         }
      }

      printTestResult(caseLabel, svTime, stdTime);
      VERIFY(svTime < stdTime, caseLabel);
   }
   {
      const std::string caseLabel{"std::copy: SboVector on heap vs std::vector"};

      constexpr std::size_t numRuns = 1000;
      constexpr std::size_t BufCap = 10;
      constexpr std::size_t numElems = 100;
      std::list<int> src;
      for (int i = 0; i < numElems; ++i)
         src.push_back(i);
      int64_t svTime = 0;
      int64_t stdTime = 0;

      {
         MicroBenchmark measure{stdTime};

         for (int k = 0; k < numRuns; ++k)
         {
            std::vector<int> v;
            std::copy(src.begin(), src.end(), std::back_inserter(v));
         }
      }
      {
         MicroBenchmark measure{svTime};

         for (int k = 0; k < numRuns; ++k)
         {
            SboVector<int, BufCap> sv;
            std::copy(src.begin(), src.end(), std::back_inserter(sv));
         }
      }

      printTestResult(caseLabel, svTime, stdTime);
      VERIFY(svTime < stdTime, caseLabel);
   }
}

} // namespace


///////////////////

void TestSboVectorPerformance()
{
#ifdef NDEBUG
   TestPushBackLValue();
   TestCopyIntoContainer();
#else  // !NDEBUG
   std::cout << "Performance tests skipped - Use Release config for performance tests.\n";
#endif // NDEBUG
}
