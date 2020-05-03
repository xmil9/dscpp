#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <string>
#include <vector>


namespace
{
///////////////////

struct PodWithDefaultCtor
{
   PodWithDefaultCtor() { ++defaultCtorCalls; }

   int i = 1;
   double d = 2.0;
   bool b = true;

   inline static int defaultCtorCalls = 0;
   inline static void resetCallCount() { defaultCtorCalls = 0; }
};


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
   virtual void funcA() = 0;
};


struct DerivedWithVtable : public AbstractBase
{
   void funcA() override { i = 10; }

   int i = 0;
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

      SboVector<DerivedWithVtable, 10> sv;

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
      const std::string caseLabel{"SboVector default ctor for built-in type."};

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
}

} // namespace


///////////////////

void TestSboVector()
{
   TestSboVectorDefaultCtor();
   TestSboVectorCtorForElementCountAndValue();
}
