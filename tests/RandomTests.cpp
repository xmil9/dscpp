//
// Mar-2023, Michael Lindner
// MIT license
//
#include "RandomTests.h"
#include "Random.h"
#include "TestUtil.h"
#include <algorithm>
#include <iterator>
#include <numeric>

using namespace ds;


namespace
{
///////////////////

template <typename T> bool equal(T a, T b) noexcept
{
   constexpr T Eps = 0.00001f;
   if constexpr (std::is_floating_point_v<T>)
      return std::abs(a - b) <= Eps;
   else
      return a == b;
}

// Checks that the gaps between element values of the given array are between given
// min and max values.
template <typename T> bool verifyUniform(std::vector<T> vals, T minGap, T maxGap)
{
   std::sort(vals.begin(), vals.end());

   std::vector<T> diffs;
   std::adjacent_difference(vals.begin(), vals.end(), std::back_inserter(diffs));

   // The first element of the differences is a copy of the first element of the values,
   // so skip it when looking at the diffs.
   const auto minmaxPos = std::minmax_element(diffs.begin() + 1, diffs.end());

   return *minmaxPos.first >= minGap && *minmaxPos.second <= maxGap;
}

template <typename T> bool verifyOpenRange(const std::vector<T>& vals, T min, T max)
{
   const auto match = std::find_if_not(
      vals.begin(), vals.end(), [min, max](T val) { return min <= val && val < max; });
   return match == vals.end();
}

template <typename T> bool verifyClosedRange(const std::vector<T>& vals, T min, T max)
{
   const auto match = std::find_if_not(
      vals.begin(), vals.end(), [min, max](T val) { return min <= val && val <= max; });
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

      VERIFY(verifyUniform(vals, 0.f, .1f), caseLabel);
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

      VERIFY(verifyUniform(valsA, 0.f, .1f), caseLabel);
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
      const std::string caseLabel = "Random ctor for range";

      using Fp = double;
      constexpr std::size_t numVals = 500;
      constexpr Fp min = 101.0;
      constexpr Fp max = 150.0;

      std::vector<Fp> vals;
      Random<Fp> rnd{min, max};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rnd.next());

      VERIFY(verifyUniform(vals, .000001, 1.), caseLabel);
      VERIFY(verifyOpenRange(vals, min, max), caseLabel);
   }
}

void testRandomCtorWithRangeValuesAndSeed()
{
   {
      const std::string caseLabel = "Random ctor for range with seed";

      using Fp = double;
      constexpr std::size_t numVals = 500;
      constexpr unsigned int seed = 123456789;
      constexpr Fp min = 101.0;
      constexpr Fp max = 150.0;

      std::vector<Fp> valsA;
      Random<Fp> rndA{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsA.push_back(rndA.next());

      VERIFY(verifyUniform(valsA, .000001, 1.), caseLabel);
      VERIFY(verifyOpenRange(valsA, min, max), caseLabel);

      std::vector<Fp> valsB;
      Random<Fp> rndB{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         valsB.push_back(rndB.next());

      VERIFY(valsA == valsB, caseLabel);
   }
}

///////////////////

void testRandomIntCtorWithRangeValues()
{
   {
      const std::string caseLabel = "RandomInt ctor for range";

      using Int = int;
      constexpr std::size_t numVals = 300;
      constexpr Int min = 10;
      constexpr Int max = 20;

      std::vector<Int> vals;
      RandomInt<Int> rnd{min, max};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rnd.next());

      VERIFY(std::find(vals.begin(), vals.end(), min) != vals.end(), caseLabel);
      VERIFY(std::find(vals.begin(), vals.end(), max) != vals.end(), caseLabel);
      // No value gap should be more than one, i.e. all values are present.
      VERIFY(verifyUniform(vals, 0, 1), caseLabel);
      // No value is outside the range.
      VERIFY(verifyClosedRange(vals, min, max), caseLabel);
   }
   {
      const std::string caseLabel = "RandomInt ctor for range of negative values";

      using Int = int;
      constexpr std::size_t numVals = 300;
      constexpr Int min = -10;
      constexpr Int max = -1;

      std::vector<Int> vals;
      RandomInt<Int> rnd{min, max};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rnd.next());

      VERIFY(std::find(vals.begin(), vals.end(), min) != vals.end(), caseLabel);
      VERIFY(std::find(vals.begin(), vals.end(), max) != vals.end(), caseLabel);
      // No value gap should be more than one, i.e. all values are present.
      VERIFY(verifyUniform(vals, 0, 1), caseLabel);
      // No value is outside the range.
      VERIFY(verifyClosedRange(vals, min, max), caseLabel);
   }
}

void testRandomIntCtorWithRangeValuesAndSeed()
{
   {
      const std::string caseLabel = "RandomInt ctor with seed";

      using Int = long;
      constexpr std::size_t numVals = 300;
      constexpr unsigned int seed = 123456789;
      constexpr Int min = 1;
      constexpr Int max = 10;

      std::vector<Int> vals;
      RandomInt<Int> rndA{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rndA.next());

      VERIFY(std::find(vals.begin(), vals.end(), min) != vals.end(), caseLabel);
      VERIFY(std::find(vals.begin(), vals.end(), max) != vals.end(), caseLabel);
      // No value gap should be more than one, i.e. all values are present.
      VERIFY(verifyUniform(vals, 0l, 1l), caseLabel);
      // No value is outside the range.
      VERIFY(verifyClosedRange(vals, min, max), caseLabel);

      std::vector<Int> repeated;
      RandomInt<Int> rndB{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         repeated.push_back(rndB.next());

      VERIFY(vals == repeated, caseLabel);
   }
   {
      const std::string caseLabel = "RandomInt ctor with seed and negative values";

      using Int = long;
      constexpr std::size_t numVals = 300;
      constexpr unsigned int seed = 123456789;
      constexpr Int min = -10;
      constexpr Int max = -1;

      std::vector<Int> vals;
      RandomInt<Int> rndA{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         vals.push_back(rndA.next());

      VERIFY(std::find(vals.begin(), vals.end(), min) != vals.end(), caseLabel);
      VERIFY(std::find(vals.begin(), vals.end(), max) != vals.end(), caseLabel);
      // No value gap should be more than one, i.e. all values are present.
      VERIFY(verifyUniform(vals, 0l, 1l), caseLabel);
      // No value is outside the range.
      VERIFY(verifyClosedRange(vals, min, max), caseLabel);

      std::vector<Int> repeated;
      RandomInt<Int> rndB{min, max, seed};
      for (std::size_t i = 0; i < numVals; ++i)
         repeated.push_back(rndB.next());

      VERIFY(vals == repeated, caseLabel);
   }
}

///////////////////

void testPermute()
{
   {
      const std::string caseLabel = "permute() for integer sequence";

      using Val = short;
      constexpr std::size_t numVals = 300;

      std::vector<Val> vals(numVals, 0);
      Val counter{0};
      std::ranges::generate_n(vals.begin(), numVals, [&counter](){ return counter++; });
      const auto original = vals;

      permute(vals.begin(), vals.end());

      VERIFY(std::ranges::is_permutation(vals, original), caseLabel);
      // It is legal for the permutation to be the same as the original but should be
      // very unlikely, so let's use it as a check here.
      VERIFY(vals != original, caseLabel);
   }
   {
      const std::string caseLabel = "permute() for string sequence";

      std::vector<std::string> seq{"aa", "bb", "cc", "dd", "ee", "ff", "gg", "hh", "ii", "jj", "kk"};
      const auto original = seq;

      permute(seq.begin(), seq.end());

      VERIFY(std::ranges::is_permutation(seq, original), caseLabel);
      // It is legal for the permutation to be the same as the original but should be
      // very unlikely, so let's use it as a check here.
      VERIFY(seq != original, caseLabel);
   }
   {
      const std::string caseLabel = "permute() range interface";

      using Val = double;
      constexpr std::size_t numVals = 300;

      std::vector<Val> vals(numVals, 0);
      Val counter{0};
      std::ranges::generate_n(vals.begin(), numVals, [&counter](){ return counter++; });
      const auto original = vals;

      permute(vals);

      VERIFY(std::ranges::is_permutation(vals, original), caseLabel);
      // It is legal for the permutation to be the same as the original but should be
      // very unlikely, so let's use it as a check here.
      VERIFY(vals != original, caseLabel);
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
   testRandomIntCtorWithRangeValues();
   testRandomIntCtorWithRangeValuesAndSeed();
   testPermute();
}
