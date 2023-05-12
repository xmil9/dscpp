//
// Feb-2023, Michael Lindner
// MIT license
//
#pragma once
#include <functional>
#include <vector>

namespace ds
{

///////////////////

// Insertion sort
// Cormen, pg 16
// Take each element and insert backwards into already sorted front of sequence.
// Efficient for small number of elements. Sorts in place, no extra space needed.
// Worst time: O(n^2) (for reverse sorted)
// Avg time: O(n^2)

// Iterator interface
template <typename Iter, typename Compare = std::less<typename Iter::value_type>>
void InsertionSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   using Elem = typename Iter::value_type;

   if (first == last)
      return;

   for (Iter it = first + 1; it < last; ++it)
   {
      const Elem val = *it;

      Iter pos = it;
      while (pos > first && cmp(val, *(pos - 1)))
      {
         *pos = *(pos - 1);
         --pos;
      }

      *pos = val;
   }
}

// Container interface
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void InsertionSort(Container& seq, Compare cmp = {}) noexcept
{
   InsertionSort(std::begin(seq), std::end(seq), cmp);
}

///////////////////

// Merge sort
// Cormen, pg 30
// Divide sequence into smaller and smaller sub sequences and merge then back together
// while sorting the elements. Does not sort in place, needs additional space.
// Time: O(nlgn)

namespace internal
{
// Helper function to merge two subsequences back together.
template <typename Iter, typename Compare>
void Merge(Iter first, Iter mid, Iter last, Compare cmp) noexcept
{
   using Elem = typename Iter::value_type;

   const std::vector<Elem> subA(first, mid);
   const std::vector<Elem> subB(mid, last);

   Iter merged = first;
   auto posA = subA.begin();
   auto endA = subA.end();
   auto posB = subB.begin();
   auto endB = subB.end();

   // Copy the element that wins the comparision between the next elements in each
   // subsequence to the merged sequence.
   while (posA < endA && posB < endB)
      *merged++ = cmp(*posA, *posB) ? *posA++ : *posB++;

   // Flush the subsequence that still has unmerged elemens to the merged sequence.
   const bool flushA = posB == endB;
   auto& flushPos = flushA ? posA : posB;
   auto& flushEnd = flushA ? endA : endB;

   while (flushPos < flushEnd)
      *merged++ = *flushPos++;
}
} // namespace internal

// Iterator interface
template <typename Iter, typename Compare = std::less<typename Iter::value_type>>
void MergeSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   // Base case - sequence is fully sorted.
   const auto len = std::distance(first, last);
   if (len < 2)
      return;

   // Divide into subsequences and sort those.
   Iter mid = first + len / 2;
   MergeSort(first, mid, cmp);
   MergeSort(mid, last, cmp);

   // Merge subsequences back together.
   internal::Merge(first, mid, last, cmp);
}

// Container interface
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void MergeSort(Container& seq, Compare cmp = {}) noexcept
{
   MergeSort(std::begin(seq), std::end(seq), cmp);
}

///////////////////

// Bubble sort
// Cormen, pg 40
// Swap consecutive elements until squence is sorted.
// Sorts in place, no extra space needed.
// Time: O(n^2)

// Iterator interface
template <typename Iter, typename Compare = std::less<typename Iter::value_type>>
void BubbleSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   for (Iter i = first; i != last; ++i)
      for (Iter j = i + 1; j != last; ++j)
         if (cmp(*j, *i))
            std::swap(*i, *j);
}

// Container interface
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void BubbleSort(Container& seq, Compare cmp = {}) noexcept
{
   BubbleSort(std::begin(seq), std::end(seq), cmp);
}

} // namespace ds
