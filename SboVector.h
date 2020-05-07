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
#include <type_traits>


///////////////////

// Vector type that use the small buffer optimization to avoid heap usage for
// sizes below a given number of elements.
// Just like for std::vector the template parameter type must be
// - copy-constructible
// - copy-assignable
// - erasable
// Where erasable is defined as p->~T() (a call to the type's dtor through a pointer
// to T) being valid. This allows class types with accessible dtors and scalar types.
// It excludes array types, function types, reference types, and void.
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

   bool inBuffer() const;
   bool onHeap() const;

 private:
   constexpr T* buffer();
   constexpr const T* buffer() const;
   template<typename U, typename ElemIter>
   void constructFrom(const U& other, ElemIter first);
   void destroy();
   template<typename U, typename ElemIter>
   void copyFrom(const U& other, ElemIter first);
   void moveFrom(SboVector&& other);
   void allocate(std::size_t cap);
   void reallocate(std::size_t newCap);
   void deallocate();
   static T* allocateMem(std::size_t cap);
   static void deallocateMem(T* mem);
   std::size_t recalcCapacity(std::size_t minCap) const;

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
   if (count > BufferCapacity)
      allocate(count);
   std::uninitialized_fill_n(m_data, count, value);
   m_size = count;
}


template <typename T, std::size_t N> SboVector<T, N>::SboVector(const SboVector& other)
{
   constructFrom(other, other.m_data);
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(SboVector&& other)
{
   moveFrom(std::move(other));
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::initializer_list<T> ilist)
{
   constructFrom(ilist, ilist.begin());
}


template <typename T, std::size_t N> SboVector<T, N>::~SboVector()
{
   destroy();
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(const SboVector& other)
{
   copyFrom(other, other.m_data);
   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(SboVector&& other)
{
   destroy();
   moveFrom(std::move(other));
   return *this;
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(std::initializer_list<T> ilist)
{
   copyFrom(ilist, ilist.begin());
   return *this;
}


template <typename T, std::size_t N>
void SboVector<T, N>::assign(size_type count, const T& value)
{
   // todo
}


template <typename T, std::size_t N>
T& SboVector<T, N>::operator[](std::size_t pos)
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
      reallocate(recalcCapacity(m_size + 1));
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
template<typename U, typename ElemIter>
void SboVector<T, N>::constructFrom(const U& other, ElemIter first)
{
   if (other.size() > BufferCapacity)
      allocate(other.size());
   std::uninitialized_copy_n(first, other.size(), m_data);
   m_size = other.size();
}


template <typename T, std::size_t N> void SboVector<T, N>::destroy()
{
   std::destroy_n(m_data, size());
   if (onHeap())
      deallocate();
}


template <typename T, std::size_t N> 
template<typename U, typename ElemIter>
void SboVector<T, N>::copyFrom(const U& other, ElemIter first)
{
   bool dealloc = false;

   // Perform allocation up front.
   // Try to reuse existing heap memory if possible.
   T* newData = nullptr;
   const bool needsHeap = other.size() > BufferCapacity;
   const bool fitsIntoExistingMem = other.size() <= m_capacity;
   if (needsHeap && !fitsIntoExistingMem)
   {
      dealloc = true;
      newData = allocateMem(other.size());
   }

   // Clean up old data.
   std::destroy_n(m_data, size());
   if (onHeap() && dealloc)
      deallocateMem(m_data);

   // Set up new data.
   if (needsHeap)
   {
      if (newData)
      {
         m_data = newData;
         m_capacity = other.size();
      }
   }
   else
   {
      m_data = buffer();
      m_capacity = BufferCapacity;
   }
   std::uninitialized_copy_n(first, other.size(), m_data);
   m_size = other.size();
}


template <typename T, std::size_t N> 
void SboVector<T, N>::moveFrom(SboVector&& other)
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


template <typename T, std::size_t N> void SboVector<T, N>::allocate(std::size_t cap)
{
   m_data = allocateMem(cap);
   m_capacity = cap;
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
      destroy();
      m_data = newData;
      m_capacity = newCap;
   }
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocate()
{
   if (onHeap())
   {
      deallocateMem(m_data);
      m_data = buffer();
   }
}


template <typename T, std::size_t N> T* SboVector<T, N>::allocateMem(std::size_t cap)
{
#ifdef VS_COMPILER
   // Visual Studio does not support std::aligned_alloc.
   return reinterpret_cast<T*>(_aligned_malloc(cap * sizeof(T), alignof(T)));
#else
   return reinterpret_cast<T*>(std::aligned_alloc(alignof(T), cap * sizeof(T)));
#endif
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocateMem(T* mem)
{
#ifdef VS_COMPILER
      _aligned_free(mem);
#else
      std::free(mem);
#endif
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::recalcCapacity(std::size_t minCap) const
{
   const std::size_t maxCap = max_size();
   if (m_capacity > maxCap / 2)
      return maxCap;
   return std::max(2 * m_capacity, minCap);
}
