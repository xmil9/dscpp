//
// Feb-2023, Michael Lindner
// MIT license
//
#include "MathAlgTests.h"
#include "MathAlg.h"
#include "TestUtil.h"
#include <deque>

using namespace ds;


namespace
{
///////////////////

void TestFindMaxSubsequence()
{
   {
      const std::string caseLabel{"TestFindMaxSubsequence for integers"};

      std::vector<int> seq{7, 2, 3, 5, 9, 6};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence for floats"};

      std::vector<float> seq{7.f, 2.f, 3.f, 5.f, 9.f, 6.f};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 5, 7.f};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence with empty sequence"};

      std::vector<int> seq;
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence with sequence of one element"};

      std::vector<int> seq{3};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(),
                                   std::numeric_limits<int>::lowest()};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence with sequence of two elements increasing"};

      std::vector<int> seq{3, 5};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence with sequence of two elements decreasing"};

      std::vector<int> seq{5, 3};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence with sequence of two elements equal"};

      std::vector<int> seq{5, 5};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence - 3 elements, full seq"};

      std::vector<int> seq{3, 5, 8};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence - 3 elements, seq at start"};

      std::vector<int> seq{3, 5, 2};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence - 3 elements, seq at end"};

      std::vector<int> seq{3, 1, 4};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 3};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence - 3 elements, all equal"};

      std::vector<int> seq{3, 3, 3};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      // Only make sure max value is zero.
      VERIFY(res.max == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 3 elements, decreasing, seq at start"};

      std::vector<int> seq{8, 6, 1};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, -2};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 3 elements, decreasing, seq at end"};

      std::vector<int> seq{8, 2, 1};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), -1};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{"TestFindMaxSubsequence - 4 elements, full seq"};

      std::vector<int> seq{3, 5, 8, 11};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.end(), 8};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 4 elements, seq is 3 elems at front"};

      std::vector<int> seq{3, 5, 8, 6};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 4 elements, seq is 3 elems at end"};

      std::vector<int> seq{6, 5, 8, 11};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.end(), 6};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 4 elements, seq is 2 elems at front"};

      std::vector<int> seq{1, 11, 8, 10};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin(), seq.begin() + 2, 10};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 4 elements, seq is 2 elems in middle"};

      std::vector<int> seq{4, 3, 8, 7};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 1, seq.begin() + 3, 5};
      VERIFY(res == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "TestFindMaxSubsequence - 4 elements, seq is 2 elems at end"};

      std::vector<int> seq{4, 5, 2, 7};
      const auto res = FindMaxSubsequence(std::begin(seq), std::end(seq));

      const decltype(res) expected{seq.begin() + 2, seq.end(), 5};
      VERIFY(res == expected, caseLabel);
   }
}

} // namespace


///////////////////

void TestMathAlg()
{
   TestFindMaxSubsequence();
}
