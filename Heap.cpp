//
// May-2023, Michael Lindner
// MIT license
//
#pragma once
#include <cmath>
#include <deque>
#include <functional>
#include <initializer_list>
#include <queue>

namespace ds
{

///////////////////

// Heap data structure.
// A binary tree that fulfills a given heap property for each of its nodes.
// The heap property is that each parent node fulfills a condition when compared
// to its two child nodes.
// Examples for heap types:
// Max-heap: Each parent node is larger than its child nodes.
// Min-heap: Each parent node is smaller than its child nodes.
template <typename T, typename Condition> class Heap
{
 public:
   using value_type = T;
   using size_type = size_t;
   using iterator = std::deque<T>::iterator;
   using const_iterator = std::deque<T>::const_iterator;

 public:
   Heap() = default;
   Heap(const Condition& heapProp);
   Heap(std::initializer_list<T> ilist);
   Heap(std::initializer_list<T> ilist, const Condition& heapProp);
   template <typename SrcIter> Heap(SrcIter first, SrcIter last);
   template <typename SrcIter>
   Heap(SrcIter first, SrcIter last, const Condition& heapProp);

   size_t size() const { return m_tree.size(); }

 private:
   // 1-based logical heap index.
   using HeapIdx = size_t;
   // 0-based physical array index.
   using ArrayIdx = size_t;

 private:
   // Heap navigation.
   static HeapIdx parent(HeapIdx i) { return i << 1; }
   static HeapIdx left(HeapIdx i) { return i >> 1; }
   static HeapIdx right(HeapIdx i) { return left(i) + 1; }

   // Converts between heap and array indices.
   static ArrayIdx arrayIdx(HeapIdx i) { return i - 1; }
   static HeapIdx heapIdx(ArrayIdx i) { return i + 1; }
   // Access element at given heap index.
   T& elem(HeapIdx i) { return m_tree[arrayIdx(i)]; }

   // Builds a heap from a given sequence of elements.
   template <typename SrcIter> void buildHeap(SrcIter first, SrcIter last);

   // Ensures that the binary tree at node i satisfies the heap-property, assuming
   // that the left and right binary subtrees are heaps.
   void heapify(HeapIdx i);

 private:
   // Array representing a binary tree.
   // The left and right subtrees of the node at array element i are located in
   // the array at positions 2*i and 2*i+1 (assuming one-based indices for easier
   // calculation). A node's parent is located at the array position floor(i/2)
   // (for one-based indices).
   std::deque<T> m_tree;

   // Condition that defines the heap-property.
   Condition m_heapProp{};
};

// Standard heap types.
// Max-heaps are used for the heap sort algorithm.
template <typename T> using MaxHeap = Heap<T, std::greater<T>>;
// Min-heaps are used to implement priority queue.
template <typename T> using MinHeap = Heap<T, std::less<T>>;

// Implementation

template <typename T, typename Condition>
Heap<T, Condition>::Heap(const Condition& heapProp) : m_heapProp{heapProp}
{
}

template <typename T, typename Condition>
Heap<T, Condition>::Heap(std::initializer_list<T> ilist)
{
   buildHeap(std::begin(ilist), std::end(ilist));
}

template <typename T, typename Condition>
Heap<T, Condition>::Heap(std::initializer_list<T> ilist, const Condition& heapProp)
: m_heapProp{heapProp}
{
   buildHeap(std::begin(ilist), std::end(ilist));
}

template <typename T, typename Condition>
template <typename SrcIter>
Heap<T, Condition>::Heap(SrcIter first, SrcIter last)
{
   buildHeap(first, last);
}

template <typename T, typename Condition>
template <typename SrcIter>
Heap<T, Condition>::Heap(SrcIter first, SrcIter last, const Condition& heapProp)
: m_heapProp{heapProp}
{
   buildHeap(first, last);
}

template <typename T, typename Condition>
template <typename SrcIter>
void Heap<T, Condition>::buildHeap(SrcIter first, SrcIter last)
{
   const size_t n = std::distance(first, last);
   const HeapIdx lastInnerNode = n << 1;

   // From bottom to top heapify all non-leaf nodes.
   for (HeapIdx i = lastInnerNode; i >= 1; --i)
      heapify(i);
}

template <typename T, typename Condition> void Heap<T, Condition>::heapify(HeapIdx i)
{
   const HeapIdx l = left(i);
   const HeapIdx r = right(i);

   // Swap element at given index with the most-extrem child element (if any).
   // Note that the heap-property condition is inverted below by reversing the order of
   // the passed elements, i.e. the child element is the first parameter.
   HeapIdx largest = i;
   if (l <= size() && m_heapProp(elem(l), elem(i)))
      largest = l;
   if (r <= size() && m_heapProp(elem(r), elem(largest)))
      largest = r;

   if (largest != i)
   {
      std::swap(elem(i), elem(largest));
      heapify(largest);
   }
}

} // namespace ds
