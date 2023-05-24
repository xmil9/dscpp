//
// May-2023, Michael Lindner
// MIT license
//
#include "PriorityQueueTests.h"
#include "PriorityQueue.h"
#include "TestUtil.h"

using namespace ds;


namespace
{
///////////////////

void testPriorityQueueDefaultCtor()
{
   {
      const std::string caseLabel{"PriorityQueue default ctor"};

      const PriorityQueue<int> q;
      VERIFY(q.empty(), caseLabel);
   }
}

void testPriorityQueueCtorWithCompare()
{
   {
      const std::string caseLabel{"PriorityQueue ctor with comparision"};

      const PriorityQueue<int, std::less<int>> q{std::less<int>()};
      VERIFY(q.empty(), caseLabel);
   }
}

void testPriorityQueueCtorWithInitializerList()
{
   {
      const std::string caseLabel{"PriorityQueue ctor with initializer list"};

      const PriorityQueue<int> q{{3, 15, 11, 99, 46}};
      VERIFY(q.size() == 5, caseLabel);
   }
}

void testPriorityQueueCtorWithInitializerListAndCompare()
{
   {
      const std::string caseLabel{
         "PriorityQueue ctor with initializer list and comparision"};

      const PriorityQueue<short, std::greater<short>> q{{3, 15, 11, 99, 46}, {}};
      VERIFY(q.size() == 5, caseLabel);
   }
}

void testPriorityQueueCtorWithIterators()
{
   {
      const std::string caseLabel{"PriorityQueue ctor with iterators"};

      const std::vector<double> source{3., 15., 11., 99., 46.};
      const PriorityQueue<double> q{source.begin(), source.end()};
      VERIFY(q.size() == 5, caseLabel);
   }
}

void testPriorityQueueCtorWithIteratorsAndCompare()
{
   {
      const std::string caseLabel{"PriorityQueue ctor with iterators and comparision"};

      const std::vector<double> source{3., 15., 11., 99., 46.};
      const PriorityQueue<double, std::greater<double>> q{
         source.begin(), source.end(), {}};
      VERIFY(q.size() == 5, caseLabel);
   }
}

void testPriorityQueueSize()
{
   {
      const std::string caseLabel{"PriorityQueue::size() for empty queue"};

      const PriorityQueue<int> q;
      VERIFY(q.size() == 0, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::size() for populated queue"};

      const PriorityQueue<int> q{{3, 6, 800, 34, 444, 2}};
      VERIFY(q.size() == 6, caseLabel);
   }
}

void testPriorityQueueEmpty()
{
   {
      const std::string caseLabel{"PriorityQueue::empty() for empty queue"};

      const PriorityQueue<int> q;
      VERIFY(q.empty(), caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::empty() for populated queue"};

      const PriorityQueue<int> q{{3, 6, 800, 34, 444, 2}};
      VERIFY(!q.empty(), caseLabel);
   }
}

void testPriorityQueuePop()
{
   {
      const std::string caseLabel{"PriorityQueue::pop() throws for empty queue"};

      PriorityQueue<int> q;
      VERIFY_THROW([&q]() { q.pop(); }, std::runtime_error, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::pop() for queue with one element"};

      PriorityQueue<int> q{{3}};
      const int popped = q.pop();

      VERIFY(popped == 3, caseLabel);
      VERIFY(q.empty(), caseLabel);
   }
   {
      const std::string caseLabel{
         "PriorityQueue::pop() for queue with multiple elements"};

      PriorityQueue<int> q{{3, 6, 800, 34, 444, 2}};

      const std::vector<int> expected{2, 3, 6, 34, 444, 800};
      for (size_t i = 0; i < expected.size(); ++i)
      {
         VERIFY(q.pop() == expected[i], caseLabel);
         VERIFY(q.size() == expected.size() - (i + 1), caseLabel);
      }

      VERIFY(q.empty(), caseLabel);
   }
   {
      const std::string caseLabel{
         "PriorityQueue::pop() for queue with multiple elements and custom comparision"};

      PriorityQueue<short, std::greater<short>> q{{3, 6, 800, 34, 444, 2},
                                                  std::greater<short>{}};

      const std::vector<int> expected{800, 444, 34, 6, 3, 2};
      for (size_t i = 0; i < expected.size(); ++i)
      {
         VERIFY(q.pop() == expected[i], caseLabel);
         VERIFY(q.size() == expected.size() - (i + 1), caseLabel);
      }

      VERIFY(q.empty(), caseLabel);
   }
}

void testPriorityQueueTop()
{
   {
      const std::string caseLabel{"PriorityQueue::top() throws for empty queue"};

      PriorityQueue<int> q;
      VERIFY_THROW([&q]() { q.top(); }, std::runtime_error, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::top() for queue with one element"};

      PriorityQueue<int> q{{3}};
      VERIFY(q.top() == 3, caseLabel);
      VERIFY(q.size() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "PriorityQueue::top() for queue with multiple elements"};

      PriorityQueue<int> q{{3, 6, 800, 34, 444, 2}};
      const size_t initialSize = q.size();

      VERIFY(q.top() == 2, caseLabel);
      VERIFY(q.size() == initialSize, caseLabel);

      q.pop();
      VERIFY(q.top() == 3, caseLabel);
      VERIFY(q.size() == initialSize - 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "PriorityQueue::top() for queue with multiple elements and custom comparision"};

      PriorityQueue<short, std::greater<short>> q{{3, 6, 800, 34, 444, 2},
                                                  std::greater<short>{}};
      const size_t initialSize = q.size();

      VERIFY(q.top() == 800, caseLabel);
      VERIFY(q.size() == initialSize, caseLabel);

      q.pop();
      VERIFY(q.top() == 444, caseLabel);
      VERIFY(q.size() == initialSize - 1, caseLabel);
   }
}

void testPriorityQueueInsert()
{
   {
      const std::string caseLabel{"PriorityQueue::insert() into empty queue"};

      PriorityQueue<int> q;
      q.insert(20);

      VERIFY(q.top() == 20, caseLabel);
      VERIFY(q.size() == 1, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::insert() into populated queue"};

      PriorityQueue<int> q{{44, 2, 100, 32, 10}};
      const size_t initialSize = q.size();

      q.insert(20);

      VERIFY(q.size() == initialSize + 1, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::insert() into min-queue as smallest element"};

      PriorityQueue<int> q{{44, 22, 100, 32, 102}};
      const size_t initialSize = q.size();

      q.insert(20);

      VERIFY(q.size() == initialSize + 1, caseLabel);
      VERIFY(q.top() == 20, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::insert() into max-queue as largest element"};

      PriorityQueue<int, std::greater<int>> q{{44, 22, 100, 32, 102}, {}};
      const size_t initialSize = q.size();

      q.insert(200);

      VERIFY(q.size() == initialSize + 1, caseLabel);
      VERIFY(q.top() == 200, caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::insert() multiple elements"};

      PriorityQueue<int, std::greater<int>> q{{44, 22, 100, 32, 102}, {}};
      q.insert(200);
      q.insert(2);
      q.insert(50);


      const std::vector<int> expected{200, 102, 100, 50, 44, 32, 22, 2};
      VERIFY(q.size() == expected.size(), caseLabel);
      for (size_t i = 0; i < expected.size(); ++i)
         VERIFY(q.pop() == expected[i], caseLabel);
   }
   {
      const std::string caseLabel{"PriorityQueue::insert() mixed insertions and popping"};

      PriorityQueue<int, std::greater<int>> q{{44, 22, 100, 32, 102}, {}};
      q.pop();
      q.insert(200);
      q.insert(2);
      q.pop();
      q.pop();
      q.insert(50);
      q.pop();
      q.insert(-1);
      q.insert(7);

      const std::vector<int> expected{44, 32, 22, 7, 2, -1};
      VERIFY(q.size() == expected.size(), caseLabel);
      for (size_t i = 0; i < expected.size(); ++i)
         VERIFY(q.pop() == expected[i], caseLabel);
   }
}

} // namespace


///////////////////

void testPriorityQueue()
{
   testPriorityQueueDefaultCtor();
   testPriorityQueueCtorWithCompare();
   testPriorityQueueCtorWithInitializerList();
   testPriorityQueueCtorWithInitializerListAndCompare();
   testPriorityQueueCtorWithIterators();
   testPriorityQueueCtorWithIteratorsAndCompare();
   testPriorityQueueSize();
   testPriorityQueueEmpty();
   testPriorityQueuePop();
   testPriorityQueueTop();
   testPriorityQueueInsert();
}
