#include "SortTests.h"
#include "Sort.h"
#include "TestUtil.h"

using namespace ds;


namespace
{
///////////////////

void TestInsertionSort()
{
   {
      const std::string caseLabel{"InsertionSort for sorting integers"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
}

} // namespace


///////////////////

void TestSort()
{
   TestInsertionSort();
}
