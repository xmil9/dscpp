//
// Mar-2023, Michael Lindner
// MIT license
//
#pragma once
#include "MathAlg.h"
#include "MatrixView.h"
#include <array>
#include <cassert>
#include <vector>

namespace ds
{
///////////////////

// Add matrix views.
template <typename Val>
MatrixView<Val>& add(const MatrixView<Val>& a, const MatrixView<Val>& b,
                     MatrixView<Val>& c)
{
   // Assert compatible dimensions.
   assert(a.rows() == b.rows() && a.columns() == b.columns());
   assert(a.rows() == c.rows() && a.columns() == c.columns());

   const size_t numRows = a.rows();
   const size_t numCols = a.columns();

   for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numCols; ++j)
         c(i, j) = a(i, j) + b(i, j);

   return c;
}

// Subtract matrix views.
template <typename Val>
MatrixView<Val>& subtract(const MatrixView<Val>& a, const MatrixView<Val>& b,
                          MatrixView<Val>& c)
{
   // Assert compatible dimensions.
   assert(a.rows() == b.rows() && a.columns() == b.columns());
   assert(a.rows() == c.rows() && a.columns() == c.columns());

   const size_t numRows = a.rows();
   const size_t numCols = a.columns();

   for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numCols; ++j)
         c(i, j) = a(i, j) - b(i, j);

   return c;
}

// Simple matrix multiplication using nested loops.
// Cormen, pg 75
// Time: O(n^3)
template <typename Val>
MatrixView<Val>& multiplyIterative(const MatrixView<Val>& a, const MatrixView<Val>& b,
                                   MatrixView<Val>& c)
{
   // Assert compatible dimensions.
   assert(a.columns() == b.rows() && a.rows() == b.columns());
   assert(c.columns() == a.columns() && c.rows() == b.rows());

   const size_t numRows = b.rows();
   const size_t numCols = a.columns();

   for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numCols; ++j)
      {
         c(i, j) = {};
         for (size_t k = 0; k < numCols; ++k)
            c(i, j) += a(i, k) * b(k, j);
      }

   return c;
}

// Multiplies two matrices by partitioning each into four submatrices, recursively
// multiplying the submatrices, and combining them into the result.
// Limitation: Only implemented for square matrices with the matrix dimension a power of
// two. Other matrices would have to be split into submatrices in other ways and padded
// with zeros.
// Cormen, pg 77
// Time: O(n^3)
template <typename Val>
MatrixView<Val>& multiplyRecursive(const MatrixView<Val>& a, const MatrixView<Val>& b,
                                   MatrixView<Val>& c)
{
   // Square matices.
   assert(a.rows() == a.columns());
   assert(a.rows() == a.columns());
   // Matices have same dimensions.
   assert(a.rows() == b.rows() && a.columns() == b.columns());
   assert(a.rows() == c.rows() && a.columns() == c.columns());
   // Dimensions must be power of two.
   assert(isPow2(a.rows()));

   using MV = MatrixView<Val>;
   const auto n = a.rows();

   // Base case - 1x1 matrices.
   if (n == 1)
   {
      c(0, 0) += a(0, 0) * b(0, 0);
      return c;
   }

   // Divide matrices into four submatrices and multiply those.
   const auto mid = n / 2;

   const MV a11{a, 0, mid - 1, 0, mid - 1};
   const MV a12{a, 0, mid - 1, mid, n - 1};
   const MV a21{a, mid, n - 1, 0, mid - 1};
   const MV a22{a, mid, n - 1, mid, n - 1};

   const MV b11{b, 0, mid - 1, 0, mid - 1};
   const MV b12{b, 0, mid - 1, mid, n - 1};
   const MV b21{b, mid, n - 1, 0, mid - 1};
   const MV b22{b, mid, n - 1, mid, n - 1};

   MV c11{c, 0, mid - 1, 0, mid - 1};
   MV c12{c, 0, mid - 1, mid, n - 1};
   MV c21{c, mid, n - 1, 0, mid - 1};
   MV c22{c, mid, n - 1, mid, n - 1};

   // Note that because of the += operation in the 1x1 base case, the following
   // two lines are equivalent to:
   // c11 = a11*b11 + a12*b21
   multiplyRecursive(a11, b11, c11);
   multiplyRecursive(a12, b21, c11);

   // c12 = a11*b12 + a12*b22
   multiplyRecursive(a11, b12, c12);
   multiplyRecursive(a12, b22, c12);

   // c21 = a21*b11 + a22*b21
   multiplyRecursive(a21, b11, c21);
   multiplyRecursive(a22, b21, c21);

   // c22 = a21*b12 + a22*b22
   multiplyRecursive(a21, b12, c22);
   multiplyRecursive(a22, b22, c22);

   return c;
}

// Strassen algorithm for matrix multiplication.
// Limitation: Only implemented for square matrices with the matrix dimension a power of
// two. Other matrices would have to be split into submatrices in other ways and padded
// with zeros.
// Time: O(n^lg7) = O(n^2.81)
template <typename Val>
MatrixView<Val>& multiplyStrassen(const MatrixView<Val>& a, const MatrixView<Val>& b,
                                  MatrixView<Val>& c)
{
   // Square matices.
   assert(a.rows() == a.columns());
   // Matices have same dimensions.
   assert(a.rows() == b.rows() && a.columns() == b.columns());
   assert(a.rows() == c.rows() && a.columns() == c.columns());
   // Dimensions must be power of two.
   assert(isPow2(a.rows()));

   using MV = MatrixView<Val>;
   const auto n = a.rows();

   // Base case - 1x1 matrices.
   if (n == 1)
   {
      c(0, 0) += a(0, 0) * b(0, 0);
      return c;
   }

   // Divide matrices into four submatrices.
   const auto mid = n / 2;

   const MV a11{a, 0, mid - 1, 0, mid - 1};
   const MV a12{a, 0, mid - 1, mid, n - 1};
   const MV a21{a, mid, n - 1, 0, mid - 1};
   const MV a22{a, mid, n - 1, mid, n - 1};

   const MV b11{b, 0, mid - 1, 0, mid - 1};
   const MV b12{b, 0, mid - 1, mid, n - 1};
   const MV b21{b, mid, n - 1, 0, mid - 1};
   const MV b22{b, mid, n - 1, mid, n - 1};

   MV c11{c, 0, mid - 1, 0, mid - 1};
   MV c12{c, 0, mid - 1, mid, n - 1};
   MV c21{c, mid, n - 1, 0, mid - 1};
   MV c22{c, mid, n - 1, mid, n - 1};

   // Create intermediate matrices.
   // The Strassen alg reduces the number of recursive multiplication calls (i.e. pruning
   // the recursion tree) by using these intermediate matrices.

   std::array<std::vector<Val>, 17> matPool;
   for (auto& mat : matPool)
      mat.resize(mid * mid, {});

   // Internal function to create a view of a matrix from the pool.
   auto makeView = [&matPool, mid](size_t matIdx)
   { return MV{matPool[matIdx].data(), mid, 0, mid - 1, 0, mid - 1}; };

   // Intermediate matrix sums.
   std::array<MV, 10> s{makeView(0), makeView(1), makeView(2), makeView(3), makeView(4),
                        makeView(5), makeView(6), makeView(7), makeView(8), makeView(9)};

   // Calculate intermediate sums.

   subtract(b12, b22, s[0]);
   add(a11, a12, s[1]);
   add(a21, a22, s[2]);
   subtract(b21, b11, s[3]);
   add(a11, a22, s[4]);
   add(b11, b22, s[5]);
   subtract(a12, a22, s[6]);
   add(b21, b22, s[7]);
   subtract(a11, a21, s[8]);
   add(b11, b12, s[9]);

   // Intermediate matrix products.
   std::array<MV, 7> p{makeView(10), makeView(11), makeView(12), makeView(13),
                       makeView(14), makeView(15), makeView(16)};

   // Calculate products through recursive Strassen multiplication.
   multiplyStrassen(a11, s[0], p[0]);
   multiplyStrassen(s[1], b22, p[1]);
   multiplyStrassen(s[2], b11, p[2]);
   multiplyStrassen(a22, s[3], p[3]);
   multiplyStrassen(s[4], s[5], p[4]);
   multiplyStrassen(s[6], s[7], p[5]);
   multiplyStrassen(s[8], s[9], p[6]);

   // Combine back into result matrix.
   add(p[4], p[3], c11);
   subtract(c11, p[1], c11);
   add(c11, p[5], c11);

   add(p[0], p[1], c12);

   add(p[2], p[3], c21);

   add(p[4], p[0], c22);
   subtract(c22, p[2], c22);
   subtract(c22, p[6], c22);

   return c;
}

template <typename Val>
MatrixView<Val>& multiply(const MatrixView<Val>& a, const MatrixView<Val>& b,
                          MatrixView<Val>& c)
{
   c.clear();
   // Use simple iterative method because the implemented Strassen method is limited to
   // square matrices with a dimension that is a power of two.
   return multiplyIterative(a, b, c);
}

} // namespace ds
