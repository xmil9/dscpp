#pragma once

#ifdef _MSC_VER
#define VS_COMPILER
#endif

#include "TypeTraitsEx.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iterator>
#ifdef VS_COMPILER
#include <malloc.h>
#endif
#include <memory>
#include <stdexcept>
#include <type_traits>


namespace ds
{

// Forward declarations for iterator class templates.
template <typename T, std::size_t N> class SboVectorIterator;
template <typename T, std::size_t N> class SboVectorConstIterator;


///////////////////

// Vector type that use the small buffer optimization to avoid heap usage for
// sizes below a given number of elements.
//
// Just like for std::vector the template parameter type must be
// - copy-constructible
// - copy-assignable
// - erasable
// Where erasable is defined as p->~T() (a call to the type's dtor through a pointer
// to T) being valid. This allows class types with accessible dtors and scalar types.
// It excludes array types, function types, reference types, and void.
//
// The accepted iterator type for operations that use iterators into other containers
// is a Forward iterator. std::vector accepts Input iterators that permit only a single
// pass over the elements. The accepted iterator type for SboVectors is more strict
// to allow multiple passes.
//
// When transfering elements from one SboVector to another there are multiple possible
// strategies for where to place the elements and how to transfer their data:
// 1. Steal the heap memory of the source SboVector if the source is not used anymore.
// 2. Use the internal buffer.
// 3. Reuse the existing heap allocation if it is large enough.
// 4. Allocate new memory on the heap.
// The strategies are listed in order of preference. Not all strategies can be used
// in any situation, e.g. 1. can only be used for move operations.

template <typename T, std::size_t N> class SboVector
{
   // Requirements for T (std::vector has the same requirements for its T).
   static_assert(std::is_copy_constructible_v<T>,
                 "Element type must be copy-constructible.");
   static_assert(std::is_copy_assignable_v<T>, "Element type must be copy-assignable.");
   static_assert(std::is_copy_assignable_v<T>, "Element type must be copy-assignable.");
   static_assert(!std::is_array_v<T>,
                 "Element type must be erasable, i.e. calls to p->~T() must be valid. "
                 "Array types are not erasable.");
   static_assert(!std::is_function_v<T>,
                 "Element type must be erasable, i.e. calls to p->~T() must be valid. "
                 "Function types are not erasable.");
   static_assert(!std::is_reference_v<T>,
                 "Element type must be erasable, i.e. calls to p->~T() must be valid. "
                 "Reference types are not erasable.");
   static_assert(!std::is_same_v<T, void>,
                 "Element type must be erasable, i.e. calls to p->~T() must be valid. "
                 "Type void is not erasable.");
   // Requirements for N.
   static_assert(N > 0, "Zero-sized buffer is not supported. Use std::vector.");

 public:
   using value_type = T;
   using reference = T&;
   using const_reference = const T&;
   using pointer = T*;
   using const_pointer = const T*;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using iterator = SboVectorIterator<T, N>;
   using const_iterator = SboVectorConstIterator<T, N>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   static constexpr std::size_t BufferCapacity = N;

 public:
   SboVector() = default;
   explicit SboVector(std::size_t count, const T& value);
   template <typename FwdIter, typename = std::enable_if_t<IsIterator_v<FwdIter>, void>>
   SboVector(FwdIter first, FwdIter last);
   SboVector(std::initializer_list<T> ilist);
   SboVector(const SboVector& other);
   SboVector(SboVector&& other);
   ~SboVector();

   SboVector& operator=(const SboVector& other);
   SboVector& operator=(SboVector&& other);
   SboVector& operator=(std::initializer_list<T> ilist);

   void assign(size_type count, const T& value);
   template <typename FwdIter> void assign(FwdIter first, FwdIter last);
   void assign(std::initializer_list<T> ilist);

   T& at(std::size_t pos);
   const T& at(std::size_t pos) const;
   T& operator[](std::size_t pos);
   const T& operator[](std::size_t pos) const;
   T& front();
   const T& front() const;
   T& back();
   const T& back() const;
   T* data() noexcept;
   const T* data() const noexcept;

   iterator begin() noexcept;
   iterator end() noexcept;
   const_iterator begin() const noexcept;
   const_iterator end() const noexcept;
   const_iterator cbegin() const noexcept;
   const_iterator cend() const noexcept;
   reverse_iterator rbegin() noexcept;
   reverse_iterator rend() noexcept;
   const_reverse_iterator rbegin() const noexcept;
   const_reverse_iterator rend() const noexcept;
   const_reverse_iterator crbegin() const noexcept;
   const_reverse_iterator crend() const noexcept;

   bool empty() const noexcept;
   std::size_t size() const noexcept;
   constexpr std::size_t max_size() const noexcept;
   std::size_t capacity() const noexcept;
   void reserve(std::size_t cap);
   void shrink_to_fit();

   void clear() noexcept;
   iterator erase(const_iterator pos);
   iterator erase(const_iterator first, const_iterator last);
   iterator insert(const_iterator pos, const T& value);
   iterator insert(const_iterator pos, T&& value);
   iterator insert(const_iterator pos, size_type count, const T& value);
   template <typename FwdIter>
   iterator insert(const_iterator pos, FwdIter first, FwdIter last);
   iterator insert(const_iterator pos, std::initializer_list<T> ilist);
   void push_back(const T& value);
   void push_back(T&& value);
   void pop_back();
   template <typename... Args> iterator emplace(const_iterator pos, Args&&... args);
   template <typename... Args> reference emplace_back(Args&&... args);
   void resize(size_type count);
   void resize(size_type count, const value_type& value);
   void swap(SboVector& other);

   bool inBuffer() const noexcept;
   bool onHeap() const noexcept;

 private:
   constexpr T* buffer();
   constexpr const T* buffer() const;

   // Workflows - combinations of memory and data operations.
   void constructFrom(std::size_t n, const T& value);
   template <typename FwdIter> void constructFrom(FwdIter first, std::size_t n);
   void prepareDataForPopulating(std::size_t n);
   void assignFrom(std::size_t n, const T& value);
   template <typename FwdIter> void assignFrom(FwdIter first, std::size_t n);
   void adjustDataForAssignment(std::size_t n);
   void moveFrom(SboVector&& other);
   iterator insertOf(const_iterator pos, T&& value);
   iterator insertOf(const_iterator pos, std::size_t n, const T& value);
   template <typename FwdIter, typename = std::enable_if_t<IsIterator_v<FwdIter>, void>>
   iterator insertOf(const_iterator pos, FwdIter first, std::size_t n);
   void adjustDataForInsertion(const_iterator pos, std::size_t n);
   void adjustDataForResize(std::size_t n);
   void clean();

   // Data operations.
   void populateWith(std::size_t n, const T& value);
   template <typename FwdIter> void populateWith(FwdIter first, std::size_t n);
   void destroy();

   // Memory operations.
   static constexpr bool fitsIntoBuffer(std::size_t size);
   void allocate(std::size_t cap);
   void deallocate();
   void reallocate(std::size_t newCap);
   void reallocateMore(std::size_t newCap);
   void reallocateLess(std::size_t newCap);
   std::size_t recalcCapacity(std::size_t minCap) const;

   static T* allocateMem(std::size_t cap);
   static void deallocateMem(T* mem, std::size_t cap);

 private:
   // Internal buffer.
   std::aligned_storage_t<sizeof(T), alignof(T)> m_buffer[N];
   // Beginning of data (heap or buffer).
   T* m_data = buffer();
   // Number of occupied elements.
   std::size_t m_size = 0;
   // Number of allocated elements.
   std::size_t m_capacity = BufferCapacity;

   // Memory instrumentation.
#ifdef SBOVEC_MEM_INSTR
 public:
   static int64_t allocatedCapacity() { return m_allocatedCap; }
   static void resetAllocatedCapacity() { m_allocatedCap = 0; }

 private:
   inline static int64_t m_allocatedCap = 0;
#endif // SBOVEC_MEM_INSTR
};


///////////////////

namespace internals
{

template <typename T> void copyAndDestroyOverlapped(T* first, std::size_t count, T* dest)
{
   T* last = first + count;
   for (; first < last; ++first, ++dest)
   {
      std::uninitialized_copy_n(first, 1, dest);
      std::destroy_at(first);
   }
}


template <typename T>
void copyAndDestroyOverlappedBackward(T* first, std::size_t count, T* dest)
{
   T* rFirst = first + count - 1;
   T* rLast = first - 1;
   T* rFirstDest = dest + count - 1;
   for (; rFirst > rLast; --rFirst, --rFirstDest)
   {
      std::uninitialized_copy_n(rFirst, 1, rFirstDest);
      std::destroy_at(rFirst);
   }
}


template <typename T> void moveOverlappedBackwards(T* first, std::size_t count, T* dest)
{
   T* rFirst = first + count - 1;
   T* rLast = first - 1;
   T* rFirstDest = dest + count - 1;
   for (; rFirst > rLast; --rFirst, --rFirstDest)
      std::uninitialized_move_n(rFirst, 1, rFirstDest);
}


// Available in C++20.
template <typename T, typename... Args> constexpr T* construct_at(T* p, Args&&... args)
{
   return ::new (p) T(std::forward<Args>(args)...);
}


///////////////////

// Depending on the element type moves or copies a of elements.
// Does not make accomodations for overlapping ranges.
template <typename Iter> void relocate_n(Iter first, std::size_t n, Iter dest)
{
   using value_type = typename std::iterator_traits<Iter>::value_type;

   if constexpr (std::is_move_constructible_v<value_type>)
   {
      std::uninitialized_move_n(first, n, dest);
   }
   else
   {
      std::uninitialized_copy_n(first, n, dest);
      std::destroy_n(first, n);
   }
}


// Depending on the element type moves or copies a possibly overlapping range of elements
// to the left (from higher to lower memory addresses).
template <typename Iter> void relocateLeftOverlapped(Iter first, Iter last, Iter dest)
{
   using value_type = typename std::iterator_traits<Iter>::value_type;

   if constexpr (std::is_move_constructible_v<value_type>)
      std::uninitialized_move(first, last, dest);
   else
      copyAndDestroyOverlapped(first, last - first, dest);
}


// Depending on the element type moves or copies a possibly overlapping range of elements
// to the right (from lower to higher memory addresses).
template <typename Iter> void relocateRightOverlapped(Iter first, Iter last, Iter dest)
{
   using value_type = typename std::iterator_traits<Iter>::value_type;

   if constexpr (std::is_move_constructible_v<value_type>)
      moveOverlappedBackwards(first, last - first, dest);
   else
      copyAndDestroyOverlappedBackward(first, last - first, dest);
}

} // namespace internals


///////////////////

template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::size_t count, const T& value)
{
   constructFrom(count, value);
}


template <typename T, std::size_t N>
template <typename FwdIter, typename>
SboVector<T, N>::SboVector(FwdIter first, FwdIter last)
{
   constructFrom(first, std::distance(first, last));
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::initializer_list<T> ilist)
{
   constructFrom(ilist.begin(), ilist.size());
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(const SboVector& other)
{
   constructFrom(other.data(), other.size());
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(SboVector&& other)
{
   moveFrom(std::move(other));
}


template <typename T, std::size_t N> SboVector<T, N>::~SboVector()
{
   clean();
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(const SboVector& other)
{
   assignFrom(other.begin(), other.size());
   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(SboVector&& other)
{
   clean();
   moveFrom(std::move(other));
   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(std::initializer_list<T> ilist)
{
   assignFrom(ilist.begin(), ilist.size());
   return *this;
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(size_type count, const T& value)
{
   assignFrom(count, value);
}


template <typename T, std::size_t N>
template <typename FwdIter>
void SboVector<T, N>::assign(FwdIter first, FwdIter last)
{
   assignFrom(first, std::distance(first, last));
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(std::initializer_list<T> ilist)
{
   assignFrom(ilist.begin(), ilist.size());
}


template <typename T, std::size_t N> T& SboVector<T, N>::at(std::size_t pos)
{
   if (pos >= m_size)
      throw std::out_of_range("SboVector - Accessing invalid element.");
   return operator[](pos);
}


template <typename T, std::size_t N> const T& SboVector<T, N>::at(std::size_t pos) const
{
   if (pos >= m_size)
      throw std::out_of_range("SboVector - Accessing invalid element.");
   return operator[](pos);
}


template <typename T, std::size_t N> T& SboVector<T, N>::operator[](std::size_t pos)
{
   return m_data[pos];
}


template <typename T, std::size_t N>
const T& SboVector<T, N>::operator[](std::size_t pos) const
{
   return m_data[pos];
}


template <typename T, std::size_t N> T& SboVector<T, N>::front()
{
   return m_data[0];
}


template <typename T, std::size_t N> const T& SboVector<T, N>::front() const
{
   return m_data[0];
}


template <typename T, std::size_t N> T& SboVector<T, N>::back()
{
   return m_data[m_size - 1];
}


template <typename T, std::size_t N> const T& SboVector<T, N>::back() const
{
   return m_data[m_size - 1];
}


template <typename T, std::size_t N> T* SboVector<T, N>::data() noexcept
{
   return m_data;
}


template <typename T, std::size_t N> const T* SboVector<T, N>::data() const noexcept
{
   return m_data;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::begin() noexcept
{
   return iterator(m_data);
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::end() noexcept
{
   return iterator(m_data + size());
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_iterator SboVector<T, N>::begin() const noexcept
{
   return const_iterator(m_data);
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_iterator SboVector<T, N>::end() const noexcept
{
   return const_iterator(m_data + size());
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_iterator SboVector<T, N>::cbegin() const noexcept
{
   return begin();
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_iterator SboVector<T, N>::cend() const noexcept
{
   return end();
}


template <typename T, std::size_t N>
typename SboVector<T, N>::reverse_iterator SboVector<T, N>::rbegin() noexcept
{
   return reverse_iterator(end());
}


template <typename T, std::size_t N>
typename SboVector<T, N>::reverse_iterator SboVector<T, N>::rend() noexcept
{
   return reverse_iterator(begin());
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_reverse_iterator SboVector<T, N>::rbegin() const noexcept
{
   return crbegin();
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_reverse_iterator SboVector<T, N>::rend() const noexcept
{
   return crend();
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_reverse_iterator SboVector<T, N>::crbegin() const noexcept
{
   return const_reverse_iterator(cend());
}


template <typename T, std::size_t N>
typename SboVector<T, N>::const_reverse_iterator SboVector<T, N>::crend() const noexcept
{
   return const_reverse_iterator(cbegin());
}


template <typename T, std::size_t N> bool SboVector<T, N>::empty() const noexcept
{
   return (m_size == 0);
}


template <typename T, std::size_t N> std::size_t SboVector<T, N>::size() const noexcept
{
   return m_size;
}

template <typename T, std::size_t N>
constexpr std::size_t SboVector<T, N>::max_size() const noexcept
{
   return std::numeric_limits<difference_type>::max();
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::capacity() const noexcept
{
   return m_capacity;
}


template <typename T, std::size_t N> void SboVector<T, N>::reserve(std::size_t cap)
{
   // Vector spec calls for exception when request exceeds the max size.
   if (cap > max_size())
      throw std::length_error("SboVector - Exceeding max size.");
   // Does nothing when requested capacity is less than the current capacity.
   if (cap <= capacity())
      return;

   reallocate(cap);
}


template <typename T, std::size_t N> void SboVector<T, N>::shrink_to_fit()
{
   if (onHeap() && size() < capacity())
      reallocate(size());
}


template <typename T, std::size_t N> void SboVector<T, N>::clear() noexcept
{
   destroy();
   m_size = 0;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::erase(const_iterator pos)
{
   return erase(pos, pos + 1);
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::erase(const_iterator first,
                                                          const_iterator last)
{
   if (first >= cend())
      return end();
   if (last > cend())
      last = cend();

   const auto offset = first - cbegin();
   const auto count = last - first;
   if (count > 0)
   {
      std::destroy_n(first.m_elem, count);

      const bool relocateTail = last < cend();
      if (relocateTail)
         internals::relocateLeftOverlapped(last.m_elem, data() + size(), first.m_elem);

      m_size -= count;
   }

   return begin() + offset;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::insert(const_iterator pos,
                                                           const T& value)
{
   return insertOf(pos, 1, value);
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::insert(const_iterator pos, T&& value)
{
   return insertOf(pos, std::move(value));
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator
SboVector<T, N>::insert(const_iterator pos, size_type count, const T& value)
{
   return insertOf(pos, count, value);
}


template <typename T, std::size_t N>
template <typename FwdIter>
typename SboVector<T, N>::iterator SboVector<T, N>::insert(const_iterator pos,
                                                           FwdIter first, FwdIter last)
{
   return insertOf(pos, first, std::distance(first, last));
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::insert(const_iterator pos,
                                                           std::initializer_list<T> ilist)
{
   return insertOf(pos, ilist.begin(), ilist.size());
}


template <typename T, std::size_t N> void SboVector<T, N>::push_back(const T& value)
{
   insert(begin() + size(), value);
}


template <typename T, std::size_t N> void SboVector<T, N>::push_back(T&& value)
{
   insert(begin() + size(), std::move(value));
}


template <typename T, std::size_t N> void SboVector<T, N>::pop_back()
{
   // We don't need to worry about an empty vector case because calling pop_back on an
   // empty vector is UB according to the standard.
   --m_size;
   std::destroy_at(data() + m_size);
}


template <typename T, std::size_t N>
template <typename... Args>
typename SboVector<T, N>::iterator SboVector<T, N>::emplace(const_iterator pos,
                                                            Args&&... args)
{
   const std::size_t diff = pos - cbegin();
   adjustDataForInsertion(pos, 1);
   internals::construct_at(data() + diff, std::forward<Args>(args)...);
   return begin() + diff;
}


template <typename T, std::size_t N>
template <typename... Args>
typename SboVector<T, N>::reference SboVector<T, N>::emplace_back(Args&&... args)
{
   emplace(begin() + size(), std::forward<Args>(args)...);
   return *(begin() + size() - 1);
}


template <typename T, std::size_t N> void SboVector<T, N>::resize(size_type count)
{
   adjustDataForResize(count);
   if (count > m_size)
      std::uninitialized_default_construct_n(data() + size(), count - size());
   m_size = count;
}


template <typename T, std::size_t N>
void SboVector<T, N>::resize(size_type count, const value_type& value)
{
   adjustDataForResize(count);
   if (count > m_size)
      std::uninitialized_fill_n(data() + size(), count - size(), value);
   m_size = count;
}


template <typename T, std::size_t N> void SboVector<T, N>::swap(SboVector& other)
{
   // Cases:
   // - Both on heap: Swap heap pointers.
   // - Both in buffer:
   //   Swap elements up to size of smaller vector, relocate the rest from larger to
   //   smaller vector.
   // - One on heap, one in buffer:
   //   Relocate buffered elements to buffer of heap vector, reassign heap pointer.

   const bool thisOnHeap = onHeap();
   const bool otherOnHeap = other.onHeap();

   if (thisOnHeap && otherOnHeap)
   {
      std::swap(m_data, other.m_data);
   }
   else if (!thisOnHeap && !otherOnHeap)
   {
      SboVector& larger = size() >= other.size() ? *this : other;
      SboVector& smaller = size() >= other.size() ? other : *this;
      const std::size_t commonSize = smaller.size();

      for (std::size_t i = 0; i < commonSize; ++i)
      {
         using std::swap; // Enable ADL.
         swap(smaller.m_data[i], larger.m_data[i]);
      }

      T* src = larger.data() + commonSize;
      T* dest = smaller.data() + commonSize;
      const std::size_t numRelocate = larger.size() - commonSize;
      internals::relocate_n(src, numRelocate, dest);
      m_data = buffer();
      other.m_data = other.buffer();
   }
   else // One on heap, one in buffer.
   {
      SboVector& heap = onHeap() ? *this : other;
      SboVector& buffer = onHeap() ? other : *this;

      internals::relocate_n(buffer.buffer(), buffer.size(), heap.buffer());
      buffer.m_data = heap.m_data;
      heap.m_data = heap.buffer();
   }

   std::swap(m_size, other.m_size);
   std::swap(m_capacity, other.m_capacity);
}


template <typename T, std::size_t N> bool SboVector<T, N>::inBuffer() const noexcept
{
   return (m_data == buffer());
}


template <typename T, std::size_t N> bool SboVector<T, N>::onHeap() const noexcept
{
   return !inBuffer();
}


template <typename T, std::size_t N> constexpr T* SboVector<T, N>::buffer()
{
   return reinterpret_cast<T*>(m_buffer);
}


template <typename T, std::size_t N> constexpr const T* SboVector<T, N>::buffer() const
{
   return reinterpret_cast<const T*>(m_buffer);
}


template <typename T, std::size_t N>
void SboVector<T, N>::constructFrom(std::size_t n, const T& value)
{
   prepareDataForPopulating(n);
   populateWith(n, value);
}


template <typename T, std::size_t N>
template <typename FwdIter>
void SboVector<T, N>::constructFrom(FwdIter first, std::size_t n)
{
   prepareDataForPopulating(n);
   populateWith(first, n);
}


template <typename T, std::size_t N>
void SboVector<T, N>::prepareDataForPopulating(std::size_t n)
{
   // Cases:
   // - Use the buffer.
   // - Make a new heap allocation.

   if (!fitsIntoBuffer(n))
   {
      allocate(n);
      m_capacity = n;
   }
}


template <typename T, std::size_t N>
void SboVector<T, N>::assignFrom(std::size_t n, const T& value)
{
   adjustDataForAssignment(n);
   populateWith(n, value);
}


template <typename T, std::size_t N>
template <typename FwdIter>
void SboVector<T, N>::assignFrom(FwdIter first, std::size_t n)
{
   adjustDataForAssignment(n);
   populateWith(first, n);
}


template <typename T, std::size_t N>
void SboVector<T, N>::adjustDataForAssignment(std::size_t n)
{
   // Cases:
   // - Use the buffer.
   // - Reuse an existing heap allocation.
   // - Make a new heap allocation.

   const bool fitsBuffer = fitsIntoBuffer(n);
   const bool canReuseHeap = onHeap() && m_capacity >= n;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(n);

   // Clean up existing data.
   destroy();
   if (fitsBuffer || allocHeap)
      deallocate();

   // Set up new data.
   if (fitsBuffer)
   {
      m_capacity = BufferCapacity;
   }
   else if (canReuseHeap)
   {
      // Capacity stays the same.
   }
   else
   {
      assert(allocHeap && newData);
      m_data = newData;
      m_capacity = n;
   }
}


template <typename T, std::size_t N> void SboVector<T, N>::moveFrom(SboVector&& other)
{
   // Cases:
   // - Steal heap allocation.
   // - Use buffer.

   const auto srcSize = other.size();
   const bool canSteal = other.onHeap();

   if (canSteal)
   {
      m_data = other.m_data;
      m_capacity = other.m_capacity;
      // Reset other's data to prevent deallocation of the stolen memory below.
      other.m_data = other.buffer();
   }
   else
   {
      // Elements must fit into the buffer.
      assert(fitsIntoBuffer(srcSize));
      std::uninitialized_move_n(other.m_data, srcSize, m_data);
      m_capacity = BufferCapacity;
   }

   m_size = srcSize;
   // Reset other size to prevent destruction of the stolen or moved objects.
   other.m_size = 0;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::insertOf(const_iterator pos,
                                                             T&& value)
{
   const std::size_t diff = pos - cbegin();
   adjustDataForInsertion(pos, 1);
   std::uninitialized_move_n(&value, 1, data() + diff);
   return begin() + diff;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator
SboVector<T, N>::insertOf(const_iterator pos, std::size_t n, const T& value)
{
   const std::size_t diff = pos - cbegin();
   if (n > 0)
   {
      adjustDataForInsertion(pos, n);
      for (int i = 0; i < n; ++i)
         std::uninitialized_copy_n(&value, 1, data() + diff + i);
   }
   return begin() + diff;
}


template <typename T, std::size_t N>
template <typename FwdIter, typename>
typename SboVector<T, N>::iterator SboVector<T, N>::insertOf(const_iterator pos,
                                                             FwdIter first, std::size_t n)
{
   const std::size_t diff = pos - cbegin();
   if (n > 0)
   {
      adjustDataForInsertion(pos, n);
      std::uninitialized_copy_n(first, n, data() + diff);
   }
   return begin() + diff;
}


template <typename T, std::size_t N>
void SboVector<T, N>::adjustDataForInsertion(const_iterator pos, std::size_t n)
{
   // Cases:
   // - In buffer and enough capacity to stay in buffer.
   // - In buffer and not enough capacity. Allocate heap.
   // - On heap and enough capacity.
   // - On heap and not enough capacity. Need to reallocate.

   const std::size_t posOffset = pos - cbegin();
   const std::size_t newSize = size() + n;
   const bool fitsBuffer = fitsIntoBuffer(newSize);
   const bool canReuseHeap = onHeap() && m_capacity >= newSize;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;
   const std::size_t newCap = allocHeap ? recalcCapacity(newSize) : capacity();

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* dest = data();
   if (allocHeap)
      dest = allocateMem(newCap);

   const std::size_t tailSize = cend() - pos;
   const std::size_t frontSize = posOffset;

   if (tailSize > 0)
   {
      T* tailSrc = data() + posOffset;
      T* tailDest = dest + posOffset + n;
      internals::relocateRightOverlapped(tailSrc, tailSrc + tailSize, tailDest);
   }

   const bool relocateFront = allocHeap;
   if (frontSize > 0 && relocateFront)
   {
      // Relocating the front only happens for reallocations, so we don't need to
      // worry about overlaps.
      internals::relocate_n(data(), frontSize, dest);
   }

   m_size = newSize;
   if (allocHeap)
   {
      deallocate();
      m_data = dest;
      m_capacity = newCap;
   }
}


template <typename T, std::size_t N>
void SboVector<T, N>::adjustDataForResize(std::size_t n)
{
   // Cases:
   // - New size == old size: Nothing to do.
   // - New size > old size:
   //    - In buffer and enough capacity to stay in buffer.
   //    - In buffer and not enough capacity. Allocate heap.
   //    - On heap and enough capacity.
   //    - On heap and not enough capacity. Need to reallocate.
   // - New size < old size: Destroy elements. Keep storage.

   if (n > m_size)
   {
      const bool fitsBuffer = fitsIntoBuffer(n);
      const bool canReuseHeap = onHeap() && m_capacity >= n;
      const bool allocHeap = !fitsBuffer && !canReuseHeap;
      if (allocHeap)
         reallocate(n);
   }
   else
   {
      std::destroy_n(data() + n, size() - n);
   }
}


template <typename T, std::size_t N> void SboVector<T, N>::clean()
{
   destroy();
   deallocate();
}


template <typename T, std::size_t N>
void SboVector<T, N>::populateWith(std::size_t n, const T& value)
{
   std::uninitialized_fill_n(m_data, n, value);
   m_size = n;
}


template <typename T, std::size_t N>
template <typename FwdIter>
void SboVector<T, N>::populateWith(FwdIter first, std::size_t n)
{
   std::uninitialized_copy_n(first, n, m_data);
   m_size = n;
}


template <typename T, std::size_t N> void SboVector<T, N>::destroy()
{
   std::destroy_n(m_data, size());
}


template <typename T, std::size_t N>
constexpr bool SboVector<T, N>::fitsIntoBuffer(std::size_t size)
{
   return size <= BufferCapacity;
}


template <typename T, std::size_t N> void SboVector<T, N>::allocate(std::size_t cap)
{
   m_data = allocateMem(cap);
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocate()
{
   if (onHeap())
   {
      deallocateMem(m_data, m_capacity);
      m_data = buffer();
   }
}


template <typename T, std::size_t N> void SboVector<T, N>::reallocate(std::size_t newCap)
{
   // Cannot reallocate to less than what the current elements occupy.
   assert(newCap >= size());
   if (newCap < size())
      return;

   if (newCap > capacity())
      reallocateMore(newCap);
   else if (newCap < capacity())
      reallocateLess(newCap);
}


template <typename T, std::size_t N>
void SboVector<T, N>::reallocateMore(std::size_t newCap)
{
   assert(newCap > capacity());

   // This case requires no work and should be caught earlier.
   assert(newCap > BufferCapacity);
   if (newCap <= BufferCapacity)
      return;

   T* newData = allocateMem(newCap);
   internals::relocate_n(data(), size(), newData);

   // Will only dealloc if current data is on heap.
   deallocate();

   m_data = newData;
   m_capacity = newCap;
}


template <typename T, std::size_t N>
void SboVector<T, N>::reallocateLess(std::size_t newCap)
{
   assert(newCap < capacity());

   // Possible situations are:
   // 1. New capacity is larger than buffer capacity. We need to allocate heap
   //    memory and relocate the data into it.
   // 2. New capacity is less than buffer capacity but current data is on heap.
   //    We need to relocate the data into the buffer and deallocate the heap.
   // 3. New capacity is less than buffer capacity and current data is in the
   //    buffer. Nothing to do.

   if (newCap <= BufferCapacity && inBuffer())
      return;

   // Only cases 1. and 2. left.

   const bool allocHeap = newCap > BufferCapacity;
   T* newData = allocHeap ? allocateMem(newCap) : buffer();

   internals::relocate_n(data(), size(), newData);

   // Will only dealloc if current data is on heap.
   deallocate();

   m_data = newData;
   m_capacity = std::max(newCap, BufferCapacity);
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::recalcCapacity(std::size_t minCap) const
{
   const std::size_t maxCap = max_size();
   if (m_capacity > maxCap / 2)
      return maxCap;
   return std::max(2 * m_capacity, minCap);
}


template <typename T, std::size_t N> T* SboVector<T, N>::allocateMem(std::size_t cap)
{
#ifdef VS_COMPILER
   // Visual Studio does not support std::aligned_alloc.
   T* mem = reinterpret_cast<T*>(_aligned_malloc(cap * sizeof(T), alignof(T)));
#else
   T* mem = reinterpret_cast<T*>(std::aligned_alloc(alignof(T), cap * sizeof(T)));
#endif
   if (!mem)
      throw std::runtime_error("SboVector - Failed to allocate memory.");

#ifdef SBOVEC_MEM_INSTR
   m_allocatedCap += cap;
#endif // SBOVEC_MEM_INSTR

   return mem;
}


template <typename T, std::size_t N>
void SboVector<T, N>::deallocateMem(T* mem, std::size_t cap)
{
#ifdef VS_COMPILER
   _aligned_free(mem);
#else
   std::free(mem);
#endif

#ifdef SBOVEC_MEM_INSTR
   m_allocatedCap -= cap;
#else
   (void)cap;
#endif // SBOVEC_MEM_INSTR
}


///////////////////

template <typename T, std::size_t N>
bool operator==(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}


template <typename T, std::size_t N>
bool operator!=(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return !(a == b);
}


template <typename T, std::size_t N>
bool operator<(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}


template <typename T, std::size_t N>
bool operator>(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return b < a;
}


template <typename T, std::size_t N>
bool operator<=(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return !(a > b);
}


template <typename T, std::size_t N>
bool operator>=(const SboVector<T, N>& a, const SboVector<T, N>& b)
{
   return !(a < b);
}


template <typename T, std::size_t N> void swap(SboVector<T, N>& a, SboVector<T, N>& b)
{
   a.swap(b);
}


///////////////////

// Const iterator for SboVector.
template <typename T, std::size_t N> class SboVectorConstIterator
{
   template <typename T, std::size_t N> friend class SboVector;

 public:
   using SV = SboVector<T, N>;
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename SV::value_type;
   using difference_type = typename SV::difference_type;
   using pointer = typename SV::const_pointer;
   using reference = typename SV::const_reference;

 protected:
   // Only the vector and non-const iterator classes need access to ctor.
   explicit SboVectorConstIterator(T* elem) noexcept;

 public:
   SboVectorConstIterator() = default;
   ~SboVectorConstIterator() = default;
   SboVectorConstIterator(const SboVectorConstIterator&) = default;

   SboVectorConstIterator& operator=(const SboVectorConstIterator&) = default;

   const value_type& operator*() const;
   const value_type* operator->() const;
   const value_type& operator[](difference_type offset) const;
   SboVectorConstIterator& operator++();
   SboVectorConstIterator operator++(int);
   SboVectorConstIterator& operator--();
   SboVectorConstIterator operator--(int);
   SboVectorConstIterator& operator+=(difference_type offset);
   SboVectorConstIterator& operator-=(difference_type offset);

   friend void swap(SboVectorConstIterator<T, N>& a, SboVectorConstIterator<T, N>& b)
   {
      std::swap(a.m_elem, b.m_elem);
   }

   friend bool operator==(const SboVectorConstIterator<T, N>& a,
                          const SboVectorConstIterator<T, N>& b)
   {
      return a.m_elem == b.m_elem;
   }

   friend bool operator!=(const SboVectorConstIterator<T, N>& a,
                          const SboVectorConstIterator<T, N>& b)
   {
      return !(a == b);
   }

   friend SboVectorConstIterator<T, N> operator+(const SboVectorConstIterator<T, N>& it,
                                                 difference_type offset)
   {
      auto res = it;
      return (res += offset);
   }

   friend SboVectorConstIterator<T, N> operator+(difference_type offset,
                                                 const SboVectorConstIterator<T, N>& it)
   {
      return (it + offset);
   }

   friend SboVectorConstIterator<T, N> operator-(const SboVectorConstIterator<T, N>& it,
                                                 difference_type offset)
   {
      auto res = it;
      return (res -= offset);
   }

   friend difference_type operator-(const SboVectorConstIterator<T, N>& a,
                                    const SboVectorConstIterator<T, N>& b)
   {
      return a.m_elem - b.m_elem;
   }

   friend bool operator<(const SboVectorConstIterator<T, N>& a,
                         const SboVectorConstIterator<T, N>& b)
   {
      return a.m_elem < b.m_elem;
   }

   friend bool operator>(const SboVectorConstIterator<T, N>& a,
                         const SboVectorConstIterator<T, N>& b)
   {
      return a.m_elem > b.m_elem;
   }

   friend bool operator<=(const SboVectorConstIterator<T, N>& a,
                          const SboVectorConstIterator<T, N>& b)
   {
      return !(a > b);
   }

   friend bool operator>=(const SboVectorConstIterator<T, N>& a,
                          const SboVectorConstIterator<T, N>& b)
   {
      return !(a < b);
   }

 protected:
   T* m_elem = nullptr;
};


template <typename T, std::size_t N>
SboVectorConstIterator<T, N>::SboVectorConstIterator(T* elem) noexcept : m_elem{elem}
{
}


template <typename T, std::size_t N>
const typename SboVectorConstIterator<T, N>::value_type&
   SboVectorConstIterator<T, N>::operator*() const
{
   return *m_elem;
}


template <typename T, std::size_t N>
const typename SboVectorConstIterator<T, N>::value_type*
   SboVectorConstIterator<T, N>::operator->() const
{
   return m_elem;
}


template <typename T, std::size_t N>
const typename SboVectorConstIterator<T, N>::value_type&
   SboVectorConstIterator<T, N>::operator[](difference_type offset) const
{
   return *(m_elem + offset);
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N>& SboVectorConstIterator<T, N>::operator++()
{
   ++m_elem;
   return *this;
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N> SboVectorConstIterator<T, N>::operator++(int)
{
   auto before = *this;
   ++(*this);
   return before;
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N>& SboVectorConstIterator<T, N>::operator--()
{
   --m_elem;
   return *this;
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N> SboVectorConstIterator<T, N>::operator--(int)
{
   auto before = *this;
   --(*this);
   return before;
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N>&
SboVectorConstIterator<T, N>::operator+=(difference_type offset)
{
   m_elem += offset;
   return *this;
}


template <typename T, std::size_t N>
SboVectorConstIterator<T, N>&
SboVectorConstIterator<T, N>::operator-=(difference_type offset)
{
   m_elem -= offset;
   return *this;
}


///////////////////

// Iterator for SboVector.
// Needs to be derived from non-const iterator type, so that functions that take
// const iterators can also be called with non-const iterators.
template <typename T, std::size_t N>
class SboVectorIterator : public SboVectorConstIterator<T, N>
{
   template <typename T, std::size_t N> friend class SboVector;

 public:
   using SboVectorConstIterator<T, N>::SV;
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename SV::value_type;
   using difference_type = typename SV::difference_type;
   using pointer = typename SV::pointer;
   using reference = typename SV::reference;

 private:
   // Only the vector class need access to ctor.
   explicit SboVectorIterator(T* elem) noexcept;

 public:
   SboVectorIterator() = default;
   SboVectorIterator(const SboVectorIterator&) = default;
   SboVectorIterator& operator=(const SboVectorIterator&) = default;

   value_type& operator*();
   const value_type& operator*() const;
   value_type* operator->();
   const value_type* operator->() const;
   value_type& operator[](difference_type offset);
   const value_type& operator[](difference_type offset) const;
   SboVectorIterator& operator++();
   SboVectorIterator operator++(int);
   SboVectorIterator& operator--();
   SboVectorIterator operator--(int);
   SboVectorIterator& operator+=(difference_type offset);
   SboVectorIterator& operator-=(difference_type offset);

   friend void swap(SboVectorIterator<T, N>& a, SboVectorIterator<T, N>& b)
   {
      std::swap(a.m_elem, b.m_elem);
   }

   friend bool operator==(const SboVectorIterator<T, N>& a,
                          const SboVectorIterator<T, N>& b)
   {
      return a.m_elem == b.m_elem;
   }

   friend bool operator!=(const SboVectorIterator<T, N>& a,
                          const SboVectorIterator<T, N>& b)
   {
      return !(a == b);
   }

   friend SboVectorIterator<T, N> operator+(const SboVectorIterator<T, N>& it,
                                            difference_type offset)
   {
      auto res = it;
      return (res += offset);
   }

   friend SboVectorIterator<T, N> operator+(difference_type offset,
                                            const SboVectorIterator<T, N>& it)
   {
      return (it + offset);
   }

   friend SboVectorIterator<T, N> operator-(const SboVectorIterator<T, N>& it,
                                            difference_type offset)
   {
      auto res = it;
      return (res -= offset);
   }

   friend difference_type operator-(const SboVectorIterator<T, N>& a,
                                    const SboVectorIterator<T, N>& b)
   {
      return a.m_elem - b.m_elem;
   }

   friend bool operator<(const SboVectorIterator<T, N>& a,
                         const SboVectorIterator<T, N>& b)
   {
      return a.m_elem < b.m_elem;
   }

   friend bool operator>(const SboVectorIterator<T, N>& a,
                         const SboVectorIterator<T, N>& b)
   {
      return a.m_elem > b.m_elem;
   }

   friend bool operator<=(const SboVectorIterator<T, N>& a,
                          const SboVectorIterator<T, N>& b)
   {
      return !(a > b);
   }

   friend bool operator>=(const SboVectorIterator<T, N>& a,
                          const SboVectorIterator<T, N>& b)
   {
      return !(a < b);
   }
};


template <typename T, std::size_t N>
SboVectorIterator<T, N>::SboVectorIterator(T* elem) noexcept
: SboVectorConstIterator<T, N>{elem}
{
}


template <typename T, std::size_t N>
typename SboVectorIterator<T, N>::value_type& SboVectorIterator<T, N>::operator*()
{
   return *this->m_elem;
}


template <typename T, std::size_t N>
const typename SboVectorIterator<T, N>::value_type&
   SboVectorIterator<T, N>::operator*() const
{
   return *this->m_elem;
}


template <typename T, std::size_t N>
typename SboVectorIterator<T, N>::value_type* SboVectorIterator<T, N>::operator->()
{
   return this->m_elem;
}


template <typename T, std::size_t N>
const typename SboVectorIterator<T, N>::value_type*
   SboVectorIterator<T, N>::operator->() const
{
   return this->m_elem;
}


template <typename T, std::size_t N>
typename SboVectorIterator<T, N>::value_type&
   SboVectorIterator<T, N>::operator[](difference_type offset)
{
   return *(this->m_elem + offset);
}


template <typename T, std::size_t N>
const typename SboVectorIterator<T, N>::value_type&
   SboVectorIterator<T, N>::operator[](difference_type offset) const
{
   return *(this->m_elem + offset);
}


template <typename T, std::size_t N>
SboVectorIterator<T, N>& SboVectorIterator<T, N>::operator++()
{
   ++this->m_elem;
   return *this;
}


template <typename T, std::size_t N>
SboVectorIterator<T, N> SboVectorIterator<T, N>::operator++(int)
{
   auto before = *this;
   ++(*this);
   return before;
}


template <typename T, std::size_t N>
SboVectorIterator<T, N>& SboVectorIterator<T, N>::operator--()
{
   --this->m_elem;
   return *this;
}


template <typename T, std::size_t N>
SboVectorIterator<T, N> SboVectorIterator<T, N>::operator--(int)
{
   auto before = *this;
   --(*this);
   return before;
}


template <typename T, std::size_t N>
SboVectorIterator<T, N>& SboVectorIterator<T, N>::operator+=(difference_type offset)
{
   this->m_elem += offset;
   return *this;
}


template <typename T, std::size_t N>
SboVectorIterator<T, N>& SboVectorIterator<T, N>::operator-=(difference_type offset)
{
   this->m_elem -= offset;
   return *this;
}

} // namespace ds
