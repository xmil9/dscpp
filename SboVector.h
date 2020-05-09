#pragma once

#ifdef _MSC_VER
#define VS_COMPILER
#endif

#include <algorithm>
#include <cassert>
#include <cstdlib>
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

   T& operator[](std::size_t pos);
   const T& operator[](std::size_t pos) const;

   std::size_t size() const noexcept;
   constexpr std::size_t max_size() const noexcept;
   std::size_t capacity() const noexcept;
   bool empty() const noexcept;
   void reserve(std::size_t capacity);
   void shrink_to_fit();

   void clear() noexcept;
   void push_back(T val);

   bool in_buffer() const;
   bool on_heap() const;

 private:
   constexpr T* buffer();
   constexpr const T* buffer() const;

   template <typename U, typename ElemIter>
   void construct_from(const U& other, ElemIter first);
   template <typename U, typename ElemIter>
   void copy_from(const U& other, ElemIter first);
   void move_from(SboVector&& other);

   template <typename U, typename ElemIter>
   void copy_elements(const U& other, ElemIter first);
   template <typename U, typename ElemIter>
   void move_elements(const U& other, ElemIter first);
   void fill_elements(size_type count, const T& value);
   void destroy_elements();

   static constexpr bool fitsIntoBuffer(std::size_t size);
   void allocate_or_reuse(std::size_t cap);
   void allocate(std::size_t cap);
   void deallocate();

   static T* allocate_or_reuse_mem(std::size_t cap, std::size_t availCap);
   static T* allocate_mem(std::size_t cap);
   static void deallocate_mem(T* mem);

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

   fill_elements(count, value);
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

   copy_elements(other, other.m_data);
   m_size = srcSize;
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(SboVector&& other)
{
   // Available strategies are to use steal the heap allocation or use the buffer.

   const auto srcSize = other.size();
   const bool canSteal = other.on_heap();

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
      move_elements(other, other.m_data);
      m_capacity = BufferCapacity;
   }

   m_size = srcSize;

   // Clear other instance.
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

   copy_elements(ilist, ilist.begin());
   m_size = srcSize;
}


template <typename T, std::size_t N> SboVector<T, N>::~SboVector()
{
   destroy_elements();
   deallocate();
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(const SboVector& other)
{
   // Available strategies are to use the buffer, to reuse an existing heap
   // allocation, or make a new heap allocation.

   const auto srcSize = other.size();
   const bool fitsBuffer = fitsIntoBuffer(srcSize);
   const bool canReuseHeap = on_heap() && m_capacity >= srcSize;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* newData = nullptr;
   if (allocHeap)
      newData = allocate_mem(srcSize);

   // Clean up existing data.
   destroy_elements();
   if (fitsBuffer || allocHeap)
      deallocate();

   // Set up new data.
   if (fitsBuffer)
   {
      copy_elements(other, other.m_data);
      m_capacity = BufferCapacity;
      m_size = srcSize;
   }
   else if (canReuseHeap)
   {
      copy_elements(other, other.m_data);
      // Capacity stays the same.
      m_size = srcSize;
   }
   else
   {
      assert(allocHeap && newData);
      m_data = newData;
      copy_elements(other, other.m_data);
      m_capacity = srcSize;
      m_size = srcSize;
   }

   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(SboVector&& other)
{
   const auto srcSize = other.size();

   const bool canSteal = other.on_heap();
   const bool canReuseHeap = on_heap() && m_capacity <= srcSize;

   destroy_elements();

   // Best option is stealing the heap memory including its contents.
   if (canSteal)
   {
      deallocate();
      m_data = other.m_data;
      m_capacity = other.m_capacity;
      m_size = srcSize;
      // Reset other data to buffer to prevent deallocation of the
      // stolen memory.
      other.m_data = other.buffer();
   }
   // Next best option is using the buffer.
   else if (fitsIntoBuffer(srcSize))
   {
      deallocate();
      move_elements(other, other.m_data);
      m_capacity = BufferCapacity;
      m_size = srcSize;
   }
   // Next best option is reusing the heap memory.
   else if (canReuseHeap)
   {
      move_elements(other, other.m_data);
      // Capacity stays the same.
      m_size = srcSize;
   }
   // Last option is allocating heap memory.
   else
   {
      deallocate();
      allocate(srcSize);
      move_elements(other, other.m_data);
      m_capacity = srcSize;
      m_size = srcSize;
   }

   // Clear other instance.
   other.deallocate();
   other.m_capacity = BufferCapacity;
   other.m_size = 0;

   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(std::initializer_list<T> ilist)
{
   copy_from(ilist, ilist.begin());
   return *this;
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(size_type count, const T& value)
{
   // Perform allocation up front.
   T* newData = allocate_or_reuse_mem(count, m_capacity);
   const bool dealloc = newData != nullptr;

   // Clean up old data.
   std::destroy_n(m_data, size());
   if (on_heap() && dealloc)
      deallocate_mem(m_data);

   // Set up new data.
   const bool fitsIntoBuffer = count <= BufferCapacity;
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
         m_capacity = count;
      }
   }
   std::uninitialized_fill_n(m_data, count, value);
   m_size = count;
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


template <typename T, std::size_t N> bool SboVector<T, N>::empty() const noexcept
{
   return (m_size == 0);
}


template <typename T, std::size_t N> void SboVector<T, N>::reserve(std::size_t capacity)
{
   if (capacity > m_capacity)
      reallocate(capacity);
}


template <typename T, std::size_t N> void SboVector<T, N>::shrink_to_fit()
{
   if (on_heap() && m_size < m_capacity)
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


template <typename T, std::size_t N> bool SboVector<T, N>::in_buffer() const
{
   return (m_data == buffer());
}


template <typename T, std::size_t N> bool SboVector<T, N>::on_heap() const
{
   return !in_buffer();
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
   if (on_heap() && dealloc)
      deallocate_mem(m_data);

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
   if (other.on_heap())
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
template <typename U, typename ElemIter>
void SboVector<T, N>::copy_elements(const U& other, ElemIter first)
{
   std::uninitialized_copy_n(first, other.size(), m_data);
}


template <typename T, std::size_t N>
template <typename U, typename ElemIter>
void SboVector<T, N>::move_elements(const U& other, ElemIter first)
{
   std::uninitialized_move_n(first, other.size(), m_data);
}


template <typename T, std::size_t N>
void SboVector<T, N>::fill_elements(size_type count, const T& value)
{
   std::uninitialized_fill_n(m_data, count, value);
}


template <typename T, std::size_t N> void SboVector<T, N>::destroy_elements()
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
   m_data = allocate_mem(cap);
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocate()
{
   if (on_heap())
   {
      deallocate_mem(m_data);
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

   return allocate_mem(cap);
}


template <typename T, std::size_t N> T* SboVector<T, N>::allocate_mem(std::size_t cap)
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


template <typename T, std::size_t N> void SboVector<T, N>::deallocate_mem(T* mem)
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
      destroy_elements();
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
