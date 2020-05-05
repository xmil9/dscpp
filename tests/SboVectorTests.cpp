#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <string>
#include <vector>


namespace
{
///////////////////

// Verifies that SboVector works for types with a default ctor.
struct PodWithDefaultCtor
{
   PodWithDefaultCtor() { ++defaultCtorCalls; }
   PodWithDefaultCtor(const PodWithDefaultCtor& other)
   : d{other.d}, i{other.i}, b{other.b}
   {
      ++copyCtorCalls;
   }
   PodWithDefaultCtor(PodWithDefaultCtor&& other)
   {
      std::swap(d, other.d);
      std::swap(i, other.i);
      std::swap(b, other.b);
      ++moveCtorCalls;
   }
   PodWithDefaultCtor& operator=(const PodWithDefaultCtor& other)
   {
      d = other.d;
      i = other.i;
      b = other.b;
      ++assignmentCalls;
   }

   double d = 2.0;
   int i = 1;
   bool b = true;

   inline static int defaultCtorCalls = 0;
   inline static int copyCtorCalls = 0;
   inline static int moveCtorCalls = 0;
   inline static int assignmentCalls = 0;

   inline static void resetCallCount()
   {
      defaultCtorCalls = 0;
      copyCtorCalls = 0;
      moveCtorCalls = 0;
      assignmentCalls = 0;
   }
};


// Verifies that SboVector works for types without a default ctor.
struct PodWithoutDefaultCtor
{
   PodWithoutDefaultCtor(int ii, double dd, bool bb) : i{ii}, d{dd}, b{bb}
   {
      ++ctorCalls;
   }
   PodWithoutDefaultCtor(const PodWithoutDefaultCtor& other)
   : d{other.d}, i{other.i}, b{other.b}
   {
      ++copyCtorCalls;
   }
   PodWithoutDefaultCtor(PodWithoutDefaultCtor&& other)
   {
      std::swap(i, other.i);
      std::swap(d, other.d);
      std::swap(b, other.b);
      ++moveCtorCalls;
   }
   PodWithoutDefaultCtor& operator=(const PodWithoutDefaultCtor& other)
   {
      i = other.i;
      d = other.d;
      b = other.b;
      ++assignmentCalls;
   }

   int i;
   double d;
   bool b;

   inline static int ctorCalls = 0;
   inline static int copyCtorCalls = 0;
   inline static int moveCtorCalls = 0;
   inline static int assignmentCalls = 0;

   inline static void resetCallCount()
   {
      ctorCalls = 0;
      copyCtorCalls = 0;
      moveCtorCalls = 0;
      assignmentCalls = 0;
   }
};


struct AbstractBase
{
   virtual ~AbstractBase() = default;
   virtual int getValue() = 0;
};


// Verifies that SboVector works for types that have vtables and that virtual
// function calls get dispatched correctly.
class DerivedWithVtableA : public AbstractBase
{
 public:
   explicit DerivedWithVtableA(int init) : i{init} {}

   int getValue() override { return i++; }

 private:
   int i;
};

class DerivedWithVtableB : public AbstractBase
{
 public:
   explicit DerivedWithVtableB(int init) : i{init} {}

   int getValue() override
   {
      int prev = i;
      i += 2;
      return prev;
   }

 private:
   int i;
};


struct InstrumentedDtor
{
   explicit InstrumentedDtor(int ii) : i{ii} {}
   ~InstrumentedDtor() { ++dtorCalls; }

   int i = 0;

   inline static std::size_t dtorCalls = 0;
   inline static void resetCallCount() { dtorCalls = 0; }
};


///////////////////

void TestSboVectorDefaultCtor()
{
   {
      const std::string caseLabel{
         "SboVector default ctor for pod-type with default ctor."};
      PodWithDefaultCtor::resetCallCount();

      SboVector<PodWithDefaultCtor, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector default ctor for pod-type without default ctor."};
      PodWithoutDefaultCtor::resetCallCount();

      SboVector<PodWithoutDefaultCtor, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::ctorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector default ctor for class with vtable."};

      SboVector<DerivedWithVtableA, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector default ctor for std::string."};

      SboVector<std::string, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
   }
   {
      const std::string caseLabel{"SboVector default ctor for scalar type."};

      SboVector<int, 10> sv;

      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
   }
}


void TestSboVectorCtorForElementCountAndValue()
{
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for pod-type with default ctor where elements "
         "fit into internal buffer."};
      PodWithDefaultCtor::resetCallCount();

      PodWithDefaultCtor val;
      val.i = 2;
      SboVector<PodWithDefaultCtor, 10> sv{5, val};

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 1, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 5, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for pod-type with default ctor where elements "
         "don't fit into internal buffer."};
      PodWithDefaultCtor::resetCallCount();

      PodWithDefaultCtor val;
      val.i = 3;
      SboVector<PodWithDefaultCtor, 10> sv{20, val};

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 1, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 20, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for pod-type without "
         "default ctor where elements fit into internal buffer."};
      PodWithDefaultCtor::resetCallCount();

      PodWithoutDefaultCtor val{2, 3.0, false};
      SboVector<PodWithoutDefaultCtor, 10> sv{5, val};

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::ctorCalls == 1, caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == 5, caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
      {
         VERIFY(sv[i].i == 2, caseLabel);
         VERIFY(sv[i].d == 3.0, caseLabel);
         VERIFY(sv[i].b == false, caseLabel);
      }
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for pod-type without "
         "default ctor where elements don't fit into internal buffer."};
      PodWithoutDefaultCtor::resetCallCount();

      PodWithoutDefaultCtor val{2, 3.0, false};
      SboVector<PodWithoutDefaultCtor, 10> sv{20, val};

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::ctorCalls == 1, caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == 20, caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
      {
         VERIFY(sv[i].i == 2, caseLabel);
         VERIFY(sv[i].d == 3.0, caseLabel);
         VERIFY(sv[i].b == false, caseLabel);
      }
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for class with vtable "
                                  "where elements fit into internal buffer."};

      DerivedWithVtableA val{1};
      SboVector<DerivedWithVtableA, 10> sv{5, val};

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].getValue() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector count-and-value ctor for class with vtable "
                                  "where elements don't fit into internal buffer."};

      DerivedWithVtableA val{1};
      SboVector<DerivedWithVtableA, 10> sv{20, val};

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].getValue() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for std::string where elements "
         "fit into internal buffer."};

      std::string val{"abc"};
      SboVector<std::string, 10> sv{5, val};

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == "abc", caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for std::string where elements "
         "don't fit into internal buffer."};

      std::string val{"abc"};
      SboVector<std::string, 10> sv{20, val};

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == val, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for scalar type where elements "
         "fit into internal buffer."};

      const int val = 1000;
      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> sv(5, val);

      VERIFY(sv.size() == 5, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == val, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector count-and-value ctor for scalar type where elements "
         "don't fit into internal buffer."};

      int val = 1000;
      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> sv(20, val);

      VERIFY(sv.size() == 20, caseLabel);
      VERIFY(sv.capacity() == 20, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == val, caseLabel);
   }
}


void TestSboVectorCopyCtor()
{
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type with default ctor where elements "
         "fit into internal buffer."};

      PodWithDefaultCtor val;
      SboVector<PodWithDefaultCtor, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithDefaultCtor::resetCallCount();
      SboVector<PodWithDefaultCtor, 10> copy{src};

      VERIFY(copy.size() == 5, caseLabel);
      VERIFY(copy.capacity() == 10, caseLabel);
      VERIFY(copy.inBuffer(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 5, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type with default ctor where elements don't "
         "fit into internal buffer."};

      PodWithDefaultCtor val;
      SboVector<PodWithDefaultCtor, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithDefaultCtor::resetCallCount();
      SboVector<PodWithDefaultCtor, 10> copy{src};

      VERIFY(copy.size() == 20, caseLabel);
      VERIFY(copy.capacity() == 20, caseLabel);
      VERIFY(copy.onHeap(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 20, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type without default ctor where elements "
         "fit into internal buffer."};

      PodWithoutDefaultCtor val{1, 1.0, true};
      SboVector<PodWithoutDefaultCtor, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithoutDefaultCtor::resetCallCount();
      SboVector<PodWithoutDefaultCtor, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.inBuffer(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::ctorCalls == 0, caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == copy.size(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == i, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector copy ctor for pod-type without default ctor where elements don't "
         "fit into internal buffer."};

      PodWithoutDefaultCtor val{1, 1.0, true};
      SboVector<PodWithoutDefaultCtor, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithoutDefaultCtor::resetCallCount();
      SboVector<PodWithoutDefaultCtor, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.onHeap(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::ctorCalls == 0, caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == copy.size(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i].i == src[i].i, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy ctor for std::string where elements "
                                  "fit into internal buffer."};

      std::string val;
      SboVector<std::string, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.inBuffer(), caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i] == src[i], caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy ctor for std::string where elements "
                                  "don't fit into internal buffer."};

      std::string val;
      SboVector<std::string, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.onHeap(), caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i] == src[i], caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy ctor for scalar type where elements "
                                  "fit into internal buffer."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(5, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.inBuffer(), caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i] == src[i], caseLabel);
   }
   {
      const std::string caseLabel{"SboVector copy ctor for scalar type where elements "
                                  "don't fit into internal buffer."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(20, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> copy{src};

      VERIFY(copy.size() == src.size(), caseLabel);
      VERIFY(copy.capacity() == src.capacity(), caseLabel);
      VERIFY(copy.onHeap(), caseLabel);
      for (int i = 0; i < copy.size(); ++i)
         VERIFY(copy[i] == src[i], caseLabel);
   }
}


void TestSboVectorMoveCtor()
{
   {
      const std::string caseLabel{
         "SboVector move ctor for pod-type with default ctor where elements "
         "fit into internal buffer."};

      PodWithDefaultCtor val;
      SboVector<PodWithDefaultCtor, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithDefaultCtor::resetCallCount();
      SboVector<PodWithDefaultCtor, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.inBuffer(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::moveCtorCalls == 5, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector move ctor for pod-type with default ctor where elements "
         "don't fit into internal buffer."};

      PodWithDefaultCtor val;
      SboVector<PodWithDefaultCtor, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i].i = i;

      PodWithDefaultCtor::resetCallCount();
      SboVector<PodWithDefaultCtor, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.onHeap(), caseLabel);
      VERIFY(PodWithDefaultCtor::defaultCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::copyCtorCalls == 0, caseLabel);
      // None of the elements move ctors executed because the SboVector simply
      // stole the pointer to the heap memory.
      VERIFY(PodWithDefaultCtor::moveCtorCalls == 0, caseLabel);
      VERIFY(PodWithDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i].i == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for std::string where elements "
                                  "fit into internal buffer."};

      std::string val;
      SboVector<std::string, 10> src{5, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.inBuffer(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == std::to_string(i), caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for std::string where elements "
                                  "don't fit into internal buffer."};

      std::string val;
      SboVector<std::string, 10> src{20, val};
      for (int i = 0; i < src.size(); ++i)
         src[i] = std::to_string(i);

      SboVector<std::string, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.onHeap(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == std::to_string(i), caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for scalar type where elements "
                                  "fit into internal buffer."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(5, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 5, caseLabel);
      VERIFY(moveDest.capacity() == 10, caseLabel);
      VERIFY(moveDest.inBuffer(), caseLabel);
      for (int i = 0; i < moveDest.size(); ++i)
         VERIFY(moveDest[i] == i, caseLabel);
      // Verify moved-from instance is empty.
      VERIFY(src.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector move ctor for scalar type where elements "
                                  "don't fit into internal buffer."};

      // Call with parenthesis to prevent ctor for initializer list to be selected.
      SboVector<int, 10> src(20, 0);
      for (int i = 0; i < src.size(); ++i)
         src[i] = i;

      SboVector<int, 10> moveDest{std::move(src)};

      VERIFY(moveDest.size() == 20, caseLabel);
      VERIFY(moveDest.capacity() == 20, caseLabel);
      VERIFY(moveDest.onHeap(), caseLabel);
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
         "SboVector initializer list ctor for pod-type where elements "
         "fit into internal buffer."};

      PodWithoutDefaultCtor::resetCallCount();
      SboVector<PodWithoutDefaultCtor, 10> sv{
         {1, 1.0, true}, {2, 2.0, true}, {3, 3.0, true}, {4, 4.0, true}};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == 4, caseLabel);
      VERIFY(PodWithoutDefaultCtor::moveCtorCalls == 0, caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for pod-type where elements don't "
         "fit into internal buffer."};

      PodWithoutDefaultCtor::resetCallCount();
      SboVector<PodWithoutDefaultCtor, 10> sv{
         {1, 1.0, true}, {2, 2.0, true},   {3, 3.0, true},   {4, 4.0, true},
         {5, 5.0, true}, {6, 6.0, true},   {7, 7.0, true},   {8, 8.0, true},
         {9, 9.0, true}, {10, 10.0, true}, {11, 11.0, true}, {12, 12.0, true},
      };

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      VERIFY(PodWithoutDefaultCtor::copyCtorCalls == 12, caseLabel);
      VERIFY(PodWithoutDefaultCtor::moveCtorCalls == 0, caseLabel);
      VERIFY(PodWithoutDefaultCtor::assignmentCalls == 0, caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i].i == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for std::string where elements "
         "fit into internal buffer."};

      SboVector<std::string, 10> sv{"1", "2", "3", "4"};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == std::to_string(i + 1), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for std::string where elements "
         "don't fit into internal buffer."};

      SboVector<std::string, 10> sv{"1", "2", "3", "4",  "5",  "6",
                                    "7", "8", "9", "10", "11", "12"};

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == std::to_string(i + 1), caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for scala type where elements "
         "fit into internal buffer."};

      SboVector<int, 10> sv{1, 2, 3, 4};

      VERIFY(sv.size() == 4, caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
      VERIFY(sv.inBuffer(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector initializer list ctor for scalar type on heap."};

      SboVector<int, 10> sv{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

      VERIFY(sv.size() == 12, caseLabel);
      VERIFY(sv.capacity() == 12, caseLabel);
      VERIFY(sv.onHeap(), caseLabel);
      for (int i = 0; i < sv.size(); ++i)
         VERIFY(sv[i] == i + 1, caseLabel);
   }
}


void TestSboVectorDtor()
{
   {
      const std::string caseLabel{"SboVector dtor for internal buffer."};

      InstrumentedDtor::resetCallCount();
      {
         SboVector<InstrumentedDtor, 10> sv{InstrumentedDtor{1}, InstrumentedDtor{2},
                                            InstrumentedDtor{3}};
         VERIFY(sv.inBuffer(), caseLabel);
      }

      VERIFY(InstrumentedDtor::dtorCalls == 3, caseLabel);
   }
   {
      const std::string caseLabel{"SboVector dtor for heap."};

      InstrumentedDtor::resetCallCount();
      {
         SboVector<InstrumentedDtor, 3> sv{InstrumentedDtor{1}, InstrumentedDtor{2},
                                           InstrumentedDtor{3}, InstrumentedDtor{4},
                                           InstrumentedDtor{5}};
         VERIFY(sv.onHeap(), caseLabel);
      }

      VERIFY(InstrumentedDtor::dtorCalls == 5, caseLabel);
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
}
