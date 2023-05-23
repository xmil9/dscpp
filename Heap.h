//
// May-2023, Michael Lindner
// MIT license
//
#pragma once
#include <cassert>
#include <stdexcept>

namespace ds
{

///////////////////

// HeapView data structure.
// Does not store elements itself but uses given storage.
// Conceptuall builds a binary tree that fulfills a given heap property for each of its
// nodes. The heap property is a given condition that each parent node fulfills when
// compared to its two child nodes.
// Predefined HeapView types (see type aliases below):
//  Max-heap: Each parent node is larger than its child nodes.
//  Min-heap: Each parent node is smaller than its child nodes.
template <typename T, typename Condition> class HeapView
{
 public:
   using value_type = T;
   using size_type = size_t;

   HeapView() = default;
   HeapView(T* vals, size_t numVals, const Condition& heapProp = {});
   template <typename Container>
   explicit HeapView(Container& arrayLike, const Condition& heapProp = {});

   size_t size() const noexcept { return m_heapSize; }
   bool empty() const noexcept { return m_heapSize == 0; }

   // Returns the top element of the heap.
   const T& top() const;

   // Returns and removes the top element of the heap.
   // Calling it on an empty heap raises an exception.
   // Internally this also sorts that element to the correct place in the given
   // storage. Calling pop() repeatedly until the heap is empty will sort the
   // underlying storage in reverse order.
   T pop();

   ////////////////

   // The following functions allow higher-level data structures to manipulate
   // a heap's internals. They are not meant to be called for simple heap usage
   // and can result in invalid heap states if used incorrectly.

   // 1-based logical heap index.
   using HeapIdx = size_t;
   // 0-based physical array index.
   using ArrayIdx = size_t;

   // Heap navigation.
   static HeapIdx parent(HeapIdx i) noexcept { return i >> 1; }
   static HeapIdx left(HeapIdx i) noexcept { return i << 1; }
   static HeapIdx right(HeapIdx i) noexcept { return left(i) + 1; }

   // Checks if two elements fulfill the heap condition.
   bool compare(HeapIdx a, HeapIdx b) const { return m_heapProp(elem(a), elem(b)); }

   // Swaps two elements in the underlying storage.
   void exchange(HeapIdx a, HeapIdx b) { std::swap(elem(a), elem(b)); }

   // Updates the heap data without changing the heap's layout.
   void set(T* vals);
   void set(T* vals, size_t numVals);
   // Updates the heap data and rebuilds the heap's layout.
   void reset(T* vals, size_t numVals);

 private:
   // Converts between heap and array indices.
   static ArrayIdx arrayIdx(HeapIdx i) { return i - 1; }
   static HeapIdx heapIdx(ArrayIdx i) { return i + 1; }

   // Access element at given heap index.
   const T& elem(HeapIdx i) const { return m_root[arrayIdx(i)]; }
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
template <typename T> using MaxHeap = HeapView<T, std::greater<T>>;
// Min-heaps are used to implement priority queue.
template <typename T> using MinHeap = HeapView<T, std::less<T>>;

// Implementation

template <typename T, typename Condition>
HeapView<T, Condition>::HeapView(T* vals, size_t numVals, const Condition& heapProp)
: m_root{vals}, m_heapSize{numVals}, m_heapProp{heapProp}
{
   assert((numVals > 0 && vals) || numVals == 0);
   buildHeap();
}

template <typename T, typename Condition>
template <typename Container>
HeapView<T, Condition>::HeapView(Container& arrayLike, const Condition& heapProp)
: HeapView{arrayLike.data(), arrayLike.size(), heapProp}
{
}

template <typename T, typename Condition> const T& HeapView<T, Condition>::top() const
{
   if (empty())
      throw std::runtime_error("Cannot access top of an empty heap.");
   return m_root[0];
}

template <typename T, typename Condition> T HeapView<T, Condition>::pop()
{
   if (empty())
      throw std::runtime_error("Cannot pop from an empty heap.");

   // Swap first (next one in sort order) with last element. This puts the element into
   // its correct order. Reduce the heap size by one to exclude the now sorted last
   // element from the data structure.
   std::swap(m_root[0], m_root[--m_heapSize]);

   // Restore the heap property for the element that got swapped into the root position.
   heapify(1);

   // Return sorted element.
   return m_root[m_heapSize];
}

template <typename T, typename Condition> void HeapView<T, Condition>::set(T* vals)
{
   assert((m_heapSize > 0 && vals) || m_heapSize == 0);
   m_root = vals;
}

template <typename T, typename Condition>
void HeapView<T, Condition>::set(T* vals, size_t numVals)
{
   assert((numVals > 0 && vals) || numVals == 0);
   m_root = vals;
   m_heapSize = numVals;
}

template <typename T, typename Condition>
void HeapView<T, Condition>::reset(T* vals, size_t numVals)
{
   set(vals, numVals);
   buildHeap();
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
   if (l <= size() && compare(l, i))
      largest = l;
   if (r <= size() && compare(r, largest))
      largest = r;

   if (largest != i)
   {
      exchange(i, largest);
      heapify(largest);
   }
}

} // namespace ds
