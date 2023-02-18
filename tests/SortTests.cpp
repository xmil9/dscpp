#include "SortTests.h"
#include "Sort.h"
#include "TestUtil.h"
#include <deque>

using namespace ds;


namespace
{
///////////////////

void TestInsertionSort()
{
   {
      const std::string caseLabel{
         "InsertionSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "InsertionSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      InsertionSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "InsertionSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "InsertionSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      InsertionSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"InsertionSort for sorting empty container"};

      std::vector<int> seq;
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"InsertionSort for sorting container with one element"};

      std::vector<int> seq{100};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"InsertionSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"InsertionSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      InsertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"InsertionSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      InsertionSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "InsertionSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      InsertionSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}

void TestMergeSort()
{
   {
      const std::string caseLabel{
         "MergeSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      MergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "MergeSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      MergeSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "MergeSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      MergeSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "MergeSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      MergeSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MergeSort for sorting empty container"};

      std::vector<int> seq;
      MergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MergeSort for sorting container with one element"};

      std::vector<int> seq{100};
      MergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MergeSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      MergeSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MergeSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      MergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MergeSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      MergeSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "MergeSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      MergeSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}


void TestBubbleSort()
{
   {
      const std::string caseLabel{
         "BubbleSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      BubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "BubbleSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      BubbleSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "BubbleSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      BubbleSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "BubbleSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      BubbleSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"BubbleSort for sorting empty container"};

      std::vector<int> seq;
      BubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"BubbleSort for sorting container with one element"};

      std::vector<int> seq{100};
      BubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"BubbleSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      BubbleSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"BubbleSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      BubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"BubbleSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      BubbleSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "BubbleSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      BubbleSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}

} // namespace


///////////////////

void TestSort()
{
   TestInsertionSort();
   TestMergeSort();
   TestBubbleSort();
}
