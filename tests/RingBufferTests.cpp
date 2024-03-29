#include "RingBuffer.h"
#include "TestUtil.h"
#include <iostream>
#include <string>

using namespace ds;


///////////////////

namespace
{

void testRingBufferDefaultCtor()
{
   {
      const std::string caseLabel{"RingBuffer default ctor"};
      RingBuffer<int, 10> rb;
      VERIFY(rb.empty(), caseLabel);
      VERIFY(rb.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer default ctor for N=0"};
      RingBuffer<double, 0> rb;
      VERIFY(rb.empty(), caseLabel);
      VERIFY(rb.capacity() == 0, caseLabel);
   }
}


void testRingBufferIteratorCtor()
{
   {
      const std::string caseLabel{"RingBuffer iterator ctor"};
      std::vector<int> seq{1, 2, 3};
      RingBuffer<int, 10> rb{seq.begin(), seq.end()};
      VERIFY(rb.size() == 3, caseLabel);
      VERIFY(rb[0] == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer iterator ctor for wrapped buffer"};
      std::vector<int> seq{1, 2, 3, 4, 5};
      RingBuffer<int, 4> rb{seq.begin(), seq.end()};
      VERIFY(rb.size() == 4, caseLabel);
      VERIFY(rb[0] == 2, caseLabel);
      VERIFY(rb[3] == 5, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer iterator ctor for empty sequence"};
      std::vector<int> seq{};
      RingBuffer<int, 4> rb{seq.begin(), seq.end()};
      VERIFY(rb.size() == 0, caseLabel);
   }
}


void testRingBufferInitializerListCtor()
{
   {
      const std::string caseLabel{"RingBuffer initializer list ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      VERIFY(rb.size() == 3, caseLabel);
      VERIFY(rb[0] == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer initializer list ctor for wrapped buffer"};
      RingBuffer<int, 4> rb{1, 2, 3, 4, 5};
      VERIFY(rb.size() == 4, caseLabel);
      VERIFY(rb[0] == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer initializer list ctor for empty initializer list"};
      std::initializer_list<int> ilist{};
      RingBuffer<int, 4> rb{ilist};
      VERIFY(rb.size() == 0, caseLabel);
   }
}


void testRingBufferCopyCtor()
{
   {
      const std::string caseLabel{"RingBuffer copy ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto copy{rb};
      VERIFY(copy.size() == 3, caseLabel);
      VERIFY(copy[0] == 1, caseLabel);
   }
}


void testRingBufferMoveCtor()
{
   {
      const std::string caseLabel{"RingBuffer move ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto moved{std::move(rb)};
      VERIFY(moved.size() == 3, caseLabel);
      VERIFY(moved[0] == 1, caseLabel);
   }
}


void testRingBufferCopyAssignment()
{
   {
      const std::string caseLabel{"RingBuffer copy assignment operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBuffer<int, 10> copy;
      copy = rb;
      VERIFY(copy.size() == 3, caseLabel);
      VERIFY(copy[0] == 1, caseLabel);
   }
}


void testRingBufferMoveAssignment()
{
   {
      const std::string caseLabel{"RingBuffer move assignment operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBuffer<int, 10> moved;
      moved = std::move(rb);
      VERIFY(moved.size() == 3, caseLabel);
      VERIFY(moved[0] == 1, caseLabel);
   }
}


void testRingBufferSize()
{
   {
      const std::string caseLabel{"RingBuffer::size()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      VERIFY(rb.size() == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::size() for wrapped buffer"};
      RingBuffer<int, 4> rb{1, 2, 3, 4, 5, 6};
      VERIFY(rb.size() == 4, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::size() for empty buffer"};
      RingBuffer<int, 10> rb;
      VERIFY(rb.size() == 0, caseLabel);
   }
}


void testRingBufferCapacity()
{
   {
      const std::string caseLabel{"RingBuffer::capacity() "};
      RingBuffer<int, 10> rb{1, 2, 3};
      VERIFY(rb.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::capacity() for empty buffer"};
      RingBuffer<bool, 10> rb;
      VERIFY(rb.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::capacity() for large size"};
      RingBuffer<int, 1000> rb;
      VERIFY(rb.capacity() == 1000, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::capacity() for wrapped buffer"};
      RingBuffer<int, 2> rb{1, 2, 3};
      VERIFY(rb.capacity() == 2, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::capacity() for N=0"};
      RingBuffer<int, 0> rb;
      VERIFY(rb.capacity() == 0, caseLabel);
   }
}


void testRingBufferEmpty()
{
   {
      const std::string caseLabel{"RingBuffer::empty() for empty buffer"};
      RingBuffer<int, 10> rb;
      VERIFY(rb.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::empty() for non-empty buffer"};
      RingBuffer<float, 100> rb{10.0};
      VERIFY(!rb.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::empty() for wrapped buffer"};
      RingBuffer<int, 5> rb{1, 2, 3, 4, 5, 6, 7};
      VERIFY(!rb.empty(), caseLabel);
   }
}


void testRingBufferFull()
{
   {
      const std::string caseLabel{"RingBuffer::full() for full buffer"};
      RingBuffer<int, 3> rb{1, 2, 3};
      VERIFY(rb.full(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::full() for empty buffer"};
      RingBuffer<int, 10> rb;
      VERIFY(!rb.full(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::full() for wrapped buffer"};
      RingBuffer<int, 5> rb{1, 2, 3, 4, 5, 6, 7};
      VERIFY(rb.full(), caseLabel);
   }
}


void testRingBufferAt()
{
   {
      const std::string caseLabel{"RingBuffer::at() for access"};
      RingBuffer<int, 3> rb{1, 2, 3};
      VERIFY(rb.at(0) == 1, caseLabel);
      VERIFY(rb.at(1) == 2, caseLabel);
      VERIFY(rb.at(2) == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::at() for modification"};
      RingBuffer<int, 3> rb{1, 2, 3};
      rb.at(0) = 10;
      VERIFY(rb.at(0) == 10, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::at() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      VERIFY(rb.at(0) == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::at() for access past physical size"};
      try
      {
         RingBuffer<int, 3> rb{1, 2};
         rb.at(10);
         // Should not get here.
         VERIFY(false, caseLabel);
      }
      catch (...)
      {
         // Expected to get here.
      }
   }
   {
      const std::string caseLabel{"RingBuffer::at() for access past logical size"};
      try
      {
         RingBuffer<int, 3> rb{1, 2};
         rb.at(2);
         // Should not get here.
         VERIFY(false, caseLabel);
      }
      catch (...)
      {
         // Expected to get here.
      }
   }
}


void testRingBufferAtConst()
{
   {
      const std::string caseLabel{"RingBuffer::at() const"};
      const RingBuffer<int, 3> rb{1, 2, 3};
      VERIFY(rb.at(0) == 1, caseLabel);
      VERIFY(rb.at(1) == 2, caseLabel);
      VERIFY(rb.at(2) == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::at() const for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      VERIFY(rb.at(0) == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::at() const for access past physical size"};
      try
      {
         const RingBuffer<int, 3> rb{1, 2};
         rb.at(10);
         // Should not get here.
         VERIFY(false, caseLabel);
      }
      catch (...)
      {
         // Expected to get here.
      }
   }
   {
      const std::string caseLabel{"RingBuffer::at() const for access past logical size"};
      try
      {
         const RingBuffer<int, 3> rb{1, 2};
         rb.at(2);
         // Should not get here.
         VERIFY(false, caseLabel);
      }
      catch (...)
      {
         // Expected to get here.
      }
   }
}


void testRingBufferIndexOperator()
{
   {
      const std::string caseLabel{"RingBuffer index operator for access"};
      RingBuffer<int, 3> rb{1, 2, 3};
      VERIFY(rb[0] == 1, caseLabel);
      VERIFY(rb[1] == 2, caseLabel);
      VERIFY(rb[2] == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer index operator for modification"};
      RingBuffer<int, 3> rb{1, 2, 3};
      rb[0] = 10;
      VERIFY(rb[0] == 10, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer index operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      VERIFY(rb[0] == 3, caseLabel);
   }
}


void testRingBufferIndexOperatorConst()
{
   {
      const std::string caseLabel{"RingBuffer const index operator"};
      const RingBuffer<int, 3> rb{1, 2, 3};
      VERIFY(rb[0] == 1, caseLabel);
      VERIFY(rb[1] == 2, caseLabel);
      VERIFY(rb[2] == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer const index operator for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      VERIFY(rb[0] == 3, caseLabel);
   }
}


void testRingBufferPush()
{
   {
      const std::string caseLabel{"RingBuffer::push()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      rb.push(4);
      VERIFY(rb.size() == 4, caseLabel);
      VERIFY(rb[3] == 4, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::push() for empty buffer"};
      RingBuffer<float, 10> rb;
      rb.push(1.0);
      VERIFY(rb.size() == 1, caseLabel);
      VERIFY(rb[0] == 1.0, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::push() for full buffer"};
      RingBuffer<short, 3> rb{1, 2, 3};
      rb.push(4);
      VERIFY(rb.size() == 3, caseLabel);
      VERIFY(rb[0] == 2, caseLabel);
      VERIFY(rb[2] == 4, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::push() for wrapped buffer"};
      RingBuffer<short, 3> rb{1, 2, 3, 4};
      rb.push(5);
      VERIFY(rb.size() == 3, caseLabel);
      VERIFY(rb[0] == 3, caseLabel);
      VERIFY(rb[2] == 5, caseLabel);
   }
}


void testRingBufferPop()
{
   {
      const std::string caseLabel{"RingBuffer::pop()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto val = rb.pop();
      VERIFY(val == 3, caseLabel);
      VERIFY(rb.size() == 2, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::pop() for empty buffer"};
      try
      {
         RingBuffer<float, 10> rb;
         rb.pop();
         // Should not get here;
         VERIFY(false, caseLabel);
      }
      catch (...)
      {
         // Expected to get here.
      }
   }
   {
      const std::string caseLabel{"RingBuffer::pop() for full buffer"};
      RingBuffer<short, 3> rb{1, 2, 3};
      auto val = rb.pop();
      VERIFY(val == 3, caseLabel);
      VERIFY(rb.size() == 2, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::pop() for wrapped buffer"};
      RingBuffer<short, 3> rb{1, 2, 3, 4};
      auto val = rb.pop();
      VERIFY(val == 4, caseLabel);
      VERIFY(rb.size() == 2, caseLabel);
   }
}


void testRingBufferClear()
{
   {
      const std::string caseLabel{"RingBuffer::clear()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      rb.clear();
      VERIFY(rb.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::clear() for empty buffer"};
      RingBuffer<int, 10> rb;
      rb.clear();
      VERIFY(rb.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::clear() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      rb.clear();
      VERIFY(rb.empty(), caseLabel);
   }
}


void testRingBufferSwap()
{
   {
      const std::string caseLabel{"RingBuffer::swap()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBuffer<int, 10> other{3, 4, 5, 6};
      rb.swap(other);
      VERIFY(rb.size() == 4, caseLabel);
      VERIFY(rb[0] == 3, caseLabel);
      VERIFY(other.size() == 3, caseLabel);
      VERIFY(other[0] == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::swap() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 3> other{3, 4, 5, 6, 7, 8};
      rb.swap(other);
      VERIFY(rb.size() == 3, caseLabel);
      VERIFY(rb[0] == 6, caseLabel);
      VERIFY(other.size() == 3, caseLabel);
      VERIFY(other[0] == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::swap() for empty buffer"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 10> other;
      rb.swap(other);
      VERIFY(rb.size() == 0, caseLabel);
      VERIFY(other.size() == 5, caseLabel);
      VERIFY(other[0] == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::swap() with self"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      rb.swap(rb);
      VERIFY(rb.size() == 5, caseLabel);
      VERIFY(rb[0] == 1, caseLabel);
   }
}


void testRingBufferBegin()
{
   {
      const std::string caseLabel{"RingBuffer::begin()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto first = rb.begin();
      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::begin() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto first = rb.begin();
      VERIFY(first == rb.end(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::begin() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto first = rb.begin();
      VERIFY(*first == 3, caseLabel);
   }
}


void testRingBufferEnd()
{
   {
      const std::string caseLabel{"RingBuffer::end()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto end = rb.end();
      VERIFY(end != rb.begin(), caseLabel);
      VERIFY(end != (rb.begin() + 1), caseLabel);
      VERIFY(end != (rb.begin() + 2), caseLabel);
      VERIFY(end == (rb.begin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::end() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto end = rb.end();
      VERIFY(end == rb.begin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::end() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto end = rb.end();
      VERIFY(end != rb.begin(), caseLabel);
      VERIFY(end != (rb.begin() + 1), caseLabel);
      VERIFY(end != (rb.begin() + 2), caseLabel);
      VERIFY(end == (rb.begin() + 3), caseLabel);
   }
}


void testRingBufferBeginConst()
{
   {
      const std::string caseLabel{"RingBuffer::begin() const"};
      const RingBuffer<int, 10> rb{1, 2, 3};
      auto first = rb.begin();
      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::begin() const for empty buffer"};
      const RingBuffer<int, 10> rb{};
      auto first = rb.begin();
      VERIFY(first == rb.end(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::begin() const for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto first = rb.begin();
      VERIFY(*first == 3, caseLabel);
   }
}


void testRingBufferEndConst()
{
   {
      const std::string caseLabel{"RingBuffer::end() const"};
      const RingBuffer<int, 10> rb{1, 2, 3};
      auto end = rb.end();
      VERIFY(end != rb.begin(), caseLabel);
      VERIFY(end != (rb.begin() + 1), caseLabel);
      VERIFY(end != (rb.begin() + 2), caseLabel);
      VERIFY(end == (rb.begin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::end() const for empty buffer"};
      const RingBuffer<int, 10> rb{};
      auto end = rb.end();
      VERIFY(end == rb.begin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::end() const for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto end = rb.end();
      VERIFY(end != rb.begin(), caseLabel);
      VERIFY(end != (rb.begin() + 1), caseLabel);
      VERIFY(end != (rb.begin() + 2), caseLabel);
      VERIFY(end == (rb.begin() + 3), caseLabel);
   }
}


void testRingBufferCBegin()
{
   {
      const std::string caseLabel{"RingBuffer::cbegin()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto first = rb.cbegin();
      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::cbegin() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto first = rb.cbegin();
      VERIFY(first == rb.cend(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::cbegin() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto first = rb.cbegin();
      VERIFY(*first == 3, caseLabel);
   }
}


void testRingBufferCEnd()
{
   {
      const std::string caseLabel{"RingBuffer::cend()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto end = rb.cend();
      VERIFY(end != rb.cbegin(), caseLabel);
      VERIFY(end != (rb.cbegin() + 1), caseLabel);
      VERIFY(end != (rb.cbegin() + 2), caseLabel);
      VERIFY(end == (rb.cbegin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::cend() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto end = rb.cend();
      VERIFY(end == rb.cbegin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::cend() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto end = rb.cend();
      VERIFY(end != rb.cbegin(), caseLabel);
      VERIFY(end != (rb.cbegin() + 1), caseLabel);
      VERIFY(end != (rb.cbegin() + 2), caseLabel);
      VERIFY(end == (rb.cbegin() + 3), caseLabel);
   }
}


void testRingBufferRBegin()
{
   {
      const std::string caseLabel{"RingBuffer::rbegin()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto rfirst = rb.rbegin();
      VERIFY(*rfirst == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rbegin() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto rfirst = rb.rbegin();
      VERIFY(rfirst == rb.rend(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rbegin() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rfirst = rb.rbegin();
      VERIFY(*rfirst == 5, caseLabel);
   }
}


void testRingBufferREnd()
{
   {
      const std::string caseLabel{"RingBuffer::rend()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto rend = rb.rend();
      VERIFY(rend != rb.rbegin(), caseLabel);
      VERIFY(rend != (rb.rbegin() + 1), caseLabel);
      VERIFY(rend != (rb.rbegin() + 2), caseLabel);
      VERIFY(rend == (rb.rbegin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rend() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto rend = rb.rend();
      VERIFY(rend == rb.rbegin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rend() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rend = rb.rend();
      VERIFY(rend != rb.rbegin(), caseLabel);
      VERIFY(rend != (rb.rbegin() + 1), caseLabel);
      VERIFY(rend != (rb.rbegin() + 2), caseLabel);
      VERIFY(rend == (rb.rbegin() + 3), caseLabel);
   }
}


void testRingBufferRBeginConst()
{
   {
      const std::string caseLabel{"RingBuffer::rbegin() const"};
      const RingBuffer<int, 10> rb{1, 2, 3};
      auto rfirst = rb.rbegin();
      VERIFY(*rfirst == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rbegin() const for empty buffer"};
      const RingBuffer<int, 10> rb{};
      auto rfirst = rb.rbegin();
      VERIFY(rfirst == rb.rend(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rbegin() const for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rfirst = rb.rbegin();
      VERIFY(*rfirst == 5, caseLabel);
   }
}


void testRingBufferREndConst()
{
   {
      const std::string caseLabel{"RingBuffer::rend() const"};
      const RingBuffer<int, 10> rb{1, 2, 3};
      auto rend = rb.rend();
      VERIFY(rend != rb.rbegin(), caseLabel);
      VERIFY(rend != (rb.rbegin() + 1), caseLabel);
      VERIFY(rend != (rb.rbegin() + 2), caseLabel);
      VERIFY(rend == (rb.rbegin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rend() const for empty buffer"};
      const RingBuffer<int, 10> rb{};
      auto rend = rb.rend();
      VERIFY(rend == rb.rbegin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::rend() const for wrapped buffer"};
      const RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rend = rb.rend();
      VERIFY(rend != rb.rbegin(), caseLabel);
      VERIFY(rend != (rb.rbegin() + 1), caseLabel);
      VERIFY(rend != (rb.rbegin() + 2), caseLabel);
      VERIFY(rend == (rb.rbegin() + 3), caseLabel);
   }
}


void testRingBufferCRBegin()
{
   {
      const std::string caseLabel{"RingBuffer::crbegin()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto rfirst = rb.crbegin();
      VERIFY(*rfirst == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::crbegin() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto rfirst = rb.crbegin();
      VERIFY(rfirst == rb.crend(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::crbegin() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rfirst = rb.crbegin();
      VERIFY(*rfirst == 5, caseLabel);
   }
}


void testRingBufferCREnd()
{
   {
      const std::string caseLabel{"RingBuffer::crend()"};
      RingBuffer<int, 10> rb{1, 2, 3};
      auto rend = rb.crend();
      VERIFY(rend != rb.crbegin(), caseLabel);
      VERIFY(rend != (rb.crbegin() + 1), caseLabel);
      VERIFY(rend != (rb.crbegin() + 2), caseLabel);
      VERIFY(rend == (rb.crbegin() + 3), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::crend() for empty buffer"};
      RingBuffer<int, 10> rb;
      auto rend = rb.crend();
      VERIFY(rend == rb.crbegin(), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::crend() for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 4, 5};
      auto rend = rb.crend();
      VERIFY(rend != rb.crbegin(), caseLabel);
      VERIFY(rend != (rb.crbegin() + 1), caseLabel);
      VERIFY(rend != (rb.crbegin() + 2), caseLabel);
      VERIFY(rend == (rb.crbegin() + 3), caseLabel);
   }
}

void testRingBufferConstIteratorCtor()
{
   {
      const std::string caseLabel{"RingBufferConstIterator default ctor"};
      RingBufferConstIterator<RingBuffer<int, 10>> cit;
      // Nothing to VERIFY because a default initialized object cannot be used
      // before it gets assigned to.
   }
}


void testRingBufferConstIteratorCopyCtor()
{
   {
      const std::string caseLabel{"RingBufferConstIterator copy ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      auto copy{cit};
      VERIFY(copy == cit, caseLabel);
   }
}


void testRingBufferConstIteratorMoveCtor()
{
   {
      const std::string caseLabel{"RingBufferConstIterator move ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      auto moved{std::move(cit)};
      VERIFY(*moved == 2, caseLabel);
   }
}

void testRingBufferConstIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"RingBufferConstIterator copy assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      auto copy = cit;
      VERIFY(copy == cit, caseLabel);
   }
}


void testRingBufferConstIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"RingBufferConstIterator move assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      auto moved = std::move(cit);
      VERIFY(*moved == 2, caseLabel);
   }
}


void testRingBufferConstIteratorDerefOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      VERIFY(*cit == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferConstIterator<RingBuffer<int, 3>> cit{rb.cbegin() + 1};
      VERIFY(*cit == 7, caseLabel);
   }
}


void testRingBufferConstIteratorArrowOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator arrow operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b"};
      RingBufferConstIterator<RingBuffer<std::string, 10>> cit{rb.cbegin()};
      VERIFY(cit->length() == s.length(), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator arrow operator for wrapped buffer"};
      const std::string s = "test a";
      RingBuffer<std::string, 3> rb{"aaa", "b", "cc", "dddd", s};
      RingBufferConstIterator<RingBuffer<std::string, 3>> cit{rb.cbegin() + 2};
      VERIFY(cit->length() == s.length(), caseLabel);
   }
}


void testRingBufferConstIteratorSubscriptOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator subscript operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b", "abc", "1234"};
      const RingBufferConstIterator<RingBuffer<std::string, 10>> cit{rb.cbegin()};
      VERIFY(cit[2] == "abc", caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator const subscript operator for wrapped buffer"};
      const std::string s = "test a";
      RingBuffer<std::string, 3> rb{s, "b", "abc", "1234", "xyz"};
      const RingBufferConstIterator<RingBuffer<std::string, 3>> cit{rb.cbegin()};
      VERIFY(cit[2] == "xyz", caseLabel);
   }
}


void testRingBufferConstIteratorPrefixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator prefix behavior of prefix increment operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin();
      auto res = ++it;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator prefix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin();
      for (auto it = rb.begin(), end = rb.end(); it != end; ++it, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferConstIterator prefix increment operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - rb.size();
      for (auto it = rb.begin(), end = rb.end(); it != end; ++it, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
}


void testRingBufferConstIteratorPostfixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix behavior of postfix increment operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin();
      auto res = it++;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin();
      for (auto it = rb.begin(), end = rb.end(); it != end; it++, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix increment operator for "
         "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - rb.size();
      for (auto it = rb.begin(), end = rb.end(); it != end; it++, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
}


void testRingBufferConstIteratorPrefixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator prefix behavior of prefix decrement operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin() + 1;
      auto res = --it;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator prefix decrement operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           --it, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{"RingBufferConstIterator prefix decrement operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           --it, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
}

void testRingBufferConstIteratorPostfixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix behavior of postfix decrement operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin() + 1;
      auto res = it--;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix decrement operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           it--, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator postfix decrement operator for "
         "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           it--, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
}


void testRingBufferConstIteratorAddAssignmentOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator assignment addition operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      it += 2;
      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator assignment addition operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      it += -2;
      VERIFY(*it == 2, caseLabel);
   }
}


void testRingBufferConstIteratorAdditionOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator addition operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      auto res = it + 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator addition operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      auto res = it + (-2);
      VERIFY(*res == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator addition operator with difference as first operand"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      typename RingBuffer<int, 10>::const_iterator res = 2 + it;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferConstIterator addition operator for "
                                  "negative value with difference as first operand"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      typename RingBuffer<int, 10>::const_iterator res = (-2) + it;
      VERIFY(*res == 2, caseLabel);
   }
}


void testRingBufferConstIteratorSubAssignmentOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator assignment subtraction operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 4;
      it -= 2;
      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator assignment subtraction operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      it -= -3;
      VERIFY(*it == 4, caseLabel);
   }
}


void testRingBufferConstIteratorSubtractionOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator subtraction operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 4;
      auto res = it - 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator subtraction operator negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      auto res = it - (-3);
      VERIFY(*res == 4, caseLabel);
   }
}


void testRingBufferConstIteratorDifferenceOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator difference operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 1;
      auto b = rb.begin() + 3;
      auto diff = b - a;
      VERIFY(diff == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator difference operator for negative differences"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 1;
      auto b = rb.begin() + 3;
      auto diff = a - b;
      VERIFY(diff == -2, caseLabel);
   }
}


void testRingBufferConstIteratorEqualityOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for equality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 2;
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for inequal indices"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for inequal containers"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      const RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb2.begin() + 2;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for end iterators"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.end();
      auto b = rb.end();
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for end and last iterators"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 4;
      auto b = rb.end();
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator equality operator for unassigned iterators"};
      RingBufferConstIterator<RingBuffer<int, 10>> a, b;
      VERIFY(a == b, caseLabel);
   }
}


void testRingBufferConstIteratorInequalityOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator inequality operator for inequality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator inequality operator for equality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 2;
      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator inequality operator for inequal containers"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      const RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb2.begin() + 2;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator inequality operator for unassigned iterators"};
      RingBufferConstIterator<RingBuffer<int, 10>> a, b;
      VERIFY(!(a != b), caseLabel);
   }
}


void testRingBufferConstIteratorLessThanOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator less-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator less-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(!(a < b), caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator less-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(a < e, caseLabel);
      VERIFY(b < e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstIteratorGreaterThanOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator greater-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      VERIFY(a > b, caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator greater-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(!(a > b), caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator greater-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(e > a, caseLabel);
      VERIFY(e > b, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstIteratorLessThanEqualOperator()
{
   {
      const std::string caseLabel{"RingBufferConstIterator less-or-equal-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator less-or-equal-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(a <= b, caseLabel);
      VERIFY(b <= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator less-or-equal-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(a <= e, caseLabel);
      VERIFY(b <= e, caseLabel);
      VERIFY(e <= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstIteratorGreaterThanEqualOperator()
{
   {
      const std::string caseLabel{
         "RingBufferConstIterator greater-or-equal-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      VERIFY(a >= b, caseLabel);
      VERIFY(!(b >= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator greater-or-equal-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(a >= b, caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator greater-or-equal-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(e >= a, caseLabel);
      VERIFY(e >= b, caseLabel);
      VERIFY(e >= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstIteratorSwap()
{
   {
      const std::string caseLabel{"RingBufferConstIterator swap"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      swap(a, b);
      VERIFY(*a == 3, caseLabel);
      VERIFY(*b == 4, caseLabel);
   }
}


void testRingBufferIteratorDefaultCtor()
{
   {
      const std::string caseLabel{"RingBufferIterator default ctor"};
      RingBufferIterator<RingBuffer<int, 10>> it;
      // Nothing to VERIFY because a default initialized object cannot be used
      // before it gets assigned to.
   }
}


void testRingBufferIteratorRBAndIndexCtor()
{
   {
      const std::string caseLabel{"RingBufferIterator ctor for ring buffer and index"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin()};
      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator ctor for ring buffer and last index"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 2};
      VERIFY(*it == 3, caseLabel);
   }
}


void testRingBufferIteratorCopyCtor()
{
   {
      const std::string caseLabel{"RingBufferIterator copy ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      auto copy{it};
      VERIFY(copy == it, caseLabel);
   }
}


void testRingBufferIteratorMoveCtor()
{
   {
      const std::string caseLabel{"RingBufferIterator move ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      auto moved{std::move(it)};
      VERIFY(*moved == 2, caseLabel);
   }
}


void testRingBufferIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"RingBufferIterator copy assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      auto copy = it;
      VERIFY(copy == it, caseLabel);
   }
}


void testRingBufferIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"RingBufferIterator move assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      auto moved = std::move(it);
      VERIFY(*moved == 2, caseLabel);
   }
}


void testRingBufferIteratorConstDerefOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator const dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      VERIFY(*it == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator const dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferIterator<RingBuffer<int, 3>> it{rb.begin() + 1};
      VERIFY(*it == 7, caseLabel);
   }
}


void testRingBufferIteratorDerefOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      *it = 10;
      VERIFY(*it == 10, caseLabel);
      VERIFY(rb[1] == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferIterator<RingBuffer<int, 3>> it{rb.begin() + 1};
      *it = 10;
      VERIFY(*it == 10, caseLabel);
      VERIFY(rb[1] == 10, caseLabel);
   }
}


void testRingBufferIteratorConstArrowOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator const arrow operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b"};
      RingBufferIterator<RingBuffer<std::string, 10>> it{rb.begin()};
      VERIFY(it->length() == s.length(), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator const arrow operator for wrapped buffer"};
      const std::string s = "test a";
      RingBuffer<std::string, 3> rb{"aaa", "b", "cc", "dddd", s};
      RingBufferIterator<RingBuffer<std::string, 3>> it{rb.begin() + 2};
      VERIFY(it->length() == s.length(), caseLabel);
   }
}


void testRingBufferIteratorArrowOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator arrow operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b"};
      RingBufferIterator<RingBuffer<std::string, 10>> it{rb.begin()};
      it->append("**");
      VERIFY(*it == "test a**", caseLabel);
      VERIFY(rb[0] == "test a**", caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator const arrow operator for wrapped buffer"};
      RingBuffer<std::string, 3> rb{"aaa", "b", "cc", "dddd", "ee"};
      RingBufferIterator<RingBuffer<std::string, 3>> it{rb.begin() + 2};
      it->append("**");
      VERIFY(*it == "ee**", caseLabel);
      VERIFY(rb[2] == "ee**", caseLabel);
   }
}


void testRingBufferIteratorSubscriptOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator subscript operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b", "abc", "1234"};
      RingBufferIterator<RingBuffer<std::string, 10>> cit{rb.begin()};
      VERIFY(cit[2] == "abc", caseLabel);
      cit[2] = std::string{"cba"};
      VERIFY(cit[2] == "cba", caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator subscript operator for wrapped buffer"};
      const std::string s = "test a";
      RingBuffer<std::string, 3> rb{s, "b", "abc", "1234", "xyz"};
      RingBufferIterator<RingBuffer<std::string, 3>> cit{rb.begin()};
      VERIFY(cit[2] == "xyz", caseLabel);
      cit[2] = "zyx";
      VERIFY(cit[2] == "zyx", caseLabel);
   }
}


void testRingBufferIteratorConstSubscriptOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator const subscript operator"};
      const std::string s = "test a";
      RingBuffer<std::string, 10> rb{s, "b", "abc", "1234"};
      const RingBufferIterator<RingBuffer<std::string, 10>> cit{rb.begin()};
      VERIFY(cit[2] == "abc", caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferConstIterator const subscript operator for wrapped buffer"};
      const std::string s = "test a";
      RingBuffer<std::string, 3> rb{s, "b", "abc", "1234", "xyz"};
      const RingBufferIterator<RingBuffer<std::string, 3>> cit{rb.begin()};
      VERIFY(cit[2] == "xyz", caseLabel);
   }
}


void testRingBufferIteratorPrefixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBufferIterator prefix behavior of prefix increment operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin();
      auto res = ++it;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator prefix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin();
      for (auto it = rb.begin(), end = rb.end(); it != end; ++it, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferIterator prefix increment operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - rb.size();
      for (auto it = rb.begin(), end = rb.end(); it != end; ++it, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
}


void testRingBufferIteratorPostfixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBufferIterator postfix behavior of postfix increment operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin();
      auto res = it++;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator postfix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin();
      for (auto it = rb.begin(), end = rb.end(); it != end; it++, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferIterator postfix increment operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - rb.size();
      for (auto it = rb.begin(), end = rb.end(); it != end; it++, ++dataIt)
         VERIFY(*it == *dataIt, caseLabel);
   }
}


void testRingBufferIteratorPrefixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBufferIterator prefix behavior of prefix decrement operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin() + 1;
      auto res = --it;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator prefix decrement operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           --it, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{"RingBufferIterator prefix decrement operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           --it, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
}


void testRingBufferIteratorPostfixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBufferIterator postfix behavior of postfix decrement operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto it = rb.begin() + 1;
      auto res = it--;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator postfix decrement operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           it--, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{"RingBufferIterator postfix decrement operator for "
                                  "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto dataIt = data.begin() + data.size() - 1;
      for (auto it = rb.begin() + rb.size() - 1, start = rb.begin(); it != start;
           it--, --dataIt)
      {
         VERIFY(*it == *dataIt, caseLabel);
      }
   }
}


void testRingBufferIteratorAddAssignmentOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator assignment addition operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      it += 2;
      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator assignment addition operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      it += -2;
      VERIFY(*it == 2, caseLabel);
   }
}


void testRingBufferIteratorAdditionOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator addition operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      auto res = it + 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator addition operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      auto res = it + (-2);
      VERIFY(*res == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator addition operator with difference type as first operand"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      typename RingBuffer<int, 10>::iterator res = 2 + it;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferIterator addition operator for negative "
                                  "value with difference type as first operand"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 3;
      typename RingBuffer<int, 10>::iterator res = (-2) + it;
      VERIFY(*res == 2, caseLabel);
   }
}


void testRingBufferIteratorSubAssignmentOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator assignment subtraction operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 4;
      it -= 2;
      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator assignment subtraction operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      it -= -3;
      VERIFY(*it == 4, caseLabel);
   }
}


void testRingBufferIteratorSubtractionOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator subtraction operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin() + 4;
      auto res = it - 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator subtraction operator negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto it = rb.begin();
      auto res = it - (-3);
      VERIFY(*res == 4, caseLabel);
   }
}


void testRingBufferIteratorDifferenceOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator difference operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 1;
      auto b = rb.begin() + 3;
      auto diff = b - a;
      VERIFY(diff == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator difference operator for negative differences"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 1;
      auto b = rb.begin() + 3;
      auto diff = a - b;
      VERIFY(diff == -2, caseLabel);
   }
}


void testRingBufferIteratorEqualityOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator equality operator for equality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 2;
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator equality operator for inequal indices"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator equality operator for inequal containers"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb2.begin() + 2;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator equality operator for end iterators"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.end();
      auto b = rb.end();
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator equality operator for end and last iterators"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 4;
      auto b = rb.end();
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator equality operator for unassigned iterators"};
      RingBufferIterator<RingBuffer<int, 10>> a, b;
      VERIFY(a == b, caseLabel);
   }
}


void testRingBufferIteratorInequalityOperator()
{
   {
      const std::string caseLabel{
         "RingBufferIterator inequality operator for inequality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"RingBufferIterator inequality operator for equality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 2;
      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator inequality operator for inequal containers"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb2.begin() + 2;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator inequality operator for unassigned iterators"};
      RingBufferIterator<RingBuffer<int, 10>> a, b;
      VERIFY(!(a != b), caseLabel);
   }
}


void testRingBufferIteratorLessThanOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator less-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator less-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(!(a < b), caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator less-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(a < e, caseLabel);
      VERIFY(b < e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferIteratorGreaterThanOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator greater-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      VERIFY(a > b, caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator greater-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(!(a > b), caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator greater-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(e > a, caseLabel);
      VERIFY(e > b, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}

void testRingBufferIteratorLessThanEqualOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator less-or-equal-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 3;
      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator less-or-equal-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(a <= b, caseLabel);
      VERIFY(b <= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator less-or-equal-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(a <= e, caseLabel);
      VERIFY(b <= e, caseLabel);
      VERIFY(e <= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferIteratorGreaterThanEqualOperator()
{
   {
      const std::string caseLabel{"RingBufferIterator greater-or-equal-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      VERIFY(a >= b, caseLabel);
      VERIFY(!(b >= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator greater-or-equal-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = a;
      VERIFY(a >= b, caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBufferIterator greater-or-equal-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 2;
      auto b = rb.begin() + 4;
      auto e = rb.end();
      VERIFY(e >= a, caseLabel);
      VERIFY(e >= b, caseLabel);
      VERIFY(e >= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferIteratorSwap()
{
   {
      const std::string caseLabel{"RingBufferConstIterator swap"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.begin() + 3;
      auto b = rb.begin() + 2;
      swap(a, b);
      VERIFY(*a == 3, caseLabel);
      VERIFY(*b == 4, caseLabel);
   }
}


void testRingBufferConstReverseIteratorCtor()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator default ctor"};
      RingBuffer<int, 10>::const_reverse_iterator crit;
      // Nothing to VERIFY because a default initialized object cannot be used
      // before it gets assigned to.
   }
}


void testRingBufferConstReverseIteratorFromIterator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator ctor for iterator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      VERIFY(*crit == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator ctor for end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBuffer<int, 10>::const_reverse_iterator crit(rb.cend());
      VERIFY(*crit == 3, caseLabel);
   }
}


void testRingBufferConstReverseIteratorCopyCtor()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator copy ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      auto copy{crit};
      VERIFY(copy == crit, caseLabel);
   }
}


void testRingBufferConstReverseIteratorMoveCtor()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator move ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      auto moved{std::move(crit)};
      VERIFY(*moved == 1, caseLabel);
   }
}


void testRingBufferConstReverseIteratorCopyAssignment()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator copy assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      auto copy = crit;
      VERIFY(copy == crit, caseLabel);
   }
}


void testRingBufferConstReverseIteratorMoveAssignment()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator move assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      auto moved = std::move(crit);
      VERIFY(*moved == 1, caseLabel);
   }
}


void testRingBufferConstReverseIteratorDerefOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferConstIterator<RingBuffer<int, 10>> cit{rb.cbegin() + 1};
      RingBuffer<int, 10>::const_reverse_iterator crit(cit);
      VERIFY(*crit == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferConstIterator<RingBuffer<int, 3>> cit{rb.cbegin() + 1};
      RingBuffer<int, 3>::const_reverse_iterator crit(cit);
      VERIFY(*crit == 6, caseLabel);
   }
}


void testRingBufferConstReverseIteratorArrowOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator arrow operator"};
      using Rb_t = RingBuffer<std::string, 10>;
      const std::string s = "test a";
      Rb_t rb{s, "b"};
      RingBufferConstIterator<Rb_t> cit{rb.cbegin() + 1};
      Rb_t::const_reverse_iterator crit(cit);
      VERIFY(crit->length() == s.length(), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator arrow operator for wrapped buffer"};
      using Rb_t = RingBuffer<std::string, 3>;
      const std::string s = "test a";
      Rb_t rb{"aaa", "b", "cc", "dddd", s};
      Rb_t::const_reverse_iterator crit(rb.cend());
      VERIFY(crit->length() == s.length(), caseLabel);
   }
}


void testRingBufferConstReverseIteratorPrefixIncOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator prefix behavior of "
                                  "prefix increment operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin();
      auto res = ++rit;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator prefix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataRit = data.rbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; ++rit, ++dataRit)
         VERIFY(*rit == *dataRit, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator prefix increment operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{80, 70, 60, 50, 40};
      auto expectedIt = expected.begin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; ++rit, ++expectedIt)
         VERIFY(*rit == *expectedIt, caseLabel);
   }
}


void testRingBufferConstReverseIteratorPostfixIncOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator postfix behavior "
                                  "of postfix increment operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin();
      auto res = rit++;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator postfix increment operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataRit = data.rbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; rit++, ++dataRit)
         VERIFY(*rit == *dataRit, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator postfix increment operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{80, 70, 60, 50, 40};
      auto expectedIt = expected.cbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; rit++, ++expectedIt)
         VERIFY(*rit == *expectedIt, caseLabel);
   }
}


void testRingBufferConstReverseIteratorPrefixDecOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator prefix behavior of "
                                  "prefix decrement operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin() + 1;
      auto res = --rit;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator prefix decrement operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto expectedIt = data.cbegin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           --rit, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator prefix decrement operator for "
         "iterating wrapped buffer"};
      std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto expectedIt = data.cbegin() + data.size() - rb.size();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           --rit, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
}


void testRingBufferConstReverseIteratorPostfixDecOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator postfix behavior "
                                  "of postfix decrement operator"};
      const RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin() + 1;
      auto res = rit--;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator postfix decrement operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      const RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto expectedIt = data.begin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           rit--, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator postfix decrement operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      const RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{40, 50, 60, 70, 80};
      auto expectedIt = expected.cbegin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           rit--, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
}


void testRingBufferConstReverseIteratorAddAssignmentOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator assignment addition operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      rit += 3;
      VERIFY(*rit == 2, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator assignment "
                                  "addition operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 3;
      rit += -2;
      VERIFY(*rit == 4, caseLabel);
   }
}


void testRingBufferConstReverseIteratorAdditionOperator()
{
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator addition operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      auto res = rit + 3;
      VERIFY(*res == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator addition operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 3;
      auto res = rit + (-2);
      VERIFY(*res == 4, caseLabel);
   }
}


void testRingBufferConstReverseIteratorSubAssignmentOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator assignment subtraction operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 4;
      rit -= 2;
      VERIFY(*rit == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator assignment "
                                  "subtraction operator for negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      rit -= -3;
      VERIFY(*rit == 2, caseLabel);
   }
}


void testRingBufferConstReverseIteratorSubtractionOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator subtraction operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 4;
      auto res = rit - 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator subtraction operator negative value"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      auto res = rit - (-3);
      VERIFY(*res == 2, caseLabel);
   }
}


void testRingBufferConstReverseIteratorDifferenceOperator()
{
   // Iterator difference does not seem to be defined for the reverse iterator adaptor.
   //{
   //   const std::string caseLabel{"RingBuffer::const_reverse_iterator difference
   //   operator"}; const RingBuffer<int, 10> rb{1, 2, 3, 4, 5}; auto a = rb.rbegin() + 1;
   //   auto b = rb.rbegin() + 3;
   //   auto diff = b - a;
   //   VERIFY(diff == 2, caseLabel);
   //}
   //{
   //   const std::string caseLabel{
   //      "RingBuffer::const_reverse_iterator difference operator for negative
   //      differences"};
   //   const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
   //   auto a = rb.rbegin() + 1;
   //   auto b = rb.rbegin() + 3;
   //   auto diff = a - b;
   //   VERIFY(diff == -2, caseLabel);
   //}
}


void testRingBufferConstReverseIteratorEqualityOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator equality operator for equality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 2;
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator equality operator for inequal indices"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator equality operator for inequal containers"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      const RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb2.rbegin() + 2;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator equality operator for end iterators"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rend();
      auto b = rb.rend();
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator equality operator "
                                  "for end and last iterators"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 4;
      auto b = rb.rend();
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator equality operator for unassigned iterators"};
      RingBuffer<int, 10>::const_reverse_iterator a, b;
      VERIFY(a == b, caseLabel);
   }
}


void testRingBufferConstReverseIteratorInequalityOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator inequality operator for inequality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator inequality operator for equality"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 2;
      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator inequality operator for inequal containers"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      const RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb2.rbegin() + 2;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator inequality "
                                  "operator for unassigned iterators"};
      RingBuffer<int, 10>::const_reverse_iterator a, b;
      VERIFY(!(a != b), caseLabel);
   }
}


void testRingBufferConstReverseIteratorLessThanOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator less-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator less-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(!(a < b), caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator less-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(a < e, caseLabel);
      VERIFY(b < e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstReverseIteratorGreaterThanOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator greater-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 3;
      auto b = rb.rbegin() + 2;
      VERIFY(a > b, caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator greater-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(!(a > b), caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator greater-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(e > a, caseLabel);
      VERIFY(e > b, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstReverseIteratorLessThanEqualOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator less-or-equal-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator less-or-equal-than "
                                  "operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(a <= b, caseLabel);
      VERIFY(b <= a, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator less-or-equal-than "
                                  "operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(a <= e, caseLabel);
      VERIFY(b <= e, caseLabel);
      VERIFY(e <= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferConstReverseIteratorGreaterThanEqualOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::const_reverse_iterator greater-or-equal-than operator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 3;
      auto b = rb.rbegin() + 2;
      VERIFY(a >= b, caseLabel);
      VERIFY(!(b >= a), caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator "
                                  "greater-or-equal-than operator for same iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(a >= b, caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::const_reverse_iterator "
                                  "greater-or-equal-than operator with end iterator"};
      const RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(e >= a, caseLabel);
      VERIFY(e >= b, caseLabel);
      VERIFY(e >= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferReverseIteratorCtor()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator default ctor"};
      RingBuffer<int, 10>::reverse_iterator rit;
      // Nothing to VERIFY because a default initialized object cannot be used
      // before it gets assigned to.
   }
}


void testRingBufferReverseIteratorFromIterator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator ctor for iterator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      VERIFY(*rit == 1, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator ctor for end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBuffer<int, 10>::reverse_iterator rit(rb.end());
      VERIFY(*rit == 3, caseLabel);
   }
}


void testRingBufferReverseIteratorCopyCtor()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator copy ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      auto copy{rit};
      VERIFY(copy == rit, caseLabel);
   }
}


void testRingBufferReverseIteratorMoveCtor()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator move ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      auto moved{std::move(rit)};
      VERIFY(*moved == 1, caseLabel);
   }
}


void testRingBufferReverseIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator copy assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      auto copy = rit;
      VERIFY(copy == rit, caseLabel);
   }
}


void testRingBufferReverseIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator move assignment ctor"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      auto moved = std::move(rit);
      VERIFY(*moved == 1, caseLabel);
   }
}


void testRingBufferReverseIteratorConstDerefOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator const dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      VERIFY(*rit == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator const dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferIterator<RingBuffer<int, 3>> it{rb.begin() + 1};
      RingBuffer<int, 3>::reverse_iterator rit(it);
      VERIFY(*rit == 6, caseLabel);
   }
}

void testRingBufferReverseIteratorDerefOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator dereference operator"};
      RingBuffer<int, 10> rb{1, 2, 3};
      RingBufferIterator<RingBuffer<int, 10>> it{rb.begin() + 1};
      RingBuffer<int, 10>::reverse_iterator rit(it);
      *rit = 10;
      VERIFY(*rit == 10, caseLabel);
      VERIFY(rb[0] == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator dereference operator for wrapped buffer"};
      RingBuffer<int, 3> rb{1, 2, 3, 5, 6, 7, 8};
      RingBufferIterator<RingBuffer<int, 3>> it{rb.begin() + 1};
      RingBuffer<int, 3>::reverse_iterator rit(it);
      *rit = 0;
      VERIFY(*rit == 0, caseLabel);
      VERIFY(rb[0] == 0, caseLabel);
   }
}

void testRingBufferReverseIteratorConstArrowOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator const arrow operator"};
      using Rb_t = RingBuffer<std::string, 10>;
      const std::string s = "test a";
      Rb_t rb{s, "b"};
      RingBufferIterator<Rb_t> it{rb.begin() + 1};
      Rb_t::reverse_iterator rit(it);
      VERIFY(rit->length() == s.length(), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator const arrow operator for wrapped buffer"};
      using Rb_t = RingBuffer<std::string, 3>;
      const std::string s = "test a";
      Rb_t rb{"aaa", "b", "cc", "dddd", s};
      Rb_t::reverse_iterator rit(rb.end());
      VERIFY(rit->length() == s.length(), caseLabel);
   }
}


void testRingBufferReverseIteratorArrowOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator arrow operator"};
      using Rb_t = RingBuffer<std::string, 10>;
      const std::string s = "test a";
      Rb_t rb{s, "b"};
      RingBufferIterator<Rb_t> it{rb.begin() + 1};
      Rb_t::reverse_iterator rit(it);
      rit->append("**");
      VERIFY(*rit == "test a**", caseLabel);
      VERIFY(rb[0] == "test a**", caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator const arrow operator for wrapped buffer"};
      using Rb_t = RingBuffer<std::string, 3>;
      const std::string s = "test a";
      Rb_t rb{"aaa", "b", "cc", "dddd", s};
      Rb_t::reverse_iterator rit(rb.end());
      rit->append("**");
      VERIFY(*rit == "test a**", caseLabel);
      VERIFY(rb[rb.size() - 1] == "test a**", caseLabel);
   }
}


void testRingBufferReverseIteratorPrefixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix behavior of prefix increment operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin();
      auto res = ++rit;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix increment operator for iterating"};
      std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataRit = data.rbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; ++rit, ++dataRit)
         VERIFY(*rit == *dataRit, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix increment operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{80, 70, 60, 50, 40};
      auto expectedIt = expected.begin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; ++rit, ++expectedIt)
         VERIFY(*rit == *expectedIt, caseLabel);
   }
}


void testRingBufferReverseIteratorPostfixIncOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix behavior of postfix increment operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin();
      auto res = rit++;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix increment operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto dataRit = data.rbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; rit++, ++dataRit)
         VERIFY(*rit == *dataRit, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix increment operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{80, 70, 60, 50, 40};
      auto expectedIt = expected.cbegin();
      for (auto rit = rb.rbegin(), rend = rb.rend(); rit != rend; rit++, ++expectedIt)
         VERIFY(*rit == *expectedIt, caseLabel);
   }
}


void testRingBufferReverseIteratorPrefixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix behavior of prefix decrement operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin() + 1;
      auto res = --rit;
      VERIFY(*res == 20, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix decrement operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto expectedIt = data.cbegin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           --rit, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator prefix decrement operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      auto expectedIt = data.cbegin() + data.size() - rb.size();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           --rit, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
}


void testRingBufferReverseIteratorPostfixDecOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix behavior of postfix decrement operator"};
      RingBuffer<int, 10> rb{10, 20};
      auto rit = rb.rbegin() + 1;
      auto res = rit--;
      VERIFY(*res == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix decrement operator for iterating"};
      const std::vector<int> data{10, 20, 30, 40};
      RingBuffer<int, 10> rb{data.begin(), data.end()};

      auto expectedIt = data.begin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           rit--, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator postfix decrement operator for "
         "iterating wrapped buffer"};
      const std::vector<int> data{10, 20, 30, 40, 50, 60, 70, 80};
      RingBuffer<int, 5> rb{data.begin(), data.end()};

      const std::vector<int> expected{40, 50, 60, 70, 80};
      auto expectedIt = expected.cbegin();
      for (auto rit = rb.rbegin() + rb.size() - 1, rstart = rb.rbegin(); rit != rstart;
           rit--, ++expectedIt)
      {
         VERIFY(*rit == *expectedIt, caseLabel);
      }
   }
}


void testRingBufferReverseIteratorAddAssignmentOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator assignment addition operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      rit += 3;
      VERIFY(*rit == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator assignment addition operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 3;
      rit += -2;
      VERIFY(*rit == 4, caseLabel);
   }
}


void testRingBufferReverseIteratorAdditionOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator addition operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      auto res = rit + 3;
      VERIFY(*res == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator addition operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 3;
      auto res = rit + (-2);
      VERIFY(*res == 4, caseLabel);
   }
}


void testRingBufferReverseIteratorSubAssignmentOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator assignment subtraction operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 4;
      rit -= 2;
      VERIFY(*rit == 3, caseLabel);
   }
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator assignment subtraction "
                                  "operator for negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      rit -= -3;
      VERIFY(*rit == 2, caseLabel);
   }
}


void testRingBufferReverseIteratorSubtractionOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator subtraction operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin() + 4;
      auto res = rit - 2;
      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator subtraction operator negative value"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto rit = rb.rbegin();
      auto res = rit - (-3);
      VERIFY(*res == 2, caseLabel);
   }
}


void testRingBufferReverseIteratorDifferenceOperator()
{
   // Iterator difference does not seem to be defined for the reverse iterator adaptor.
   //{
   //   const std::string caseLabel{"RingBuffer::reverse_iterator difference operator"};
   //   RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
   //   auto a = rb.rbegin() + 1;
   //   auto b = rb.rbegin() + 3;
   //   auto diff = b - a;
   //   VERIFY(diff == 2, caseLabel);
   //}
   //{
   //   const std::string caseLabel{
   //      "RingBuffer::reverse_iterator difference operator for negative differences"};
   //   RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
   //   auto a = rb.rbegin() + 1;
   //   auto b = rb.rbegin() + 3;
   //   auto diff = a - b;
   //   VERIFY(diff == -2, caseLabel);
   //}
}


void testRingBufferReverseIteratorEqualityOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for equality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 2;
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for inequal indices"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for inequal containers"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb2.rbegin() + 2;
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for end iterators"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rend();
      auto b = rb.rend();
      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for end and last iterators"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 4;
      auto b = rb.rend();
      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator equality operator for unassigned iterators"};
      RingBuffer<int, 10>::const_reverse_iterator a, b;
      VERIFY(a == b, caseLabel);
   }
}


void testRingBufferReverseIteratorInequalityOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator inequality operator for inequality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator inequality operator for equality"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 2;
      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator inequality operator for inequal containers"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      RingBuffer<int, 10> rb2{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb2.rbegin() + 2;
      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator inequality operator for unassigned iterators"};
      RingBuffer<int, 10>::const_reverse_iterator a, b;
      VERIFY(!(a != b), caseLabel);
   }
}


void testRingBufferReverseIteratorLessThanOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator less-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator less-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(!(a < b), caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator less-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(a < e, caseLabel);
      VERIFY(b < e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferReverseIteratorGreaterThanOperator()
{
   {
      const std::string caseLabel{"RingBuffer::reverse_iterator greater-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 3;
      auto b = rb.rbegin() + 2;
      VERIFY(a > b, caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator greater-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(!(a > b), caseLabel);
      VERIFY(!(b > a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator greater-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(e > a, caseLabel);
      VERIFY(e > b, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferReverseIteratorLessThanEqualOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator less-or-equal-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 3;
      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator less-or-equal-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(a <= b, caseLabel);
      VERIFY(b <= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator less-or-equal-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(a <= e, caseLabel);
      VERIFY(b <= e, caseLabel);
      VERIFY(e <= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}


void testRingBufferReverseIteratorGreaterThanEqualOperator()
{
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator greater-or-equal-than operator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 3;
      auto b = rb.rbegin() + 2;
      VERIFY(a >= b, caseLabel);
      VERIFY(!(b >= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator greater-or-equal-than operator for same iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = a;
      VERIFY(a >= b, caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "RingBuffer::reverse_iterator greater-or-equal-than operator with end iterator"};
      RingBuffer<int, 10> rb{1, 2, 3, 4, 5};
      auto a = rb.rbegin() + 2;
      auto b = rb.rbegin() + 4;
      auto e = rb.rend();
      VERIFY(e >= a, caseLabel);
      VERIFY(e >= b, caseLabel);
      VERIFY(e >= e, caseLabel);
   }
   { // Different containers
     // Operation is not defined because there is no good solution.
     // Returning 'false' would leave the impression that the instances
     // are related by the opposite relation.
   }
}

} // namespace


void testRingBuffer()
{
   testRingBufferDefaultCtor();
   testRingBufferIteratorCtor();
   testRingBufferInitializerListCtor();
   testRingBufferCopyCtor();
   testRingBufferMoveCtor();
   testRingBufferCopyAssignment();
   testRingBufferMoveAssignment();
   testRingBufferSize();
   testRingBufferCapacity();
   testRingBufferEmpty();
   testRingBufferFull();
   testRingBufferAt();
   testRingBufferAtConst();
   testRingBufferIndexOperator();
   testRingBufferIndexOperatorConst();
   testRingBufferPush();
   testRingBufferPop();
   testRingBufferClear();
   testRingBufferSwap();
   testRingBufferBegin();
   testRingBufferEnd();
   testRingBufferBeginConst();
   testRingBufferEndConst();
   testRingBufferCBegin();
   testRingBufferCEnd();
   testRingBufferRBegin();
   testRingBufferREnd();
   testRingBufferRBeginConst();
   testRingBufferREndConst();
   testRingBufferCRBegin();
   testRingBufferCREnd();

   testRingBufferConstIteratorCtor();
   testRingBufferConstIteratorCopyCtor();
   testRingBufferConstIteratorMoveCtor();
   testRingBufferConstIteratorCopyAssignment();
   testRingBufferConstIteratorMoveAssignment();
   testRingBufferConstIteratorDerefOperator();
   testRingBufferConstIteratorArrowOperator();
   testRingBufferConstIteratorSubscriptOperator();
   testRingBufferConstIteratorPrefixIncOperator();
   testRingBufferConstIteratorPostfixIncOperator();
   testRingBufferConstIteratorPrefixDecOperator();
   testRingBufferConstIteratorPostfixDecOperator();
   testRingBufferConstIteratorAddAssignmentOperator();
   testRingBufferConstIteratorAdditionOperator();
   testRingBufferConstIteratorSubAssignmentOperator();
   testRingBufferConstIteratorSubtractionOperator();
   testRingBufferConstIteratorDifferenceOperator();
   testRingBufferConstIteratorEqualityOperator();
   testRingBufferConstIteratorInequalityOperator();
   testRingBufferConstIteratorLessThanOperator();
   testRingBufferConstIteratorGreaterThanOperator();
   testRingBufferConstIteratorLessThanEqualOperator();
   testRingBufferConstIteratorGreaterThanEqualOperator();
   testRingBufferConstIteratorSwap();

   testRingBufferIteratorDefaultCtor();
   testRingBufferIteratorRBAndIndexCtor();
   testRingBufferIteratorCopyCtor();
   testRingBufferIteratorMoveCtor();
   testRingBufferIteratorCopyAssignment();
   testRingBufferIteratorMoveAssignment();
   testRingBufferIteratorConstDerefOperator();
   testRingBufferIteratorDerefOperator();
   testRingBufferIteratorConstArrowOperator();
   testRingBufferIteratorArrowOperator();
   testRingBufferIteratorSubscriptOperator();
   testRingBufferIteratorConstSubscriptOperator();
   testRingBufferIteratorPrefixIncOperator();
   testRingBufferIteratorPostfixIncOperator();
   testRingBufferIteratorPrefixDecOperator();
   testRingBufferIteratorPostfixDecOperator();
   testRingBufferIteratorAddAssignmentOperator();
   testRingBufferIteratorAdditionOperator();
   testRingBufferIteratorSubAssignmentOperator();
   testRingBufferIteratorSubtractionOperator();
   testRingBufferIteratorDifferenceOperator();
   testRingBufferIteratorEqualityOperator();
   testRingBufferIteratorInequalityOperator();
   testRingBufferIteratorLessThanOperator();
   testRingBufferIteratorGreaterThanOperator();
   testRingBufferIteratorLessThanEqualOperator();
   testRingBufferIteratorGreaterThanEqualOperator();
   testRingBufferIteratorSwap();

   testRingBufferConstReverseIteratorCtor();
   testRingBufferConstReverseIteratorFromIterator();
   testRingBufferConstReverseIteratorCopyCtor();
   testRingBufferConstReverseIteratorMoveCtor();
   testRingBufferConstReverseIteratorCopyAssignment();
   testRingBufferConstReverseIteratorMoveAssignment();
   testRingBufferConstReverseIteratorDerefOperator();
   testRingBufferConstReverseIteratorArrowOperator();
   testRingBufferConstReverseIteratorPrefixIncOperator();
   testRingBufferConstReverseIteratorPostfixIncOperator();
   testRingBufferConstReverseIteratorPrefixDecOperator();
   testRingBufferConstReverseIteratorPostfixDecOperator();
   testRingBufferConstReverseIteratorAddAssignmentOperator();
   testRingBufferConstReverseIteratorAdditionOperator();
   testRingBufferConstReverseIteratorSubAssignmentOperator();
   testRingBufferConstReverseIteratorSubtractionOperator();
   testRingBufferConstReverseIteratorDifferenceOperator();
   testRingBufferConstReverseIteratorEqualityOperator();
   testRingBufferConstReverseIteratorInequalityOperator();
   testRingBufferConstReverseIteratorLessThanOperator();
   testRingBufferConstReverseIteratorGreaterThanOperator();
   testRingBufferConstReverseIteratorLessThanEqualOperator();
   testRingBufferConstReverseIteratorGreaterThanEqualOperator();

   testRingBufferReverseIteratorCtor();
   testRingBufferReverseIteratorFromIterator();
   testRingBufferReverseIteratorCopyCtor();
   testRingBufferReverseIteratorMoveCtor();
   testRingBufferReverseIteratorCopyAssignment();
   testRingBufferReverseIteratorMoveAssignment();
   testRingBufferReverseIteratorConstDerefOperator();
   testRingBufferReverseIteratorDerefOperator();
   testRingBufferReverseIteratorConstArrowOperator();
   testRingBufferReverseIteratorArrowOperator();
   testRingBufferReverseIteratorPrefixIncOperator();
   testRingBufferReverseIteratorPostfixIncOperator();
   testRingBufferReverseIteratorPrefixDecOperator();
   testRingBufferReverseIteratorPostfixDecOperator();
   testRingBufferReverseIteratorAddAssignmentOperator();
   testRingBufferReverseIteratorAdditionOperator();
   testRingBufferReverseIteratorSubAssignmentOperator();
   testRingBufferReverseIteratorSubtractionOperator();
   testRingBufferReverseIteratorDifferenceOperator();
   testRingBufferReverseIteratorEqualityOperator();
   testRingBufferReverseIteratorInequalityOperator();
   testRingBufferReverseIteratorLessThanOperator();
   testRingBufferReverseIteratorGreaterThanOperator();
   testRingBufferReverseIteratorLessThanEqualOperator();
   testRingBufferReverseIteratorGreaterThanEqualOperator();
}
