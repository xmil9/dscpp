//
// Feb-2023, Michael Lindner
// MIT license
//
#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <limits>
#include <vector>

namespace ds
{
///////////////////

// Find maximum subarray/subsequence
// Finds max difference between two values in a sequence. The lower value has to appear
// before the higher value.
// Example use: Find best buying and selling points for a stock on a timeline.
// Time (for iterative solution): O(n)


// Result data.
template <typename Iter> struct FindMaxSubsequenceResult
{
   // Position of low element.
   Iter start{};
   // Position of one after the high element.
   Iter end{};
   // Value of subsequence.
   Iter::value_type max{};
};

template <typename Iter>
bool operator==(const FindMaxSubsequenceResult<Iter>& a,
                const FindMaxSubsequenceResult<Iter>& b)
{
   return a.start == b.start && a.end == b.end && a.max == b.max;
}

template <typename Iter>
bool operator!=(const FindMaxSubsequenceResult<Iter>& a,
                const FindMaxSubsequenceResult<Iter>& b)
{
   return !(a == b);
}

template <typename Iter>
bool operator<(const FindMaxSubsequenceResult<Iter>& a,
               const FindMaxSubsequenceResult<Iter>& b)
{
   return a.max < b.max;
}

// Recursive solution
// Cormen, pg 68
// Time: O(nlgn)
// Avoid - Slower than iterative solution below!

namespace internal
{
// Find a max subsequence that is restricted to having to cross a mid point in the
// sequence. This is a much simpler problem to solve than finding an unrestricted
// subsequence.
template <typename Iter>
FindMaxSubsequenceResult<Iter> FindMaxCrossingSubsequence(Iter first, Iter mid,
                                                          Iter last) noexcept
{
   assert(first != mid);
   assert(mid != last);

   // Find min element in [first, mid).
   const auto minPos = std::min_element(first, mid);
   // Find max element in [mid, last).
   const auto maxPos = std::max_element(mid, last);

   return {minPos, maxPos + 1, *maxPos - *minPos};
}
} // namespace internal

// Iterator interface
template <typename Iter>
FindMaxSubsequenceResult<Iter> FindMaxSubsequenceRecursive(Iter first, Iter last)
{
   using Result = FindMaxSubsequenceResult<Iter>;
   using Value = Iter::value_type;

   const size_t n = std::distance(first, last);

   // Base cases - Two or less elements.
   if (n < 2)
      return {first, last, std::numeric_limits<Value>::lowest()};
   else if (n == 2)
      return {first, last, *(first + 1) - *first};

   // Divide into three cases - Lower and upper halfs of the sequence, and a
   // max subsequence that crosses the mid point.
   Iter mid = first + n / 2;
   // clang-format off
   const std::array<Result, 3> results{
      FindMaxSubsequenceRecursive(first, mid),
      FindMaxSubsequenceRecursive(mid, last),
      internal::FindMaxCrossingSubsequence(first, mid, last)};
   // clang-format on

   // Return max of the intermediate results.
   return *std::max_element(std::begin(results), std::end(results));
}

// Container interface
template <typename Container>
FindMaxSubsequenceResult<typename Container::const_iterator>
FindMaxSubsequenceRecursive(const Container& seq)
{
   return FindMaxSubsequenceRecursive(seq.begin(), seq.end());
}

// Iterative solution
// Cormen, pg 75, exercise 4.1-5
// Time: O(n)
// Faster than recursive solution above.

// Iterator interface
template <typename Iter>
FindMaxSubsequenceResult<Iter> FindMaxSubsequenceIterative(Iter first, Iter last)
{
   using Value = Iter::value_type;

   const size_t n = std::distance(first, last);
   if (n < 2)
      return {first, last, std::numeric_limits<Value>::lowest()};

   Iter subStart = first;
   Iter subEnd = first + 1;
   Value max = *subEnd - *subStart;
   // Minimum element found overall. Not necessarily the same as the start of the
   // max subsequence.
   Iter minPos = *subStart < *subEnd ? subStart : subEnd;

   for (Iter cur = first + 2; cur != last; ++cur)
   {
      // Extend subsequence if current value is larger than the end of the
      // subsequence.
      if (*cur > *subEnd)
      {
         subEnd = cur;
         max = *subEnd - *subStart;
      }

      // Difference of min overall element to current element is larger
      // than the subsequence max.
      // Reset subsequence to [minPos, cur].
      if (minPos != subStart && *cur - *minPos > max)
      {
         subStart = minPos;
         subEnd = cur;
         max = *cur - *minPos;
      }

      // Keep track if current element is the absolute min value.
      if (*cur < *minPos)
         minPos = cur;
   }

   return {subStart, subEnd + 1, max};
}

// Container interface
template <typename Container>
FindMaxSubsequenceResult<typename Container::const_iterator>
FindMaxSubsequenceIterative(const Container& seq)
{
   return FindMaxSubsequenceIterative(seq.begin(), seq.end());
}

// Preferred solution

// Iterator interface
template <typename Iter>
FindMaxSubsequenceResult<Iter> FindMaxSubsequence(Iter first, Iter last)
{
   // Iterative solution is faster with O(n) vs O(nlgn) for recursive solution.
   return FindMaxSubsequenceIterative(first, last);
}

// Container interface
template <typename Container>
FindMaxSubsequenceResult<typename Container::const_iterator>
FindMaxSubsequence(const Container& seq)
{
   return FindMaxSubsequence(seq.begin(), seq.end());
}

} // namespace ds
