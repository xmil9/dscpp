// Turn on memory instrumentation for SboVector.
#define SBOVEC_MEM_INSTR
#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <Windows.h>


namespace
{
///////////////////

struct Element
{
   Element() { ++m_instrumented.defaultCtorCalls; }
   Element(int i_) : i{i_} { ++m_instrumented.ctorCalls; }
   Element(const Element& other) : d{other.d}, i{other.i}, b{other.b}
   {
      ++m_instrumented.copyCtorCalls;
   }
   Element(Element&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      ++m_instrumented.moveCtorCalls;
   }
   ~Element() { ++m_instrumented.dtorCalls; }
   Element& operator=(const Element& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      ++m_instrumented.assignmentCalls;
   }
   Element& operator=(Element&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      ++m_instrumented.moveAssignmentCalls;
   }

   double d = 1.0;
   int i = 1;
   bool b = true;

   // Instrumentation.

   struct Measures
   {
      std::size_t defaultCtorCalls = 0;
      std::size_t ctorCalls = 0;
      std::size_t copyCtorCalls = 0;
      std::size_t moveCtorCalls = 0;
      std::size_t assignmentCalls = 0;
      std::size_t moveAssignmentCalls = 0;
      std::size_t dtorCalls = 0;
   };

   inline static Measures m_instrumented;

   static void resetInstrumentation()
   {
      m_instrumented.defaultCtorCalls = 0;
      m_instrumented.ctorCalls = 0;
      m_instrumented.copyCtorCalls = 0;
      m_instrumented.moveCtorCalls = 0;
      m_instrumented.assignmentCalls = 0;
      m_instrumented.moveAssignmentCalls = 0;
      m_instrumented.dtorCalls = 0;
   }

   static void verifyInstrumentation(const Measures& expected,
                                     const std::string& caseLabel)
   {
      VERIFY(expected.defaultCtorCalls == m_instrumented.defaultCtorCalls, caseLabel);
      VERIFY(expected.ctorCalls == m_instrumented.ctorCalls, caseLabel);
      VERIFY(expected.copyCtorCalls == m_instrumented.copyCtorCalls, caseLabel);
      VERIFY(expected.moveCtorCalls == m_instrumented.moveCtorCalls, caseLabel);
      VERIFY(expected.assignmentCalls == m_instrumented.assignmentCalls, caseLabel);
      VERIFY(expected.moveAssignmentCalls == m_instrumented.moveAssignmentCalls,
             caseLabel);
      VERIFY(expected.dtorCalls == m_instrumented.dtorCalls, caseLabel);
   }
};


///////////////////

struct NotMoveableElement
{
   NotMoveableElement() { ++m_instrumented.defaultCtorCalls; }
   NotMoveableElement(int i_) : i{i_} { ++m_instrumented.ctorCalls; }
   NotMoveableElement(const NotMoveableElement& other)
   : d{other.d}, i{other.i}, b{other.b}
   {
      ++m_instrumented.copyCtorCalls;
   }
   NotMoveableElement(NotMoveableElement&& other) = delete;
   ~NotMoveableElement() { ++m_instrumented.dtorCalls; }
   NotMoveableElement& operator=(const NotMoveableElement& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      ++m_instrumented.assignmentCalls;
   }
   NotMoveableElement& operator=(NotMoveableElement&& other) = delete;

   double d = 1.0;
   int i = 1;
   bool b = true;

   // Instrumentation.

   struct Measures
   {
      std::size_t defaultCtorCalls = 0;
      std::size_t ctorCalls = 0;
      std::size_t copyCtorCalls = 0;
      std::size_t assignmentCalls = 0;
      std::size_t dtorCalls = 0;
   };

   inline static Measures m_instrumented;

   static void resetInstrumentation()
   {
      m_instrumented.defaultCtorCalls = 0;
      m_instrumented.ctorCalls = 0;
      m_instrumented.copyCtorCalls = 0;
      m_instrumented.assignmentCalls = 0;
      m_instrumented.dtorCalls = 0;
   }

   static void verifyInstrumentation(const Measures& expected,
                                     const std::string& caseLabel)
   {
      VERIFY(expected.defaultCtorCalls == m_instrumented.defaultCtorCalls, caseLabel);
      VERIFY(expected.ctorCalls == m_instrumented.ctorCalls, caseLabel);
      VERIFY(expected.copyCtorCalls == m_instrumented.copyCtorCalls, caseLabel);
      VERIFY(expected.assignmentCalls == m_instrumented.assignmentCalls, caseLabel);
      VERIFY(expected.dtorCalls == m_instrumented.dtorCalls, caseLabel);
   }
};


///////////////////

// RAII class to verify the instrumentation measurements of a given element type.
template <typename Elem> class ElementVerifier
{
 public:
   ElementVerifier(const typename Elem::Measures& expected, std::string caseLabel)
   : m_expected{expected}, m_caseLabel{std::move(caseLabel)}
   {
      Elem::resetInstrumentation();
   }
   ~ElementVerifier() { Elem::verifyInstrumentation(m_expected, m_caseLabel); }

 private:
   typename Elem::Measures m_expected = 0;
   std::string m_caseLabel;
};


///////////////////

// RAII class to verify the memory management of a given SboVector type.
template <typename SV> class MemVerifier
{
 public:
   explicit MemVerifier(std::string caseLabel) : MemVerifier(0, caseLabel) {}
   MemVerifier(int64_t expectedCap, std::string caseLabel)
   : m_expectedCap{expectedCap}, m_caseLabel{std::move(caseLabel)}
   {
      SV::resetAllocatedCapacity();
   }
   ~MemVerifier() { VERIFY(SV::allocatedCapacity() == m_expectedCap, m_caseLabel); }

 private:
   int64_t m_expectedCap = 0;
   std::string m_caseLabel;
};


///////////////////

void TestSboVectorDefaultCtor()
{
   {
      const std::string caseLabel{"SboVector default ctor."};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Instrumentation.
      const MemVerifier<SV> memCheck{caseLabel};

      {
         const ElementVerifier<Elem> elemCheck{{0, 0, 0, 0, 0, 0, 0}, caseLabel};

         // Test.
         SV sv;

         // Verify vector state.
         VERIFY(sv.empty(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
      }
   }
}


void TestSboVectorCtorForElementCountAndValue()
{
   {
      const std::string caseLabel{"SboVector count-and-value ctor for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      using Elem = Element;
      using SV = SboVector<Element, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Preconditions.
      VERIFY(BufCap >= NumElems, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Creation of passed-in instance.
         expected.ctorCalls = 1;
         // Creation of elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of passed-in instance and elements.
         expected.dtorCalls = 1 + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv(NumElems, {2});

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == 2, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Preconditions.
      VERIFY(BufCap < NumElems, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Creation of passed-in instance.
         expected.ctorCalls = 1;
         // Creation of elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of passed-in instance and elements.
         expected.dtorCalls = 1 + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv(NumElems, {2});

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == NumElems, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == 2, caseLabel);
      }
   }
}


void TestSboVectorCopyCtor()
{
   {
      const std::string caseLabel{"SboVector copy ctor for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {1});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Preconditions.
      VERIFY(src.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{src};

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SboVector copy ctor for heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {1});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{src};

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == NumElems, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i, caseLabel);
      }
   }
}


void TestSboVectorMoveCtor()
{
   {
      const std::string caseLabel{"SboVector move ctor for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Moved elements.
         expected.moveCtorCalls = NumElems;
         // Destruction of elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{std::move(src)};

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i, caseLabel);
         // Verify moved-from instance is empty.
         VERIFY(src.size() == 0, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SboVector move ctor for heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      // Precondition.
      VERIFY(src.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // No moves because the SboVector simply stole the pointer to the heap memory.
         expected.moveCtorCalls = 0;
         // Destruction of elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{std::move(src)};

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == NumElems, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i, caseLabel);
         // Verify moved-from instance is empty.
         VERIFY(src.size() == 0, caseLabel);
      }
   }
}


void TestSboVectorInitializerListCtor()
{
   {
      const std::string caseLabel{"SboVector initializer list ctor for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 4;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Precondition.
      VERIFY(NumElems < BufCap, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copy constructing vector elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of elements in vector and initializer list.
         expected.dtorCalls = 2 * NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{{1}, {2}, {3}, {4}};

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i + 1, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SboVector initializer list ctor for heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 12;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Precondition.
      VERIFY(NumElems > BufCap, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copy constructing vector elements.
         expected.copyCtorCalls = NumElems;
         // Destruction of elements in vector and initializer list.
         expected.dtorCalls = 2 * NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         SV sv{
            {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12},
         };

         // Verify vector state.
         VERIFY(sv.size() == NumElems, caseLabel);
         VERIFY(sv.capacity() == NumElems, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i].i == i + 1, caseLabel);
      }
   }
}


void TestSboVectorDtor()
{
   {
      const std::string caseLabel{"SboVector dtor for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation.
      const MemVerifier<SV> memCheck{caseLabel};

      // Precondition.
      VERIFY(NumElems < BufCap, caseLabel);

      {
         SV sv{{1}, {2}, {3}};

         // Reset element instrumentation right before the SboVector gets destroyed
         // to only verify the destruction of the vector elements.
         Elem::resetInstrumentation();

         // Test.
         // End of scope triggers dtor.
      }

      VERIFY(Elem::m_instrumented.dtorCalls == NumElems, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector dtor for heap instance."};

      constexpr std::size_t BufCap = 3;
      constexpr std::size_t NumElems = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation.
      const MemVerifier<SV> memCheck{caseLabel};

      // Precondition.
      VERIFY(NumElems > BufCap, caseLabel);

      {
         SV sv{{1}, {2}, {3}, {4}, {5}};

         // Reset element instrumentation right before the SboVector gets destroyed
         // to only verify the destruction of the vector elements.
         Elem::resetInstrumentation();
      }

      VERIFY(Elem::m_instrumented.dtorCalls == NumElems, caseLabel);
   }
}


void TestSboVectorCopyAssignment()
{
   {
      const std::string caseLabel{
         "SboVector copy assignment of buffer instance to buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = src;
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment of heap instance to buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = src;
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment of buffer instance to heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = src;
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment of larger heap instance to smaller heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 15;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumElems > NumOrigElems, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = src;
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a larger heap allocation will trigger a new
      // allocation. Capacity will increase to larger size.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment of smaller heap instance to larger heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 15;
      constexpr std::size_t NumOrigElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumElems < NumOrigElems, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = src;
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Assigning data that needs a smaller heap allocation will reuse the existing
      // heap memory. Capacity will remain at previous (larger) size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
}


void TestSboVectorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Moved elements.
         expected.moveCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = std::move(src);
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Capacity of buffer.
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of heap "
                                  "instance to buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // No moves because the SboVector simply stole the pointer to the heap memory.
         expected.moveCtorCalls = 0;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = std::move(src);
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Will have capacity of stolen source heap memory.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of buffer "
                                  "instance to heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 5;
      constexpr std::size_t NumOrigElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(src.inBuffer(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Moved elements.
         expected.moveCtorCalls = NumElems;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = std::move(src);
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Elements fit into buffer.
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of smaller heap "
                                  "instance to larger heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 15;
      constexpr std::size_t NumOrigElems = 20;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumElems < NumOrigElems, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // No moves because the SboVector simply stole the pointer to the heap memory.
         expected.moveCtorCalls = 0;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = std::move(src);
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move assignment of larger heap "
                                  "instance to smaller heap instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 20;
      constexpr std::size_t NumOrigElems = 15;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV src(NumElems, {2});
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumElems > NumOrigElems, caseLabel);
      VERIFY(src.onHeap(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // No moves because the SboVector simply stole the pointer to the heap memory.
         expected.moveCtorCalls = 0;
         // Original elements got destroyed.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = std::move(src);
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Will take over the stolen capacity of the source.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i, caseLabel);
   }
}


void TestSboVectorInitializerListAssignment()
{
   {
      const std::string caseLabel{
         "SboVector initializer list assignment that fits in buffer to buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and for items in ilist.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = {{1}, {2}};
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that requires "
                                  "heap to buffer instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and for items in ilist.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment  "
                                  "that fits in buffer to heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and for items in ilist.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = {{1}, {2}, {3}};
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "but can reuse the heap of the target instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and for items in ilist.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "and cannot reuse the heap of the target instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 10;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing initializer list elements.
         expected.ctorCalls = NumElems;
         // Copied elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and for items in ilist.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv = {{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}};
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
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

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing assigned element.
         expected.ctorCalls = 1;
         // Populated elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and the assigned element.
         expected.dtorCalls = NumOrigElems + 1;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(NumElems, Element{10});
      }


      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector assign element value n-times. Assigned "
                                  "values require heap allocation. "
                                  "SboVector was a buffer instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing assigned element.
         expected.ctorCalls = 1;
         // Populated elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and the assigned element.
         expected.dtorCalls = NumOrigElems + 1;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(NumElems, Element{10});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned "
         "values fit into buffer. SboVector was a heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing assigned element.
         expected.ctorCalls = 1;
         // Populated elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and the assigned element.
         expected.dtorCalls = NumOrigElems + 1;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(NumElems, Elem{10});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector was a smaller heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 10;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing assigned element.
         expected.ctorCalls = 1;
         // Populated elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and the assigned element.
         expected.dtorCalls = NumOrigElems + 1;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(NumElems, Elem{10});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector was a larger heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Constructing assigned element.
         expected.ctorCalls = 1;
         // Populated elements.
         expected.copyCtorCalls = NumElems;
         // Dtor for original items in SboVector and the assigned element.
         expected.dtorCalls = NumOrigElems + 1;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(NumElems, Element{10});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Reused heap stays at larger size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
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

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const std::list<Element> src{{1}, {2}};
      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(src.begin(), src.end());
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values require heap. "
         "SboVector was a buffer instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const std::list<Element> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(src.begin(), src.end());
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned fit into buffer. "
         "SboVector was a heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const std::list<Element> src{{1}, {2}, {3}};
      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(src.begin(), src.end());
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned require heap. "
         "SboVector was a smaller heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 8;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const std::list<Element> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};
      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(src.begin(), src.end());
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned require heap. "
         "SboVector was a larger heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 8;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const std::list<Element> src{{1}, {2}, {3}, {4}, {5}, {6}, {7}};
      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements.
         expected.dtorCalls = NumOrigElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign(src.begin(), src.end());
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      // Capacity remains at larger, reused size.
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
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

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 2;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements constructed by initalizer list.
         expected.ctorCalls = NumElems;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements and in initalizer list.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign({{1}, {2}});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a buffer instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements constructed by initalizer list.
         expected.ctorCalls = NumElems;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements and in initalizer list.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values fit into buffer. "
         "SboVector was a heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 3;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems < BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements constructed by initalizer list.
         expected.ctorCalls = NumElems;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements and in initalizer list.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign({{1}, {2}, {3}});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a smaller heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 8;
      constexpr std::size_t NumOrigElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems < NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements constructed by initalizer list.
         expected.ctorCalls = NumElems;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements and in initalizer list.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector was a larger heap instance."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t NumElems = 7;
      constexpr std::size_t NumOrigElems = 8;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumOrigElems, {1});

      // Preconditions.
      VERIFY(NumElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > BufCap, caseLabel);
      VERIFY(NumOrigElems > NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements constructed by initalizer list.
         expected.ctorCalls = NumElems;
         // Assigned elements.
         expected.copyCtorCalls = NumElems;
         // Destruct original elements and in initalizer list.
         expected.dtorCalls = NumOrigElems + NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.assign({{1}, {2}, {3}, {4}, {5}, {6}, {7}});
      }

      // Verify vector state.
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == NumOrigElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
}


void TestSboVectorAt()
{
   {
      const std::string caseLabel{
         "SvoVector::at for reading from valid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for writing to valid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
      {
         sv.at(i) = 100;
         VERIFY(sv.at(i) == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::at for reading from valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for writing to valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
      {
         sv.at(i) = 100;
         VERIFY(sv.at(i) == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::at for accessing invalid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      VERIFY_THROW(([&sv, BufCap]() { sv.at(BufCap); }), std::out_of_range, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at for accessing invalid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
   }
}


void TestSboVectorAtConst()
{
   {
      const std::string caseLabel{
         "SvoVector::at const for reading from valid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for reading from valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv.at(i) == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for accessing invalid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      VERIFY_THROW(([&sv, BufCap]() { sv.at(BufCap); }), std::out_of_range, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::at const for accessing invalid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      VERIFY_THROW(([&sv]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
   }
}


void TestSboVectorSubscriptOperator()
{
   {
      const std::string caseLabel{
         "SvoVector::operator[] for reading from valid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for writing to valid index into buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
      {
         sv[i] = 100;
         VERIFY(sv[i] == 100, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for reading from valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] for writing to valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
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

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::operator[] const for accessing valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
}


void TestSboVectorFront()
{
   {
      const std::string caseLabel{"SvoVector::front for reading from buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front for writing to buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      sv.front() = 100;
      VERIFY(sv[0] == 100, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front for reading from heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::front for writing to valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      sv.front() = 100;
      VERIFY(sv[0] == 100, caseLabel);
   }
}


void TestSboVectorFrontConst()
{
   {
      const std::string caseLabel{"SvoVector::front const for buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      VERIFY(sv.front() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::front const for heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      VERIFY(sv.front() == 1, caseLabel);
   }
}


void TestSboVectorBack()
{
   {
      const std::string caseLabel{"SvoVector::back for reading from buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back for writing to buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      sv.back() = 100;
      VERIFY(sv[sv.size() - 1] == 100, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back for reading from heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      VERIFY(sv.back() == 8, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::back for writing to valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      sv.back() = 100;
      VERIFY(sv[sv.size() - 1] == 100, caseLabel);
   }
}


void TestSboVectorBackConst()
{
   {
      const std::string caseLabel{"SvoVector::back const for buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      VERIFY(sv.back() == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      VERIFY(sv.back() == 8, caseLabel);
   }
}


void TestSboVectorData()
{
   {
      const std::string caseLabel{"SvoVector::data for reading from buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::data for writing to buffer instance."};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
      {
         data[i] = 1000 + i;
         VERIFY(sv[i] == 1000 + i, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SvoVector::data for reading from heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::data for writing to valid index into heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
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

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}};

      // Precondition.
      VERIFY(sv.size() < BufCap, caseLabel);

      // Test.
      const int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance."};

      constexpr std::size_t BufCap = 5;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

      // Precondition.
      VERIFY(sv.size() > BufCap, caseLabel);

      // Test.
      const int* data = sv.data();
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(data[i] == i + 1, caseLabel);
   }
}


void TestSboVectorBegin()
{
   {
      const std::string caseLabel{"SboVector::begin for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::iterator first = sv.begin();

      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::begin for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::iterator first = sv.begin();

      VERIFY(first == sv.end(), caseLabel);
   }
}


void TestSboVectorEnd()
{
   {
      const std::string caseLabel{"SboVector::end for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      SV::iterator last = sv.end();

      // Test.
      VERIFY(last != sv.begin(), caseLabel);
      VERIFY(last == sv.begin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::end for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::iterator last = sv.end();

      VERIFY(last == sv.begin(), caseLabel);
   }
}


void TestSboVectorBeginConst()
{
   {
      const std::string caseLabel{"SboVector::begin const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_iterator first = sv.begin();

      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::begin const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_iterator first = sv.begin();

      VERIFY(first == sv.end(), caseLabel);
   }
}


void TestSboVectorEndConst()
{
   {
      const std::string caseLabel{"SboVector::end const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      SV::const_iterator last = sv.end();

      // Test.
      VERIFY(last != sv.begin(), caseLabel);
      VERIFY(last == sv.begin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::end const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_iterator last = sv.end();

      VERIFY(last == sv.begin(), caseLabel);
   }
}


void TestSboVectorCBegin()
{
   {
      const std::string caseLabel{"SboVector::cbegin const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_iterator first = sv.cbegin();

      VERIFY(*first == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::cbegin const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_iterator first = sv.cbegin();

      VERIFY(first == sv.cend(), caseLabel);
   }
}


void TestSboVectorCEnd()
{
   {
      const std::string caseLabel{"SboVector::cend const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_iterator last = sv.cend();

      VERIFY(last != sv.cbegin(), caseLabel);
      VERIFY(last == sv.cbegin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::cend const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_iterator last = sv.cend();

      VERIFY(last == sv.cbegin(), caseLabel);
   }
}


void TestSboVectorRBegin()
{
   {
      const std::string caseLabel{"SboVector::rbegin for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::reverse_iterator rfirst = sv.rbegin();

      VERIFY(*rfirst == 20, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::rbegin for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::reverse_iterator rfirst = sv.rbegin();

      VERIFY(rfirst == sv.rend(), caseLabel);
   }
}


void TestSboVectorREnd()
{
   {
      const std::string caseLabel{"SboVector::rend for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::reverse_iterator rlast = sv.rend();

      VERIFY(rlast != sv.rbegin(), caseLabel);
      VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::rend for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::reverse_iterator rlast = sv.rend();

      VERIFY(rlast == sv.rbegin(), caseLabel);
   }
}


void TestSboVectorRBeginConst()
{
   {
      const std::string caseLabel{"SboVector::rbegin const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rfirst = sv.rbegin();

      VERIFY(*rfirst == 20, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::rbegin const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rfirst = sv.rbegin();

      VERIFY(rfirst == sv.rend(), caseLabel);
   }
}


void TestSboVectorREndConst()
{
   {
      const std::string caseLabel{"SboVector::rend const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rlast = sv.rend();

      VERIFY(rlast != sv.rbegin(), caseLabel);
      VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::rend const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rlast = sv.rend();

      VERIFY(rlast == sv.rbegin(), caseLabel);
   }
}


void TestSboVectorCRBegin()
{
   {
      const std::string caseLabel{"SboVector::crbegin const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv{{1}, {2}, {20}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rfirst = sv.crbegin();

      VERIFY(*rfirst == 20, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::crbegin const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rfirst = sv.crbegin();

      VERIFY(rfirst == sv.crend(), caseLabel);
   }
}


void TestSboVectorCREnd()
{
   {
      const std::string caseLabel{"SboVector::crend const for populated vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Preconditions.
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rlast = sv.crend();

      VERIFY(rlast != sv.crbegin(), caseLabel);
      VERIFY(rlast == sv.crbegin() + sv.size(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector::crend const for empty vector."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Preconditions.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      SV::const_reverse_iterator rlast = sv.crend();

      VERIFY(rlast == sv.crbegin(), caseLabel);
   }
}


void TestSboVectorEmpty()
{
   {
      const std::string caseLabel{"SvoVector::empty for empty instance."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Precondition.
      VERIFY(sv.size() == 0, caseLabel);

      // Test.
      VERIFY(sv.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(sv.size() > 0, caseLabel);

      // Test.
      VERIFY(!sv.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty heap instance."};

      constexpr std::size_t BufCap = 5;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() > 0, caseLabel);

      // Test.
      VERIFY(!sv.empty(), caseLabel);
   }
}


void TestSboVectorSize()
{
   {
      const std::string caseLabel{"SvoVector::size for empty instance."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Precondition.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      VERIFY(sv.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      VERIFY(sv.size() == 2, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance."};

      constexpr std::size_t BufCap = 5;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(!sv.empty(), caseLabel);

      // Test.
      VERIFY(sv.size() == 6, caseLabel);
   }
}


void TestSboVectorMaxSize()
{
   {
      const std::string caseLabel{"SvoVector::max_size for buffer instance."};

      constexpr std::size_t BufCap = 10;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}};

      // Precondition.
      VERIFY(sv.inBuffer(), caseLabel);

      // Test.
      VERIFY(sv.max_size() > 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::max_size for heap instance."};

      constexpr std::size_t BufCap = 5;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv{{1}, {2}, {3}, {4}, {5}, {6}};

      // Precondition.
      VERIFY(sv.onHeap(), caseLabel);

      // Test.
      VERIFY(sv.max_size() > 0, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::max_size for empty instance."};

      constexpr std::size_t BufCap = 5;
      using SV = SboVector<int, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      const SV sv;

      // Precondition.
      VERIFY(sv.empty(), caseLabel);

      // Test.
      VERIFY(sv.max_size() > 0, caseLabel);
   }
}


void TestSboVectorReserve()
{
   {
      const std::string caseLabel{"SvoVector::reserve for capacity less than current."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t OrigCap = 10;
      constexpr std::size_t NewCap = 9;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, {5});

      // Preconditions.
      VERIFY(NewCap < sv.capacity(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // All expected values are zero because the reserve call is a no-op.
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == OrigCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::reserve for capacity larger than max size."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t OrigCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, {5});

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // All expected values are zero because the reserve call throws without making.
         // changes.
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         VERIFY_THROW(([&sv]() { sv.reserve(sv.max_size() + 1); }), std::length_error,
                      caseLabel);
      }
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "with type that is moveable."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t OrigCap = 10;
      constexpr std::size_t NewCap = 15;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, 5);

      // Preconditions.
      VERIFY(OrigCap > BufCap, caseLabel);
      VERIFY(NewCap > sv.capacity(), caseLabel);
      VERIFY(std::is_move_constructible_v<Elem>, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements are moved.
         expected.moveCtorCalls = OrigCap;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NewCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "with type that is not moveable."};

      constexpr std::size_t BufCap = 5;
      constexpr std::size_t OrigCap = 10;
      constexpr std::size_t NewCap = 15;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, 5);

      // Preconditions.
      VERIFY(OrigCap > BufCap, caseLabel);
      VERIFY(NewCap > sv.capacity(), caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copy elements to larger allocation.
         expected.copyCtorCalls = OrigCap;
         // Destroy previous elements.
         expected.dtorCalls = OrigCap;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NewCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "where current data is in buffer."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t OrigCap = 5;
      constexpr std::size_t NewCap = 15;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, 5);

      // Preconditions.
      VERIFY(OrigCap < BufCap, caseLabel);
      VERIFY(NewCap > BufCap, caseLabel);
      VERIFY(NewCap > sv.capacity(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements are moved.
         expected.moveCtorCalls = OrigCap;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NewCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::reserve for capacity larger than current "
         "where current data is in buffer with type that is not moveable."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t OrigCap = 5;
      constexpr std::size_t NewCap = 15;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SboVector<NotMoveableElement, BufCap> sv(OrigCap, 5);

      // Preconditions.
      VERIFY(OrigCap < BufCap, caseLabel);
      VERIFY(NewCap > BufCap, caseLabel);
      VERIFY(NewCap > sv.capacity(), caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copy elements to larger allocation.
         expected.copyCtorCalls = OrigCap;
         // Destroy previous elements.
         expected.dtorCalls = OrigCap;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NewCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "but smaller than buffer."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t OrigCap = 5;
      constexpr std::size_t NewCap = 8;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(OrigCap, 5);

      // Preconditions.
      VERIFY(OrigCap < BufCap, caseLabel);
      VERIFY(NewCap < BufCap, caseLabel);
      VERIFY(NewCap > OrigCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // It's a no-op.
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.reserve(NewCap);
      }

      // Verify vector state.
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.size() == OrigCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
}


void TestSboVectorShrinkToFit()
{
   {
      const std::string caseLabel{"SvoVector::shrink_to_fit for buffer instance."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumElems, {5});

      // Preconditions.
      VERIFY(sv.inBuffer(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // All expected values are zero because the call is a no-op.
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      // Nothing changed.
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with capacity fully occupied."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t NumElems = 15;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      SV sv(NumElems, {5});

      // Preconditions.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() == sv.capacity(), caseLabel);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // All expected values are zero because the call is a no-op.
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      // Nothing changed.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size still on the heap using a moveable element type."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t Cap = 15;
      constexpr std::size_t NumElems = 12;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Cause vector to allocate Cap elements.
      SV sv(Cap, {5});
      // Reduce to NumElems elements.
      sv.assign(NumElems, Elem{6});

      // Preconditions.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      static_assert(std::is_move_constructible_v<Elem>);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements are moved.
         expected.moveCtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 6, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size still on the heap using a not moveable element type."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t Cap = 15;
      constexpr std::size_t NumElems = 12;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Cause vector to allocate Cap elements.
      SV sv(Cap, {5});
      // Reduce to NumElems elements.
      sv.assign(NumElems, Elem{6});

      // Preconditions.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      static_assert(!std::is_move_constructible_v<Elem>);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Copy elements to larger allocation.
         expected.copyCtorCalls = NumElems;
         // Destroy previous elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 6, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size fitting into the buffer using a moveable element type."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t Cap = 12;
      constexpr std::size_t NumElems = 8;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Cause vector to allocate Cap elements.
      SV sv(Cap, {5});
      // Reduce to NumElems elements.
      while (sv.size() > NumElems)
         sv.erase(sv.cbegin() + sv.size() - 1);

      // Preconditions.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      static_assert(std::is_move_constructible_v<Elem>);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements are moved.
         expected.moveCtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size fitting into the buffer using a not moveable element type."};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t Cap = 12;
      constexpr std::size_t NumElems = 8;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{caseLabel};

      // Cause vector to allocate Cap elements.
      SV sv(Cap, {5});
      // Reduce to NumElems elements.
      while (sv.size() > NumElems)
         sv.erase(sv.cbegin() + sv.size() - 1);

      // Preconditions.
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.capacity() == Cap, caseLabel);
      static_assert(!std::is_move_constructible_v<Elem>);

      {
         // Element instrumentation for tested call only.
         Elem::Measures expected;
         // Elements are moved.
         // Copy elements to larger allocation.
         expected.copyCtorCalls = NumElems;
         // Destroy previous elements.
         expected.dtorCalls = NumElems;
         const ElementVerifier<Elem> elemCheck{expected, caseLabel};

         // Test.
         sv.shrink_to_fit();
      }

      // Verify vector state.
      VERIFY(sv.capacity() == NumElems, caseLabel);
      VERIFY(sv.size() == NumElems, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 5, caseLabel);
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
      const std::string caseLabel{
         "SboVectorIterator operator< for greater-than iterators."};

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
      const std::string caseLabel{
         "SboVectorIterator operator<= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator<= for greater-than iterators."};

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
      const std::string caseLabel{
         "SboVectorIterator operator> for greater-than iterators."};

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
      const std::string caseLabel{
         "SboVectorIterator operator>= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorIterator<SV> a{&sv, 0};
      SboVectorIterator<SV> b{&sv, 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator>= for greater-than iterators."};

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
      const std::string caseLabel{
         "SboVectorConstIterator equality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator equality for different vectors."};

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
      const std::string caseLabel{
         "SboVectorConstIterator inequality for different indices."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> a{&sv, 1};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator inequality for different vectors."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator+= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 0};
      it += 2;

      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator+= for negative offset."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator-= for positive offset."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {3}};

      SboVectorConstIterator<SV> it{&sv, 2};
      it -= 2;

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator-= for negative offset."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator< for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a < b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator< for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(!(a < b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator< for equal iterators."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(!(a <= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for equal iterators."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator> for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a > b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator> for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(a > b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator> for equal iterators."};

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
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for less-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 0};
      SboVectorConstIterator<SV> b{&sv, 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for greater-than iterators."};

      using SV = SboVector<int, 10>;
      SV sv{{1}, {2}, {20}};

      SboVectorConstIterator<SV> a{&sv, 2};
      SboVectorConstIterator<SV> b{&sv, 0};

      VERIFY(a >= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for equal iterators."};

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
   TestSboVectorBegin();
   TestSboVectorEnd();
   TestSboVectorBeginConst();
   TestSboVectorEndConst();
   TestSboVectorCBegin();
   TestSboVectorCEnd();
   TestSboVectorRBegin();
   TestSboVectorREnd();
   TestSboVectorRBeginConst();
   TestSboVectorREndConst();
   TestSboVectorCRBegin();
   TestSboVectorCREnd();
   TestSboVectorEmpty();
   TestSboVectorSize();
   TestSboVectorMaxSize();
   TestSboVectorReserve();
   TestSboVectorShrinkToFit();

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
