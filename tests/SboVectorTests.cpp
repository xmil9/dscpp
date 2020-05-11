#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <list>
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
      VERIFY(sv.inBuffer(), caseLabel);
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
      VERIFY(sv.inBuffer(), caseLabel);
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
      VERIFY(sv.onHeap(), caseLabel);
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
      VERIFY(src.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{src};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{src};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, Cap> sv{std::move(src)};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      SboVector<Instrumented, 10> sv{std::move(src)};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list element.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
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
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list element.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
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
         VERIFY(sv.inBuffer(), caseLabel);

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
         VERIFY(sv.onHeap(), caseLabel);

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
         "SboVector copy assignment of buffer instance to buffer instance."};

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
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
         "SboVector copy assignment of heap instance to buffer instance."};

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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
         "SboVector copy assignment of buffer instance to heap instance."};

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
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
         "SboVector copy assignment of larger heap instance to smaller heap instance."};

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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a larger heap allocation will trigger a new
      // allocation. Capacity will increase to larger size.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
         "SboVector copy assignment of smaller heap instance to larger heap instance."};

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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = src;

      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a smaller heap allocation will reuse the existing
      // heap memory. Capacity will remain at previous (larger) size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Capacity of buffer.
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will have capacity of stolen source heap memory.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Elements fit into buffer.
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = std::move(src);

      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
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
      const std::string caseLabel{
         "SboVector initializer list assignment that fits in buffer to buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list elements.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that requires "
                                  "heap to buffer instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list elements.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment  "
                                  "that fits in buffer to heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list elements.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "but can reuse the heap of the target instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 10;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list elements.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "and cannot reuse the heap of the target instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 10;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}};

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing initializer list elements.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Copied elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Dtor calls are for original item in SboVector and for items in ilist.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}


void TestSboVectorAssignElementValue()
{
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values fit in buffer. "
         "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(NumElems, Instrumented{10});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing assigned element.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Populated elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements plus the assigned element.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector assign element value n-times. Assigned "
                                  "values require heap allocation. "
                                  "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(NumElems, Instrumented{10});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing assigned element.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Populated elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements plus the assigned element.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned "
         "values fit into buffer. SboVector was a heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(NumElems, Instrumented{10});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing assigned element.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Populated elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements plus the assigned element.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector was a smaller heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 10;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(NumElems, Instrumented{10});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing assigned element.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Populated elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements plus the assigned element.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector was a larger heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 10;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(NumElems, Instrumented{10});

      VERIFY(sv.size() == NumElems, caseLabel);
      // Reused heap stays at larger size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Constructing assigned element.
      VERIFY(Instrumented::ctorCalls == 1, caseLabel);
      // Populated elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements plus the assigned element.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + 1, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
}


void TestSboVectorAssignIteratorRange()
{
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values fit in buffer. "
         "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;

      std::list<Instrumented> src{{1}, {2}};
      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(src.begin(), src.end());

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values require heap. "
         "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;

      std::list<Instrumented> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(src.begin(), src.end());

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned fit into buffer. "
         "SboVector was a heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;

      std::list<Instrumented> src{{1}, {2}, {3}};
      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(src.begin(), src.end());

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned require heap. "
         "SboVector was a smaller heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 8;
      constexpr std::size_t NumOrigElems = 7;

      std::list<Instrumented> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};
      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(src.begin(), src.end());

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned require heap. "
         "SboVector was a larger heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 8;

      std::list<Instrumented> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign(src.begin(), src.end());

      VERIFY(sv.size() == NumElems, caseLabel);
      // Capacity remains at larger, reused size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::ctorCalls == 0, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements.
      VERIFY(Instrumented::dtorCalls == NumOrigElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}


void TestSboVectorAssignInitializerList()
{
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values fit in buffer. "
         "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign({{1}, {2}});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Elements constructed by initalizer list.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements and in initalizer list.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a buffer instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems < Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Elements constructed by initalizer list.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements and in initalizer list.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values fit into buffer. "
         "SboVector was a heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign({{1}, {2}, {3}});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Elements constructed by initalizer list.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements and in initalizer list.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a smaller heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 8;
      constexpr std::size_t NumOrigElems = 7;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Elements constructed by initalizer list.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements and in initalizer list.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a larger heap instance."};

      constexpr std::size_t Cap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 8;

      SboVector<Instrumented, Cap> sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > Cap, caseLabel);
      VERIFY(NumOrigElems > Cap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      Instrumented::resetCallCount();
      sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}});

      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(Instrumented::defaultCtorCalls == 0, caseLabel);
      // Elements constructed by initalizer list.
      VERIFY(Instrumented::ctorCalls == NumElems, caseLabel);
      // Assigned elements.
      VERIFY(Instrumented::copyCtorCalls == NumElems, caseLabel);
      VERIFY(Instrumented::moveCtorCalls == 0, caseLabel);
      VERIFY(Instrumented::assignmentCalls == 0, caseLabel);
      VERIFY(Instrumented::moveAssignmentCalls == 0, caseLabel);
      // Destruct original elements and in initalizer list.
      VERIFY(Instrumented::dtorCalls == NumOrigElems + NumElems, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}


void TestSboVectorAt()
{
   {
      const std::string caseLabel{
         "SvoVector::at for reading from valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for writing to valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
      {
         sv.at(i) = 100;
         VERIFY(sv.at(i) == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::at for reading from valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for writing to valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
      {
         sv.at(i) = 100;
         VERIFY(sv.at(i) == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::at for accessing invalid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      VERIFY_THROW(([&sv, Cap]() { sv.at(Cap); }), std::out_of_range, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for accessing invalid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
   }
}


void TestSboVectorAtConst()
{
   {
      const std::string caseLabel{
         "SvoVector::at const for reading from valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for reading from valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for accessing invalid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      VERIFY_THROW(([&sv, Cap]() { sv.at(Cap); }), std::out_of_range, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for accessing invalid index into heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
   }
}


void TestSboVectorSubscriptOperator()
{
   {
      const std::string caseLabel{
         "SvoVector::operator[] for reading from valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for writing to valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
      {
         sv[i] = 100;
         VERIFY(sv[i] == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for reading from valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for writing to valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
      {
         sv[i] = 100;
         VERIFY(sv[i] == 100, caseLabel);
      }
   }
}


void TestSboVectorSubscriptOperatorConst()
{
   {
      const std::string caseLabel{
         "SvoVector::operator[] const for accessing valid index into buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] const for accessing valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
}


void TestSboVectorFront()
{
   {
      const std::string caseLabel{
         "SvoVector::front for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::front for writing to buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      sv.front() = 100;
      VERIFY(sv[0] == 100, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::front for reading from heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::front for writing to valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      sv.front() = 100;
      VERIFY(sv[0] == 100, caseLabel);
   }
}


void TestSboVectorFrontConst()
{
   {
      const std::string caseLabel{
         "SvoVector::front const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::front const for heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
}


void TestSboVectorBack()
{
   {
      const std::string caseLabel{
         "SvoVector::back for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back for writing to buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      sv.back() = 100;
      VERIFY(sv[sv.size() - 1] == 100, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back for reading from heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY(sv.back() == 8, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back for writing to valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      sv.back() = 100;
      VERIFY(sv[sv.size() - 1] == 100, caseLabel);
   }
}


void TestSboVectorBackConst()
{
   {
      const std::string caseLabel{
         "SvoVector::back const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back const for heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      VERIFY(sv.back() == 8, caseLabel);
   }
}


void TestSboVectorData()
{
   {
      const std::string caseLabel{
         "SvoVector::data for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::data for writing to buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
      {
         data[i] = 1000 + i;
         VERIFY(sv[i] == 1000 + i, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::data for reading from heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::data for writing to valid index into heap instance."};

      constexpr std::size_t Cap = 5;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
      {
         data[i] = 1000 + i;
         VERIFY(sv[i] == 1000 + i, caseLabel);
      }
   }
}


void TestSboVectorDataConst()
{
   {
      const std::string caseLabel{
         "SvoVector::data const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      const int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back const for heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > Cap, caseLabel);

      const int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
}

void TestSboVectorEmpty()
{
   {
      const std::string caseLabel{
         "SvoVector::empty for empty instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv;

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);
      VERIFY(sv.size() == 0, caseLabel);

      VERIFY(sv.empty(), caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::empty for non-empty heap instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);
      VERIFY(sv.size() > 0, caseLabel);

      VERIFY(!sv.empty(), caseLabel);
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
   TestSboVectorInitializerListAssignment();
   TestSboVectorAssignElementValue();
   TestSboVectorAssignIteratorRange();
   TestSboVectorAssignInitializerList();
   TestSboVectorAt();
   TestSboVectorAtConst();
   TestSboVectorSubscriptOperator();
   TestSboVectorSubscriptOperatorConst();
   TestSboVectorFront();
   TestSboVectorFrontConst();
   TestSboVectorBack();
   TestSboVectorBackConst();
   TestSboVectorData();
   TestSboVectorDataConst();
   TestSboVectorEmpty();
}
