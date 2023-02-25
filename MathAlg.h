//
// Feb-2023, Michael Lindner
// MIT license
//
#pragma once
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>

namespace ds
{
///////////////////

template <typename Iter> struct FindMaxSubsequenceResult
{
   Iter start{};
   Iter end{};
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

namespace internal
{
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

template <typename Iter>
FindMaxSubsequenceResult<Iter> FindMaxSubsequence(Iter first, Iter last)
{
   using Value = Iter::value_type;
   const size_t n = std::distance(first, last);

   if (n < 2)
      return {first, last, std::numeric_limits<Value>::lowest()};
   else if (n == 2)
      return {first, last, *(first + 1) - *first};

   Iter mid = first + n / 2;
   const auto lowerHalf = FindMaxSubsequence(first, mid);
   const auto upperHalf = FindMaxSubsequence(mid, last);
   const auto crossing = internal::FindMaxCrossingSubsequence(first, mid, last);

   const auto& tmp = lowerHalf.max > crossing.max ? lowerHalf : crossing;
   return upperHalf.max > tmp.max ? upperHalf : tmp;
}

} // namespace ds
