//
// Mar-2023, Michael Lindner
// MIT license
//
#pragma once
#include "MathAlg.h"
#include <array>
#include <cassert>
#include <string>
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

   std::string toString() const;

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

template <typename Val> std::string MatrixView<Val>::toString() const
{
   std::string s;

   const size_t numRows = rows();
   const size_t numCols = columns();

   for (size_t r = 0; r < numRows; ++r)
   {
      for (size_t c = 0; c < numCols; ++c)
      {
         s += std::to_string(m_data[index(r, c)]);
         if (c < numCols - 1)
            s += " ";
      }

      s+= "\n";
   }

   return s;
}

} // namespace ds
