//
// Mar-2023, Michael Lindner
// MIT license
//
#include "MatrixViewTests.h"
#include "MatrixView.h"
#include "TestUtil.h"
#include <array>
#include <vector>

using namespace ds;


namespace
{
///////////////////

void testMatrixViewCtorForMatrixStorage()
{
   {
      const std::string caseLabel{"MatrixView ctor for matrix storage for 2x3 matrix"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      const MatrixView<int> v{m.data(), 3, 0, 1, 0, 2};

      size_t mIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == m[mIdx++], caseLabel);
   }
   {
      const std::string caseLabel{
         "MatrixView ctor for matrix storage for 2x2 matrix slice"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      MatrixView<double> v{m.data(), 4, 0, 1, 1, 2};

      // clang-format off
      const std::array<double, 4> expected{
         2., 3.,
         6., 7.,
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void testMatrixViewCtorForBaseView()
{
   {
      const std::string caseLabel{"MatrixView ctor for base view for 2x1 matrix"};

      // clang-format off
      std::vector<int> m{
         7, 2, 3,
         5, 9, 6
      };
      // clang-format on

      const MatrixView<int> base{m.data(), 3, 0, 1, 0, 2};
      MatrixView<int> v{base, 0, 1, 2, 2};

      // clang-format off
      const std::array<double, 2> expected{
         3.,
         6.,
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == expected[expectedIdx++], caseLabel);
   }
   {
      const std::string caseLabel{"MatrixView ctor for base view for 2x3 matrix slice"};

      // clang-format off
      std::array<double, 16> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.,
         13., 14., 15., 16.
      };
      // clang-format on

      MatrixView<double> base{m.data(), 4, 1, 3, 0, 3};
      MatrixView<double> v{base, 1, 2, 0, 2};

      // clang-format off
      const std::array<double, 6> expected{
         9., 10., 11.,
         13., 14., 15.,
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void testMatrixViewRowsColumns()
{
   {
      const std::string caseLabel{
         "MatrixView::rows() and MatrixView::columns for 2x3 matrix"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      const MatrixView<int> v{m.data(), 3, 0, 1, 0, 2};

      VERIFY(v.rows() == 2, caseLabel);
      VERIFY(v.columns() == 3, caseLabel);
   }
   {
      const std::string caseLabel{
         "MatrixView::rows() and MatrixView::columns for 5x1 matrix"};

      std::vector<double> m{7., 2., 3., 5., 9.};
      const MatrixView<double> v{m.data(), 5, 0, 0, 0, 4};

      VERIFY(v.rows() == 1, caseLabel);
      VERIFY(v.columns() == 5, caseLabel);
   }
   {
      const std::string caseLabel{
         "MatrixView::rows() and MatrixView::columns for 1x5 matrix"};

      std::vector<double> m{7., 2., 3., 5., 9.};
      const MatrixView<double> v{m.data(), 1, 0, 4, 0, 0};

      VERIFY(v.rows() == 5, caseLabel);
      VERIFY(v.columns() == 1, caseLabel);
   }
   {
      const std::string caseLabel{
         "MatrixView::rows() and MatrixView::columns for 3x2 matrix slice"};

      // clang-format off
      std::vector<int> m{
         1, 2, 3,
         4, 5, 6,
         7, 8, 9,
         10, 11, 12,
         13, 14, 15
      };
      // clang-format on

      const MatrixView<int> v{m.data(), 3, 1, 3, 1, 2};

      VERIFY(v.rows() == 3, caseLabel);
      VERIFY(v.columns() == 2, caseLabel);
   }
   {
      const std::string caseLabel{
         "MatrixView::rows() and MatrixView::columns for 1x1 matrix slice"};

      // clang-format off
      std::vector<int> m{
         1, 2, 3,
         4, 5, 6,
         7, 8, 9,
         10, 11, 12,
         13, 14, 15
      };
      // clang-format on

      const MatrixView<int> v{m.data(), 3, 4, 4, 2, 2};

      VERIFY(v.rows() == 1, caseLabel);
      VERIFY(v.columns() == 1, caseLabel);
   }
}

void testMatrixViewClear()
{
   {
      const std::string caseLabel{"MatrixView::clear() for 2x3 matrix"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      MatrixView<int> v{m.data(), 3, 0, 1, 0, 2};

      v.clear();

      for (const auto& val : m)
         VERIFY(val == 0, caseLabel);
   }
   {
      const std::string caseLabel{"MatrixView::clear() for 2x2 matrix slice"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      MatrixView<double> v{m.data(), 4, 0, 1, 1, 2};

      v.clear();

      // clang-format off
      const std::array<double, 12> expected{
         1., 0., 0., 4.,
         5., 0., 0., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      VERIFY(m == expected, caseLabel);
   }
   {
      const std::string caseLabel{"MatrixView::clear() for 2x2 matrix slice at end"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      MatrixView<double> v{m.data(), 4, 1, 2, 2, 3};

      v.clear();

      // clang-format off
      const std::array<double, 12> expected{
         1., 2., 3., 4.,
         5., 6., 0., 0.,
         9., 10., 0., 0.
      };
      // clang-format on

      VERIFY(m == expected, caseLabel);
   }
}

void testMatrixView2DIndexOperator()
{
   {
      const std::string caseLabel{"MatrixView::operator(row, col) for 2x3 matrix"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      const MatrixView<int> v{m.data(), 3, 0, 1, 0, 2};

      size_t mIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == m[mIdx++], caseLabel);
   }
   {
      const std::string caseLabel{"MatrixView::operator(row, col) for 2x2 matrix slice"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      const MatrixView<double> v{m.data(), 4, 0, 1, 1, 2};

      // clang-format off
      const std::array<double, 4> expected{
         2., 3.,
         6., 7.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < v.rows(); ++r)
         for (size_t c = 0; c < v.columns(); ++c)
            VERIFY(v(r, c) == expected[expectedIdx++], caseLabel);
   }
   {
      const std::string caseLabel{"MatrixView::operator(row, col) for changing values"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      MatrixView<int> v{m.data(), 3, 1, 1, 1, 2};

      v(0, 1) = 100;
      VERIFY(v(0, 1) == 100, caseLabel);
      VERIFY(m[5] == 100, caseLabel);
   }
}

void testMatrixViewToString()
{
   {
      const std::string caseLabel{"MatrixView::toString()"};

      std::vector<int> m{7, 2, 3, 5, 9, 6};
      const MatrixView<int> v{m.data(), 3, 0, 1, 1, 2};

      const std::string s = v.toString();

      VERIFY(s == "2 3\n9 6\n", caseLabel);
   }
}

} // namespace


///////////////////

void testMatrixView()
{
   testMatrixViewCtorForMatrixStorage();
   testMatrixViewCtorForBaseView();
   testMatrixViewRowsColumns();
   testMatrixViewClear();
   testMatrixView2DIndexOperator();
   testMatrixViewToString();
}
