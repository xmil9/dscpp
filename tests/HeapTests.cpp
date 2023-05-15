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

void testHeapViewCtor()
{
   {
      const std::string caseLabel{"HeapView default ctor"};

      MaxHeap<int*> h;
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView ctor"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      MaxHeap<int> h{v.data(), v.size()};

      VERIFY(h.size() == v.size(), caseLabel);
   }
}

void testHeapViewSize()
{
   {
      const std::string caseLabel{"HeapView::size() for empty heap"};

      MaxHeap<int*> h;
      VERIFY(h.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::size() for populated heap"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      MaxHeap<int> h{v.data(), v.size()};

      VERIFY(h.size() == v.size(), caseLabel);
   }
}

void testHeapViewEmpty()
{
   {
      const std::string caseLabel{"HeapView::empty() for empty heap"};

      MaxHeap<int*> h;
      VERIFY(h.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"HeapView::empty() for populated heap"};

      std::vector<int> v{3, 6, 800, 34, 444, 2};
      MaxHeap<int> h{v.data(), v.size()};

      VERIFY(!h.empty(), caseLabel);
   }
}

} // namespace


///////////////////

void testHeapView()
{
   testHeapViewCtor();
   testHeapViewSize();
   testHeapViewEmpty();
}
