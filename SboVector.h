#pragma once
#include <algorithm>
#include <cassert>
#include <cstdlib>
#ifdef _MSC_VER
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
   // Requirements for T.
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
   SboVector(SboVector&& other) noexcept;
   SboVector(std::initializer_list<T> ilist);
   ~SboVector();

   SboVector& operator=(const SboVector& other);

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
   void allocate(std::size_t cap);
   void reallocate(std::size_t newCap);
   void deallocate();
   std::size_t recalcCapacity(std::size_t minCap) const;

 private:
   // Number of occupied elements.
   std::size_t m_size = 0;
   // Number of allocated elements.
   std::size_t m_capacity = BufferCapacity;
   // Beginning of data (heap or buffer).
   T* m_data = buffer();
   // Internal buffer.
   std::aligned_storage_t<sizeof(T), alignof(T)> m_buffer[N];
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
   if (other.capacity() > BufferCapacity)
      allocate(other.capacity());
   m_capacity = other.capacity();
   std::copy_n(other.m_data, other.size(), m_data);
   m_size = other.size();
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(SboVector&& other) noexcept
{
   if (other.onHeap())
   {
      // Steal heap data.
      m_data = other.m_data;
      // Reset other data pointer to buffer.
      other.m_data = other.m_buffer;
   }
   else
   {
      std::swap(m_buffer, other.m_buffer);
   }
   std::swap(m_capacity, other.m_capacity);
   std::swap(m_size, other.m_size);
}


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::initializer_list<T> ilist)
{
   if (ilist.size() > BufferCapacity)
      allocate(ilist.size());
   m_capacity = ilist.size();
   std::copy_n(ilist.begin(), ilist.size(), m_data);
   m_size = ilist.size();
}


template <typename T, std::size_t N> SboVector<T, N>::~SboVector()
{
   deallocate();
}


template <typename T, std::size_t N>
SboVector<T, N>& SboVector<T, N>::operator=(const SboVector& other)
{
   deallocate();
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


template <typename T, std::size_t N> void SboVector<T, N>::allocate(std::size_t cap)
{
#ifdef _MSC_VER
   m_data = reinterpret_cast<T*>(_aligned_malloc(cap * sizeof(T), alignof(T)));
#else
   m_data = reinterpret_cast<T*>(std::aligned_alloc(alignof(T), cap * sizeof(T)));
#endif
   m_capacity = cap;
}


template <typename T, std::size_t N> void SboVector<T, N>::reallocate(std::size_t newCap)
{
   assert(newCap >= m_size);
   if (newCap >= m_size)
   {
      T* newData = new T[newCap];
      std::copy_n(m_data, m_size, newData);
      deallocate();
      m_data = newData;
      m_capacity = newCap;
   }
}


template <typename T, std::size_t N> void SboVector<T, N>::deallocate()
{
   if (onHeap())
   {
#ifdef _MSC_VER
      _aligned_free(m_data);
#else
      std::free(m_data);
#endif
      m_data = buffer();
   }
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::recalcCapacity(std::size_t minCap) const
{
   const std::size_t maxCap = max_size();
   if (m_capacity > maxCap / 2)
      return maxCap;
   return std::max(2 * m_capacity, minCap);
}
