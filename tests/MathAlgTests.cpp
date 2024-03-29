//
// Feb-2023, Michael Lindner
// MIT license
//
#include "MathAlgTests.h"
#include "MathAlg.h"
#include "TestUtil.h"
#include <array>
#include <deque>
#include <vector>

using namespace ds;


namespace
{
///////////////////

void testIsPow2()
{
   {
      const std::string caseLabel{"isPow2"};

      VERIFY(!isPow2(-4.), caseLabel);
      VERIFY(!isPow2(0.), caseLabel);
      VERIFY(isPow2(1.), caseLabel);
      VERIFY(isPow2(2.), caseLabel);
      VERIFY(!isPow2(3.), caseLabel);
      VERIFY(isPow2(4.), caseLabel);
      VERIFY(!isPow2(6.), caseLabel);
      VERIFY(!isPow2(7.), caseLabel);
      VERIFY(!isPow2(5.), caseLabel);
      VERIFY(isPow2(8.), caseLabel);
      VERIFY(!isPow2(9.), caseLabel);
      VERIFY(!isPow2(10.), caseLabel);
      VERIFY(isPow2(16.), caseLabel);
      VERIFY(!isPow2(30.), caseLabel);
      VERIFY(isPow2(32.), caseLabel);
      VERIFY(!isPow2(50.), caseLabel);
      VERIFY(isPow2(64.), caseLabel);
      VERIFY(!isPow2(100.), caseLabel);
      VERIFY(isPow2(128.), caseLabel);
      VERIFY(!isPow2(255.), caseLabel);
      VERIFY(isPow2(256.), caseLabel);
      VERIFY(!isPow2(500.), caseLabel);
      VERIFY(isPow2(512.), caseLabel);
      VERIFY(!isPow2(1000.), caseLabel);
      VERIFY(isPow2(1024.), caseLabel);
      VERIFY(!isPow2(2046.), caseLabel);
      VERIFY(isPow2(2048.), caseLabel);
      VERIFY(!isPow2(4100.), caseLabel);
      VERIFY(isPow2(4096.), caseLabel);
      VERIFY(!isPow2(8200.), caseLabel);
      VERIFY(isPow2(8192.), caseLabel);
      VERIFY(isPow2(16384.), caseLabel);
      VERIFY(isPow2(32768.), caseLabel);
      VERIFY(!isPow2(100000.), caseLabel);
   }
   {
      const std::string caseLabel{"isPow2 for other value types"};

      VERIFY(isPow2(16), caseLabel);
      VERIFY(isPow2(16.f), caseLabel);
      VERIFY(isPow2(static_cast<short>(16)), caseLabel);
   }
}

void testFindMaxSubsequenceRecursive()
{
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive for integers"};

      std::vector<int> seq{7, 2, 3, 5, 9, 6};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive for floats"};

      std::vector<float> seq{7.f, 2.f, 3.f, 5.f, 9.f, 6.f};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7.f};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive with empty sequence"};

      std::vector<int> seq;
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive with sequence of one element"};

      std::vector<int> seq{3};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive with sequence of two elements increasing"};

      std::vector<int> seq{3, 5};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive with sequence of two elements decreasing"};

      std::vector<int> seq{5, 3};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive with sequence of two elements equal"};

      std::vector<int> seq{5, 5};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive - 3 elements, full seq"};

      std::vector<int> seq{3, 5, 8};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 3 elements, seq at start"};

      std::vector<int> seq{3, 5, 2};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive - 3 elements, seq at end"};

      std::vector<int> seq{3, 1, 4};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 3};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive - 3 elements, all equal"};

      std::vector<int> seq{3, 3, 3};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 3 elements, decreasing, seq at start"};

      std::vector<int> seq{8, 6, 1};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 3 elements, decreasing, seq at end"};

      std::vector<int> seq{8, 2, 1};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), -1};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive - 4 elements, full seq"};

      std::vector<int> seq{3, 5, 8, 11};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 8};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, seq is 3 elems at front"};

      std::vector<int> seq{3, 5, 8, 6};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, seq is 3 elems at end"};

      std::vector<int> seq{6, 5, 8, 11};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 6};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, seq is 2 elems at front"};

      std::vector<int> seq{1, 11, 8, 10};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 10};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, seq is 2 elems in middle"};

      std::vector<int> seq{4, 3, 8, 7};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, seq is 2 elems at end"};

      std::vector<int> seq{4, 5, 2, 7};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 2, seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - 4 elements, with negative values"};

      std::vector<int> seq{4, -5, 2, 7};
      const auto res = findMaxSubsequenceRecursive(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 12};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - container interface with 5 elements"};

      std::vector<int> seq{4, 1, 2, 2, 5};
      const auto res = findMaxSubsequenceRecursive(seq);

      const decltype(res) expected{seq.begin() + 1, seq.end(), 4};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceRecursive - container interface with deque"};

      std::deque<int> seq{2, 0, 3, 5, 2};
      const auto res = findMaxSubsequenceRecursive(seq);

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 4, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceRecursive - Cormen figure 4.1"};

      constexpr std::array<int, 17> seq{100, 113, 110, 85,  105, 102, 86, 63, 81,
                                        101, 94,  106, 101, 79,  94,  90, 97};
      const auto res = findMaxSubsequenceRecursive(seq);

      const decltype(res) expected{seq.begin() + 7, seq.begin() + 12, 43};
      VERIFY(res == expected, caseLabel);
   }
}

void testFindMaxSubsequenceIterative()
{
   {
      const std::string caseLabel{"findMaxSubsequenceIterative for integers"};

      std::vector<int> seq{7, 2, 3, 5, 9, 6};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative for floats"};

      std::vector<float> seq{7.f, 2.f, 3.f, 5.f, 9.f, 6.f};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7.f};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative with empty sequence"};

      std::vector<int> seq;
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative with sequence of one element"};

      std::vector<int> seq{3};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative with sequence of two elements increasing"};

      std::vector<int> seq{3, 5};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative with sequence of two elements decreasing"};

      std::vector<int> seq{5, 3};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative with sequence of two elements equal"};

      std::vector<int> seq{5, 5};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative - 3 elements, full seq"};

      std::vector<int> seq{3, 5, 8};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 3 elements, seq at start"};

      std::vector<int> seq{3, 5, 2};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative - 3 elements, seq at end"};

      std::vector<int> seq{3, 1, 4};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 3};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative - 3 elements, all equal"};

      std::vector<int> seq{3, 3, 3};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 3 elements, decreasing, seq at start"};

      std::vector<int> seq{8, 6, 1};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 3 elements, decreasing, seq at end"};

      std::vector<int> seq{8, 2, 1};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), -1};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative - 4 elements, full seq"};

      std::vector<int> seq{3, 5, 8, 11};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 8};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, seq is 3 elems at front"};

      std::vector<int> seq{3, 5, 8, 6};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, seq is 3 elems at end"};

      std::vector<int> seq{6, 5, 8, 11};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 6};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, seq is 2 elems at front"};

      std::vector<int> seq{1, 11, 8, 10};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 10};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, seq is 2 elems in middle"};

      std::vector<int> seq{4, 3, 8, 7};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, seq is 2 elems at end"};

      std::vector<int> seq{4, 5, 2, 7};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 2, seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - 4 elements, with negative values"};

      std::vector<int> seq{4, -5, 2, 7};
      const auto res = findMaxSubsequenceIterative(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 12};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - container interface with 5 elements"};

      std::vector<int> seq{4, 1, 2, 2, 5};
      const auto res = findMaxSubsequenceIterative(seq);

      const decltype(res) expected{seq.begin() + 1, seq.end(), 4};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "findMaxSubsequenceIterative - container interface with deque"};

      std::deque<int> seq{2, 0, 3, 5, 2};
      const auto res = findMaxSubsequenceIterative(seq);

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 4, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"findMaxSubsequenceIterative - Cormen figure 4.1"};

      constexpr std::array<int, 17> seq{100, 113, 110, 85,  105, 102, 86, 63, 81,
                                        101, 94,  106, 101, 79,  94,  90, 97};
      const auto res = findMaxSubsequenceIterative(seq);

      const decltype(res) expected{seq.begin() + 7, seq.begin() + 12, 43};
      VERIFY(res == expected, caseLabel);
   }
}

} // namespace


///////////////////

void testMathAlg()
{
   testIsPow2();
   testFindMaxSubsequenceRecursive();
   testFindMaxSubsequenceIterative();
}
