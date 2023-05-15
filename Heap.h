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

// HeapView data structure.
// Does not store elements itself but uses passed storage.
// Conceptuall builds a binary tree that fulfills a given heap property for each of its
// nodes. The heap property is that each parent node fulfills a condition when compared to
// its two child nodes.
// HeapView types (see type aliases below):
//  Max-heap: Each parent node is larger than its child nodes.
//  Min-heap: Each parent node is smaller than its child nodes.
template <typename T, typename Condition> class HeapView
{
 public:
   using value_type = T;
   using size_type = size_t;
   using iterator = T*;

 public:
   HeapView() = default;
   HeapView(T* elems, size_t numElems, const Condition& heapProp = {});

   size_t size() const { return m_heapSize; }
   bool empty() const { return m_heapSize == 0; }

   // Returns the top element of the heap.
   // Internally this also sorts that element at the correct place in the passed
   // in storage. Calling pop() repeatedly until the heap is empty will sort the
   // underlying data.
   const T* pop();

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
   T& elem(HeapIdx i) { return m_root[arrayIdx(i)]; }

   // Builds a heap out of the given element array.
   void buildHeap();

   // Ensures that the binary tree at node i satisfies the heap-property, assuming
   // that the left and right binary subtrees are heaps.
   void heapify(HeapIdx i);

 private:
   // Treats the passed data as an array representing a binary tree.
   // The left and right subtrees of the node at array element i are located in
   // the array at positions 2*i and 2*i+1 (assuming one-based indices for easier
   // calculation). A node's parent is located at the array position floor(i/2)
   // (for one-based indices).
   T* m_root{nullptr};
   size_t m_heapSize{0};

   // Condition that defines the heap-property.
   Condition m_heapProp{};
};

// Standard heap types.
// Max-heaps are used for the heap sort algorithm.
template <typename T>
using MaxHeap = HeapView<T, std::greater<T>>;
// Min-heaps are used to implement priority queue.
template <typename T>
using MinHeap = HeapView<T, std::less<T>>;

// Implementation

template <typename T, typename Condition>
HeapView<T, Condition>::HeapView(T* vals, size_t numElems, const Condition& heapProp)
: m_root{vals}, m_heapSize{numElems}, m_heapProp{heapProp}
{
   buildHeap();
}

template <typename T, typename Condition>
const T* HeapView<T, Condition>::pop()
{
   if (empty())
      return nullptr;

   // Swap first (next one in sort order) with last element. This puts the element into
   // its correct order. Reduce the heap size by one to exclude the now sorted last element
   // from the data structure.
   std::swap(m_root[0], m_root[--m_heapSize]);

   // Restore the heap property for the element that got swapped into the root position.
   heapify(1);

   // Return sorted element.
   return &m_root[m_heapSize];
}

template <typename T, typename Condition> void HeapView<T, Condition>::buildHeap()
{
   if (empty())
      return;

   const HeapIdx lastInnerNode = size() >> 1;

   // From bottom to top heapify all non-leaf nodes.
   for (HeapIdx i = lastInnerNode; i >= 1; --i)
      heapify(i);
}

template <typename Iter, typename Condition>
void HeapView<Iter, Condition>::heapify(HeapIdx i)
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
