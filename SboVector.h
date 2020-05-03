#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>


///////////////////

// Vector type that use the small buffer optimization to avoid heap usage for
// sizes below a given number of elements.
template <typename T, std::size_t N> class SboVector
{
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

   std::size_t size() const noexcept;
   constexpr std::size_t max_size() const noexcept;
   std::size_t capacity() const noexcept;
   bool empty() const noexcept;
   void reserve(std::size_t capacity);
   void shrink_to_fit();

   void clear() noexcept;
   void push_back(T val);

 private:
   constexpr T* buffer();
   constexpr const T* buffer() const;
   bool inBuffer() const;
   bool onHeap() const;
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
   std::byte m_buffer[N * sizeof(T)];
};


template <typename T, std::size_t N>
SboVector<T, N>::SboVector(std::size_t count, const T& value)
{
   if (count > BufferCapacity)
      allocate(count);
   m_capacity = count;
   std::fill_n(m_data, count, value);
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


template <typename T, std::size_t N> constexpr T* SboVector<T, N>::buffer()
{
   return reinterpret_cast<T*>(m_buffer);
}


template <typename T, std::size_t N> constexpr const T* SboVector<T, N>::buffer() const
{
   return reinterpret_cast<const T*>(m_buffer);
}


template <typename T, std::size_t N> bool SboVector<T, N>::inBuffer() const
{
   return (m_data == buffer());
}


template <typename T, std::size_t N> bool SboVector<T, N>::onHeap() const
{
   return !inBuffer();
}


template <typename T, std::size_t N> void SboVector<T, N>::allocate(std::size_t cap)
{
   m_data = new T[cap];
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
      delete[] m_data;
}


template <typename T, std::size_t N>
std::size_t SboVector<T, N>::recalcCapacity(std::size_t minCap) const
{
   const std::size_t maxCap = max_size();
   if (m_capacity > maxCap / 2)
      return maxCap;
   return std::max(2 * m_capacity, minCap);
}
