#pragma once
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <vector>
#include <stdexcept>


namespace ds
{
///////////////////

// Ring buffer of fixed size N.
// Will overwrite oldest elements once full.
template <typename T, std::size_t N> class RingBuffer
{
   template <typename RB> friend class RingBufferConstIterator;
   template <typename RB> friend class RingBufferIterator;

 private:
   // The internal std::array has one additional element that serves as one-past-the-end
   // position.
   static constexpr std::size_t M = N + 1;
   using BufferType = std::array<T, M>;

 public:
   using value_type = T;
   using size_type = typename BufferType::size_type;
   using difference_type = int64_t;
   using reference = T&;
   using const_reference = const T&;
   using pointer = T*;
   using const_pointer = const T*;
   using iterator = RingBufferIterator<RingBuffer<T, N>>;
   using const_iterator = RingBufferConstIterator<RingBuffer<T, N>>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 public:
   RingBuffer() = default;
   ~RingBuffer() = default;
   template <typename Iter> RingBuffer(Iter first, Iter last);
   RingBuffer(std::initializer_list<T> ilist);
   RingBuffer(const RingBuffer&) = default;
   RingBuffer& operator=(const RingBuffer&) = default;
   RingBuffer(RingBuffer&& from) = default;
   RingBuffer& operator=(RingBuffer&& from) = default;

   size_type size() const noexcept;
   constexpr size_type capacity() const noexcept;
   bool empty() const noexcept;
   bool full() const noexcept;
   T& at(size_type idx);
   const T& at(size_type idx) const;
   T& operator[](size_type idx);
   const T& operator[](size_type idx) const;
   void push(const T& val);
   T pop();
   void clear();
   void swap(RingBuffer& other);

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

 private:
   using Index = size_type;

   // Index of last populated (newest) element. End-index if the ring buffer is
   // empty.
   Index last() const;

 private:
   BufferType m_buffer;
   // Index of the first (oldest) element.
   Index m_start = 0;
   // Index of the one-past-the-last element.
   Index m_end = 0;
};


///////////////////

namespace internals
{

// Modulus N addition.
inline constexpr int64_t addModN(int64_t val, int64_t offset, std::size_t n) noexcept
{
   assert(n > 0);
   return (val + offset) % n;
}

// Modulus N subtraction.
inline constexpr int64_t subModN(int64_t val, int64_t offset, std::size_t n) noexcept
{
   assert(n > 0);
   return (val - offset) % n;
}

} // namespace internals


///////////////////

template <typename T, std::size_t N>
template <typename Iter>
RingBuffer<T, N>::RingBuffer(Iter first, Iter last)
{
   const auto inputSize = std::distance(first, last);
   const auto copySize = std::min<std::size_t>(inputSize, N);

   if (inputSize <= N)
   {
      // Simple case - Fill buffer from beginning.
      std::copy_n(first, copySize, m_buffer.begin());
      m_start = 0;
      m_end = copySize;
   }
   else
   {
      // If not all input elements fit into the buffer, populate the
      // buffer as if the elements were added one after the other. This
      // avoid having different internal layouts for equivalent situation
      // (initialization with all elements vs pushing the same elements
      // one after the other).
      Iter srcStart = first;
      std::advance(srcStart, inputSize - N);
      const size_type numWrapped = inputSize % N;
      const size_type numAtEnd = copySize - numWrapped;

      std::copy_n(srcStart, numAtEnd, m_buffer.begin() + M - numAtEnd);
      std::advance(srcStart, numAtEnd);
      std::copy_n(srcStart, numWrapped, m_buffer.begin());

      m_start = M - numAtEnd;
      m_end = numWrapped;
   }
}


template <typename T, std::size_t N>
RingBuffer<T, N>::RingBuffer(std::initializer_list<T> ilist)
: RingBuffer<T, N>(ilist.begin(), ilist.end())
{
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::size_type RingBuffer<T, N>::size() const noexcept
{
   // Make sure the end is larger than the start index to avoid multiple cases.
   return internals::subModN(m_end + M, m_start, M);
}


template <typename T, std::size_t N>
constexpr typename RingBuffer<T, N>::size_type RingBuffer<T, N>::capacity() const noexcept
{
   return N;
}


template <typename T, std::size_t N> bool RingBuffer<T, N>::empty() const noexcept
{
   return (m_start == m_end);
}


template <typename T, std::size_t N> bool RingBuffer<T, N>::full() const noexcept
{
   return (size() == N);
}


template <typename T, std::size_t N> T& RingBuffer<T, N>::at(size_type idx)
{
   if (idx >= size())
      throw std::out_of_range("Invalid index into ring buffer.");
   return m_buffer[internals::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N> const T& RingBuffer<T, N>::at(size_type idx) const
{
   if (idx >= size())
      throw std::out_of_range("Invalid index into ring buffer.");
   return m_buffer[internals::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N> T& RingBuffer<T, N>::operator[](size_type idx)
{
   return m_buffer[internals::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N>
const T& RingBuffer<T, N>::operator[](size_type idx) const
{
   return m_buffer[internals::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N> void RingBuffer<T, N>::push(const T& val)
{
   // The pushed value always goes into the slot indicated by the
   // end index.
   m_buffer[m_end] = val;

   if (full())
   {
      // Get rid of first (oldest) element.
      m_end = m_start;
      m_start = internals::addModN(m_start, 1, M);
   }
   else
   {
      m_end = internals::addModN(m_end, 1, M);
   }
}


template <typename T, std::size_t N> T RingBuffer<T, N>::pop()
{
   if (empty())
      throw std::runtime_error("Popping from empty ring buffer.");

   const Index lastIdx = last();
   T popped = m_buffer[lastIdx];
   m_end = lastIdx;
   return popped;
}


template <typename T, std::size_t N> void RingBuffer<T, N>::clear()
{
   m_start = 0;
   m_end = 0;
}


template <typename T, std::size_t N> void RingBuffer<T, N>::swap(RingBuffer& other)
{
   std::swap(m_buffer, other.m_buffer);
   std::swap(m_start, other.m_start);
   std::swap(m_end, other.m_end);
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::iterator RingBuffer<T, N>::begin() noexcept
{
   return iterator(this, 0);
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::iterator RingBuffer<T, N>::end() noexcept
{
   return iterator(this, size());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::begin() const noexcept
{
   return cbegin();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::end() const noexcept
{
   return cend();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::cbegin() const noexcept
{
   return const_iterator(this, 0);
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::cend() const noexcept
{
   return const_iterator(this, size());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::reverse_iterator RingBuffer<T, N>::rbegin() noexcept
{
   return reverse_iterator(end());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::reverse_iterator RingBuffer<T, N>::rend() noexcept
{
   return reverse_iterator(begin());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::rbegin() const
   noexcept
{
   return crbegin();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::rend() const noexcept
{
   return crend();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::crbegin() const
   noexcept
{
   return const_reverse_iterator(cend());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::crend() const noexcept
{
   return const_reverse_iterator(cbegin());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::Index RingBuffer<T, N>::last() const
{
   if (empty())
      return m_end;
   return internals::subModN(m_end, 1, M);
}


///////////////////

// Const iterator for ring buffer. Does not allow changing elements.
template <typename RB> class RingBufferConstIterator
{
   template <typename T, std::size_t N> friend class RingBuffer;

 public:
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename RB::value_type;
   using difference_type = typename RB::difference_type;
   using pointer = typename RB::const_pointer;
   using reference = typename RB::const_reference;

 protected:
   // Only the ring buffer and non-const iterator classes need access to this ctor.
   RingBufferConstIterator(const RB* rb, std::size_t idx);

 public:
   RingBufferConstIterator() = default;
   ~RingBufferConstIterator() = default;
   RingBufferConstIterator(const RingBufferConstIterator&) = default;
   RingBufferConstIterator& operator=(const RingBufferConstIterator&) = default;
   RingBufferConstIterator(RingBufferConstIterator&&) = default;
   RingBufferConstIterator& operator=(RingBufferConstIterator&&) = default;

   const value_type& operator*() const;
   const value_type* operator->() const;
   const value_type& operator[](difference_type offset) const;
   RingBufferConstIterator& operator++();
   RingBufferConstIterator operator++(int);
   RingBufferConstIterator& operator--();
   RingBufferConstIterator operator--(int);
   RingBufferConstIterator& operator+=(const difference_type offset);
   RingBufferConstIterator& operator-=(const difference_type offset);

   friend RingBufferConstIterator operator+(const RingBufferConstIterator& it,
                                            difference_type offset)
   {
      auto copy = it;
      return copy += offset;
   }

   friend RingBufferConstIterator operator+(difference_type offset,
                                            const RingBufferConstIterator& it)
   {
      return it + offset;
   }

   friend RingBufferConstIterator operator-(const RingBufferConstIterator& it,
                                            difference_type offset)
   {
      auto copy = it;
      return copy -= offset;
   }

   friend difference_type operator-(const RingBufferConstIterator& a,
                                    const RingBufferConstIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return a.m_idx - b.m_idx;
   }

   friend bool operator==(const RingBufferConstIterator& a,
                          const RingBufferConstIterator& b)
   {
      return a.m_rb == b.m_rb && a.m_idx == b.m_idx;
   }

   friend bool operator!=(const RingBufferConstIterator& a,
                          const RingBufferConstIterator& b)
   {
      return !(a == b);
   }

   friend bool operator<(const RingBufferConstIterator& a,
                         const RingBufferConstIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return a.m_idx < b.m_idx;
   }

   friend bool operator>(const RingBufferConstIterator& a,
                         const RingBufferConstIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return a.m_idx > b.m_idx;
   }

   friend bool operator<=(const RingBufferConstIterator& a,
                          const RingBufferConstIterator& b)
   {
      return !(a > b);
   }

   friend bool operator>=(const RingBufferConstIterator& a,
                          const RingBufferConstIterator& b)
   {
      return !(a < b);
   }

   friend void swap(RingBufferConstIterator& a, RingBufferConstIterator& b)
   {
      std::swap(a.m_rb, b.m_rb);
      std::swap(a.m_idx, b.m_idx);
   }

 protected:
   RB* m_rb = nullptr;
   // Index to ring buffer element. This is an abstracted index, i.e. an index that
   // should be used on an ring buffer object. It is not an index into the internal
   // buffer of the ring buffer.
   std::size_t m_idx = 0;
};


template <typename RB>
RingBufferConstIterator<RB>::RingBufferConstIterator(const RB* rb, std::size_t idx)
: m_rb{const_cast<RB*>(rb)}, m_idx{idx}
{
   assert(m_rb);
}

template <typename RB>
const typename RingBufferConstIterator<RB>::value_type&
   RingBufferConstIterator<RB>::operator*() const
{
   return (*m_rb)[m_idx];
}

template <typename RB>
const typename RingBufferConstIterator<RB>::value_type*
   RingBufferConstIterator<RB>::operator->() const
{
   return &((*m_rb)[m_idx]);
}

template <typename RB>
const typename RingBufferConstIterator<RB>::value_type&
   RingBufferConstIterator<RB>::operator[](difference_type offset) const
{
   return (*m_rb)[m_idx + offset];
}

template <typename RB>
RingBufferConstIterator<RB>& RingBufferConstIterator<RB>::operator++()
{
   ++m_idx;
   return *this;
}

template <typename RB>
RingBufferConstIterator<RB> RingBufferConstIterator<RB>::operator++(int)
{
   auto before = *this;
   ++(*this);
   return before;
}

template <typename RB>
RingBufferConstIterator<RB>& RingBufferConstIterator<RB>::operator--()
{
   --m_idx;
   return *this;
}

template <typename RB>
RingBufferConstIterator<RB> RingBufferConstIterator<RB>::operator--(int)
{
   auto before = *this;
   --(*this);
   return before;
}

template <typename RB>
RingBufferConstIterator<RB>&
RingBufferConstIterator<RB>::operator+=(const difference_type offset)
{
   m_idx += offset;
   return *this;
}

template <typename RB>
RingBufferConstIterator<RB>&
RingBufferConstIterator<RB>::operator-=(const difference_type offset)
{
   m_idx -= offset;
   return *this;
}


///////////////////

// Iterator for ring buffer. Does allow changing elements.
// Needs to be derived from non-const iterator type, so that functions that take
// const iterators can also be called with non-const iterators.
template <typename RB> class RingBufferIterator : public RingBufferConstIterator<RB>
{
   template <typename T, std::size_t N> friend class RingBuffer;

 public:
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename RB::value_type;
   using difference_type = typename RB::difference_type;
   using pointer = typename RB::pointer;
   using reference = typename RB::reference;

 private:
   // Only the ring buffer class needs access to ctor.
   RingBufferIterator(RB* rb, std::size_t idx);

 public:
   RingBufferIterator() = default;
   ~RingBufferIterator() = default;
   RingBufferIterator(const RingBufferIterator&) = default;
   RingBufferIterator& operator=(const RingBufferIterator&) = default;
   RingBufferIterator(RingBufferIterator&&) = default;
   RingBufferIterator& operator=(RingBufferIterator&&) = default;

   const value_type& operator*() const;
   value_type& operator*();
   const value_type* operator->() const;
   value_type* operator->();
   const value_type& operator[](difference_type offset) const;
   value_type& operator[](difference_type offset);
   RingBufferIterator& operator++();
   RingBufferIterator operator++(int);
   RingBufferIterator& operator--();
   RingBufferIterator operator--(int);
   RingBufferIterator& operator+=(const difference_type offset);
   RingBufferIterator& operator-=(const difference_type offset);

   friend RingBufferIterator operator+(const RingBufferIterator& it,
                                       difference_type offset)
   {
      auto copy = it;
      return copy += offset;
   }

   friend RingBufferIterator operator+(difference_type offset,
                                       const RingBufferIterator& it)
   {
      return it + offset;
   }

   friend RingBufferIterator operator-(const RingBufferIterator& it,
                                       difference_type offset)
   {
      auto copy = it;
      return copy -= offset;
   }

   friend difference_type operator-(const RingBufferIterator& a,
                                    const RingBufferIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return a.m_idx - b.m_idx;
   }

   friend bool operator==(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      return (a.m_rb == b.m_rb && a.m_idx == b.m_idx);
   }

   friend bool operator!=(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      return !(a == b);
   }

   friend bool operator<(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return (a.m_idx < b.m_idx);
   }

   friend bool operator>(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      assert(a.m_rb == b.m_rb);
      return (a.m_idx > b.m_idx);
   }

   friend bool operator<=(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      return !(a > b);
   }

   friend bool operator>=(const RingBufferIterator& a, const RingBufferIterator& b)
   {
      return !(a < b);
   }

   friend void swap(RingBufferIterator& a, RingBufferIterator& b)
   {
      std::swap(a.m_rb, b.m_rb);
      std::swap(a.m_idx, b.m_idx);
   }
};


template <typename RB>
RingBufferIterator<RB>::RingBufferIterator(RB* rb, std::size_t idx)
: RingBufferConstIterator<RB>{rb, idx}
{
}

template <typename RB>
const typename RingBufferIterator<RB>::value_type&
   RingBufferIterator<RB>::operator*() const
{
   return (*this->m_rb)[this->m_idx];
}

template <typename RB>
typename RingBufferIterator<RB>::value_type& RingBufferIterator<RB>::operator*()
{
   return (*this->m_rb)[this->m_idx];
}

template <typename RB>
const typename RingBufferIterator<RB>::value_type*
   RingBufferIterator<RB>::operator->() const
{
   return &((*this->m_rb)[this->m_idx]);
}

template <typename RB>
typename RingBufferIterator<RB>::value_type* RingBufferIterator<RB>::operator->()
{
   return &((*this->m_rb)[this->m_idx]);
}

template <typename RB>
typename RingBufferIterator<RB>::value_type&
   RingBufferIterator<RB>::operator[](difference_type offset)
{
   return (*this->m_rb)[this->m_idx + offset];
}

template <typename RB>
const typename RingBufferIterator<RB>::value_type&
   RingBufferIterator<RB>::operator[](difference_type offset) const
{
   return (*this->m_rb)[this->m_idx + offset];
}

template <typename RB> RingBufferIterator<RB>& RingBufferIterator<RB>::operator++()
{
   ++this->m_idx;
   return *this;
}

template <typename RB> RingBufferIterator<RB> RingBufferIterator<RB>::operator++(int)
{
   auto before = *this;
   ++(*this);
   return before;
}

template <typename RB> RingBufferIterator<RB>& RingBufferIterator<RB>::operator--()
{
   --this->m_idx;
   return *this;
}

template <typename RB> RingBufferIterator<RB> RingBufferIterator<RB>::operator--(int)
{
   auto before = *this;
   --(*this);
   return before;
}

template <typename RB>
RingBufferIterator<RB>& RingBufferIterator<RB>::operator+=(const difference_type offset)
{
   this->m_idx += offset;
   return *this;
}

template <typename RB>
RingBufferIterator<RB>& RingBufferIterator<RB>::operator-=(const difference_type offset)
{
   this->m_idx -= offset;
   return *this;
}

} // namespace ds
