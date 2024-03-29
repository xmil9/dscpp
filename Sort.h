//
// Feb-2023, Michael Lindner
// MIT license
//
#pragma once
#include "Heap.h"
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
void insertionSort(Iter first, Iter last, Compare cmp = {}) noexcept
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
void insertionSort(Container& seq, Compare cmp = {}) noexcept
{
   insertionSort(std::begin(seq), std::end(seq), cmp);
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
void merge(Iter first, Iter mid, Iter last, Compare cmp) noexcept
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
void mergeSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   // Base case - sequence is fully sorted.
   const auto len = std::distance(first, last);
   if (len < 2)
      return;

   // Divide into subsequences and sort those.
   Iter mid = first + len / 2;
   mergeSort(first, mid, cmp);
   mergeSort(mid, last, cmp);

   // Merge subsequences back together.
   internal::merge(first, mid, last, cmp);
}

// Container interface
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void mergeSort(Container& seq, Compare cmp = {}) noexcept
{
   mergeSort(std::begin(seq), std::end(seq), cmp);
}

///////////////////

// Bubble sort
// Cormen, pg 40
// Swap consecutive elements until squence is sorted.
// Sorts in place, no extra space needed.
// Time: O(n^2)

// Iterator interface
template <typename Iter, typename Compare = std::less<typename Iter::value_type>>
void bubbleSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   for (Iter i = first; i != last; ++i)
      for (Iter j = i + 1; j != last; ++j)
         if (cmp(*j, *i))
            std::swap(*i, *j);
}

// Container interface
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void bubbleSort(Container& seq, Compare cmp = {}) noexcept
{
   bubbleSort(std::begin(seq), std::end(seq), cmp);
}

///////////////////

// Heap sort
// Cormen, pg 151
// Uses heap data structure to repeatedly sort the next element into its correct place.
// Sorts in place, no extra space needed.
// Time: O(nlgn)
// The actual heap sort algorithm uses a max-heap to sort in ascending order but to allow
// the caller to pass their own comparision we have to use the general heap data structure
// and then reverse the output sequence.

template <typename T, typename Compare = std::less<T>>
void heapSort(T* first, size_t numElems, const Compare& cmp = {})
{
   HeapView heap(first, numElems, cmp);
   while (!heap.empty())
      heap.pop();
   
   // We have to reverse the sequence because the heap puts the most extreme element at the
   // end of the sequence.
   std::reverse(first, first + numElems);
}

template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void heapSort(Container& seq, const Compare& cmp = {})
{
   heapSort(seq.data(), seq.size(), cmp);
}

} // namespace ds
