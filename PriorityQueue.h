//
// May-2023, Michael Lindner
// MIT license
//
#pragma once
#include "Heap.h"
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <vector>

namespace ds
{

///////////////////

// Priority queue that orders elements based on a given criterium.
template <typename T, typename Criterium = std::less<T>> class PriorityQueue
{
 public:
   PriorityQueue();
   PriorityQueue(const Criterium& crit);
   PriorityQueue(std::initializer_list<T> ilist);
   PriorityQueue(std::initializer_list<T> ilist, const Criterium& crit);
   template <typename Iter> PriorityQueue(Iter first, Iter last);
   template <typename Iter> PriorityQueue(Iter first, Iter last, const Criterium& crit);

   size_t size() const noexcept { return m_heap.size(); }
   bool empty() const noexcept { return m_heap.empty(); }
   const T& top() const { return m_heap.top(); }
   T pop() { return m_heap.pop(); }
   void insert(const T& val);

 private:
   std::vector<T> m_storage;
   HeapView<T, Criterium> m_heap;
};

template <typename T, typename Criterium>
PriorityQueue<T, Criterium>::PriorityQueue() : m_storage{}, m_heap{m_storage}
{
}

template <typename T, typename Criterium>
PriorityQueue<T, Criterium>::PriorityQueue(const Criterium& crit)
: m_storage{}, m_heap{m_storage, crit}
{
}

template <typename T, typename Criterium>
PriorityQueue<T, Criterium>::PriorityQueue(std::initializer_list<T> ilist)
: m_storage{ilist}, m_heap{m_storage}
{
}

template <typename T, typename Criterium>
PriorityQueue<T, Criterium>::PriorityQueue(std::initializer_list<T> ilist,
                                         const Criterium& crit)
: m_storage{ilist}, m_heap{m_storage, crit}
{
}

template <typename T, typename Criterium>
template <typename Iter>
PriorityQueue<T, Criterium>::PriorityQueue(Iter first, Iter last)
: m_storage{first, last}, m_heap{m_storage}
{
}

template <typename T, typename Criterium>
template <typename Iter>
PriorityQueue<T, Criterium>::PriorityQueue(Iter first, Iter last, const Criterium& crit)
: m_storage{first, last}, m_heap{m_storage, crit}
{
}

template <typename T, typename Criterium>
void PriorityQueue<T, Criterium>::insert(const T& val)
{
   // Since the heap can shrink below the capacity of the storage when elements are
   // removed, we might have unused space. If not resize the storage.
   if (m_storage.size() == m_heap.size())
      m_storage.resize(m_storage.size() + 1);

   // Insert at end of heap.
   m_storage[m_heap.size()] = val;
   m_heap.set(m_storage.data(), m_heap.size() + 1);

   // Move new element up in heap until the heap property is fulfilled.
   using HeapIdx = HeapView<T, Criterium>::HeapIdx;
   HeapIdx i = m_heap.size();
   HeapIdx parentIdx = HeapView<T, Criterium>::parent(i);
   while (i > 1 && m_heap.compare(i, parentIdx))
   {
      m_heap.exchange(i, parentIdx);
      i = parentIdx;
      parentIdx = HeapView<T, Criterium>::parent(i);
   }
}

} // namespace ds
