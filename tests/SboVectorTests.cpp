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

   int i = 1;
   double d = 2.0;
   bool b = true;

   inline static int defaultCtorCalls = 0;
   inline static void resetCallCount() { defaultCtorCalls = 0; }
};


// Verifies that SboVector works for types without a default ctor.
struct PodWithoutDefaultCtor
{
   PodWithoutDefaultCtor(int ii, double dd, bool bb) : i{ii}, d{dd}, b{bb}
   {
      ++ctorCalls;
   }

   int i;
   double d;
   bool b;

   inline static int ctorCalls = 0;
   inline static void resetCallCount() { ctorCalls = 0; }
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
      const std::string caseLabel{"SboVector default ctor for std::string where elements "
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
      const std::string caseLabel{"SboVector default ctor for std::string where elements "
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
      const std::string caseLabel{"SboVector default ctor for scalar type where elements "
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
      const std::string caseLabel{"SboVector default ctor for scalar type where elements "
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

} // namespace


///////////////////

void TestSboVector()
{
   TestSboVectorDefaultCtor();
   TestSboVectorCtorForElementCountAndValue();
}
