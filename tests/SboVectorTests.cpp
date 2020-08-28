#include "SboVectorTests.h"
// Turn on memory instrumentation for SboVector.
#define SBOVEC_MEM_INSTR
#include "SboVector.h"
#include "TestUtil.h"
#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <vector>

using namespace ds;


namespace
{
///////////////////

// Used as element type for testing the SboVector class.
// Ctors and dtor are instrumented with call counters.
struct Element
{
   Element() noexcept
   {
      if (!m_paused)
         ++m_metrics.defaultCtorCalls;
   }
   Element(int i_) : i{i_}
   {
      if (!m_paused)
         ++m_metrics.ctorCalls;
   }
   Element(const Element& other) : d{other.d}, i{other.i}, b{other.b}
   {
      if (!m_paused)
         ++m_metrics.copyCtorCalls;
   }
   Element(Element&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      if (!m_paused)
         ++m_metrics.moveCtorCalls;
   }
   ~Element()
   {
      if (!m_paused)
         ++m_metrics.dtorCalls;
   }
   Element& operator=(const Element& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      if (!m_paused)
         ++m_metrics.assignmentCalls;
      return *this;
   }
   Element& operator=(Element&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      if (!m_paused)
         ++m_metrics.moveAssignmentCalls;
      return *this;
   }
   friend bool operator==(const Element& a, const Element& b)
   {
      return (a.d == b.d && a.i == b.i && a.b == b.b);
   }

   double d = 1.0;
   int i = 1;
   bool b = true;

   // Instrumentation.

   struct Metrics
   {
      std::size_t defaultCtorCalls = 0;
      std::size_t ctorCalls = 0;
      std::size_t copyCtorCalls = 0;
      std::size_t moveCtorCalls = 0;
      std::size_t assignmentCalls = 0;
      std::size_t moveAssignmentCalls = 0;
      std::size_t dtorCalls = 0;
   };

   inline static Metrics m_metrics;
   inline static bool m_paused = false;

   static void resetMetrics()
   {
      m_paused = false;
      m_metrics.defaultCtorCalls = 0;
      m_metrics.ctorCalls = 0;
      m_metrics.copyCtorCalls = 0;
      m_metrics.moveCtorCalls = 0;
      m_metrics.assignmentCalls = 0;
      m_metrics.moveAssignmentCalls = 0;
      m_metrics.dtorCalls = 0;
   }

   static void pauseMetrics() { m_paused = true; }

   static void verifyMetrics(const Metrics& expected, const std::string& caseLabel)
   {
      VERIFY(expected.defaultCtorCalls == m_metrics.defaultCtorCalls, caseLabel);
      VERIFY(expected.ctorCalls == m_metrics.ctorCalls, caseLabel);
      VERIFY(expected.copyCtorCalls == m_metrics.copyCtorCalls, caseLabel);
      VERIFY(expected.moveCtorCalls == m_metrics.moveCtorCalls, caseLabel);
      VERIFY(expected.assignmentCalls == m_metrics.assignmentCalls, caseLabel);
      VERIFY(expected.moveAssignmentCalls == m_metrics.moveAssignmentCalls, caseLabel);
      VERIFY(expected.dtorCalls == m_metrics.dtorCalls, caseLabel);
   }
};


///////////////////

// Used as element type for testing the SboVector class.
// Ctors and dtor are instrumented with call counters.
// Does not support move semantics.
struct NotMoveableElement
{
   NotMoveableElement() { ++m_metrics.defaultCtorCalls; }
   NotMoveableElement(int i_) : i{i_} { ++m_metrics.ctorCalls; }
   NotMoveableElement(const NotMoveableElement& other)
   : d{other.d}, i{other.i}, b{other.b}
   {
      ++m_metrics.copyCtorCalls;
   }
   NotMoveableElement(NotMoveableElement&& other) = delete;
   ~NotMoveableElement() { ++m_metrics.dtorCalls; }
   NotMoveableElement& operator=(const NotMoveableElement& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      ++m_metrics.assignmentCalls;
      return *this;
   }
   NotMoveableElement& operator=(NotMoveableElement&& other) = delete;
   friend bool operator==(const NotMoveableElement& a, const NotMoveableElement& b)
   {
      return (a.d == b.d && a.i == b.i && a.b == b.b);
   }

   double d = 1.0;
   int i = 1;
   bool b = true;

   // Instrumentation.

   struct Metrics
   {
      std::size_t defaultCtorCalls = 0;
      std::size_t ctorCalls = 0;
      std::size_t copyCtorCalls = 0;
      std::size_t assignmentCalls = 0;
      std::size_t dtorCalls = 0;
   };

   inline static Metrics m_metrics;

   static void resetMetrics()
   {
      m_metrics.defaultCtorCalls = 0;
      m_metrics.ctorCalls = 0;
      m_metrics.copyCtorCalls = 0;
      m_metrics.assignmentCalls = 0;
      m_metrics.dtorCalls = 0;
   }

   static void verifyMetrics(const Metrics& expected, const std::string& caseLabel)
   {
      VERIFY(expected.defaultCtorCalls == m_metrics.defaultCtorCalls, caseLabel);
      VERIFY(expected.ctorCalls == m_metrics.ctorCalls, caseLabel);
      VERIFY(expected.copyCtorCalls == m_metrics.copyCtorCalls, caseLabel);
      VERIFY(expected.assignmentCalls == m_metrics.assignmentCalls, caseLabel);
      VERIFY(expected.dtorCalls == m_metrics.dtorCalls, caseLabel);
   }
};


void swap(NotMoveableElement& a, NotMoveableElement& b)
{
   std::swap(a.d, b.d);
   std::swap(a.i, b.i);
   std::swap(a.b, b.b);
}


///////////////////

std::vector<int> FwdIterSource = {1000, 1001, 1002, 1003, 1004,
                                  1005, 1006, 1007, 1008, 1009};

// Forward iterator to simulated input source for test cases that require forward
// iterators.
class ForwardIter
{
 public:
   using iterator_category = std::forward_iterator_tag;
   using value_type = std::vector<int>::value_type;
   using difference_type = std::vector<int>::difference_type;
   using pointer = std::vector<int>::pointer;
   using reference = std::vector<int>::reference;
   using iterator_category = std::forward_iterator_tag;

 public:
   ForwardIter() = default;
   ForwardIter(std::size_t pos) : m_pos{pos} {}
   ForwardIter(const ForwardIter&) = default;
   ForwardIter& operator=(const ForwardIter&) = default;

   value_type& operator*() { return FwdIterSource[m_pos]; }

   const value_type& operator*() const { return FwdIterSource[m_pos]; }

   value_type* operator->() { return &FwdIterSource[m_pos]; }

   const value_type* operator->() const { return &FwdIterSource[m_pos]; }

   ForwardIter& operator++()
   {
      ++m_pos;
      return *this;
   }

   ForwardIter operator++(int)
   {
      auto before = *this;
      ++(*this);
      return before;
   }

   friend bool operator==(const ForwardIter& a, const ForwardIter& b)
   {
      return a.m_pos == b.m_pos;
   }

   friend bool operator!=(const ForwardIter& a, const ForwardIter& b)
   {
      return !(a == b);
   }

 private:
   std::size_t m_pos = 0;
};


ForwardIter inputBegin()
{
   return ForwardIter();
}


ForwardIter inputEnd()
{
   return ForwardIter(FwdIterSource.size());
}


ForwardIter makeForwardIter(std::size_t pos)
{
   return ForwardIter(pos);
}


///////////////////

// RAII class to verify the instrumentation metrics of a given element type.
template <typename Elem> class ElementVerifier
{
 public:
   ElementVerifier(const typename Elem::Metrics& expected, std::string caseLabel)
   : m_expected{expected}, m_caseLabel{std::move(caseLabel)}
   {
      Elem::resetMetrics();
   }
   ~ElementVerifier() { Elem::verifyMetrics(m_expected, m_caseLabel); }

 private:
   typename Elem::Metrics m_expected = 0;
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

template <typename Elem, std::size_t BufCap>
SboVector<Elem, BufCap> makeVector(std::size_t cap, std::initializer_list<Elem> elems)
{
   SboVector<Elem, BufCap> sv;
   sv.reserve(cap);
   for (const auto& elem : elems)
      sv.push_back(elem);
   return sv;
}


template <typename SV>
void verifyVector(const SV& sv, std::initializer_list<typename SV::value_type> values,
                  const std::string& caseLabel)
{
   VERIFY(sv.size() == values.size(), caseLabel);
   auto svPos = sv.cbegin();
   for (const auto& val : values)
      VERIFY(*(svPos++) == val, caseLabel);
}


///////////////////

// General structure of test cases.
template <typename Elem, std::size_t BufCap> class Test
{
 public:
   using SV = SboVector<Elem, BufCap>;

 public:
   Test(const std::string& caseLabel, const typename Elem::Metrics& metrics)
   : m_caseLabel{caseLabel}, m_expectedMetrics{metrics}
   {
   }
   Test(const Test&) = delete;
   Test(Test&&) = delete;
   ~Test() = default;

   void run(std::function<void()> testFn) const
   {
      // Memory instrumentation for entire scope.
      const MemVerifier<SV> memCheck{this->m_caseLabel};

      {
         // Element instrumentation for tested call only.
         const ElementVerifier<Elem> elemCheck{this->m_expectedMetrics,
                                               this->m_caseLabel};
         assert(testFn);
         testFn();
      }
   }

 private:
   const std::string m_caseLabel;
   const typename Elem::Metrics m_expectedMetrics;
};


///////////////////

void TestDefaultCtor()
{
   const std::string caseLabel{"SboVector default ctor"};

   constexpr std::size_t BufCap = 10;
   using Elem = Element;
   using SV = SboVector<Elem, BufCap>;

   const Elem::Metrics zeros;

   Test<Elem, BufCap> test{caseLabel, zeros};
   test.run([&]() {
      SV sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == BufCap, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
   });
}


void TestCtorForElementCountAndValue()
{
   {
      const std::string caseLabel{"SboVector count-and-value ctor for buffer storage"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      constexpr std::size_t numElems = 5;
      const Elem initVal{2};
      const std::initializer_list<Elem> expectedValues{initVal, initVal, initVal, initVal,
                                                       initVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv(numElems, initVal);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expectedValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for heap instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      constexpr std::size_t numElems = 12;
      const Elem initVal{2};
      const std::initializer_list<Elem> expectedValues{
         initVal, initVal, initVal, initVal, initVal, initVal,
         initVal, initVal, initVal, initVal, initVal, initVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv(numElems, initVal);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, expectedValues, caseLabel);
      });
   }
}


void TestIteratorCtor()
{
   {
      const std::string caseLabel{"SboVector iterator ctor for buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t numElems = values.size();
      std::vector<Elem> from{values};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv{from.begin(), from.end()};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector iterator ctor for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = values.size();
      std::vector<Elem> from{values};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv{from.begin(), from.end()};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector iterator ctor for const iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = values.size();
      const std::vector<Elem> from{values};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv{from.cbegin(), from.cend()};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector iterator ctor empty iterator range"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      std::vector<Elem> from;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&, BufCap]() {
         SV sv{from.begin(), from.end()};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector iterator ctor for forward iterators"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 3;
      ForwardIter fromFirst = inputBegin();
      ForwardIter fromLast = makeForwardIter(numElems);

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         static_assert(
            std::is_same_v<ForwardIter::iterator_category, std::forward_iterator_tag>);

         SV sv{fromFirst, fromLast};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.size() == numElems, caseLabel);
      });
   }
}


void TestInitializerListCtor()
{
   {
      const std::string caseLabel{"SboVector initializer list ctor for buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv{values};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector initializer list ctor for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV sv{values};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestCopyCtor()
{
   {
      const std::string caseLabel{"SboVector copy ctor for buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      // For source elements and copies.
      metrics.copyCtorCalls = 2 * numElems;
      metrics.dtorCalls = 2 * numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         const SV src{values};
         SV sv{src};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector copy ctor for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      // For source elements and copies.
      metrics.copyCtorCalls = 2 * numElems;
      metrics.dtorCalls = 2 * numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         const SV src{values};
         SV sv{src};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestMoveCtor()
{
   {
      const std::string caseLabel{"SboVector move ctor for buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      // For constructing the source elements.
      metrics.copyCtorCalls = numElems;
      // For constructing the copies.
      metrics.moveCtorCalls = numElems;
      // For destroying the copies. The source vector is empty after the move and nothing
      // needs to be destroyed.
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV src{values};
         SV sv{std::move(src)};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, values, caseLabel);
         // Verify moved-from instance is empty.
         VERIFY(src.size() == 0, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector move ctor for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = values.size();

      Elem::Metrics metrics;
      // For constructing the source elements.
      metrics.copyCtorCalls = numElems;
      // No moves because the SboVector simply stole the pointer to the heap memory.
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&, BufCap]() {
         SV src{values};
         SV sv{std::move(src)};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == numElems, caseLabel);
         verifyVector(sv, values, caseLabel);
         // Verify moved-from instance is empty.
         VERIFY(src.size() == 0, caseLabel);
      });
   }
}


void TestDtor()
{
   {
      const std::string caseLabel{"SboVector dtor for buffer instance"};

      constexpr std::size_t BufCap = 10;
      constexpr std::size_t numElems = 3;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation.
      const MemVerifier<SV> memCheck{caseLabel};

      {
         SV sv{1, 2, 3};

         // Preconditions.
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.size() == numElems, caseLabel);

         // Reset element instrumentation right before the SboVector gets destroyed
         // to only verify the destruction of the vector elements.
         Elem::resetMetrics();

         // Test.
         // End of scope triggers dtor.
      }

      VERIFY(Elem::m_metrics.dtorCalls == numElems, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector dtor for heap instance"};

      constexpr std::size_t BufCap = 3;
      constexpr std::size_t numElems = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      // Memory instrumentation.
      const MemVerifier<SV> memCheck{caseLabel};

      {
         SV sv{1, 2, 3, 4, 5};

         // Precondition.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() == numElems, caseLabel);

         // Reset element instrumentation right before the SboVector gets destroyed
         // to only verify the destruction of the vector elements.
         Elem::resetMetrics();
      }

      VERIFY(Elem::m_metrics.dtorCalls == numElems, caseLabel);
   }
}


void TestCopyAssignment()
{
   // Local function to calculate the expected metrics for copy-assignments.
   auto expectedCopyMetrics = [](std::size_t numFrom,
                                 std::size_t numTo) -> Element::Metrics {
      Element::Metrics metrics;
      // For populating vectors and copying source to destination.
      metrics.copyCtorCalls = 2 * numFrom + numTo;
      metrics.dtorCalls = 2 * numFrom + numTo;
      return metrics;
   };

   {
      const std::string caseLabel{
         "SboVector copy assignment from buffer instance to buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedCopyMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.inBuffer(), caseLabel);
         VERIFY(to.inBuffer(), caseLabel);

         to = from;

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from heap instance to buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedCopyMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.inBuffer(), caseLabel);

         to = from;

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from buffer instance to heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedCopyMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.inBuffer(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);

         to = from;

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from larger to smaller heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedCopyMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);
         VERIFY(from.size() > to.size(), caseLabel);

         to = from;

         VERIFY(to.onHeap(), caseLabel);
         // Assigning data that needs a larger heap allocation will reallocate
         // heap memory.
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector copy assignment from smaller to larger heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedCopyMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);
         VERIFY(from.size() < to.size(), caseLabel);

         to = from;

         VERIFY(to.onHeap(), caseLabel);
         // Assigning data that needs a smaller heap allocation will reuse the existing
         // heap memory. Capacity will remain at previous (larger) size.
         VERIFY(to.capacity() == numTo, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
}


void TestMoveAssignment()
{
   // Local functions to calculate the expected metrics for move-assignments.
   auto expectedMoveHeapMetrics = [](std::size_t numFrom,
                                     std::size_t numTo) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numFrom + numTo;
      // No moves because the heap allocations is stolen.
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = numFrom + numTo;
      return metrics;
   };
   auto expectedMoveBufferMetrics = [](std::size_t numFrom,
                                       std::size_t numTo) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numFrom + numTo;
      metrics.moveCtorCalls = numFrom;
      metrics.dtorCalls = numFrom + numTo;
      return metrics;
   };

   {
      const std::string caseLabel{"SboVector move assignment from buffer "
                                  "instance to buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMoveBufferMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.inBuffer(), caseLabel);
         VERIFY(to.inBuffer(), caseLabel);

         to = std::move(from);

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector move assignment from heap "
                                  "instance to buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMoveHeapMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.inBuffer(), caseLabel);

         to = std::move(from);

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector move assignment from buffer "
                                  "instance to heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMoveBufferMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.inBuffer(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);

         to = std::move(from);

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector move assignment from smaller "
                                  "to larger heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMoveHeapMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);
         VERIFY(from.size() < to.size(), caseLabel);

         to = std::move(from);

         VERIFY(to.onHeap(), caseLabel);
         // Will take over the stolen capacity of the source.
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector move assignment from larger heap "
                                  "instance to smaller heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMoveHeapMetrics(numFrom, numTo)};
      test.run([&]() {
         SV from{fromValues};
         SV to{toValues};

         VERIFY(from.onHeap(), caseLabel);
         VERIFY(to.onHeap(), caseLabel);
         VERIFY(from.size() > to.size(), caseLabel);

         to = std::move(from);

         VERIFY(to.onHeap(), caseLabel);
         // Will take over the stolen capacity of the source.
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
}


void TestInitializerListAssignment()
{
   // Local functions to calculate the expected metrics for assignment of
   // an initializer list.
   auto expectedMetrics = [](std::size_t numFrom, std::size_t numTo) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numFrom + numTo;
      metrics.dtorCalls = numFrom + numTo;
      return metrics;
   };

   {
      const std::string caseLabel{
         "SboVector initializer list assignment that fits in buffer to buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numFrom, numTo)};
      test.run([&]() {
         SV to{toValues};

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(numFrom < BufCap, caseLabel);

         to = fromValues;

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that requires "
                                  "heap to buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numFrom, numTo)};
      test.run([&]() {
         SV to{toValues};

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(numFrom > BufCap, caseLabel);

         to = fromValues;

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(to.capacity() == numFrom, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment  "
                                  "that fits in buffer to heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numFrom, numTo)};
      test.run([&]() {
         SV to{toValues};

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(numFrom < BufCap, caseLabel);

         to = fromValues;

         VERIFY(to.inBuffer(), caseLabel);
         VERIFY(to.capacity() == BufCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "but can reuse the heap of the target instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numFrom, numTo)};
      test.run([&]() {
         SV to{toValues};
         const std::size_t origCap = to.capacity();

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(numFrom > BufCap, caseLabel);
         VERIFY(numFrom < numTo, caseLabel);

         to = fromValues;

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(to.capacity() == origCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector initializer list assignment that needs heap "
                                  "and cannot reuse the heap of the target instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> fromValues{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t numFrom = fromValues.size();
      const std::initializer_list<Elem> toValues{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numTo = toValues.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numFrom, numTo)};
      test.run([&]() {
         SV to{toValues};
         const std::size_t origCap = to.capacity();

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(numFrom > BufCap, caseLabel);
         VERIFY(numFrom > numTo, caseLabel);

         to = fromValues;

         VERIFY(to.onHeap(), caseLabel);
         VERIFY(to.capacity() > origCap, caseLabel);
         verifyVector(to, fromValues, caseLabel);
      });
   }
}


void TestAssignElementValue()
{
   // Local functions to calculate the expected metrics.
   auto expectedMetrics = [](std::size_t numInitial,
                             std::size_t numAssigned) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numInitial + numAssigned;
      metrics.dtorCalls = numInitial + numAssigned;
      return metrics;
   };

   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values fit in buffer. "
         "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3};
      const std::size_t numInitial = initial.size();
      const std::size_t numAssigned = 2;
      const Elem assigned = 100;
      const std::initializer_list<Elem> expected{100, 100};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(numAssigned, assigned);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector assign element value n-times. Assigned "
                                  "values require heap allocation. "
                                  "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3};
      const std::size_t numInitial = initial.size();
      const std::size_t numAssigned = 7;
      const Elem assigned = 100;
      const std::initializer_list<Elem> expected{100, 100, 100, 100, 100, 100, 100};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);

         sv.assign(numAssigned, assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > origCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector assign element value n-times. Assigned "
                                  "values fit into buffer. SboVector is a heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numInitial = initial.size();
      const std::size_t numAssigned = 3;
      const Elem assigned = 100;
      const std::initializer_list<Elem> expected{100, 100, 100};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(numAssigned, assigned);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector is a smaller heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numInitial = initial.size();
      const std::size_t numAssigned = 10;
      const Elem assigned = 100;
      const std::initializer_list<Elem> expected{100, 100, 100, 100, 100,
                                                 100, 100, 100, 100, 100};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned > numInitial, caseLabel);

         sv.assign(numAssigned, assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > origCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign element value n-times. Assigned values require heap "
         "allocation. SboVector is a larger heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t numInitial = initial.size();
      const std::size_t numAssigned = 7;
      const Elem assigned = 100;
      const std::initializer_list<Elem> expected{100, 100, 100, 100, 100, 100, 100};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned < numInitial, caseLabel);

         sv.assign(numAssigned, assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == origCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestAssignIteratorRange()
{
   // Local functions to calculate the expected metrics.
   auto expectedMetrics = [](std::size_t numInitial,
                             std::size_t numAssigned) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numInitial + numAssigned;
      metrics.dtorCalls = numInitial + numAssigned;
      return metrics;
   };

   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values fit in buffer. "
         "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3};
      const std::size_t numAssigned = assigned.size();
      const std::list<Element> from{assigned};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(from.begin(), from.end());

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values require heap. "
         "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6};
      const std::size_t numAssigned = assigned.size();
      const std::list<Element> from{assigned};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);

         sv.assign(from.begin(), from.end());

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values fit into buffer. "
         "SboVector is a heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4};
      const std::size_t numAssigned = assigned.size();
      const std::list<Element> from{assigned};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(from.begin(), from.end());

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values require heap. "
         "SboVector is a smaller heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t numAssigned = assigned.size();
      const std::list<Element> from{assigned};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned > numInitial, caseLabel);

         sv.assign(from.begin(), from.end());

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > origCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign iterator range. Assigned values require heap. "
         "SboVector is a larger heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numAssigned = assigned.size();
      const std::list<Element> from{assigned};

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned < numInitial, caseLabel);

         sv.assign(from.begin(), from.end());

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == origCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
}


void TestAssignInitializerList()
{
   // Local functions to calculate the expected metrics.
   auto expectedMetrics = [](std::size_t numInitial,
                             std::size_t numAssigned) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = numInitial + numAssigned;
      metrics.dtorCalls = numInitial + numAssigned;
      return metrics;
   };

   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values fit in buffer. "
         "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3};
      const std::size_t numAssigned = assigned.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(assigned);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector is a buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6};
      const std::size_t numAssigned = assigned.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);

         sv.assign(assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values fit into buffer. "
         "SboVector is a heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4};
      const std::size_t numAssigned = assigned.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned < BufCap, caseLabel);

         sv.assign(assigned);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector is a smaller heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numAssigned = assigned.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned > numInitial, caseLabel);

         sv.assign(assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > origCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SboVector assign initializer list. Assigned values require heap. "
         "SboVector is a larger heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> initial{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t numInitial = initial.size();
      const std::initializer_list<Elem> assigned{1, 2, 3, 4, 5, 6};
      const std::size_t numAssigned = assigned.size();

      Test<Elem, BufCap> test{caseLabel, expectedMetrics(numInitial, numAssigned)};
      test.run([&]() {
         SV sv{initial};
         const std::size_t origCap = sv.capacity();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(numAssigned > BufCap, caseLabel);
         VERIFY(numAssigned < numInitial, caseLabel);

         sv.assign(assigned);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == origCap, caseLabel);
         verifyVector(sv, assigned, caseLabel);
      });
   }
}


void TestAt()
{
   {
      const std::string caseLabel{"SvoVector::at for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 4;
      const Elem val = 10;
      SV sv(numElems, val);

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv.at(i) == val, caseLabel);

            const Elem newVal{i};
            sv.at(i) = newVal;
            VERIFY(sv.at(i) == newVal, caseLabel);
         }
      });
   }
   {
      const std::string caseLabel{"SvoVector::at for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 7;
      const Elem val = 10;
      SV sv(numElems, val);

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv.at(i) == val, caseLabel);

            const Elem newVal{i};
            sv.at(i) = newVal;
            VERIFY(sv.at(i) == newVal, caseLabel);
         }
      });
   }
   {
      const std::string caseLabel{"SvoVector::at for invalid index"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 4;
      const Elem val = 10;
      SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY_THROW(([&]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      });
   }
}


void TestAtConst()
{
   {
      const std::string caseLabel{"SvoVector::at const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 4;
      const Elem val = 10;
      const SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(
            std::is_const_v<typename std::remove_reference<decltype(sv)>::type>);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv.at(i) == val, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::at const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 12;
      const Elem val = 10;
      const SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(
            std::is_const_v<typename std::remove_reference<decltype(sv)>::type>);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv.at(i) == val, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::at const for invalid index"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 7;
      const Elem val = 10;
      const SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         VERIFY_THROW(([&]() { sv.at(sv.size()); }), std::out_of_range, caseLabel);
      });
   }
}


void TestSubscriptOperator()
{
   {
      const std::string caseLabel{"SvoVector::operator[] for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 4;
      const Elem val = 10;
      SV sv(numElems, val);

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv[i] == val, caseLabel);

            const Elem newVal{i};
            sv[i] = newVal;
            VERIFY(sv[i] == newVal, caseLabel);
         }
      });
   }
   {
      const std::string caseLabel{"SvoVector::operator[] for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 7;
      const Elem val = 10;
      SV sv(numElems, val);

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv[i] == val, caseLabel);

            const Elem newVal{i};
            sv[i] = newVal;
            VERIFY(sv[i] == newVal, caseLabel);
         }
      });
   }
}


void TestSubscriptOperatorConst()
{
   {
      const std::string caseLabel{"SvoVector::operator[] const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 4;
      const Elem val = 10;
      const SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i] == val, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::operator[] const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::size_t numElems = 7;
      const Elem val = 10;
      const SV sv(numElems, val);

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv[i] == val, caseLabel);
      });
   }
}


void TestFront()
{
   {
      const std::string caseLabel{"SvoVector::front for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.assignmentCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         VERIFY(sv.front() == sv[0], caseLabel);

         const Elem newVal{20};
         sv.front() = newVal;
         VERIFY(sv[0] == newVal, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::front for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.assignmentCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         VERIFY(sv.front() == sv[0], caseLabel);

         const Elem newVal{20};
         sv.front() = newVal;
         VERIFY(sv[0] == newVal, caseLabel);
      });
   }
}


void TestFrontConst()
{
   {
      const std::string caseLabel{"SvoVector::front const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         VERIFY(sv.front() == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::front const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         VERIFY(sv.front() == sv[0], caseLabel);
      });
   }
}


void TestBack()
{
   {
      const std::string caseLabel{"SvoVector::back for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.assignmentCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         VERIFY(sv.back() == sv[sv.size() - 1], caseLabel);

         const Elem newVal{20};
         sv.back() = newVal;
         VERIFY(sv[sv.size() - 1] == newVal, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::back for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.assignmentCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         VERIFY(sv.back() == sv[sv.size() - 1], caseLabel);

         const Elem newVal{20};
         sv.back() = newVal;
         VERIFY(sv[sv.size() - 1] == newVal, caseLabel);
      });
   }
}


void TestBackConst()
{
   {
      const std::string caseLabel{"SvoVector::back const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         VERIFY(sv.back() == sv[sv.size() - 1], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         VERIFY(sv.back() == sv[sv.size() - 1], caseLabel);
      });
   }
}


void TestData()
{
   {
      const std::string caseLabel{"SvoVector::data for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      const std::size_t numElems = sv.size();

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv.data()[i] == sv[i], caseLabel);

            const Elem newVal{100 + i};
            sv.data()[i] = newVal;
            VERIFY(sv[i] == newVal, caseLabel);
         }
      });
   }
   {
      const std::string caseLabel{"SvoVector::data for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      const std::size_t numElems = sv.size();

      Elem::Metrics metrics;
      metrics.ctorCalls = numElems;
      metrics.assignmentCalls = numElems;
      metrics.dtorCalls = numElems;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         for (int i = 0; i < sv.size(); ++i)
         {
            VERIFY(sv.data()[i] == sv[i], caseLabel);

            const Elem newVal{100 + i};
            sv.data()[i] = newVal;
            VERIFY(sv[i] == newVal, caseLabel);
         }
      });
   }
}


void TestDataConst()
{
   {
      const std::string caseLabel{"SvoVector::data const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv.data()[i] == sv[i], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::back const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         for (int i = 0; i < sv.size(); ++i)
            VERIFY(sv.data()[i] == sv[i], caseLabel);
      });
   }
}


void TestBegin()
{
   {
      const std::string caseLabel{"SboVector::begin for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::iterator first = sv.begin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::begin for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::iterator first = sv.begin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::begin for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::iterator first = sv.begin();
         VERIFY(first == sv.end(), caseLabel);
      });
   }
}


void TestEnd()
{
   {
      const std::string caseLabel{"SboVector::end for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::iterator last = sv.end();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::end for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::iterator last = sv.end();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::end for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::iterator last = sv.end();
         VERIFY(last == sv.begin(), caseLabel);
      });
   }
}


void TestBeginConst()
{
   {
      const std::string caseLabel{"SboVector::begin const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator first = sv.begin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::begin for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator first = sv.begin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::begin for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator first = sv.begin();
         VERIFY(first == sv.end(), caseLabel);
      });
   }
}


void TestEndConst()
{
   {
      const std::string caseLabel{"SboVector::end for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator last = sv.end();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::end for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator last = sv.end();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::end for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_iterator last = sv.end();
         VERIFY(last == sv.begin(), caseLabel);
      });
   }
}


void TestCBegin()
{
   {
      const std::string caseLabel{"SboVector::cbegin const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::const_iterator first = sv.cbegin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::cbegin for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::const_iterator first = sv.cbegin();
         VERIFY(*first == sv[0], caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::cbegin for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::const_iterator first = sv.cbegin();
         VERIFY(first == sv.cend(), caseLabel);
      });
   }
}


void TestCEnd()
{
   {
      const std::string caseLabel{"SboVector::cend for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::const_iterator last = sv.cend();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::cend for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::const_iterator last = sv.cend();
         VERIFY(last == sv.begin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::cend for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::const_iterator last = sv.cend();
         VERIFY(last == sv.begin(), caseLabel);
      });
   }
}


void TestRBegin()
{
   {
      const std::string caseLabel{"SboVector::rbegin for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::reverse_iterator rfirst = sv.rbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rbegin for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::reverse_iterator rfirst = sv.rbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rbegin for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::reverse_iterator rfirst = sv.rbegin();
         VERIFY(rfirst == sv.rend(), caseLabel);
      });
   }
}


void TestREnd()
{
   {
      const std::string caseLabel{"SboVector::rend for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rend for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rend for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin(), caseLabel);
      });
   }
}


void TestRBeginConst()
{
   {
      const std::string caseLabel{"SboVector::rbegin const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rfirst = sv.rbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rbegin const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rfirst = sv.rbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rbegin const for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rfirst = sv.rbegin();
         VERIFY(rfirst == sv.rend(), caseLabel);
      });
   }
}


void TestREndConst()
{
   {
      const std::string caseLabel{"SboVector::rend const for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rend const for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::rend const for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);
         // Ref types are always non-const, so remove the reference.
         static_assert(std::is_const_v<std::remove_reference<decltype(sv)>::type>);

         SV::const_reverse_iterator rlast = sv.rend();
         VERIFY(rlast == sv.rbegin(), caseLabel);
      });
   }
}


void TestCRBegin()
{
   {
      const std::string caseLabel{"SboVector::crbegin for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::const_reverse_iterator rfirst = sv.crbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::crbegin for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::const_reverse_iterator rfirst = sv.crbegin();
         VERIFY(*rfirst == sv.back(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::crbegin for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::const_reverse_iterator rfirst = sv.crbegin();
         VERIFY(rfirst == sv.crend(), caseLabel);
      });
   }
}


void TestCREnd()
{
   {
      const std::string caseLabel{"SboVector::crend for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);

         SV::const_reverse_iterator rlast = sv.crend();
         VERIFY(rlast == sv.crbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::crend for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6, 7};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);

         SV::const_reverse_iterator rlast = sv.crend();
         VERIFY(rlast == sv.crbegin() + sv.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SboVector::crend for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         SV::const_reverse_iterator rlast = sv.crend();
         VERIFY(rlast == sv.crbegin(), caseLabel);
      });
   }
}


void TestEmpty()
{
   {
      const std::string caseLabel{"SvoVector::empty for empty instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.size() == 0, caseLabel);

         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.size() > 0, caseLabel);

         VERIFY(!sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() > 0, caseLabel);

         VERIFY(!sv.empty(), caseLabel);
      });
   }
}


void TestSize()
{
   {
      const std::string caseLabel{"SvoVector::size for empty instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         VERIFY(sv.size() == 0, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3};
      SV sv{values};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         VERIFY(sv.size() == values.size(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::empty for non-empty buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6};
      SV sv{values};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         VERIFY(sv.size() == values.size(), caseLabel);
      });
   }
}


void TestMaxSize()
{
   {
      const std::string caseLabel{"SvoVector::max_size for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         VERIFY(sv.max_size() > 0, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::max_size for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv{1, 2, 3, 4, 5, 6};
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         VERIFY(sv.max_size() > 0, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::max_size for empty instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         VERIFY(sv.empty(), caseLabel);

         VERIFY(sv.max_size() > 0, caseLabel);
      });
   }
}


void TestReserve()
{
   {
      const std::string caseLabel{"SvoVector::reserve for capacity less than current"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 9;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(reserveCap < sv.capacity(), caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than max size"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t initialCap = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY_THROW(([&sv]() { sv.reserve(sv.max_size() + 1); }), std::length_error,
                      caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "with type that is moveable"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 15;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.moveCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(initialCap > BufCap, caseLabel);
         VERIFY(reserveCap > sv.capacity(), caseLabel);
         VERIFY(std::is_move_constructible_v<Elem>, caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == reserveCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "with type that is not moveable"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 15;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 2 * initialCap;
      metrics.dtorCalls = 2 * initialCap;

      Test<Elem, BufCap> test(caseLabel, metrics);
      test.run([&]() {
         SV sv{values};
         VERIFY(initialCap > BufCap, caseLabel);
         VERIFY(reserveCap > sv.capacity(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == reserveCap, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "where current data is in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 15;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.moveCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(initialCap < BufCap, caseLabel);
         VERIFY(reserveCap > initialCap, caseLabel);
         VERIFY(reserveCap > sv.capacity(), caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == reserveCap, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::reserve for capacity larger than current "
         "where current data is in buffer with type that is not moveable"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 15;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 2 * initialCap;
      metrics.dtorCalls = 2 * initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(initialCap < BufCap, caseLabel);
         VERIFY(reserveCap > initialCap, caseLabel);
         VERIFY(reserveCap > sv.capacity(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == reserveCap, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::reserve for capacity larger than current "
                                  "but smaller than buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t initialCap = values.size();
      constexpr std::size_t reserveCap = 8;

      // Reserve call is a no-op but vector needs to be constructed and destroyed.
      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(initialCap < BufCap, caseLabel);
         VERIFY(reserveCap < BufCap, caseLabel);
         VERIFY(reserveCap > initialCap, caseLabel);

         sv.reserve(reserveCap);

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestShrinkToFit()
{
   {
      const std::string caseLabel{"SvoVector::shrink_to_fit for buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5};
      const std::size_t initialCap = values.size();

      // Operation is a no-op but vector needs to be constructed and destroyed.
      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with capacity fully occupied"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialCap = values.size();

      // Operation is a no-op but vector needs to be constructed and destroyed.
      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialCap;
      metrics.dtorCalls = initialCap;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() == sv.capacity(), caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == initialCap, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size still on the heap using a moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 12;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      // Moving once for reserving initial cap and once for shrinking cap.
      metrics.moveCtorCalls = 2 * initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() < sv.capacity(), caseLabel);
         VERIFY(std::is_move_constructible_v<Elem>, caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == initialSize, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size still on the heap using a not moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 12;

      Elem::Metrics metrics;
      // Copying once for initialization, once for reserving initial cap and once for
      // shrinking cap.
      metrics.copyCtorCalls = 3 * initialSize;
      metrics.dtorCalls = 3 * initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() < sv.capacity(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == initialSize, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size fitting into the buffer using a moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 8;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      // Moving once for reserving initial cap and once for shrinking cap.
      metrics.moveCtorCalls = 2 * initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() < BufCap, caseLabel);
         VERIFY(std::is_move_constructible_v<Elem>, caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::shrink_to_fit for heap instance with "
         "shrunken size fitting into the buffer using a not moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 8;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 3 * initialSize;
      metrics.dtorCalls = 3 * initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() < BufCap, caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         sv.shrink_to_fit();

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestClear()
{
   {
      const std::string caseLabel{"SvoVector::clear for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         sv.clear();

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::clear for buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.clear();

         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::clear for heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.clear();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialSize, caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
}


void TestEraseSingleElement()
{
   {
      const std::string caseLabel{"SvoVector::erase element of buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 1;
      const std::initializer_list<Elem> expected{1, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase only element"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 0;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.end(), caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 1;
      const std::initializer_list<Elem> expected{1, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};

         SV::iterator next = sv.erase(sv.cbegin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin());

         VERIFY(next == sv.end(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase element of heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 1;
      const std::initializer_list<Elem> expected{1, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase first element"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 0;
      const std::initializer_list<Elem> expected{2, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase last element of heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const std::size_t erasedPos = initialSize - 1;
      const std::initializer_list<Elem> expected{1, 2, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase element of heap instance that "
                                  "makes elements fit into buffer"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t erasedPos = 1;
      const std::initializer_list<Elem> expected{1, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - erasedPos - 1;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(initialSize == BufCap + 1, caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         // Data is still on heap even though it would fit into buffer.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase all elements of heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);

         while (!sv.empty())
            sv.erase(sv.begin() + sv.size() - 1);

         // Allocation is still on heap even though it is empty.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase non-moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      constexpr int erasedPos = 1;
      const std::initializer_list<Elem> expected{1, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + initialSize - erasedPos - 1;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         SV::iterator next = sv.erase(sv.begin() + erasedPos);

         VERIFY(next == sv.begin() + erasedPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestEraseIteratorRange()
{
   // Local function to calculate the expected metrics for erasing.
   auto expectedErasureMetrics = [](std::size_t initialSize,
                                    std::size_t lastPos) -> Element::Metrics {
      Element::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize - lastPos;
      metrics.dtorCalls = initialSize;
      return metrics;
   };

   {
      const std::string caseLabel{"SvoVector::erase empty range"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         SV::iterator first = sv.begin();
         SV::iterator last = sv.begin();
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(first, last);

         VERIFY(next == last, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range of buffer instance"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 2;
      const std::size_t lastPos = 5;
      const std::initializer_list<Elem> expected{1, 2, 6, 7, 8};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::erase range of all elements of buffer instance"};

      constexpr std::size_t BufCap = 4;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 0;
      const std::size_t lastPos = initialSize;

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.end(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range of heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 2;
      const std::size_t lastPos = 5;
      const std::initializer_list<Elem> expected{1, 2, 6, 7, 8, 9};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range at front"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 0;
      const std::size_t lastPos = 3;
      const std::initializer_list<Elem> expected{4, 5, 6, 7, 8, 9};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range at tail"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 6;
      const std::size_t lastPos = initialSize;
      const std::initializer_list<Elem> expected{1, 2, 3, 4, 5, 6};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range using const-iterators"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 2;
      const std::size_t lastPos = 5;
      const std::initializer_list<Elem> expected{1, 2, 6, 7, 8, 9};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.cbegin() + firstPos, sv.cbegin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range for empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin(), sv.end());

         VERIFY(next == sv.end(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range of heap instance that makes "
                                  "elements fit into buffer"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 2;
      const std::size_t lastPos = 7;
      const std::initializer_list<Elem> expected{1, 2, 8, 9};

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         // Data is still on heap even though it would fit into buffer.
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::erase full range of elements of heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 0;
      const std::size_t lastPos = initialSize;

      Test<Elem, BufCap> test{caseLabel, expectedErasureMetrics(initialSize, lastPos)};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(lastPos - firstPos == initialSize, caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.end(), caseLabel);
         // Data is still on heap even though it would fit into buffer.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::erase range for non-moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{1, 2, 3, 4, 5, 6, 7, 8, 9};
      const std::size_t initialSize = values.size();
      const std::size_t firstPos = 2;
      const std::size_t lastPos = 5;
      const std::initializer_list<Elem> expected{1, 2, 6, 7, 8, 9};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + initialSize - lastPos;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         SV::iterator next = sv.erase(sv.begin() + firstPos, sv.begin() + lastPos);

         VERIFY(next == sv.begin() + firstPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestInsertSingleValue()
{
   {
      const std::string caseLabel{"SvoVector::insert value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value at beginning"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{100, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value at end"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.end(), insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.cbegin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert value for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1 + numRelocated;
      metrics.dtorCalls = initialSize + 1 + numRelocated;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, insertedVal);

         VERIFY(*inserted == insertedVal, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestInsertSingleRValue()
{
   {
      const std::string caseLabel{"SvoVector::insert r-value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, insertedVal, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = numRelocated + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, insertedVal, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, insertedVal, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + numRelocated + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, insertedVal, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value at beginning"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{insertedVal, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value at end"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.end(), std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value into heap "
                                  "instance using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, insertedVal, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted =
            sv.insert(sv.cbegin() + insertPos, std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert r-value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{insertedVal};

      Elem::Metrics metrics;
      metrics.moveCtorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), std::move(insertedElem));

         VERIFY(inserted->i == insertedVal, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestInsertValueMultipleTimes()
{
   {
      const std::string caseLabel{"SvoVector::insert value multiple times into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= BufCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value multiple times into buffer "
                                  "instance with not enough buffer capacity left"};

      constexpr std::size_t BufCap = 7;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > BufCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value multiple times into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      constexpr std::size_t initialCap = 15;
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= sv.capacity(), caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value multiple times into heap "
                                  "instance not enough capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize;
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > sv.capacity(), caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value multiple times at front"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize;
      const std::initializer_list<Elem> expected{100, 100, 100, 100, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), numInserted, insertedElem);

         VERIFY(inserted == sv.begin(), caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value multiple times at end"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      const std::size_t insertPos = initialSize;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= BufCap, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert value multiple times using a const-iterator"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted =
            sv.insert(sv.cbegin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert value multiple times into empty vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      Elem insertedElem = 100;
      constexpr std::size_t numInserted = 4;
      const std::initializer_list<Elem> expected{100, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numInserted;
      metrics.dtorCalls = numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), numInserted, insertedElem);

         VERIFY(inserted == sv.begin(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert value multiple times for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;
      constexpr std::size_t numInserted = 4;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 100, 100, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numRelocated + numInserted;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         SV::iterator inserted =
            sv.insert(sv.begin() + insertPos, numInserted, insertedElem);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert value zero times into vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      Elem insertedElem = 100;
      constexpr std::size_t insertPos = 3;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator expectedIter = sv.begin() + insertPos;
         SV::iterator inserted = sv.insert(sv.begin() + insertPos, 0, insertedElem);

         VERIFY(inserted == expectedIter, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestInsertRange()
{
   {
      const std::string caseLabel{"SvoVector::insert iterator range into buffer "
                                  "instance with enough capacity to fit into buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range into buffer "
                                  "instance with not enough capacity left"};

      constexpr std::size_t BufCap = 6;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 20;
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range into heap "
                                  "instance with not enough capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize;
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range at front"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::initializer_list<Elem> expected{101, 102, 103, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), srcFirst, srcLast);

         VERIFY(inserted == sv.begin(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range at end"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 101, 102, 103};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.end(), srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert iterator range using a const-iterator"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.cbegin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert iterator range into empty vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::initializer_list<Elem> expected{101, 102, 103};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numInserted;
      metrics.dtorCalls = numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), srcFirst, srcLast);

         VERIFY(inserted == sv.begin(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert iterator range for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = src.begin() + 4;
      constexpr std::size_t insertPos = 3;
      const std::size_t numInserted = std::distance(srcFirst, srcLast);
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 101, 102, 103, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numRelocated + numInserted;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert empty iterator range into vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      std::vector<Elem> src{100, 101, 102, 103, 104, 105};
      auto srcFirst = src.begin() + 1;
      auto srcLast = srcFirst;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(srcFirst == srcLast, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert forward iterator range"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t numInserted = 3;
      auto srcFirst = inputBegin();
      auto srcLast = makeForwardIter(numInserted);
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 1000, 1001, 1002, 3, 4};

      Elem::Metrics metrics;
      // The input iterator value type is 'int'. The ctor calls are for constructing
      // the vector elements from int values.
      metrics.ctorCalls = numInserted;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         static_assert(
            std::is_same_v<ForwardIter::iterator_category, std::forward_iterator_tag>);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, srcFirst, srcLast);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestInsertInitializerList()
{
   {
      const std::string caseLabel{"SvoVector::insert initializer list into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert initializer list at front of vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      const std::initializer_list<Elem> expected{100, 101, 102, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), src);

         VERIFY(inserted == sv.begin(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert initializer list at end of vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100, 101, 102};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.end(), src);

         VERIFY(inserted == sv.begin() + initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert initializer list into buffer "
                                  "instance with not enough capacity left"};

      constexpr std::size_t BufCap = 6;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > BufCap, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert initializer list into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 20;
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted <= sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert initializer list into heap "
                                  "instance with not enough capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize;
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + numInserted > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert initializer list using a const-iterator"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numInserted;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.cbegin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert initializer list into empty vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = numInserted;
      metrics.dtorCalls = numInserted;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.insert(sv.begin(), src);

         VERIFY(inserted == sv.begin(), caseLabel);
         verifyVector(sv, src, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::insert initializer list for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src{100, 101, 102};
      const std::size_t numInserted = src.size();
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 101, 102, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + numRelocated + numInserted;
      metrics.dtorCalls = metrics.copyCtorCalls;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == sv.begin() + insertPos, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::insert empty initializer list into vector"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> src;
      constexpr std::size_t insertPos = 3;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(src.size() == 0, caseLabel);

         SV::iterator expectedPos = sv.begin() + insertPos;
         SV::iterator inserted = sv.insert(sv.begin() + insertPos, src);

         VERIFY(inserted == expectedPos, caseLabel);
         verifyVector(sv, values, caseLabel);
      });
   }
}


void TestPushBackLValue()
{
   {
      const std::string caseLabel{"SvoVector::push_back l-value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back l-value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back l-value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back l-value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back l-value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::push_back l-value for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const Elem insertedVal = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         sv.push_back(insertedVal);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "Detect ambiguity in SboVector::insertOf overloads. Will fail to compile"};

      SboVector<int, 10> sv;
      sv.push_back(1000);
   }
}


void TestPushBackRValue()
{
   {
      const std::string caseLabel{"SvoVector::push_back r-value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back r-value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back r-value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      constexpr int insertedVal = 100;
      Elem insertedElem = insertedVal;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back r-value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back r-value into heap "
                                  "instance using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, insertedVal};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + 1;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::push_back r-value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      constexpr int insertedVal = 100;
      Elem insertedElem = 100;
      const std::initializer_list<Elem> expected{insertedVal};

      Elem::Metrics metrics;
      metrics.moveCtorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         sv.push_back(std::move(insertedElem));

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestPopBack()
{
   {
      const std::string caseLabel{"SvoVector::pop_back for buffer "
                                  "instance with more than one element"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> expected{0, 1, 2, 3};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.size() > 1, caseLabel);

         sv.pop_back();

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::pop_back for vector with one element"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.size() == 1, caseLabel);

         sv.pop_back();

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::pop_back for empty vector"};
      // Standard: This is UB.
   }
   {
      const std::string caseLabel{"SvoVector::pop_back for heap instance with more than "
                                  "one element above buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t initialSize = values.size();
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 7};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() > BufCap + 1, caseLabel);

         sv.pop_back();

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::pop_back for removing elements until heap "
                                  "instance is within buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5};
      const std::size_t initialSize = values.size();
      const std::size_t numPops = 2;
      const std::initializer_list<Elem> expected{0, 1, 2, 3};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.size() - numPops < BufCap, caseLabel);

         for (int i = 0; i < numPops; ++i)
            sv.pop_back();

         // Remains on heap.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
}


void TestEmplace()
{
   {
      const std::string caseLabel{"SvoVector::emplace value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = numRelocated;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value at beginning"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{100, 0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin(), emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value at end"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = 0;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(!sv.empty(), caseLabel);

         SV::iterator inserted = sv.emplace(sv.end(), emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::iterator inserted = sv.emplace(sv.cbegin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{100};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin(), emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace value for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      constexpr std::size_t insertPos = 3;
      const std::size_t numRelocated = initialSize - insertPos;
      const std::initializer_list<Elem> expected{0, 1, 2, 100, 3, 4};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize + numRelocated;
      metrics.dtorCalls = initialSize + 1 + numRelocated;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::iterator inserted = sv.emplace(sv.begin() + insertPos, emplacedArg);

         VERIFY(inserted->i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace elements with reference argument"};

      struct A
      {
         A(const std::string& s_) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace(sv.begin(), std::string("abc"));
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::emplace elements with pointer argument"};

      struct A
      {
         A(const char* s_) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace(sv.begin(), "abc");
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::emplace elements with array argument"};

      struct A
      {
         A(const char s_[10]) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      char val[10] = "abc";
      sv.emplace(sv.begin(), val);
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::emplace elements with multiple arguments"};

      struct A
      {
         A(const std::string& s_, int i_, double d_) : s{s_}, i{i_}, d{d_} {}
         std::string s;
         int i;
         double d;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace(sv.begin(), "abc", 10, 3.14);
      VERIFY(sv[0].s == "abc", caseLabel);
      VERIFY(sv[0].i == 10, caseLabel);
      VERIFY(sv[0].d == 3.14, caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::emplace elements with r-value argument"};

      struct A
      {
         A(std::string&& s_) : s{std::move(s_)} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace(sv.begin(), "abc");
      VERIFY(sv[0].s == "abc", caseLabel);
   }
}


void TestEmplaceBack()
{
   {
      const std::string caseLabel{"SvoVector::emplace_back value into buffer "
                                  "instance with enough capacity to remain in buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back value into buffer "
                                  "instance with max-ed out buffer capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() == BufCap, caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back value into heap "
                                  "instance with unused capacity left"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize + 5;
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= initialCap, caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back value into heap "
                                  "instance with max-ed out capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back value using a const-iterator"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 > sv.capacity(), caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialSize, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back value into empty vector"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.dtorCalls = 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv;
         VERIFY(sv.empty(), caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace_back value for non-moveable element type"};

      constexpr std::size_t BufCap = 10;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const int emplacedArg = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, emplacedArg};

      Elem::Metrics metrics;
      metrics.ctorCalls = 1;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize + 1;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(sv.size() + 1 <= BufCap, caseLabel);

         SV::reference inserted = sv.emplace_back(emplacedArg);

         VERIFY(inserted.i == emplacedArg, caseLabel);
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == BufCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace_back elements with reference argument"};

      struct A
      {
         A(const std::string& s_) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace_back(std::string("abc"));
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace_back elements with pointer argument"};

      struct A
      {
         A(const char* s_) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace_back("abc");
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{"SvoVector::emplace_back elements with array argument"};

      struct A
      {
         A(const char s_[10]) : s{s_} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      char val[10] = "abc";
      sv.emplace_back(val);
      VERIFY(sv[0].s == "abc", caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace_back elements with multiple arguments"};

      struct A
      {
         A(const std::string& s_, int i_, double d_) : s{s_}, i{i_}, d{d_} {}
         std::string s;
         int i;
         double d;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace_back("abc", 10, 3.14);
      VERIFY(sv[0].s == "abc", caseLabel);
      VERIFY(sv[0].i == 10, caseLabel);
      VERIFY(sv[0].d == 3.14, caseLabel);
   }
   {
      const std::string caseLabel{
         "SvoVector::emplace_back elements with r-value argument"};

      struct A
      {
         A(std::string&& s_) : s{std::move(s_)} {}
         std::string s;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;

      SV sv;
      sv.emplace_back("abc");
      VERIFY(sv[0].s == "abc", caseLabel);
   }
}


void TestResizeWithDefaultValue()
{
   {
      const std::string caseLabel{"SvoVector::resize with default value to same size"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = initialSize;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo == sv.size(), caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for buffer "
                                  "instance to larger size that still fits into buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 8;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 1, 1, 1};

      Elem::Metrics metrics;
      metrics.defaultCtorCalls = resizeTo - initialSize;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo <= BufCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for buffer "
                                  "instance to larger size that needs heap"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 8;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 1, 1, 1, 1};

      Elem::Metrics metrics;
      metrics.defaultCtorCalls = resizeTo - initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo > BufCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::resize with default value for heap "
         "instance to larger size that fits into the current capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 15;
      const std::size_t resizeTo = 10;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 1, 1, 1};

      Elem::Metrics metrics;
      metrics.defaultCtorCalls = resizeTo - initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo < initialCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::resize with default value for heap "
         "instance to larger size fits that doesn't fit into the current capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize;
      const std::size_t resizeTo = 10;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 1, 1, 1};

      Elem::Metrics metrics;
      metrics.defaultCtorCalls = resizeTo - initialSize;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo > initialCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for buffer "
                                  "instance to smaller size"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 5;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo <= BufCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for heap "
                                  "instance to smaller size that fits into buffer"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 4;
      const std::initializer_list<Elem> expected{0, 1, 2, 3};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo < BufCap, caseLabel);

         sv.resize(resizeTo);

         // Remains on heap.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for heap "
                                  "instance to smaller size that still needs heap"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 7;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo > BufCap, caseLabel);

         sv.resize(resizeTo);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for buffer "
                                  "instance to zero size"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.resize(0);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with default value for heap "
                                  "instance to zero size"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.resize(0);

         // Remains on heap.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
}


void TestResizeWithValue()
{
   {
      const std::string caseLabel{"SvoVector::resize with given value to same size"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = initialSize;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo == sv.size(), caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for buffer "
                                  "instance to larger size that still fits into buffer"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 8;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = resizeTo;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo <= BufCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for buffer "
                                  "instance to larger size that needs heap"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 8;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 100, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = resizeTo;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo > BufCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.onHeap(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::resize with given value for heap "
         "instance to larger size that fits into the current capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = 15;
      const std::size_t resizeTo = 10;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = resizeTo;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         sv.reserve(initialCap);
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo < initialCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::resize with given value for heap "
         "instance to larger size fits that doesn't fit into the current capacity"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t initialCap = initialSize;
      const std::size_t resizeTo = 10;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6, 100, 100, 100};

      Elem::Metrics metrics;
      metrics.moveCtorCalls = initialSize;
      metrics.copyCtorCalls = resizeTo;
      metrics.dtorCalls = resizeTo;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo > sv.size(), caseLabel);
         VERIFY(resizeTo > initialCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() > initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for buffer "
                                  "instance to smaller size"};

      constexpr std::size_t BufCap = 10;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 5;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo <= BufCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.inBuffer(), caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for heap "
                                  "instance to smaller size that fits into buffer"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 4;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo < BufCap, caseLabel);

         sv.resize(resizeTo, value);

         // Remains on heap.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for heap "
                                  "instance to smaller size that still needs heap"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7, 8};
      const std::size_t initialSize = values.size();
      const std::size_t resizeTo = 7;
      const Elem value = 100;
      const std::initializer_list<Elem> expected{0, 1, 2, 3, 4, 5, 6};

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         const std::size_t initialCap = sv.capacity();
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);
         VERIFY(resizeTo < sv.size(), caseLabel);
         VERIFY(resizeTo > BufCap, caseLabel);

         sv.resize(resizeTo, value);

         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.capacity() == initialCap, caseLabel);
         verifyVector(sv, expected, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for buffer "
                                  "instance to zero size"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3};
      const std::size_t initialSize = values.size();
      const Elem value = 100;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.resize(0, value);

         VERIFY(sv.inBuffer(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::resize with given value for heap "
                                  "instance to zero size"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> values{0, 1, 2, 3, 4, 5, 6, 7};
      const std::size_t initialSize = values.size();
      const Elem value = 100;

      Elem::Metrics metrics;
      metrics.copyCtorCalls = initialSize;
      metrics.dtorCalls = initialSize;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV sv{values};
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(!sv.empty(), caseLabel);

         sv.resize(0, value);

         // Remains on heap.
         VERIFY(sv.onHeap(), caseLabel);
         VERIFY(sv.empty(), caseLabel);
      });
   }
}


void TestSwap()
{
   {
      const std::string caseLabel{"SvoVector::swap of two heap instances"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1, 2, 3, 4, 5, 6};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103, 104,
                                                105, 106, 107, 108};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeA + sizeB;
      metrics.dtorCalls = sizeA + sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         const std::size_t capA = svA.capacity();
         VERIFY(svA.onHeap(), caseLabel);
         SV svB{valuesB};
         const std::size_t capB = svB.capacity();
         VERIFY(svB.onHeap(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.onHeap(), caseLabel);
         VERIFY(svA.capacity() == capB, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.onHeap(), caseLabel);
         VERIFY(svB.capacity() == capA, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of two buffer instances"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103};
      const std::size_t sizeB = valuesB.size();

      const std::size_t numElemSwaps = std::min(sizeA, sizeB);
      const std::size_t numElemMoves = std::max(sizeA, sizeB) - numElemSwaps;
      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeA + sizeB;
      metrics.moveCtorCalls = numElemSwaps + numElemMoves;
      metrics.moveAssignmentCalls = 2 * numElemSwaps;
      metrics.dtorCalls = sizeA + sizeB + numElemSwaps;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         VERIFY(svA.inBuffer(), caseLabel);
         SV svB{valuesB};
         VERIFY(svB.inBuffer(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.inBuffer(), caseLabel);
         VERIFY(svA.capacity() == BufCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
   {
      const std::string caseLabel{
         "SvoVector::swap of two buffer instances with a non-moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103};
      const std::size_t sizeB = valuesB.size();

      // NotMoveableElement provides a swap overload, so not ctors or assignments
      // are made for a swap.
      const std::size_t numElemCopies = std::max(sizeA, sizeB) - std::min(sizeA, sizeB);
      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeA + sizeB + numElemCopies;
      metrics.dtorCalls = sizeA + sizeB + numElemCopies;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         VERIFY(svA.inBuffer(), caseLabel);
         SV svB{valuesB};
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         svA.swap(svB);

         VERIFY(svA.inBuffer(), caseLabel);
         VERIFY(svA.capacity() == BufCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of between buffer and heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1, 3};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103, 104, 105, 106};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeA + sizeB;
      metrics.moveCtorCalls = sizeA;
      metrics.dtorCalls = sizeA + sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         VERIFY(svA.inBuffer(), caseLabel);
         SV svB{valuesB};
         const std::size_t heapCap = svB.capacity();
         VERIFY(svB.onHeap(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.onHeap(), caseLabel);
         VERIFY(svA.capacity() == heapCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of between buffer and heap instance "
                                  "with a non-moveable element type"};

      constexpr std::size_t BufCap = 5;
      using Elem = NotMoveableElement;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1, 3};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103, 104, 105, 106};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = 2 * sizeA + sizeB;
      metrics.dtorCalls = 2 * sizeA + sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         VERIFY(svA.inBuffer(), caseLabel);
         SV svB{valuesB};
         const std::size_t heapCap = svB.capacity();
         VERIFY(svB.onHeap(), caseLabel);
         VERIFY(!std::is_move_constructible_v<Elem>, caseLabel);

         svA.swap(svB);

         VERIFY(svA.onHeap(), caseLabel);
         VERIFY(svA.capacity() == heapCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of between empty and heap instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesB{100, 101, 102, 103, 104, 105, 106};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeB;
      metrics.dtorCalls = sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA;
         VERIFY(svA.empty(), caseLabel);
         SV svB{valuesB};
         const std::size_t heapCap = svB.capacity();
         VERIFY(svB.onHeap(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.onHeap(), caseLabel);
         VERIFY(svA.capacity() == heapCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         VERIFY(svB.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of between empty and buffer instance"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesB{100, 101};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeB;
      metrics.moveCtorCalls = sizeB;
      metrics.dtorCalls = sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA;
         VERIFY(svA.empty(), caseLabel);
         SV svB{valuesB};
         VERIFY(svB.inBuffer(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.inBuffer(), caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.empty(), caseLabel);
      });
   }
   {
      const std::string caseLabel{"SvoVector::swap of two empty vectors"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      Elem::Metrics zeros;

      Test<Elem, BufCap> test{caseLabel, zeros};
      test.run([&]() {
         SV svA;
         VERIFY(svA.empty(), caseLabel);
         SV svB;
         VERIFY(svB.empty(), caseLabel);

         svA.swap(svB);

         VERIFY(svA.empty(), caseLabel);
         VERIFY(svB.empty(), caseLabel);
      });
   }
}


void TestEquality()
{
   {
      const std::string caseLabel{"Equality of equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"Equality of empty vectors"};

      SboVector<int, 5> a;
      SboVector<int, 5> b;

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"Equality of vectors with unequal sizes"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3, 4};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{"Equality of vectors with unequal elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 10, 3};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{"Equality of non-empty with empty vector"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b;

      VERIFY(!(a == b), caseLabel);
   }
}


void TestInequality()
{
   {
      const std::string caseLabel{"Inequality of equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{"Inequality of empty vectors"};

      SboVector<int, 5> a;
      SboVector<int, 5> b;

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{"Inequality of vectors with unequal sizes"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3, 4};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"Inequality of vectors with unequal elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 10, 3};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"Inequality of non-empty with empty vector"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b;

      VERIFY(a != b, caseLabel);
   }
}


void TestLessThan()
{
   {
      const std::string caseLabel{"Less-than for vectors with different sizes"};

      SboVector<int, 5> a{0, 1, 2};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{"Less-than for vectors with different elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 10, 2, 3};

      VERIFY(a < b, caseLabel);
      VERIFY(!(b < a), caseLabel);
   }
   {
      const std::string caseLabel{"Less-than for equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(!(a < b), caseLabel);
   }
}


void TestGreaterThan()
{
   {
      const std::string caseLabel{"Greater-than for vectors with different sizes"};

      SboVector<int, 5> a{0, 1, 2};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(!(a > b), caseLabel);
      VERIFY(b > a, caseLabel);
   }
   {
      const std::string caseLabel{"Greater-than for vectors with different elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 10, 2, 3};

      VERIFY(!(a > b), caseLabel);
      VERIFY(b > a, caseLabel);
   }
   {
      const std::string caseLabel{"Greater-than for equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(!(a > b), caseLabel);
   }
}


void TestLessOrEqualThan()
{
   {
      const std::string caseLabel{"Less-or-equal-than for vectors with different sizes"};

      SboVector<int, 5> a{0, 1, 2};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{
         "Less-or-equal-than for vectors with different elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 10, 2, 3};

      VERIFY(a <= b, caseLabel);
      VERIFY(!(b <= a), caseLabel);
   }
   {
      const std::string caseLabel{"Less-or-equal-than for equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(a <= b, caseLabel);
   }
}


void TestGreaterOrEqualThan()
{
   {
      const std::string caseLabel{
         "Greater-or-equal-than for vectors with different sizes"};

      SboVector<int, 5> a{0, 1, 2};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(!(a >= b), caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{
         "Greater-or-equal-than for vectors with different elements"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 10, 2, 3};

      VERIFY(!(a >= b), caseLabel);
      VERIFY(b >= a, caseLabel);
   }
   {
      const std::string caseLabel{"Greater-or-equal-than for equal vectors"};

      SboVector<int, 5> a{0, 1, 2, 3};
      SboVector<int, 5> b{0, 1, 2, 3};

      VERIFY(a >= b, caseLabel);
   }
}


void TestStandaloneSwap()
{
   {
      const std::string caseLabel{"Standalone swap"};

      constexpr std::size_t BufCap = 5;
      using Elem = Element;
      using SV = SboVector<Elem, BufCap>;

      const std::initializer_list<Elem> valuesA{0, 1, 3};
      const std::size_t sizeA = valuesA.size();
      const std::initializer_list<Elem> valuesB{100, 101, 102, 103, 104, 105, 106};
      const std::size_t sizeB = valuesB.size();

      Elem::Metrics metrics;
      metrics.copyCtorCalls = sizeA + sizeB;
      metrics.moveCtorCalls = sizeA;
      metrics.dtorCalls = sizeA + sizeB;

      Test<Elem, BufCap> test{caseLabel, metrics};
      test.run([&]() {
         SV svA{valuesA};
         VERIFY(svA.inBuffer(), caseLabel);
         SV svB{valuesB};
         const std::size_t heapCap = svB.capacity();
         VERIFY(svB.onHeap(), caseLabel);

         swap(svA, svB);

         VERIFY(svA.onHeap(), caseLabel);
         VERIFY(svA.capacity() == heapCap, caseLabel);
         verifyVector(svA, valuesB, caseLabel);
         VERIFY(svB.inBuffer(), caseLabel);
         VERIFY(svB.capacity() == BufCap, caseLabel);
         verifyVector(svB, valuesA, caseLabel);
      });
   }
}


///////////////////

void TestIteratorCopyCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator copy ctor"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};
      Iter copy{it};

      VERIFY(*copy == 1, caseLabel);
   }
}


void TestIteratorMoveCtor()
{
   {
      const std::string caseLabel{"SboVectorIterator move ctor"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};
      Iter dest{std::move(it)};

      VERIFY(*dest == 1, caseLabel);
   }
}


void TestIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator copy assignment"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};
      Iter copy;
      copy = it;

      VERIFY(*copy == 1, caseLabel);
   }
}


void TestIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator move assignment"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};
      Iter dest;
      dest = std::move(it);

      VERIFY(*dest == 1, caseLabel);
   }
}


void TestIteratorIndirectionOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator indirection operator for reading"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator indirection operator for writing"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin()};
      *it = 3;

      VERIFY(*it == 3, caseLabel);
   }
}


void TestIteratorIndirectionOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const indirection operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2};

      const Iter it{sv.begin()};

      VERIFY(*it == 1, caseLabel);
   }
}


void TestIteratorDereferenceOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator dereference operator for reading"};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1, 2}, {3, 4}};

      Iter it{sv.begin() + 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator dereference operator for writing"};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1, 2}, {3, 4}};

      Iter it{sv.begin()};
      it->i = 100;
      it->d = 0.001;

      VERIFY(it->i == 100, caseLabel);
      VERIFY(it->d == 0.001, caseLabel);
   }
}


void TestIteratorDereferenceOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const dereference operator"};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1, 2}, {3, 4}};

      const Iter it{sv.begin() + 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
}


void TestIteratorSubscriptOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator operator[] for reading"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1}, {2}, {3}, {4}};

      Iter it{sv.begin() + 1};

      VERIFY(it[2] == 4, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator[] for writing"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1}, {2}, {3}, {4}};

      Iter it{sv.begin()};
      it[2] = 0;

      VERIFY(it[2] == 0, caseLabel);
   }
}


void TestIteratorSubscriptOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorIterator const operator[] for reading"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1}, {2}, {3}, {4}};

      const Iter it{sv.begin()};

      VERIFY(it[2] == 3, caseLabel);
   }
}


void TestIteratorPrefixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1}, {2}, {3}};

      Iter it{sv.begin()};
      auto same = ++it;

      VERIFY(*it == 2, caseLabel);
      VERIFY(*same == 2, caseLabel);
   }
}


void TestIteratorPostfixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{{1}, {2}, {3}};

      Iter it{sv.begin()};
      auto prev = it++;

      VERIFY(*it == 2, caseLabel);
      VERIFY(*prev == 1, caseLabel);
   }
}


void TestIteratorPrefixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix decrement operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto same = --it;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*same == 1, caseLabel);
   }
}


void TestIteratorPostfixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto prev = it--;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestIteratorSwap()
{
   {
      const std::string caseLabel{"SboVectorIterator swap"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 1};

      swap(a, b);

      VERIFY(*a == 2, caseLabel);
      VERIFY(*b == 1, caseLabel);
   }
}


void TestIteratorEquality()
{
   {
      const std::string caseLabel{"SboVectorIterator equality for equal values"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{sv.begin()};

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator equality for different indices"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 1};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator equality for different vectors"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};
      SV other{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{other.begin()};

      VERIFY(!(a == b), caseLabel);
   }
}


void TestIteratorInequality()
{
   {
      const std::string caseLabel{"SboVectorIterator inequality for equal values"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{sv.begin()};

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator inequality for different indices"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 1};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator inequality for different vectors"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};
      SV other{1, 2, 3};

      Iter a{sv.begin()};
      Iter b{other.begin()};

      VERIFY(a != b, caseLabel);
   }
}


void TestIteratorAdditionAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator operator+= for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      it += 2;

      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator+= for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      it += -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestIteratorSubtractionAssignment()
{
   {
      const std::string caseLabel{"SboVectorIterator operator-= for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      it -= 2;

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator-= for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      it -= -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestIteratorAdditionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorIterator iterator plus offset for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = it + 2;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator iterator plus offset for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = it + -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestIteratorAdditionOfOffsetAndIterator()
{
   {
      const std::string caseLabel{
         "SboVectorIterator offset plus iterator for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = 2 + it;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator offset plus iterator for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = -1 + it;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestIteratorSubtractionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorIterator iterator minus offset for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = it - 2;

      VERIFY(*res == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator iterator minus offset for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = it - -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestIteratorSubtractionOfIterators()
{
   {
      const std::string caseLabel{"SboVectorIterator iterator minus iterator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};
      const std::size_t diff = a - b;

      VERIFY(diff == 2, caseLabel);
   }
}


void TestIteratorLessThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator< for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(a < b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator< for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(!(a < b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator< for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(!(a < b), caseLabel);
   }
}


void TestIteratorLessOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator<= for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator<= for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(!(a <= b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator<= for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(a <= b, caseLabel);
   }
}


void TestIteratorGreaterThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator> for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(!(a > b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator> for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(a > b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator> for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(!(a > b), caseLabel);
   }
}


void TestIteratorGreaterOrEqualThan()
{
   {
      const std::string caseLabel{"SboVectorIterator operator>= for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorIterator operator>= for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(a >= b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorIterator operator>= for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(a >= b, caseLabel);
   }
}


///////////////////

void TestConstIteratorCopyCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator copy ctor"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin() + 1};
      Iter copy{it};

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestConstIteratorMoveCtor()
{
   {
      const std::string caseLabel{"SboVectorConstIterator move ctor"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin() + 1};
      Iter dest{std::move(it)};

      VERIFY(*dest == 2, caseLabel);
   }
}


void TestConstIteratorCopyAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator copy assignment"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin() + 1};
      Iter copy;
      copy = it;

      VERIFY(*copy == 2, caseLabel);
   }
}


void TestConstIteratorMoveAssignment()
{
   {
      const std::string caseLabel{"SboVectorConstIterator move assignment"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin() + 1};
      Iter dest;
      dest = std::move(it);

      VERIFY(*dest == 2, caseLabel);
   }
}


void TestConstIteratorIndirectionOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const indirection operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2};

      Iter it{sv.begin() + 1};

      VERIFY(*it == 2, caseLabel);
   }
}


void TestConstIteratorDereferenceOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const dereference operator"};

      struct A
      {
         int i = 0;
         double d = 0;
      };

      constexpr std::size_t BufCap = 10;
      using Elem = A;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{{1, 2}, {3, 4}};

      Iter it{sv.begin() + 1};

      VERIFY(it->i == 3, caseLabel);
      VERIFY(it->d == 4, caseLabel);
   }
}


void TestConstIteratorSubscriptOperatorConst()
{
   {
      const std::string caseLabel{"SboVectorConstIterator const operator[] for reading"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3, {4}};

      Iter it{sv.begin() + 1};

      VERIFY(it[2] == 4, caseLabel);
   }
}


void TestConstIteratorPrefixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto same = ++it;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*same == 3, caseLabel);
   }
}


void TestConstIteratorPostfixIncrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto prev = it++;

      VERIFY(*it == 3, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestConstIteratorPrefixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix decrement operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto same = --it;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*same == 1, caseLabel);
   }
}


void TestConstIteratorPostfixDecrementOperator()
{
   {
      const std::string caseLabel{"SboVectorConstIterator prefix increment operator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 1};
      auto prev = it--;

      VERIFY(*it == 1, caseLabel);
      VERIFY(*prev == 2, caseLabel);
   }
}


void TestConstIteratorSwap()
{
   {
      const std::string caseLabel{"SboVectorConstIterator swap"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{sv.begin() + 2};

      swap(a, b);

      VERIFY(*a == 3, caseLabel);
      VERIFY(*b == 2, caseLabel);
   }
}


void TestConstIteratorEquality()
{
   {
      const std::string caseLabel{"SboVectorConstIterator equality for equal values"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{sv.begin() + 1};

      VERIFY(a == b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator equality for different indices"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{sv.begin() + 2};

      VERIFY(!(a == b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator equality for different vectors"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};
      SV other{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{other.begin() + 1};

      VERIFY(!(a == b), caseLabel);
   }
}


void TestConstIteratorInequality()
{
   {
      const std::string caseLabel{"SboVectorConstIterator inequality for equal values"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{sv.begin() + 1};

      VERIFY(!(a != b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator inequality for different indices"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{sv.begin() + 2};

      VERIFY(a != b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator inequality for different vectors"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};
      SV other{1, 2, 3};

      Iter a{sv.begin() + 1};
      Iter b{other.begin() + 1};

      VERIFY(a != b, caseLabel);
   }
}


void TestConstIteratorAdditionAssignment()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator+= for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      it += 2;

      VERIFY(*it == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator+= for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      it += -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestConstIteratorSubtractionAssignment()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator-= for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      it -= 2;

      VERIFY(*it == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator-= for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      it -= -1;

      VERIFY(*it == 2, caseLabel);
   }
}


void TestConstIteratorAdditionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator plus offset for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = it + 2;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator plus offset for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = it + -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestConstIteratorAdditionOfOffsetAndIterator()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator offset plus iterator for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = 2 + it;

      VERIFY(*res == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator offset plus iterator for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = -1 + it;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestConstIteratorSubtractionOfIteratorAndOffset()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator minus offset for positive offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin() + 2};
      Iter res = it - 2;

      VERIFY(*res == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator iterator minus offset for negative offset"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 3};

      Iter it{sv.begin()};
      Iter res = it - -1;

      VERIFY(*res == 2, caseLabel);
   }
}


void TestConstIteratorSubtractionOfIterators()
{
   {
      const std::string caseLabel{"SboVectorConstIterator iterator minus iterator"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};
      const std::size_t diff = a - b;

      VERIFY(diff == 2, caseLabel);
   }
}


void TestConstIteratorLessThan()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator< for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(a < b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator< for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(!(a < b), caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator< for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(!(a < b), caseLabel);
   }
}


void TestConstIteratorLessOrEqualThan()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(a <= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(!(a <= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator<= for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(a <= b, caseLabel);
   }
}


void TestConstIteratorGreaterThan()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator> for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(!(a > b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator> for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(a > b, caseLabel);
   }
   {
      const std::string caseLabel{"SboVectorConstIterator operator> for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(!(a > b), caseLabel);
   }
}


void TestConstIteratorGreaterOrEqualThan()
{
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for less-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin()};
      Iter b{sv.begin() + 2};

      VERIFY(!(a >= b), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for greater-than iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin()};

      VERIFY(a >= b, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVectorConstIterator operator>= for equal iterators"};

      constexpr std::size_t BufCap = 10;
      using Elem = int;
      using SV = SboVector<Elem, BufCap>;
      using Iter = SboVectorConstIterator<Elem, BufCap>;

      SV sv{1, 2, 20};

      Iter a{sv.begin() + 2};
      Iter b{sv.begin() + 2};

      VERIFY(a >= b, caseLabel);
   }
}

} // namespace


///////////////////

void TestSboVector()
{
   TestDefaultCtor();
   TestCtorForElementCountAndValue();
   TestIteratorCtor();
   TestInitializerListCtor();
   TestCopyCtor();
   TestMoveCtor();
   TestDtor();
   TestCopyAssignment();
   TestMoveAssignment();
   TestInitializerListAssignment();
   TestAssignElementValue();
   TestAssignIteratorRange();
   TestAssignInitializerList();
   TestAt();
   TestAtConst();
   TestSubscriptOperator();
   TestSubscriptOperatorConst();
   TestFront();
   TestFrontConst();
   TestBack();
   TestBackConst();
   TestData();
   TestDataConst();
   TestBegin();
   TestEnd();
   TestBeginConst();
   TestEndConst();
   TestCBegin();
   TestCEnd();
   TestRBegin();
   TestREnd();
   TestRBeginConst();
   TestREndConst();
   TestCRBegin();
   TestCREnd();
   TestEmpty();
   TestSize();
   TestMaxSize();
   TestReserve();
   TestShrinkToFit();
   TestClear();
   TestEraseSingleElement();
   TestEraseIteratorRange();
   TestInsertSingleValue();
   TestInsertSingleRValue();
   TestInsertValueMultipleTimes();
   TestInsertRange();
   TestInsertInitializerList();
   TestPushBackLValue();
   TestPushBackRValue();
   TestPopBack();
   TestEmplace();
   TestEmplaceBack();
   TestResizeWithDefaultValue();
   TestResizeWithValue();
   TestSwap();
   TestEquality();
   TestInequality();
   TestLessThan();
   TestGreaterThan();
   TestLessOrEqualThan();
   TestGreaterOrEqualThan();
   TestStandaloneSwap();

   TestIteratorCopyCtor();
   TestIteratorMoveCtor();
   TestIteratorCopyAssignment();
   TestIteratorMoveAssignment();
   TestIteratorIndirectionOperator();
   TestIteratorIndirectionOperatorConst();
   TestIteratorDereferenceOperator();
   TestIteratorDereferenceOperatorConst();
   TestIteratorSubscriptOperator();
   TestIteratorSubscriptOperatorConst();
   TestIteratorPrefixIncrementOperator();
   TestIteratorPostfixIncrementOperator();
   TestIteratorPrefixDecrementOperator();
   TestIteratorPostfixDecrementOperator();
   TestIteratorSwap();
   TestIteratorEquality();
   TestIteratorInequality();
   TestIteratorAdditionAssignment();
   TestIteratorSubtractionAssignment();
   TestIteratorAdditionOfIteratorAndOffset();
   TestIteratorAdditionOfOffsetAndIterator();
   TestIteratorSubtractionOfIteratorAndOffset();
   TestIteratorSubtractionOfIterators();
   TestIteratorLessThan();
   TestIteratorLessOrEqualThan();
   TestIteratorGreaterThan();
   TestIteratorGreaterOrEqualThan();

   TestConstIteratorCopyCtor();
   TestConstIteratorMoveCtor();
   TestConstIteratorCopyAssignment();
   TestConstIteratorMoveAssignment();
   TestConstIteratorIndirectionOperatorConst();
   TestConstIteratorDereferenceOperatorConst();
   TestConstIteratorSubscriptOperatorConst();
   TestConstIteratorPrefixIncrementOperator();
   TestConstIteratorPostfixIncrementOperator();
   TestConstIteratorPrefixDecrementOperator();
   TestConstIteratorPostfixDecrementOperator();
   TestConstIteratorSwap();
   TestConstIteratorEquality();
   TestConstIteratorInequality();
   TestConstIteratorAdditionAssignment();
   TestConstIteratorSubtractionAssignment();
   TestConstIteratorAdditionOfIteratorAndOffset();
   TestConstIteratorAdditionOfOffsetAndIterator();
   TestConstIteratorSubtractionOfIteratorAndOffset();
   TestConstIteratorSubtractionOfIterators();
   TestConstIteratorLessThan();
   TestConstIteratorLessOrEqualThan();
   TestConstIteratorGreaterThan();
   TestConstIteratorGreaterOrEqualThan();
}
