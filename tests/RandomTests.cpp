//
// Mar-2023, Michael Lindner
// MIT license
//
#include "RandomTests.h"
#include "Random.h"
#include "TestUtil.h"

using namespace ds;


namespace
{
///////////////////

template <typename T> bool equal(T a, T b)
{
   constexpr T Eps = 0.00001f;
   if constexpr (std::is_floating_point_v<T>)
      return std::abs(a - b) <= Eps;
   else
      return a == b;
}

template <typename T> bool verifyUniqueness(std::vector<T> vals, size_t numDuplicatesAllowed = 1)
{
   std::sort(vals.begin(), vals.end());
   const auto equalValIter =
      std::adjacent_find(vals.begin(), vals.end(), [](T a, T b) { return equal(a, b); });
   
   const size_t numDuplicates = std::distance(equalValIter, vals.end());
   return numDuplicates <= numDuplicatesAllowed;
}


template <typename T> bool verifyOpenRange(const std::vector<T>& vals, T min, T max)
{
   const auto match = std::find_if_not(
      vals.begin(), vals.end(), [min, max](T val) { return min <= val && val < max; });
   return match == vals.end();
}

///////////////////

void testRandomDefaultCtor()
{
   {
      const std::string caseLabel = "Random default ctor";

      using Fp = float;
      constexpr std::size_t numVals = 100;
      std::vector<Fp> vals;

      Random<Fp> rnd;
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rnd.next());

      VERIFY(verifyUniqueness(vals), caseLabel);
      VERIFY(verifyOpenRange(vals, 0.f, 1.f), caseLabel);
   }
}

void testRandomCtorWithSeed()
{
   {
      const std::string caseLabel = "Random ctor with seed";

      using Fp = float;
      constexpr std::size_t numVals = 100;
      constexpr unsigned int seed = 123456789;

      std::vector<Fp> valsA;
      Random<Fp> rndA{seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsA.push_back(rndA.next());

      VERIFY(verifyUniqueness(valsA), caseLabel);
      VERIFY(verifyOpenRange(valsA, 0.0f, 1.0f), caseLabel);

      std::vector<Fp> valsB;
      Random<Fp> rndB{seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsB.push_back(rndB.next());

      VERIFY(valsA == valsB, caseLabel);
   }
}

void testRandomCtorWithRangeValues()
{
   {
      const std::string caseLabel = "Random ctor with range";

      using Fp = double;
      constexpr std::size_t numVals = 100;
      constexpr Fp min = 101.0;
      constexpr Fp max = 200.0;

      std::vector<Fp> vals;
      Random<Fp> rnd{min, max};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rnd.next());

      VERIFY(verifyUniqueness(vals), caseLabel);
      VERIFY(verifyOpenRange(vals, min, max), caseLabel);
   }
}

void testRandomCtorWithRangeValuesAndSeed()
{
   {
      const std::string caseLabel = "Random ctor with seed";

      using Fp = double;
      constexpr std::size_t numVals = 100;
      constexpr unsigned int seed = 123456789;
      constexpr Fp min = 101.0;
      constexpr Fp max = 200.0;

      std::vector<Fp> valsA;
      Random<Fp> rndA{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsA.push_back(rndA.next());

      VERIFY(verifyUniqueness(valsA), caseLabel);
      VERIFY(verifyOpenRange(valsA, min, max), caseLabel);

      std::vector<Fp> valsB;
      Random<Fp> rndB{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsB.push_back(rndB.next());

      VERIFY(valsA == valsB, caseLabel);
   }
}

} // namespace


///////////////////

void testRandom()
{
   testRandomDefaultCtor();
   testRandomCtorWithSeed();
   testRandomCtorWithRangeValues();
   testRandomCtorWithRangeValuesAndSeed();
}
