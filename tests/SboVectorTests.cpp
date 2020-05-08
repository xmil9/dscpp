#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <string>
#include <vector>


namespace
{
///////////////////

struct Instrumented
{
   Instrumented() { ++defaultCtorCalls; }
   Instrumented(int i_) : i{i_} { ++ctorCalls; }
   Instrumented(const Instrumented& other) : d{other.d}, i{other.i}, b{other.b}
   {
      ++copyCtorCalls;
   }
   Instrumented(Instrumented&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      ++moveCtorCalls;
   }
   ~Instrumented() { ++dtorCalls; }
   Instrumented& operator=(const Instrumented& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      ++assignmentCalls;
   }
   Instrumented& operator=(Instrumented&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      ++moveAssignmentCalls;
   }

   double d = 1.0;
   int i = 1;
   bool b = true;

   inline static std::size_t defaultCtorCalls = 0;
   inline static std::size_t ctorCalls = 0;
   inline static std::size_t copyCtorCalls = 0;
   inline static std::size_t moveCtorCalls = 0;
   inline static std::size_t assignmentCalls = 0;
   inline static std::size_t moveAssignmentCalls = 0;
   inline static std::size_t dtorCalls = 0;

   inline static void resetCallCount()
   {
      defaultCtorCalls = 0;
      ctorCalls = 0;
      copyCtorCalls = 0;
      moveCtorCalls = 0;
      assignmentCalls = 0;
      moveAssignmentCalls = 0;
      dtorCalls = 0;
   }
};


///////////////////

void TestSboVectorDefaultCtor()
{
   {
      const std::string caseLabel{"SboVector default ctor."};

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 0, caseLabel);
   }
}


void TestSboVectorCtorForElementCountAndValue()
{
   {
      const std::string caseLabel{"SboVector count-and-value ctor for buffer instance."};

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv(5, {2});

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Creation of passed-in instance.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Creation of elements.
      VERIFY(Instrumented::copyCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruction of passed-in instance. The element instances will be destructed
      // later.
      VERIFY(Instrumented::dtorCalls == 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 2, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for heap instance."};
      
      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv(20, {2});

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Creation of passed-in instance.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Creation of elements.
      VERIFY(Instrumented::copyCtorCalls == 20, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruction of passed-in instance. The element instances will be destructed
      // later.
      VERIFY(Instrumented::dtorCalls == 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 2, caseLabel);
   }
}


void TestSboVectorCopyCtor()
{
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type with default ctor in buffer."};

      SboVector<Instrumented, 10> src{5, Instrumented{}};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> copy{src};

      VERIFY(copy.size() == 5, caseLabel);
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type with default ctor on heap."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> copy{src};

      VERIFY(copy.size() == 20, caseLabel);
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 20, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
}


void TestSboVectorMoveCtor()
{
   {
      const std::string caseLabel{
         "SboVector move ctor for pod-type with default ctor in buffer."};

      Instrumented val;
      SboVector<Instrumented, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector move ctor for pod-type with default ctor on heap."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for std::string in buffer."};

      std::string val;
      SboVector<std::string, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.in_buffer(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == std::to_string(i), caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for std::string on heap."};

      std::string val;
      SboVector<std::string, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.on_heap(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == std::to_string(i), caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for scalar type in buffer."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(5, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.in_buffer(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for scalar type on heap."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(20, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.on_heap(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
}


void TestSboVectorInitializerListCtor()
{
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for pod-type in buffer."};

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv{{1}, {2}, {3}, {4}};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 4, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for pod-type on heap."};

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv{
         {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12},
      };

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 12, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for std::string in buffer."};

      SboVector<std::string, 10> sv{"1", "2", "3", "4"};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == std::to_string(i + 1), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for std::string on heap."};

      SboVector<std::string, 10> sv{"1", "2", "3", "4",  "5",  "6",
                                    "7", "8", "9", "10", "11", "12"};

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == std::to_string(i + 1), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for scalar type in buffer."};

      SboVector<int, 10> sv{1, 2, 3, 4};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for scalar type on heap."};

      SboVector<int, 10> sv{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
}


void TestSboVectorDtor()
{
   {
      const std::string caseLabel{"SboVector dtor for internal buffer."};

      {
         SboVector<Instrumented, 10> sv{{1}, {2}, {3}};
         VERIFY(sv.in_buffer(), caseLabel);

         // Rest call counts before the SboVector gets destructed.
         Instrumented::resetCallCount();
      }

      VERIFY(Instrumented::dtorCalls == 3, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector dtor for heap."};

      {
         SboVector<Instrumented, 3> sv{{1}, {2}, {3}, {4}, {5}};
         VERIFY(sv.on_heap(), caseLabel);

         // Rest call counts before the SboVector gets destructed.
         Instrumented::resetCallCount();
      }

      VERIFY(Instrumented::dtorCalls == 5, caseLabel);
   }
}


void TestSboVectorCopyAssignment()
{
   {
      const std::string caseLabel{"SboVector copy assignment for pod-type from buffer "
                                  "instance to buffer instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{3, Instrumented{}};
      VERIFY(copy.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      copy = src;

      VERIFY(copy.size() == 5, caseLabel);
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 3, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment for pod-type from heap instance to buffer instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{3, Instrumented{}};
      VERIFY(copy.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      copy = src;

      VERIFY(copy.size() == 20, caseLabel);
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 20, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 3, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment for pod-type from buffer instance to heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{20, Instrumented{}};
      VERIFY(copy.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      copy = src;

      VERIFY(copy.size() == 5, caseLabel);
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 20, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy assignment for pod-type from larger "
                                  "heap instance to smaller heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{15, Instrumented{}};
      VERIFY(copy.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      copy = src;

      VERIFY(copy.size() == 20, caseLabel);
      // Assigning data that needs a larger heap allocation will trigger a new
      // allocation. Capacity will increase to larger size.
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 20, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 15, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy assignment for pod-type from smaller "
                                  "heap instance to larger heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{15, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{20, Instrumented{}};
      VERIFY(copy.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      copy = src;

      VERIFY(copy.size() == 15, caseLabel);
      // Assigning data that needs a smaller heap allocation will reuse the existing
      // heap memory. Capacity will remain at previous (larger) size.
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 15, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::dtorCalls == 20, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
}


void TestSboVectorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to buffer instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{3, Instrumented{}};

      // Assert preconditions of test.
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      copy = std::move(src);

      VERIFY(copy.size() == 5, caseLabel);
      // Capacity of buffer.
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved source elements.
      VERIFY(Instrumented::moveCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == 3, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of heap "
                                  "instance to buffer instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{3, Instrumented{}};

      // Assert preconditions of test.
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      copy = std::move(src);

      VERIFY(copy.size() == 20, caseLabel);
      // Will have capacity of stolen source heap memory.
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == 3, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{20, Instrumented{}};

      // Assert preconditions of test.
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(copy.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      copy = std::move(src);

      VERIFY(copy.size() == 5, caseLabel);
      // Elements fit into buffer.
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved source elements.
      VERIFY(Instrumented::moveCtorCalls == 5, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == 20, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of smaller heap "
                                  "instance to larger heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{30, Instrumented{}};

      // Assert preconditions of test.
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(src.size() < copy.size(), caseLabel);

      Instrumented::resetCallCount();
      copy = std::move(src);

      VERIFY(copy.size() == 20, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == 30, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of larger heap "
                                  "instance to smaller heap instance."};

      Instrumented val;
      SboVector<Instrumented, 10> src{30, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, 10> copy{20, Instrumented{}};

      // Assert preconditions of test.
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(src.size() > copy.size(), caseLabel);

      Instrumented::resetCallCount();
      copy = std::move(src);

      VERIFY(copy.size() == 30, caseLabel);
      // Will allocate capacity for moved elements.
      VERIFY(copy.capacity() == 30, caseLabel);
      VERIFY(copy.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved source elements.
      VERIFY(Instrumented::moveCtorCalls == 30, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == 20, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
}


void TestSboVectorInitializerListAssignment()
{
   {
      const std::string caseLabel{"SboVector initializer list assignment for pod-type "
                                  "from buffer instance to buffer instance."};

      SboVector<Instrumented, 10> sv{3, {1}};
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}};

      VERIFY(sv.size() == 2, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      // Ctor calls are for constructing items in ilist.
      VERIFY(Instrumented::ctorCalls == 2, caseLabel);
      // Copy ctor calls are for assigning items into SboVector.
      VERIFY(Instrumented::copyCtorCalls == 2, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == 5, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment for pod-type "
                                  "from heap instance to buffer instance."};

      SboVector<Instrumented, 10> sv{20, {1}};
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}};

      VERIFY(sv.size() == 2, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      // Ctor calls are for constructing items in ilist.
      VERIFY(Instrumented::ctorCalls == 2, caseLabel);
      // Copy ctor calls are for assigning items into SboVector.
      VERIFY(Instrumented::copyCtorCalls == 2, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == 22, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment for pod-type "
                                  "from buffer instance to heap instance."};

      SboVector<Instrumented, 5> sv{3, {1}};
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};

      VERIFY(sv.size() == 7, caseLabel);
      VERIFY(sv.capacity() == 7, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      // Ctor calls are for constructing items in ilist.
      VERIFY(Instrumented::ctorCalls == 7, caseLabel);
      // Copy ctor calls are for assigning items into SboVector.
      VERIFY(Instrumented::copyCtorCalls == 7, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == 10, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment for pod-type "
                                  "from smaller heap instance to larger heap instance."};

      SboVector<Instrumented, 5> sv{7, {1}};
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}};

      VERIFY(sv.size() == 9, caseLabel);
      // Assigning data that needs a larger heap allocation will trigger a new
      // allocation. Capacity will increase to larger size.
      VERIFY(sv.capacity() == 9, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      // Ctor calls are for constructing items in ilist.
      VERIFY(Instrumented::ctorCalls == 9, caseLabel);
      // Copy ctor calls are for assigning items into SboVector.
      VERIFY(Instrumented::copyCtorCalls == 9, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == 16, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment for pod-type "
                                  "from larger heap instance to smaller heap instance."};

      SboVector<Instrumented, 5> sv{9, {2}};
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};

      VERIFY(sv.size() == 7, caseLabel);
      // Assigning data that needs a smaller heap allocation will reuse the existing
      // heap memory. Capacity will remain at previous (larger) size.
      VERIFY(sv.capacity() == 9, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      // Ctor calls are for constructing items in ilist.
      VERIFY(Instrumented::ctorCalls == 7, caseLabel);
      // Copy ctor calls are for assigning items into SboVector.
      VERIFY(Instrumented::copyCtorCalls == 7, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == 16, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}

} // namespace


///////////////////

void TestSboVector()
{
   TestSboVectorDefaultCtor();
   TestSboVectorCtorForElementCountAndValue();
   // TestSboVectorCopyCtor();
   // TestSboVectorMoveCtor();
   // TestSboVectorInitializerListCtor();
   // TestSboVectorDtor();
   // TestSboVectorCopyAssignment();
   // TestSboVectorMoveAssignment();
   // TestSboVectorInitializerListAssignment();
}
