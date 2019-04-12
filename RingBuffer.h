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


///////////////////

namespace rb_details
{

// Modulus N addition.
inline constexpr int64_t addModN(int64_t val, int64_t offset, std::size_t n) noexcept
{
   return (val + offset) % n;
}

// Modulus N subtraction.
inline constexpr int64_t subModN(int64_t val, int64_t offset, std::size_t n) noexcept
{
   return (val - offset) % n;
}

} // namespace rb_details


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

   size_type size() const;
   constexpr size_type max_size() const;
   bool empty() const;
   bool full() const;
   T& at(size_type idx);
   const T& at(size_type idx) const;
   T& operator[](size_type idx);
   const T& operator[](size_type idx) const;
   void push(const T& val);
   T pop();
   void clear();
   void swap(RingBuffer& other);

   iterator begin();
   iterator end();
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator cbegin() const;
   const_iterator cend() const;
   reverse_iterator rbegin();
   reverse_iterator rend();
   const_reverse_iterator rbegin() const;
   const_reverse_iterator rend() const;
   const_reverse_iterator crbegin() const;
   const_reverse_iterator crend() const;

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
typename RingBuffer<T, N>::size_type RingBuffer<T, N>::size() const
{
   // Make sure the end is larger than the start index to avoid multiple cases.
   return rb_details::subModN(m_end + M, m_start, M);
}


template <typename T, std::size_t N>
constexpr typename RingBuffer<T, N>::size_type RingBuffer<T, N>::max_size() const
{
   return N;
}


template <typename T, std::size_t N> bool RingBuffer<T, N>::empty() const
{
   return (m_start == m_end);
}


template <typename T, std::size_t N> bool RingBuffer<T, N>::full() const
{
   return (size() == N);
}


template <typename T, std::size_t N> T& RingBuffer<T, N>::at(size_type idx)
{
   if (idx >= size())
      throw std::out_of_range("Invalid index into ring buffer.");
   return m_buffer[rb_details::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N> const T& RingBuffer<T, N>::at(size_type idx) const
{
   if (idx >= size())
      throw std::out_of_range("Invalid index into ring buffer.");
   return m_buffer[rb_details::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N> T& RingBuffer<T, N>::operator[](size_type idx)
{
   return m_buffer[rb_details::addModN(m_start, idx, M)];
}


template <typename T, std::size_t N>
const T& RingBuffer<T, N>::operator[](size_type idx) const
{
   return m_buffer[rb_details::addModN(m_start, idx, M)];
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
      m_start = rb_details::addModN(m_start, 1, M);
   }
   else
   {
      m_end = rb_details::addModN(m_end, 1, M);
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
typename RingBuffer<T, N>::iterator RingBuffer<T, N>::begin()
{
   return iterator(this, 0);
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::iterator RingBuffer<T, N>::end()
{
   return iterator(this, size());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::begin() const
{
   return cbegin();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::end() const
{
   return cend();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::cbegin() const
{
   return const_iterator(this, 0);
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_iterator RingBuffer<T, N>::cend() const
{
   return const_iterator(this, size());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::reverse_iterator RingBuffer<T, N>::rbegin()
{
   return reverse_iterator(end());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::reverse_iterator RingBuffer<T, N>::rend()
{
   return reverse_iterator(begin());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::rbegin() const
{
   return crbegin();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::rend() const
{
   return crend();
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::crbegin() const
{
   return const_reverse_iterator(cend());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::const_reverse_iterator RingBuffer<T, N>::crend() const
{
   return const_reverse_iterator(cbegin());
}


template <typename T, std::size_t N>
typename RingBuffer<T, N>::Index RingBuffer<T, N>::last() const
{
   if (empty())
      return m_end;
   return rb_details::subModN(m_end, 1, M);
}


///////////////////

// Const iterator for ring buffer. Does not allow changing elements.
template <typename RB> class RingBufferConstIterator
{
 public:
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename RB::value_type;
   using difference_type = typename RB::difference_type;
   using pointer = typename RB::const_pointer;
   using reference = typename RB::const_reference;

 public:
   RingBufferConstIterator() = default;
   RingBufferConstIterator(const RB* rb, std::size_t idx);
   ~RingBufferConstIterator() = default;
   RingBufferConstIterator(const RingBufferConstIterator&) = default;
   RingBufferConstIterator& operator=(const RingBufferConstIterator&) = default;
   RingBufferConstIterator(RingBufferConstIterator&&) = default;
   RingBufferConstIterator& operator=(RingBufferConstIterator&&) = default;

   const value_type& operator*() const;
   const value_type* operator->() const;
   RingBufferConstIterator& operator++();
   RingBufferConstIterator operator++(int);
   RingBufferConstIterator& operator--();
   RingBufferConstIterator operator--(int);
   RingBufferConstIterator& operator+=(const difference_type offset);
   RingBufferConstIterator operator+(const difference_type offset) const;
   RingBufferConstIterator& operator-=(const difference_type offset);
   RingBufferConstIterator operator-(const difference_type offset) const;
   // Difference of two iterators.
   difference_type operator-(const RingBufferConstIterator& rhs) const;
   bool operator==(const RingBufferConstIterator& rhs) const;
   bool operator!=(const RingBufferConstIterator& rhs) const;
   bool operator<(const RingBufferConstIterator& rhs) const;
   bool operator>(const RingBufferConstIterator& rhs) const;
   bool operator<=(const RingBufferConstIterator& rhs) const;
   bool operator>=(const RingBufferConstIterator& rhs) const;

 private:
   const RB* m_rb = nullptr;
   // Index to ring buffer element. This is an abstracted index, i.e. an index that
   // should be used on an ring buffer object. It is not an index into the internal
   // buffer of the ring buffer.
   std::size_t m_idx = 0;
};


template <typename RB>
RingBufferConstIterator<RB>::RingBufferConstIterator(const RB* rb, std::size_t idx)
: m_rb{rb}, m_idx{idx}
{
   assert(m_rb);
}

template <typename RB>
const typename RingBufferConstIterator<RB>::value_type& RingBufferConstIterator<RB>::
operator*() const
{
   return (*m_rb)[m_idx];
}

template <typename RB>
const typename RingBufferConstIterator<RB>::value_type* RingBufferConstIterator<RB>::
operator->() const
{
   return &((*m_rb)[m_idx]);
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
RingBufferConstIterator<RB>& RingBufferConstIterator<RB>::
operator+=(const difference_type offset)
{
   m_idx += offset;
   return *this;
}

template <typename RB>
RingBufferConstIterator<RB> RingBufferConstIterator<RB>::
operator+(const difference_type offset) const
{
   auto copy = *this;
   return (copy += offset);
}

template <typename RB>
RingBufferConstIterator<RB>& RingBufferConstIterator<RB>::
operator-=(const difference_type offset)
{
   m_idx -= offset;
   return *this;
}

template <typename RB>
RingBufferConstIterator<RB> RingBufferConstIterator<RB>::
operator-(const difference_type offset) const
{
   auto copy = *this;
   return (copy -= offset);
}

template <typename RB>
typename RingBufferConstIterator<RB>::difference_type RingBufferConstIterator<RB>::
operator-(const RingBufferConstIterator& rhs) const
{
   assert(m_rb == rhs.m_rb);
   return m_idx - rhs.m_idx;
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator==(const RingBufferConstIterator& rhs) const
{
   return (m_rb == rhs.m_rb && m_idx == rhs.m_idx);
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator!=(const RingBufferConstIterator& rhs) const
{
   return !(*this == rhs);
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator<(const RingBufferConstIterator& rhs) const
{
   assert(m_rb == rhs.m_rb);
   return (m_idx < rhs.m_idx);
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator>(const RingBufferConstIterator& rhs) const
{
   return !(*this < rhs || *this == rhs);
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator<=(const RingBufferConstIterator& rhs) const
{
   return !(*this > rhs);
}

template <typename RB>
bool RingBufferConstIterator<RB>::operator>=(const RingBufferConstIterator& rhs) const
{
   return !(*this < rhs);
}


///////////////////

// Iterator for ring buffer. Does allow changing elements.
template <typename RB> class RingBufferIterator
{
 public:
   using iterator_category = std::random_access_iterator_tag;
   using value_type = typename RB::value_type;
   using difference_type = typename RB::difference_type;
   using pointer = typename RB::pointer;
   using reference = typename RB::reference;

 public:
   RingBufferIterator() = default;
   RingBufferIterator(RB* rb, std::size_t idx);
   ~RingBufferIterator() = default;
   RingBufferIterator(const RingBufferIterator&) = default;
   RingBufferIterator& operator=(const RingBufferIterator&) = default;
   RingBufferIterator(RingBufferIterator&&) = default;
   RingBufferIterator& operator=(RingBufferIterator&&) = default;

   const value_type& operator*() const;
   value_type& operator*();
   const value_type* operator->() const;
   value_type* operator->();
   RingBufferIterator& operator++();
   RingBufferIterator operator++(int);
   RingBufferIterator& operator--();
   RingBufferIterator operator--(int);
   RingBufferIterator& operator+=(const difference_type offset);
   RingBufferIterator operator+(const difference_type offset) const;
   RingBufferIterator& operator-=(const difference_type offset);
   RingBufferIterator operator-(const difference_type offset) const;
   // Difference of two iterators.
   difference_type operator-(const RingBufferIterator& rhs) const;
   bool operator==(const RingBufferIterator& rhs) const;
   bool operator!=(const RingBufferIterator& rhs) const;
   bool operator<(const RingBufferIterator& rhs) const;
   bool operator>(const RingBufferIterator& rhs) const;
   bool operator<=(const RingBufferIterator& rhs) const;
   bool operator>=(const RingBufferIterator& rhs) const;

 private:
   RB* m_rb = nullptr;
   typename RB::Index m_idx = 0;
};


template <typename RB>
RingBufferIterator<RB>::RingBufferIterator(RB* rb, std::size_t idx) : m_rb{rb}, m_idx{idx}
{
   assert(m_rb);
}

template <typename RB>
const typename RingBufferIterator<RB>::value_type& RingBufferIterator<RB>::
operator*() const
{
   return (*m_rb)[m_idx];
}

template <typename RB>
typename RingBufferIterator<RB>::value_type& RingBufferIterator<RB>::operator*()
{
   return (*m_rb)[m_idx];
}

template <typename RB>
const typename RingBufferIterator<RB>::value_type* RingBufferIterator<RB>::
operator->() const
{
   return &((*m_rb)[m_idx]);
}

template <typename RB>
typename RingBufferIterator<RB>::value_type* RingBufferIterator<RB>::operator->()
{
   return &((*m_rb)[m_idx]);
}

template <typename RB> RingBufferIterator<RB>& RingBufferIterator<RB>::operator++()
{
   ++m_idx;
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
   --m_idx;
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
   m_idx += offset;
   return *this;
}

template <typename RB>
RingBufferIterator<RB> RingBufferIterator<RB>::
operator+(const difference_type offset) const
{
   auto copy = *this;
   return (copy += offset);
}

template <typename RB>
RingBufferIterator<RB>& RingBufferIterator<RB>::operator-=(const difference_type offset)
{
   m_idx -= offset;
   return *this;
}

template <typename RB>
RingBufferIterator<RB> RingBufferIterator<RB>::
operator-(const difference_type offset) const
{
   auto copy = *this;
   return (copy -= offset);
}

template <typename RB>
typename RingBufferIterator<RB>::difference_type RingBufferIterator<RB>::
operator-(const RingBufferIterator& rhs) const
{
   assert(m_rb == rhs.m_rb);
   return m_idx - rhs.m_idx;
}

template <typename RB>
bool RingBufferIterator<RB>::operator==(const RingBufferIterator& rhs) const
{
   return (m_rb == rhs.m_rb && m_idx == rhs.m_idx);
}

template <typename RB>
bool RingBufferIterator<RB>::operator!=(const RingBufferIterator& rhs) const
{
   return !(*this == rhs);
}

template <typename RB>
bool RingBufferIterator<RB>::operator<(const RingBufferIterator& rhs) const
{
   assert(m_rb == rhs.m_rb);
   return (m_idx < rhs.m_idx);
}

template <typename RB>
bool RingBufferIterator<RB>::operator>(const RingBufferIterator& rhs) const
{
   return !(*this < rhs || *this == rhs);
}

template <typename RB>
bool RingBufferIterator<RB>::operator<=(const RingBufferIterator& rhs) const
{
   return !(*this > rhs);
}

template <typename RB>
bool RingBufferIterator<RB>::operator>=(const RingBufferIterator& rhs) const
{
   return !(*this < rhs);
}
