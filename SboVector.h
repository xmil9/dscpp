#pragma once

#ifdef _MSC_VER
#define VS_COMPILER
#endif

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
                 "Element type must be copy-constructible");
   static_assert(std::is_copy_assignable_v<T>, "Element type must be copy-assignable");
   static_assert(std::is_copy_assignable_v<T>, "Element type must be copy-assignable");
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

   static constexpr std::size_t BufferCapacity = N;

 public:
   SboVector() = default;
   explicit SboVector(std::size_t count, const T& value);
   SboVector(const SboVector& other);
   SboVector(SboVector&& other);
   SboVector(std::initializer_list<T> ilist);
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

   bool empty() const noexcept;
   std::size_t size() const noexcept;
   constexpr std::size_t max_size() const noexcept;
   std::size_t capacity() const noexcept;
   void reserve(std::size_t capacity);
   void shrink_to_fit();

   void clear() noexcept;
   void push_back(T val);

   bool inBuffer() const;
   bool onHeap() const;

 private:
   constexpr T* buffer();
   constexpr const T* buffer() const;

   template <typename U, typename ElemIter>
   void construct_from(const U& other, ElemIter first);
   template <typename U, typename ElemIter>
   void copy_from(const U& other, ElemIter first);
   void move_from(SboVector&& other);

   template <typename ElemIter> void copyElements(ElemIter first, std::size_t count);
   template <typename ElemIter> void moveElements(ElemIter first, std::size_t count);
   void fillElements(size_type count, const T& value);
   void destroyElements();

   static constexpr bool fitsIntoBuffer(std::size_t size);
   void allocate_or_reuse(std::size_t cap);
   void allocate(std::size_t cap);
   void deallocate();

   static T* allocate_or_reuse_mem(std::size_t cap, std::size_t availCap);
   static T* allocateMem(std::size_t cap);
   static void deallocateMem(T* mem);

   void reallocate(std::size_t newCap);
   std::size_t recalc_capacity(std::size_t minCap) const;

 private:
   // Internal buffer.
   std::aligned_storage_t<sizeof(T), alignof(T)> m_buffer[N];
   // Beginning of data (heap or buffer).
   T* m_data = buffer();
   // Number of occupied elements.
   std::size_t m_size = 0;
   // Number of allocated elements.
   std::size_t m_capacity = BufferCapacity;
};


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::size_t count, const T& value)
{
   // Available strategies are to use the buffer or make a new heap allocation.

   const bool allocHeap = !fitsIntoBuffer(count);

   if (allocHeap)
   {
      allocate(count);
      m_capacity = count;
   }

   fillElements(count, value);
   m_size = count;
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(const SboVector& other)
{
   // Available strategies are to use the buffer or make a new heap allocation.

   const auto srcSize = other.size();
   const bool allocHeap = !fitsIntoBuffer(srcSize);

   if (allocHeap)
   {
      allocate(srcSize);
      m_capacity = srcSize;
   }

   copyElements(other.m_data, srcSize);
   m_size = srcSize;
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(SboVector&& other)
{
   // Available strategies are to use steal the heap allocation or use the buffer.
   // A heap allocations does't make sense as strategys since stealing is faster.

   const auto srcSize = other.size();
   const bool canSteal = other.onHeap();

   if (canSteal)
   {
      m_data = other.m_data;
      m_capacity = other.m_capacity;
      // Reset other data to prevent deallocation of the stolen memory.
      other.m_data = other.buffer();
   }
   else
   {
      // Elements must fit into the buffer.
      assert(fitsIntoBuffer(srcSize));
      moveElements(other.m_data, srcSize);
      m_capacity = BufferCapacity;
   }

   m_size = srcSize;

   // Clear source object.
   other.deallocate();
   other.m_capacity = BufferCapacity;
   other.m_size = 0;
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::initializer_list<T> ilist)
{
   // Available strategies are to use the buffer or make a new heap allocation.

   const auto srcSize = ilist.size();
   const bool allocHeap = !fitsIntoBuffer(srcSize);

   if (allocHeap)
   {
      allocate(srcSize);
      m_capacity = srcSize;
   }

   copyElements(ilist.begin(), srcSize);
   m_size = srcSize;
}


template <typename T, std::size_t N> SboVector<T, N>::~SboVector()
{
   destroyElements();
   deallocate();
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(const SboVector& other)
{
   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const auto srcSize = other.size();
   const bool fitsBuffer = fitsIntoBuffer(srcSize);
   const bool canReuseHeap = onHeap() && m_capacity >= srcSize;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(srcSize);

   // Clean up existing data.
   destroyElements();
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
      m_capacity = srcSize;
   }

   copyElements(other.m_data, srcSize);
   m_size = srcSize;

   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(SboVector&& other)
{
   // Available strategies are to steal the heap allocation or to use the buffer.
   // Heap reuse or a allocation don't make sense as strategies since stealing
   // is faster.

   const auto srcSize = other.size();
   const bool canSteal = other.onHeap();

   // Clean up existing data.
   destroyElements();
   deallocate();

   // Set up new data.
   if (canSteal)
   {
      m_data = other.m_data;
      m_capacity = other.m_capacity;
      // Reset source object to buffer to prevent deallocation of the
      // stolen memory.
      other.m_data = other.buffer();
   }
   else
   {
      // Elements must fit into the buffer.
      assert(fitsIntoBuffer(srcSize));
      moveElements(other.m_data, srcSize);
      m_capacity = BufferCapacity;
   }

   m_size = srcSize;

   // Clear source object.
   other.deallocate();
   other.m_capacity = BufferCapacity;
   other.m_size = 0;

   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(std::initializer_list<T> ilist)
{
   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const auto srcSize = ilist.size();
   const bool fitsBuffer = fitsIntoBuffer(srcSize);
   const bool canReuseHeap = onHeap() && m_capacity >= srcSize;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(srcSize);

   // Clean up existing data.
   destroyElements();
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
      m_capacity = srcSize;
   }

   copyElements(ilist.begin(), srcSize);
   m_size = srcSize;

   return *this;
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(size_type count, const T& value)
{
   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const bool fitsBuffer = fitsIntoBuffer(count);
   const bool canReuseHeap = onHeap() && m_capacity >= count;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(count);

   // Clean up existing data.
   destroyElements();
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
      m_capacity = count;
   }

   fillElements(count, value);
   m_size = count;
}


template <typename T, std::size_t N>
template <typename FwdIter>
void SboVector<T, N>::assign(FwdIter first, FwdIter last)
{
   // The accepted iterator type for std::vector is an input iterator. Input iterators
   // permit only a single pass over the elements. The accepted iterator type here is
   // more relaxed, it is a forward iterator that allows multiple passes.

   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const std::size_t count = std::distance(first, last);
   const bool fitsBuffer = fitsIntoBuffer(count);
   const bool canReuseHeap = onHeap() && m_capacity >= count;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(count);

   // Clean up existing data.
   destroyElements();
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
      m_capacity = count;
   }

   copyElements(first, count);
   m_size = count;
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(std::initializer_list<T> ilist)
{
   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const std::size_t count = ilist.size();
   const bool fitsBuffer = fitsIntoBuffer(count);
   const bool canReuseHeap = onHeap() && m_capacity >= count;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocateMem(count);

   // Clean up existing data.
   destroyElements();
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
      m_capacity = count;
   }

   copyElements(ilist.begin(), count);
   m_size = count;
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


template <typename T, std::size_t N> void SboVector<T, N>::reserve(std::size_t capacity)
{
   if (capacity > m_capacity)
      reallocate(capacity);
}


template <typename T, std::size_t N> void SboVector<T, N>::shrink_to_fit()
{
   if (onHeap() && m_size < m_capacity)
      reallocate(m_size);
}


template <typename T, std::size_t N> void SboVector<T, N>::clear() noexcept
{
   // todo
}


template <typename T, std::size_t N> void SboVector<T, N>::push_back(T val)
{
   if (m_size == m_capacity)
      reallocate(recalc_capacity(m_size + 1));
   m_data[m_size++] = std::move(val);
}


template <typename T, std::size_t N> bool SboVector<T, N>::inBuffer() const
{
   return (m_data == buffer());
}


template <typename T, std::size_t N> bool SboVector<T, N>::onHeap() const
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
template <typename U, typename ElemIter>
void SboVector<T, N>::construct_from(const U& other, ElemIter first)
{
   if (other.size() > BufferCapacity)
      allocate(other.size());
   std::uninitialized_copy_n(first, other.size(), m_data);
   m_size = other.size();
}


template <typename T, std::size_t N>
template <typename U, typename ElemIter>
void SboVector<T, N>::copy_from(const U& other, ElemIter first)
{
   // Perform allocation up front.
   T* newData = allocate_or_reuse_mem(other.size(), m_capacity);
   const bool dealloc = newData != nullptr;

   // Clean up old data.
   std::destroy_n(m_data, size());
   if (onHeap() && dealloc)
      deallocateMem(m_data);

   // Set up new data.
   const bool fitsIntoBuffer = other.size() <= BufferCapacity;
   if (fitsIntoBuffer)
   {
      m_data = buffer();
      m_capacity = BufferCapacity;
   }
   else
   {
      if (newData)
      {
         m_data = newData;
         m_capacity = other.size();
      }
   }
   std::uninitialized_copy_n(first, other.size(), m_data);
   m_size = other.size();
}


template <typename T, std::size_t N> void SboVector<T, N>::move_from(SboVector&& other)
{
   if (other.onHeap())
   {
      // Steal heap memory.
      m_data = other.m_data;
   }
   else
   {
      std::uninitialized_move_n(other.buffer(), other.size(), buffer());
   }
   m_capacity = other.m_capacity;
   m_size = other.m_size;

   // Clear other.
   other.m_data = other.buffer();
   other.m_capacity = BufferCapacity;
   other.m_size = 0;
}


template <typename T, std::size_t N>
template <typename ElemIter>
void SboVector<T, N>::copyElements(ElemIter first, std::size_t count)
{
   std::uninitialized_copy_n(first, count, m_data);
}


template <typename T, std::size_t N>
template <typename ElemIter>
void SboVector<T, N>::moveElements(ElemIter first, std::size_t count)
{
   std::uninitialized_move_n(first, count, m_data);
}


template <typename T, std::size_t N>
void SboVector<T, N>::fillElements(size_type count, const T& value)
{
   std::uninitialized_fill_n(m_data, count, value);
}


template <typename T, std::size_t N> void SboVector<T, N>::destroyElements()
{
   std::destroy_n(m_data, size());
}


template <typename T, std::size_t N>
constexpr bool SboVector<T, N>::fitsIntoBuffer(std::size_t size)
{
   return size <= BufferCapacity;
}


template <typename T, std::size_t N>
void SboVector<T, N>::allocate_or_reuse(std::size_t cap)
{
   m_data = allocate_or_reuse_mem(cap, m_capacity);
}


template <typename T, std::size_t N> void SboVector<T, N>::allocate(std::size_t cap)
{
   m_data = allocateMem(cap);
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocate()
{
   if (onHeap())
   {
      deallocateMem(m_data);
      m_data = buffer();
   }
}


template <typename T, std::size_t N>
T* SboVector<T, N>::allocate_or_reuse_mem(std::size_t cap, std::size_t availCap)
{
   if (fitsIntoBuffer(cap))
      return nullptr;

   const bool fitsIntoExistingMem = cap <= availCap;
   if (fitsIntoExistingMem)
      return nullptr;

   return allocateMem(cap);
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
   return mem;
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocateMem(T* mem)
{
#ifdef VS_COMPILER
   _aligned_free(mem);
#else
   std::free(mem);
#endif
}


template <typename T, std::size_t N> void SboVector<T, N>::reallocate(std::size_t newCap)
{
   assert(newCap >= m_size);
   if (newCap >= m_size)
   {
      T* newData = new T[newCap];
      if constexpr (std::is_move_constructible_v<T>)
         std::uninitialized_move_n(m_data, m_size, newData);
      else
         std::uninitialized_copy_n(m_data, m_size, newData);
      destroyElements();
      m_data = newData;
      m_capacity = newCap;
   }
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::recalc_capacity(std::size_t minCap) const
{
   const std::size_t maxCap = max_size();
   if (m_capacity > maxCap / 2)
      return maxCap;
   return std::max(2 * m_capacity, minCap);
}


///////////////////

// Iterator for SboVector.
template <typename SV> class SboVectorIterator
{
 public:
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename SV::value_type;
   using difference_type = typename SV::difference_type;
   using pointer = typename SV::pointer;
   using reference = typename SV::reference;

 public:
   SboVectorIterator() = default;
   SboVectorIterator(SV* sv, std::size_t idx);
   ~SboVectorIterator() = default;
   SboVectorIterator(const SboVectorIterator&) = default;
   SboVectorIterator(SboVectorIterator&& other);

   SboVectorIterator& operator=(const SboVectorIterator&) = default;
   SboVectorIterator& operator=(SboVectorIterator&& other);

   const value_type& operator*() const;
   value_type& operator*();
   const value_type* operator->() const;
   value_type* operator->();
   SboVectorIterator& operator++();
   SboVectorIterator operator++(int);
   SboVectorIterator& operator--();
   SboVectorIterator operator--(int);

   friend void swap(SboVectorIterator<SV>& a, SboVectorIterator<SV>& b)
   {
      std::swap(a.m_sv, b.m_sv);
      std::swap(a.m_idx, b.m_idx);
   }

   friend bool operator==(const SboVectorIterator<SV>& a, const SboVectorIterator<SV>& b)
   {
      return a.m_sv == b.m_sv && a.m_idx == b.m_idx;
   }

   friend bool operator!=(const SboVectorIterator<SV>& a, const SboVectorIterator<SV>& b)
   {
      return !(a == b);
   }

 private:
   SV* m_sv = nullptr;
   std::size_t m_idx = 0;
};


template <typename SV>
SboVectorIterator<SV>::SboVectorIterator(SV* sv, std::size_t idx) : m_sv{sv}, m_idx{idx}
{
}

template <typename SV> SboVectorIterator<SV>::SboVectorIterator(SboVectorIterator&& other)
{
   swap(*this, other);
}


template <typename SV>
SboVectorIterator<SV>& SboVectorIterator<SV>::operator=(SboVectorIterator&& other)
{
   m_sv = other.m_sv;
   m_idx = other.m_idx;
   other.m_sv = nullptr;
   other.m_idx = 0;
   return *this;
}


template <typename SV>
const typename SboVectorIterator<SV>::value_type& SboVectorIterator<SV>::operator*() const
{
   return (*m_sv)[m_idx];
}


template <typename SV>
typename SboVectorIterator<SV>::value_type& SboVectorIterator<SV>::operator*()
{
   return (*m_sv)[m_idx];
}


template <typename SV>
const typename SboVectorIterator<SV>::value_type*
   SboVectorIterator<SV>::operator->() const
{
   return &(*m_sv)[m_idx];
}


template <typename SV>
typename SboVectorIterator<SV>::value_type* SboVectorIterator<SV>::operator->()
{
   return &(*m_sv)[m_idx];
}


template <typename SV> SboVectorIterator<SV>& SboVectorIterator<SV>::operator++()
{
   ++m_idx;
   return *this;
}


template <typename SV> SboVectorIterator<SV> SboVectorIterator<SV>::operator++(int)
{
   auto before = *this;
   ++(*this);
   return before;
}


template <typename SV>
SboVectorIterator<SV>& SboVectorIterator<SV>::operator--()
{
   --m_idx;
   return *this;
}


template <typename SV>
SboVectorIterator<SV> SboVectorIterator<SV>::operator--(int)
{
   auto before = *this;
   --(*this);
   return before;
}
