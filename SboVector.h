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
   void push_back(const T& value);
   void push_back(T&& value);

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
   void reallocate(std::size_t newCap);
   void reallocateMore(std::size_t newCap);
   void reallocateLess(std::size_t newCap);

   static T* allocate_or_reuse_mem(std::size_t cap, std::size_t availCap);
   static T* allocateMem(std::size_t cap);
   static void deallocateMem(T* mem, std::size_t cap);

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

   // Memory instrumentation.
#ifdef SBOVEC_MEM_INSTR
 public:
   static int64_t allocatedCapacity() { return m_allocatedCap; }
   static void resetAllocatedCapacity() { m_allocatedCap = 0; }

 private:
   inline static int64_t m_allocatedCap = 0;
#endif // SBOVEC_MEM_INSTR
};


namespace details
{

template <typename ElemIter>
void copyAndDestroyBackward(ElemIter first, std::size_t count, ElemIter dest)
{
   ElemIter rFirst = first + count - 1;
   ElemIter rLast = first - 1;
   ElemIter rFirstDest = dest + count - 1;
   for (; rFirst > rLast; --rFirst, --rFirstDest)
   {
      std::uninitialized_copy_n(rFirst, 1, rFirstDest);
      std::destroy_at(rFirst);
   }
}

} // namespace details


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
   destroyElements();
   m_size = 0;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::erase(const_iterator pos)
{
   // std::vector requirements.
   static_assert(std::is_move_assignable_v<T>, "Element type must be move-assignable.");

   if (pos >= cend())
      return end();

   std::destroy_at(pos.m_elem);

   const auto offset = pos - cbegin();
   iterator it = begin() + offset;
   std::move(it + 1, end(), it);
   --m_size;

   return begin() + offset;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::erase(const_iterator first,
                                                          const_iterator last)
{
   // std::vector requirements.
   static_assert(std::is_move_assignable_v<T>, "Element type must be move-assignable.");

   if (first >= cend())
      return end();
   if (last >= cend())
      last = cend();

   const auto offset = first - cbegin();
   iterator firstIt = begin() + offset;
   const auto size = last - first;
   iterator lastIt = begin() + offset + size;
   if (size == 0)
      return lastIt;

   std::destroy_n(first.m_elem, size);

   if (lastIt < end())
      std::move(lastIt, end(), firstIt);
   m_size -= size;

   return begin() + offset;
}


template <typename T, std::size_t N>
typename SboVector<T, N>::iterator SboVector<T, N>::insert(const_iterator pos,
                                                           const T& value)
{
   // std::vector requirements.
   static_assert(std::is_copy_assignable_v<T>, "Element type must be copy-assignable.");

   // Cases:
   // - In buffer and enough capacity to stay in buffer.
   // - In buffer and not enough capacity. Allocate heap.
   // - On heap and enough capacity.
   // - On heap and not enough capacity. Need to reallocate.

   const std::size_t count = 1;
   const std::size_t newSize = size() + count;
   const bool fitsBuffer = fitsIntoBuffer(newSize);
   const bool canReuseHeap = onHeap() && m_capacity >= newSize;
   const bool allocHeap = !fitsBuffer && !canReuseHeap;

   // Perform allocation up front to prevent inconsistencies if allocation
   // fails.
   T* dest = data();
   if (allocHeap)
      dest = allocateMem(newSize);

   const std::size_t posOffset = pos - cbegin();
   const std::size_t rearSize = cend() - pos;
   const std::size_t frontSize = pos - cbegin();

   if (rearSize > 0)
   {
      T* rearSrc = data() + posOffset;
      T* rearDest = dest + posOffset + count;
      // When copying overlapping range to the right we have to copy backwards.
      details::copyAndDestroyBackward(rearSrc, rearSize, rearDest);
   }

   std::uninitialized_copy_n(&value, 1, dest + posOffset);

   const bool relocateFront = allocHeap;
   if (frontSize > 0 && relocateFront)
   {
      T* frontSrc = data();
      T* frontDest = dest;
      std::uninitialized_copy_n(frontSrc, frontSize, frontDest);
      std::destroy_n(frontSrc, frontSize);
   }

   m_size = newSize;
   if (allocHeap)
   {
      deallocate();
      m_data = dest;
      m_capacity = newSize;
   }

   return begin() + posOffset;
}


template <typename T, std::size_t N> void SboVector<T, N>::push_back(const T& value)
{
   // todo

   // assume it fits
   *(m_data + m_size++) = value;
}


template <typename T, std::size_t N> void SboVector<T, N>::push_back(T&& value)
{
   // todo

   // assume it fits
   *(m_data + m_size++) = std::move(value);
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
      deallocate();

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

   if constexpr (std::is_move_constructible_v<T>)
   {
      std::uninitialized_move_n(data(), size(), newData);
   }
   else
   {
      std::uninitialized_copy_n(data(), size(), newData);
      destroyElements();
   }

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
   const bool relocateDataIntoBuffer = (newCap <= BufferCapacity) && onHeap();

   T* newData = allocHeap ? allocateMem(newCap) : buffer();

   if constexpr (std::is_move_constructible_v<T>)
   {
      std::uninitialized_move_n(data(), size(), newData);
   }
   else
   {
      std::uninitialized_copy_n(data(), size(), newData);
      destroyElements();
   }

   // Will only dealloc if current data is on heap.
   deallocate();

   m_data = newData;
   m_capacity = newCap;
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
#endif // SBOVEC_MEM_INSTR
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
