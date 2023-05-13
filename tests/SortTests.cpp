//
// Feb-2023, Michael Lindner
// MIT license
//
#include "SortTests.h"
#include "Sort.h"
#include "TestUtil.h"
#include <deque>

using namespace ds;


namespace
{
///////////////////

void testInsertionSort()
{
   {
      const std::string caseLabel{
         "insertionSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      insertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "insertionSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      insertionSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "insertionSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      insertionSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "insertionSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      insertionSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"insertionSort for sorting empty container"};

      std::vector<int> seq;
      insertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"insertionSort for sorting container with one element"};

      std::vector<int> seq{100};
      insertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"insertionSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      insertionSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"insertionSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      insertionSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"insertionSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      insertionSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "insertionSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      insertionSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}

void testMergeSort()
{
   {
      const std::string caseLabel{
         "mergeSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      mergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "mergeSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      mergeSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "mergeSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      mergeSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "mergeSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      mergeSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"mergeSort for sorting empty container"};

      std::vector<int> seq;
      mergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"mergeSort for sorting container with one element"};

      std::vector<int> seq{100};
      mergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"mergeSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      mergeSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"mergeSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      mergeSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"mergeSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      mergeSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "mergeSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      mergeSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}


void testBubbleSort()
{
   {
      const std::string caseLabel{
         "bubbleSort for sorting integers with default comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      bubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "bubbleSort for sorting integers with greater-than comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      bubbleSort(std::begin(seq), std::end(seq), std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "bubbleSort for sorting strings with default comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      bubbleSort(std::begin(seq), std::end(seq));

      const std::vector<std::string> expected{"aa", "ab", "ba", "cd", "fa", "fe"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "bubbleSort for sorting strings with greater-than comparision"};

      std::vector<std::string> seq{"cd", "aa", "ab", "fe", "ba", "fa"};
      bubbleSort(std::begin(seq), std::end(seq), std::greater<std::string>());

      const std::vector<std::string> expected{"fe", "fa", "cd", "ba", "ab", "aa"};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"bubbleSort for sorting empty container"};

      std::vector<int> seq;
      bubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected;
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"bubbleSort for sorting container with one element"};

      std::vector<int> seq{100};
      bubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"bubbleSort for sorting std::deque container"};

      std::deque<int> seq{100, 50, 30, 75, 200, 1};
      bubbleSort(std::begin(seq), std::end(seq));

      const std::deque<int> expected{1, 30, 50, 75, 100, 200};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"bubbleSort for sorting already sorted container"};

      std::vector<int> seq{100, 101, 102, 103};
      bubbleSort(std::begin(seq), std::end(seq));

      const std::vector<int> expected{100, 101, 102, 103};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{"bubbleSort range interface for vector"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      bubbleSort(seq);

      const std::vector<int> expected{2, 3, 5, 6, 7, 9};
      VERIFY(seq == expected, caseLabel);
   }
   {
      const std::string caseLabel{
         "bubbleSort range interface for vector and custom comparision"};

      std::vector<int> seq{7, 3, 2, 9, 5, 6};
      bubbleSort(seq, std::greater<int>());

      const std::vector<int> expected{9, 7, 6, 5, 3, 2};
      VERIFY(seq == expected, caseLabel);
   }
}

} // namespace


///////////////////

void testSort()
{
   testInsertionSort();
   testMergeSort();
   testBubbleSort();
}
