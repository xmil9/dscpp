//
// Mar-2023, Michael Lindner
// MIT license
//
#include "LinearAlgebraTests.h"
#include "LinearAlgebra.h"
#include "TestUtil.h"
#include <array>
#include <vector>

using namespace ds;


namespace
{
///////////////////

void TestMatrixAddition()
{
   {
      const std::string caseLabel{
         "add(MatrixView, MatrixView, MatrixView&) for 2x3 matrix slices"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 4, 0, 1, 1, 3};
      const MatrixView<double> vb{m.data(), 4, 1, 2, 0, 2};

      std::array<double, 6> sum;
      MatrixView vsum{sum.data(), 3, 0, 1, 0, 2};
      add(va, vb, vsum);

      // clang-format off
      const std::array<double, 6> expected{
         7., 9., 11.,
         15., 17., 19
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vsum.rows(); ++r)
         for (size_t c = 0; c < vsum.columns(); ++c)
            VERIFY(vsum(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void TestMatrixSubtraction()
{
   {
      const std::string caseLabel{
         "subtract(MatrixView, MatrixView, MatrixView&) for 2x2 matrix slices"};

      // clang-format off
      std::array<double, 12> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 4, 0, 1, 0, 1};
      const MatrixView<double> vb{m.data(), 4, 1, 2, 2, 3};

      std::array<double, 4> diff;
      MatrixView vdiff{diff.data(), 2, 0, 1, 0, 1};
      subtract(va, vb, vdiff);

      // clang-format off
      const std::array<double, 4> expected{
         -6., -6.,
         -6., -6.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vdiff.rows(); ++r)
         for (size_t c = 0; c < vdiff.columns(); ++c)
            VERIFY(vdiff(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void TestMatrixMultiplyIterative()
{
   {
      const std::string caseLabel{
         "multiplyIterative(MatrixView, MatrixView, MatrixView&) for 3x3 matrix slices"};

      // clang-format off
      std::array<double, 16> m{
         1., 2., 3., 4.,
         5., 6., 7., 8.,
         9., 10., 11., 12.,
         13., 14., 15., 16.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 4, 0, 2, 1, 3};
      const MatrixView<double> vb{m.data(), 4, 1, 3, 0, 2};

      std::array<double, 9> prod;
      MatrixView vprod{prod.data(), 3, 0, 2, 0, 2};
      multiplyIterative(va, vb, vprod);

      // clang-format off
      const std::array<double, 9> expected{
         89., 98., 107.,
         197., 218., 239.,
         305., 338., 371.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vprod.rows(); ++r)
         for (size_t c = 0; c < vprod.columns(); ++c)
            VERIFY(vprod(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void TestMatrixMultiplyRecursive()
{
   {
      const std::string caseLabel{
         "multiplyRecursive(MatrixView, MatrixView, MatrixView&) for 4x4 matrix slices"};

      // clang-format off
      std::array<double, 20> m{
         1., 2., 3., 4., 5.,
         6., 7., 8., 9., 10.,
         10., 11., 12., 1., 2.,
         3., 4., 5., 6., 7.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 5, 0, 3, 0, 3};
      const MatrixView<double> vb{m.data(), 5, 0, 3, 1, 4};

      std::array<double, 16> prod;
      for (auto& v : prod)
         v = 0.;
      MatrixView vprod{prod.data(), 4, 0, 3, 0, 3};
      multiplyRecursive(va, vb, vprod);

      // clang-format off
      const decltype(prod) expected{
         65., 75., 49., 59.,
         185., 215., 149., 179.,
         233., 267., 157., 191.,
         113., 131., 89., 107.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vprod.rows(); ++r)
         for (size_t c = 0; c < vprod.columns(); ++c)
            VERIFY(vprod(r, c) == expected[expectedIdx++], caseLabel);
   }
}

void TestMatrixMultiplyStrassen()
{
   {
      const std::string caseLabel{
         "multiplyStrassen(MatrixView, MatrixView, MatrixView&) for 1x1 matrix slices"};

      // clang-format off
      std::array<double, 20> m{
         1., 2., 3., 4., 5.,
         6., 7., 8., 9., 10.,
         10., 11., 12., 1., 2.,
         3., 4., 5., 6., 7.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 5, 0, 0, 2, 2};
      const MatrixView<double> vb{m.data(), 5, 3, 3, 1, 1};

      std::array<double, 1> prod;
      for (auto& v : prod)
         v = 0.;
      MatrixView vprod{prod.data(), 1, 0, 0, 0, 0};
      multiplyStrassen(va, vb, vprod);

      // clang-format off
      const decltype(prod) expected{
         12.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vprod.rows(); ++r)
         for (size_t c = 0; c < vprod.columns(); ++c)
            VERIFY(vprod(r, c) == expected[expectedIdx++], caseLabel);
   }
   {
      const std::string caseLabel{
         "multiplyStrassen(MatrixView, MatrixView, MatrixView&) for 2x2 matrix slices"};

      // clang-format off
      std::array<double, 20> m{
         1., 2., 3., 4., 5.,
         6., 7., 8., 9., 10.,
         10., 11., 12., 1., 2.,
         3., 4., 5., 6., 7.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 5, 1, 2, 2, 3};
      const MatrixView<double> vb{m.data(), 5, 2, 3, 3, 4};

      std::array<double, 4> prod;
      for (auto& v : prod)
         v = 0.;
      MatrixView vprod{prod.data(), 2, 0, 1, 0, 1};
      multiplyStrassen(va, vb, vprod);

      // clang-format off
      const decltype(prod) expected{
         62., 79.,
         18., 31.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vprod.rows(); ++r)
         for (size_t c = 0; c < vprod.columns(); ++c)
            VERIFY(vprod(r, c) == expected[expectedIdx++], caseLabel);
   }
   {
      const std::string caseLabel{
         "multiplyStrassen(MatrixView, MatrixView, MatrixView&) for 4x4 matrix slices"};

      // clang-format off
      std::array<double, 20> m{
         1., 2., 3., 4., 5.,
         6., 7., 8., 9., 10.,
         10., 11., 12., 1., 2.,
         3., 4., 5., 6., 7.
      };
      // clang-format on

      const MatrixView<double> va{m.data(), 5, 0, 3, 0, 3};
      const MatrixView<double> vb{m.data(), 5, 0, 3, 1, 4};

      std::array<double, 16> prod;
      for (auto& v : prod)
         v = 0.;
      MatrixView vprod{prod.data(), 4, 0, 3, 0, 3};
      multiplyStrassen(va, vb, vprod);

      // clang-format off
      const decltype(prod) expected{
         65., 75., 49., 59.,
         185., 215., 149., 179.,
         233., 267., 157., 191.,
         113., 131., 89., 107.
      };
      // clang-format on

      size_t expectedIdx = 0;
      for (size_t r = 0; r < vprod.rows(); ++r)
         for (size_t c = 0; c < vprod.columns(); ++c)
            VERIFY(vprod(r, c) == expected[expectedIdx++], caseLabel);
   }
}

///////////////////

} // namespace


///////////////////

void TestLinearAlgebra()
{
   TestMatrixAddition();
   TestMatrixSubtraction();
   TestMatrixMultiplyIterative();
   TestMatrixMultiplyRecursive();
   TestMatrixMultiplyStrassen();
}
