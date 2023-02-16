//
// Feb-2023, Michael Lindner
// MIT license
//
#pragma once
#include <functional>

namespace ds
{

///////////////////

// Insertion sort
// Cormen, pg 16
// Take each element and insert backwards into already sorted front of sequence.
// Efficient for small number of elements.
// Worst time: O(n^2) (for reverse sorted)
// Avg time: O(n^2)

// Iterator interface
template <typename Iter, typename Compare = std::less<typename Iter::value_type>>
void InsertionSort(Iter first, Iter last, Compare cmp = {}) noexcept
{
   if (first == last)
      return;

   for (Iter it = first + 1; it < last; ++it)
   {
      const Iter::value_type val = *it;

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

} // namespace ds
