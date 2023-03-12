//
// Mar-2023, Michael Lindner
// MIT license
//
#pragma once
#include <array>
#include <cassert>
#include <vector>

namespace ds
{
///////////////////

// Light-weight matrix view.
template <typename Val> class MatrixView
{
 public:
   // Construct directly from some matrix storage. The row and column indices define the
   // slice of the view.
   MatrixView(Val* mat, size_t rowOffset, size_t rowStart, size_t rowEnd, size_t colStart,
              size_t colEnd) noexcept;
   // Construct from another view. The row and column indices are relative to the base
   // view.
   MatrixView(const MatrixView<Val>& base, size_t rowStartOff, size_t rowEndOff,
              size_t colStartOff, size_t colEndOff) noexcept;

   size_t rows() const noexcept { return m_rowEnd - m_rowStart + 1; }
   size_t columns() const noexcept { return m_colEnd - m_colStart + 1; }
   void clear();

   const Val& operator()(size_t r, size_t c) const { return m_data[index(r, c)]; }
   Val& operator()(size_t r, size_t c) { return m_data[index(r, c)]; }

 private:
   size_t index(size_t r, size_t c) const;

 private:
   // Pointer to matrix values. Assumed to be in continuous row-major memory.
   Val* m_data = nullptr;
   // Offset to get from one row to the next. Usually the number of columns of
   // the original matrix.
   size_t m_rowOffset = 0;
   // Start and end zero-based index (inclusive) of rows.
   size_t m_rowStart = 0;
   size_t m_rowEnd = 0;
   // Start and end zero-based index (inclusive) of columns.
   size_t m_colStart = 0;
   size_t m_colEnd = 0;
};

template <typename Val>
MatrixView<Val>::MatrixView(Val* mat, size_t rowOffset, size_t rowStart, size_t rowEnd,
                            size_t colStart, size_t colEnd) noexcept
: m_data{mat}, m_rowOffset{rowOffset}, m_rowStart{rowStart}, m_rowEnd{rowEnd},
  m_colStart{colStart}, m_colEnd{colEnd}
{
   assert(m_data);
   assert(m_rowStart <= m_rowEnd);
   assert(m_colStart <= m_colEnd);
}

template <typename Val>
MatrixView<Val>::MatrixView(const MatrixView<Val>& base, size_t fromRow, size_t toRow,
                            size_t fromCol, size_t toCol) noexcept
: m_data{base.m_data}, m_rowOffset{base.m_rowOffset},
  m_rowStart{base.m_rowStart + fromRow}, m_rowEnd{base.m_rowStart + toRow},
  m_colStart{base.m_colStart + fromCol}, m_colEnd{base.m_colStart + toCol}
{
   assert(m_data);
   assert(m_rowStart <= m_rowEnd);
   assert(m_colStart <= m_colEnd);
}

template <typename Val> void MatrixView<Val>::clear()
{
   const size_t numRows = rows();
   const size_t numCols = columns();

   for (size_t r = 0; r < numRows; ++r)
      for (size_t c = 0; c < numCols; ++c)
         m_data[index(r, c)] = {};
}

template <typename Val> size_t MatrixView<Val>::index(size_t r, size_t c) const
{
   assert(r < rows() && c < columns());
   return (m_rowStart + r) * m_rowOffset + m_colStart + c;
}

namespace internal
{
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
         for (size_t i = 0; i < numCols; ++i)
            c(i, j) += a(i, i) * b(i, j);
      }

   return c;
}

// Multiplies two matrices by partitioning each into four submatrices, recursively
// multiplying the submatrices, and combining them into the result.
// Cormen, pg 77
// Time: O(n^3)
template <typename Val>
MatrixView<Val>& multiplyRecursive(const MatrixView<Val>& a, const MatrixView<Val>& b,
                                   MatrixView<Val>& c)
{
   // Square matices.
   assert(a.rows() == a.columns());
   // Matices have same dimensions.
   assert(a.rows() == b.rows() && a.columns() == b.columns());
   assert(a.rows() == c.rows() && a.columns() == c.columns());

   using MV = MatrixView<Val>;
   const auto n = a.rows();

   // Base case - 1x1 matrices.
   if (n == 1)
   {
      c(0, 0) += a(0, 0) * b(0, 0);
      return;
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

   using MV = MatrixView<Val>;
   const auto n = a.rows();

   // Base case - 1x1 matrices.
   if (n == 1)
   {
      c(0, 0) += a(0, 0) * b(0, 0);
      return;
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

   auto makeView = [&matPool, mid](size_t matIdx)
   { return MV{matPool[matIdx].data(), mid, 0, mid - 1, 0, mid - 1}; };

   // Intermediate matrix sums.
   std::array<MV, 10> s;

   // Assign matrix storage to views.
   size_t poolIdx = 0;
   for (auto& view : s)
      view = makeView(poolIdx++);

   // Calculate intermediate sums.
   subtract(b12, b22, s[0]);
   add(a11, a12, s[1]);
   add(a21, a22, s[2]);
   subtract(b21, b11, s[3]);
   add(a11, a22, s[4]);
   add(b11, b22, s[5]);
   subtract(a12, a22, s[6]);
   add(b12, b22, s[7]);
   subtract(a11, a21, s[8]);
   add(b11, b12, s[9]);

   // Intermediate matrix products.
   std::array<MV, 7> p;

   // Assign matrix storage to views.
   for (auto& view : p)
      view = makeView(poolIdx++);

   // Calculate products through recursive Strassen multiplication.
   multiplyStrassen(a11, s[0], p[0]);
   multiplyStrassen(s[1], b22, p[1]);
   multiplyStrassen(s[2], b11, p[2]);
   multiplyStrassen(a22, s[3], p[3]);
   multiplyStrassen(s[4], s[5], p[4]);
   multiplyStrassen(s[6], s[7], p[5]);
   multiplyStrassen(s[8], s[9], p[6]);

   // Combine back into result matrix.
   add(p[1], p[5], c11);
   subtract(p[3], c11, c11);
   add(p[4], c11, c11);

   add(p[0], p[1], c12);

   add(p[2], p[3], c21);

   subract(p[2], p[6], c22);
   subtract(p[0], c22, c22);
   add(p[4], c22, c22);

   return c;
}

} // namespace internal

template <typename Val>
MatrixView<Val>& multiply(const MatrixView<Val>& a, const MatrixView<Val>& b,
                          MatrixView<Val>& c)
{
   c.clear();
   return multiplyStrassen(a, b, c);
}

} // namespace ds
