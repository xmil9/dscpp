//
// May-2023, Michael Lindner
// MIT license
//
#include "HeapTests.h"
#include "Heap.h"
#include "TestUtil.h"
#include <vector>

using namespace ds;


namespace
{
///////////////////

void testHeapViewDefaultCtor()
{
   {
      const std::string caseLabel{"HeapView default ctor"};

      const MaxHeap<int> h;
      VERIFY(h.empty(), caseLabel);
   }
}

void testHeapViewCtorForElementPointer()
{
   {
      const std::string caseLabel{"HeapView::HeapView(T*, size_t) ctor"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const MaxHeap<int> h{v.data(), v.size()};

      VERIFY(h.size() == v.size(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::HeapView(T*, size_t) ctor for no elements"};

      const MaxHeap<int> h{nullptr, 0};
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{
         "HeapView::HeapView(T*, size_t) ctor with heap property"};

      struct CustomLess
      {
         bool operator()(int a, int b) const { return a < b; }
      };
      CustomLess myLess;

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const HeapView<int, CustomLess> h{v.data(), v.size(), myLess};

      VERIFY(h.size() == v.size(), caseLabel);
   }
}

void testHeapViewCtorForContainer()
{
   {
      const std::string caseLabel{"HeapView::HeapView(Container&) ctor"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const MaxHeap<int> h{v};

      VERIFY(h.size() == v.size(), caseLabel);
   }
   {
      const std::string caseLabel{
         "HeapView::HeapView(Container&) ctor for empty container"};

      std::vector<int> v;
      const MaxHeap<int> h{v};
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{
         "HeapView::HeapView(Container&) ctor with heap property"};

      struct CustomLess
      {
         bool operator()(int a, int b) const { return a < b; }
      };
      CustomLess myLess;

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const HeapView<int, CustomLess> h{v, myLess};

      VERIFY(h.size() == v.size(), caseLabel);
   }
}

void testHeapViewSize()
{
   {
      const std::string caseLabel{"HeapView::size() for empty heap"};

      const MaxHeap<int> h;
      VERIFY(h.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::size() for populated heap"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const MaxHeap<int> h{v.data(), v.size()};

      VERIFY(h.size() == v.size(), caseLabel);
   }
}

void testHeapViewEmpty()
{
   {
      const std::string caseLabel{"HeapView::empty() for empty heap"};

      const MaxHeap<int> h;
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::empty() for populated heap"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      const MaxHeap<int> h{v.data(), v.size()};

      VERIFY(!h.empty(), caseLabel);
   }
}

void testHeapViewPop()
{
   {
      const std::string caseLabel{"HeapView::pop() throws for empty heap"};

      MaxHeap<int> h;
      VERIFY_THROW([&h]() { h.pop(); }, std::runtime_error, caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::pop() for heap with one element"};

      std::vector<int> v{3};
      MaxHeap<int> h{v};
      const int popped = h.pop();

      VERIFY(popped == 3, caseLabel);
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::pop() for max heap with multiple elements"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      MaxHeap<int> h{v};

      const std::vector<int> expected{800, 444, 34, 6, 3, 2};
      for (size_t i = 0; i < expected.size(); ++i)
      {
         VERIFY(h.pop() == expected[i], caseLabel);
         VERIFY(h.size() == v.size() - (i + 1), caseLabel);
      }
         
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::pop() for min heap with multiple elements"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      MinHeap<int> h{v};

      const std::vector<int> expected{2, 3, 6, 34, 444, 800};
      for (size_t i = 0; i < expected.size(); ++i)
      {
         VERIFY(h.pop() == expected[i], caseLabel);
         VERIFY(h.size() == v.size() - (i + 1), caseLabel);
      }
         
      VERIFY(h.empty(), caseLabel);
   }
}

} // namespace


///////////////////

void testHeapView()
{
   testHeapViewDefaultCtor();
   testHeapViewCtorForElementPointer();
   testHeapViewCtorForContainer();
   testHeapViewSize();
   testHeapViewEmpty();
   testHeapViewPop();
}
