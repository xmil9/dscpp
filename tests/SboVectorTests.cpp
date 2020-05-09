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

      constexpr std::size_t Cap = 10;

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
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

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;

      // Precondition.
      VERIFY(Cap >= NumElems, caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv(NumElems, {2});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Creation of passed-in instance.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Creation of elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruction of passed-in instance. The element instances will be destroyed
      // later.
      VERIFY(Instrumented::dtorCalls == 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 2, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;

      // Precondition.
      VERIFY(Cap < NumElems, caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv(NumElems, {2});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Creation of passed-in instance.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Creation of elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruction of passed-in instance. The element instances will be destroyed
      // later.
      VERIFY(Instrumented::dtorCalls == 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 2, caseLabel);
   }
}


void TestSboVectorCopyCtor()
{
   {
      const std::string caseLabel{"SboVector copy ctor for buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;

      SboVector<Instrumented, Cap> src(NumElems, {1});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{src};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Neither source nor copied elements got destroyed yet.
      VERIFY(Instrumented::dtorCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy ctor for heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {1});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{src};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Neither source nor copied elements got destroyed yet.
      VERIFY(Instrumented::dtorCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
}


void TestSboVectorMoveCtor()
{
   {
      const std::string caseLabel{"SboVector move ctor for buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{std::move(src)};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved elements.
      VERIFY(Instrumented::moveCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // No elements got destroyed.
      VERIFY(Instrumented::dtorCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv{std::move(src)};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // No elements got destroyed.
      VERIFY(Instrumented::dtorCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
}


void TestSboVectorInitializerListCtor()
{
   {
      const std::string caseLabel{"SboVector initializer list ctor for buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 4;

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{{1}, {2}, {3}, {4}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Elements in inilializer list got destroyed.
      VERIFY(Instrumented::dtorCalls == NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list ctor for heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 12;

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{
         {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12},
      };

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Elements in inilializer list got destroyed.
      VERIFY(Instrumented::dtorCalls == NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}


void TestSboVectorDtor()
{
   {
      const std::string caseLabel{"SboVector dtor for buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 3;

      {
         SboVector<Instrumented, Cap> sv{{1}, {2}, {3}};
         // Precondition.
         VERIFY(sv.in_buffer(), caseLabel);

         // Reset call counts before the SboVector gets destroyed.
         Instrumented::resetCallCount();
      }

      VERIFY(Instrumented::dtorCalls == NumElems, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector dtor for heap instance."};

      constexpr std::size_t Cap = 3;
      constexpr std::size_t NumElems = 5;

      {
         SboVector<Instrumented, Cap> sv{{1}, {2}, {3}, {4}, {5}};
         // Precondition.
         VERIFY(sv.on_heap(), caseLabel);

         // Reset call counts before the SboVector gets destroyed.
         Instrumented::resetCallCount();
      }

      VERIFY(Instrumented::dtorCalls == NumElems, caseLabel);
   }
}


void TestSboVectorCopyAssignment()
{
   {
      const std::string caseLabel{
         "SboVector copy assignment from buffer instance to buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from heap instance to buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from buffer instance to heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from larger heap instance to smaller heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 15;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumElems > NumOrigElems, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a larger heap allocation will trigger a new
      // allocation. Capacity will increase to larger size.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from smaller heap instance to larger heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 15;
      constexpr std::size_t NumOrigElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumElems < NumOrigElems, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a smaller heap allocation will reuse the existing
      // heap memory. Capacity will remain at previous (larger) size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
}


void TestSboVectorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Capacity of buffer.
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved elements.
      VERIFY(Instrumented::moveCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of heap "
                                  "instance to buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will have capacity of stolen source heap memory.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(src.in_buffer(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Elements fit into buffer.
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.in_buffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // Moved source elements.
      VERIFY(Instrumented::moveCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Original elements got destroyed.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of smaller heap "
                                  "instance to larger heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 15;
      constexpr std::size_t NumOrigElems = 20;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumElems < NumOrigElems, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of larger heap "
                                  "instance to smaller heap instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 15;

      SboVector<Instrumented, Cap> src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumElems > NumOrigElems, caseLabel);
      VERIFY(src.on_heap(), caseLabel);
      VERIFY(sv.on_heap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.on_heap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destructed previous elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
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
   TestSboVectorCopyCtor();
   TestSboVectorMoveCtor();
   TestSboVectorInitializerListCtor();
   TestSboVectorDtor();
   TestSboVectorCopyAssignment();
   TestSboVectorMoveAssignment();
   // TestSboVectorInitializerListAssignment();
}
