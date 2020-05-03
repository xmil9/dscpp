#include "SboVector.h"
#include "TestUtil.h"
#include <iostream>
#include <string>


namespace
{
///////////////////

struct PodWithDefaultCtor
{
   int i = 1;
   double d = 2.0;
   bool b = true;
};


struct PodWithoutDefaultCtor
{
   PodWithoutDefaultCtor(int ii, double dd, bool bb) : i{ii}, d{dd}, b{bb} {}

   int i;
   double d;
   bool b;
};


struct AbstractBase
{
   virtual ~AbstractBase() = default;
   virtual void funcA() = 0;
};


struct DerivedWithVtable : public AbstractBase
{
   void funcA() override
   {
      i = 10;
   }

   int i = 0;
};


///////////////////

void TestSboVectorDefaultCtor()
{
   {
      const std::string caseLabel{
         "SboVector default ctor for pod-type with default ctor."};
      SboVector<PodWithDefaultCtor, 10> sv;
      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector default ctor for pod-type without default ctor."};
      SboVector<PodWithoutDefaultCtor, 10> sv;
      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector default ctor for class with vtable."};
      SboVector<DerivedWithVtable, 10> sv;
      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector default ctor for std::string."};
      SboVector<std::string, 10> sv;
      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
   }
   {
      const std::string caseLabel{
         "SboVector default ctor for built-in type."};
      SboVector<int, 10> sv;
      VERIFY(sv.empty(), caseLabel);
      VERIFY(sv.capacity() == 10, caseLabel);
   }
}

void TestSboVectorCtorForElementCountAndValue()
{
}

} // namespace


///////////////////

void TestSboVector()
{
   TestSboVectorDefaultCtor();
   TestSboVectorCtorForElementCountAndValue();
}
