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
      const std::string caseLabel{"SvoVector::front for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front for writing to buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      sv.front() = 100;
      VERIFY(sv[0] == 100, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front for reading from heap instance."};

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
      const std::string caseLabel{"SvoVector::front const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front const for heap instance."};

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
      const std::string caseLabel{"SvoVector::back for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back for writing to buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      sv.back() = 100;
      VERIFY(sv[sv.size() - 1] == 100, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back for reading from heap instance."};

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
      const std::string caseLabel{"SvoVector::back const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance."};

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
      const std::string caseLabel{"SvoVector::data for reading from buffer instance."};

      constexpr std::size_t Cap = 10;
      SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::data for writing to buffer instance."};

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
      const std::string caseLabel{"SvoVector::data for reading from heap instance."};

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
      const std::string caseLabel{"SvoVector::data const for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < Cap, caseLabel);

      const int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance."};

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
      const std::string caseLabel{"SvoVector::empty for empty instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv;

      // Precondition.
      VERIFY(sv.size() == 0, caseLabel);

      VERIFY(sv.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(sv.size() > 0, caseLabel);

      VERIFY(!sv.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() > 0, caseLabel);

      VERIFY(!sv.empty(), caseLabel);
   }
}


void TestSboVectorSize()
{
   {
      const std::string caseLabel{"SvoVector::size for empty instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv;

      // Precondition.
      VERIFY(sv.empty(), caseLabel);

      VERIFY(sv.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(!sv.empty(), caseLabel);

      VERIFY(sv.size() == 2, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(!sv.empty(), caseLabel);

      VERIFY(sv.size() == 6, caseLabel);
   }
}


void TestSboVectorMaxSize()
{
   {
      const std::string caseLabel{"SvoVector::max_size for buffer instance."};

      constexpr std::size_t Cap = 10;
      const SboVector<int, Cap> sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);

      VERIFY(sv.max_size() > 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::max_size for heap instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);

      VERIFY(sv.max_size() > 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::max_size for empty instance."};

      constexpr std::size_t Cap = 5;
      const SboVector<int, Cap> sv;

      // Precondition.
      VERIFY(sv.empty(), caseLabel);

      VERIFY(sv.max_size() > 0, caseLabel);
   }
}


///////////////////

void TestSboVectorIteratorDefaultCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator default ctor."};

      using SV = SboVector<int, 10>;
      SboVectorIterator<SV> it;

      VERIFY(it == SboVectorIterator<SV>(nullptr, 0), caseLabel);
   }
}


void TestSboVectorIteratorVectorAndIndexCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator ctor for SboVector and index."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorIteratorCopyCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator copy ctor."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};
      SboVectorIterator<SV> copy{it};

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestSboVectorIteratorMoveCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator move ctor."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};
      SboVectorIterator<SV> dest{std::move(it)};

      VERIFY(*dest == 2, caseLabel);
      VERIFY(it == SboVectorIterator<SV>(), caseLabel);
   }
}


void TestSboVectorIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator copy assignment."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};
      SboVectorIterator<SV> copy;
      copy = it;

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestSboVectorIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator move assignment."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};
      SboVectorIterator<SV> dest;
      dest = std::move(it);

      VERIFY(*dest == 2, caseLabel);
      VERIFY(it == SboVectorIterator<SV>(), caseLabel);
   }
}


void TestSboVectorIteratorIndirectionOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator indirection operator for reading."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};

      VERIFY(*it == 2, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator indirection operator for writing."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorIterator<SV> it{&sv, 1};
      *it = 3;

      VERIFY(*it == 3, caseLabel);
   }
}


void TestSboVectorIteratorIndirectionOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const indirection operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      const SboVectorIterator<SV> it{&sv, 1};

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorIteratorDereferenceOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator dereference operator for reading."};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      using SV = SboVector<A, 10>;
      SV sv{{1, 2}, {3, 4}};

      SboVectorIterator<SV> it{&sv, 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator dereference operator for writing."};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      using SV = SboVector<A, 10>;
      SV sv{{1, 2}, {3, 4}};

      SboVectorIterator<SV> it{&sv, 1};
      it->i = 100;
      it->d = 0.001;

      VERIFY(it->i == 100, caseLabel);
      VERIFY(it->d == 0.001, caseLabel);
   }
}


void TestSboVectorIteratorDereferenceOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const dereference operator."};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      using SV = SboVector<A, 10>;
      SV sv{{1, 2}, {3, 4}};

      const SboVectorIterator<SV> it{&sv, 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
}


void TestSboVectorIteratorSubscriptOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator operator[] for reading."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}, {4}};

      SboVectorIterator<SV> it{&sv, 1};

      VERIFY(it[2] == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator[] for writing."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}, {4}};

      SboVectorIterator<SV> it{&sv, 1};
      it[2] = 0;

      VERIFY(it[2] == 0, caseLabel);
   }
}


void TestSboVectorIteratorSubscriptOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const operator[] for reading."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}, {4}};

      const SboVectorIterator<SV> it{&sv, 1};

      VERIFY(it[2] == 4, caseLabel);
   }
}


void TestSboVectorIteratorPrefixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 1};
      auto same = ++it;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*same == 3, caseLabel);
   }
}


void TestSboVectorIteratorPostfixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 1};
      auto prev = it++;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestSboVectorIteratorPrefixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix decrement operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 1};
      auto same = --it;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*same == 1, caseLabel);
   }
}


void TestSboVectorIteratorPostfixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 1};
      auto prev = it--;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestSboVectorIteratorSwap()
{
   {
      const std::string caseLabel{"SboVectorIterator swap."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&sv, 2};

      swap(a, b);

      VERIFY(*a == 3, caseLabel);
      VERIFY(*b == 2, caseLabel);
   }
}


void TestSboVectorIteratorEquality()
{
   {
      const std::string caseLabel{"SboVectorIterator equality for equal values."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&sv, 1};

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator equality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator equality for different vectors."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};
      SV other{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&other, 1};

      VERIFY(!(a == b), caseLabel);
   }
}


void TestSboVectorIteratorInequality()
{
   {
      const std::string caseLabel{"SboVectorIterator inequality for equal values."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&sv, 1};

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator inequality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator inequality for different vectors."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};
      SV other{{1}, {2}, {3}};

      SboVectorIterator<SV> a{&sv, 1};
      SboVectorIterator<SV> b{&other, 1};

      VERIFY(a != b, caseLabel);
   }
}


void TestSboVectorIteratorAdditionAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator operator+= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 0};
      it += 2;

      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator+= for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 2};
      it += -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorIteratorSubtractionAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator operator-= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 2};
      it -= 2;

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator-= for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 0};
      it -= -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorIteratorAdditionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorIterator iterator plus offset for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 0};
      SboVectorIterator<SV> res = it + 2;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator iterator plus offset for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 2};
      SboVectorIterator<SV> res = it + -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorIteratorAdditionOfOffsetAndIterator()
{
   {
      const std::string caseLabel{
         "SboVectorIterator offset plus iterator for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 0};
      SboVectorIterator<SV> res = 2 + it;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator offset plus iterator for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 2};
      SboVectorIterator<SV> res = -1 + it;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorIteratorSubtractionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorIterator iterator minus offset for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 2};
      SboVectorIterator<SV> res = it - 2;

      VERIFY(*res == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator iterator minus offset for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorIterator<SV> it{&sv, 0};
      SboVectorIterator<SV> res = it - -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorIteratorSubtractionOfIterators()
{
   {
      const std::string caseLabel{"SboVectorIterator iterator minus iterator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 0};
      const std::size_t diff = a - b;

      VERIFY(diff == 2, caseLabel);
   }
}


void TestSboVectorIteratorLessThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator< for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a < b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator< for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 0};

      VERIFY(!(a < b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator< for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a < b), caseLabel);
   }
}


void TestSboVectorIteratorLessOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator<= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator<= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 0};

      VERIFY(!(a <= b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator<= for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
}


void TestSboVectorIteratorGreaterThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator> for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a > b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator> for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 0};

      VERIFY(a > b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator> for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a > b), caseLabel);
   }
}


void TestSboVectorIteratorGreaterOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator>= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator>= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 0};

      VERIFY(a >= b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator>= for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 2};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a >= b, caseLabel);
   }
}


///////////////////

void TestSboVectorConstIteratorDefaultCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator default ctor."};

      using SV = SboVector<int, 10>;
      SboVectorConstIterator<SV> it;

      VERIFY(it == SboVectorConstIterator<SV>(nullptr, 0), caseLabel);
   }
}


void TestSboVectorConstIteratorVectorAndIndexCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator ctor for SboVector and index."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorCopyCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator copy ctor."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};
      SboVectorConstIterator<SV> copy{it};

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorMoveCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator move ctor."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};
      SboVectorConstIterator<SV> dest{std::move(it)};

      VERIFY(*dest == 2, caseLabel);
      VERIFY(it == SboVectorConstIterator<SV>(), caseLabel);
   }
}


void TestSboVectorConstIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator copy assignment."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};
      SboVectorConstIterator<SV> copy;
      copy = it;

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator move assignment."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};
      SboVectorConstIterator<SV> dest;
      dest = std::move(it);

      VERIFY(*dest == 2, caseLabel);
      VERIFY(it == SboVectorConstIterator<SV>(), caseLabel);
   }
}


void TestSboVectorConstIteratorIndirectionOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const indirection operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}};

      SboVectorConstIterator<SV> it{&sv, 1};

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorDereferenceOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const dereference operator."};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      using SV = SboVector<A, 10>;
      SV sv{{1, 2}, {3, 4}};

      SboVectorConstIterator<SV> it{&sv, 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
}


void TestSboVectorConstIteratorSubscriptOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const operator[] for reading."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}, {4}};

      SboVectorConstIterator<SV> it{&sv, 1};

      VERIFY(it[2] == 4, caseLabel);
   }
}


void TestSboVectorConstIteratorPrefixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 1};
      auto same = ++it;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*same == 3, caseLabel);
   }
}


void TestSboVectorConstIteratorPostfixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 1};
      auto prev = it++;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorPrefixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix decrement operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 1};
      auto same = --it;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*same == 1, caseLabel);
   }
}


void TestSboVectorConstIteratorPostfixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 1};
      auto prev = it--;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorSwap()
{
   {
      const std::string caseLabel{"SboVectorConstIterator swap."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 2};

      swap(a, b);

      VERIFY(*a == 3, caseLabel);
      VERIFY(*b == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorEquality()
{
   {
      const std::string caseLabel{"SboVectorConstIterator equality for equal values."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 1};

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator equality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator equality for different vectors."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};
      SV other{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&other, 1};

      VERIFY(!(a == b), caseLabel);
   }
}


void TestSboVectorConstIteratorInequality()
{
   {
      const std::string caseLabel{"SboVectorConstIterator inequality for equal values."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 1};

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator inequality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator inequality for different vectors."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};
      SV other{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&other, 1};

      VERIFY(a != b, caseLabel);
   }
}


void TestSboVectorConstIteratorAdditionAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator+= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      it += 2;

      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator+= for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      it += -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorSubtractionAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator-= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      it -= 2;

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator-= for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      it -= -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorAdditionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator plus offset for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      SboVectorConstIterator<SV> res = it + 2;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator plus offset for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      SboVectorConstIterator<SV> res = it + -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorAdditionOfOffsetAndIterator()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator offset plus iterator for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      SboVectorConstIterator<SV> res = 2 + it;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator offset plus iterator for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      SboVectorConstIterator<SV> res = -1 + it;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorSubtractionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator minus offset for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      SboVectorConstIterator<SV> res = it - 2;

      VERIFY(*res == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator minus offset for negative offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      SboVectorConstIterator<SV> res = it - -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorSubtractionOfIterators()
{
   {
      const std::string caseLabel{"SboVectorConstIterator iterator minus iterator."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};
      const std::size_t diff = a - b;

      VERIFY(diff == 2, caseLabel);
   }
}


void TestSboVectorConstIteratorLessThan()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator< for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a < b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator< for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(!(a < b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator< for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a < b), caseLabel);
   }
}


void TestSboVectorConstIteratorLessOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator<= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator<= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(!(a <= b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator<= for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
}


void TestSboVectorConstIteratorGreaterThan()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator> for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a > b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator> for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(a > b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator> for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a > b), caseLabel);
   }
}


void TestSboVectorConstIteratorGreaterOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorConstIterator operator>= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator>= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(a >= b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator>= for equal iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a >= b, caseLabel);
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
   TestSboVectorSize();
   TestSboVectorMaxSize();

   TestSboVectorIteratorDefaultCtor();
   TestSboVectorIteratorVectorAndIndexCtor();
   TestSboVectorIteratorCopyCtor();
   TestSboVectorIteratorMoveCtor();
   TestSboVectorIteratorCopyAssignment();
   TestSboVectorIteratorMoveAssignment();
   TestSboVectorIteratorIndirectionOperator();
   TestSboVectorIteratorIndirectionOperatorConst();
   TestSboVectorIteratorDereferenceOperator();
   TestSboVectorIteratorDereferenceOperatorConst();
   TestSboVectorIteratorSubscriptOperator();
   TestSboVectorIteratorSubscriptOperatorConst();
   TestSboVectorIteratorPrefixIncrementOperator();
   TestSboVectorIteratorPostfixIncrementOperator();
   TestSboVectorIteratorPrefixDecrementOperator();
   TestSboVectorIteratorPostfixDecrementOperator();
   TestSboVectorIteratorSwap();
   TestSboVectorIteratorEquality();
   TestSboVectorIteratorInequality();
   TestSboVectorIteratorAdditionAssignment();
   TestSboVectorIteratorSubtractionAssignment();
   TestSboVectorIteratorAdditionOfIteratorAndOffset();
   TestSboVectorIteratorAdditionOfOffsetAndIterator();
   TestSboVectorIteratorSubtractionOfIteratorAndOffset();
   TestSboVectorIteratorSubtractionOfIterators();
   TestSboVectorIteratorLessThan();
   TestSboVectorIteratorLessOrEqualThan();
   TestSboVectorIteratorGreaterThan();
   TestSboVectorIteratorGreaterOrEqualThan();

   TestSboVectorConstIteratorDefaultCtor();
   TestSboVectorConstIteratorVectorAndIndexCtor();
   TestSboVectorConstIteratorCopyCtor();
   TestSboVectorConstIteratorMoveCtor();
   TestSboVectorConstIteratorCopyAssignment();
   TestSboVectorConstIteratorMoveAssignment();
   TestSboVectorConstIteratorIndirectionOperatorConst();
   TestSboVectorConstIteratorDereferenceOperatorConst();
   TestSboVectorConstIteratorSubscriptOperatorConst();
   TestSboVectorConstIteratorPrefixIncrementOperator();
   TestSboVectorConstIteratorPostfixIncrementOperator();
   TestSboVectorConstIteratorPrefixDecrementOperator();
   TestSboVectorConstIteratorPostfixDecrementOperator();
   TestSboVectorConstIteratorSwap();
   TestSboVectorConstIteratorEquality();
   TestSboVectorConstIteratorInequality();
   TestSboVectorConstIteratorAdditionAssignment();
   TestSboVectorConstIteratorSubtractionAssignment();
   TestSboVectorConstIteratorAdditionOfIteratorAndOffset();
   TestSboVectorConstIteratorAdditionOfOffsetAndIterator();
   TestSboVectorConstIteratorSubtractionOfIteratorAndOffset();
   TestSboVectorConstIteratorSubtractionOfIterators();
   TestSboVectorConstIteratorLessThan();
   TestSboVectorConstIteratorLessOrEqualThan();
   TestSboVectorConstIteratorGreaterThan();
   TestSboVectorConstIteratorGreaterOrEqualThan();
}
