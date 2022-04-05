#ifndef _FRN_LIB_MATH_MATRIX_H
#define _FRN_LIB_MATH_MATRIX_H

#include <iomanip>
#include <sstream>

#include "frn/lib/math/ring.h"
#include "frn/lib/math/vec.h"
#include "frn/lib/tools.h"

namespace frn::lib {
namespace math {

/**
 * @brief A two dimensional matrix over a ring.
 *
 * @tparam T the type of the ring.
 */
template <typename T, VALID_IF_RING(T)>
class Matrix {
 public:
  /**
   * @brief The type of the ring that this matrix is defined over.
   */
  using ValueType = T;

  /**
   * @brief Construct an nrows-by-ncols matrix using T's default constructor.
   *
   * @param nrows the number of rows.
   * @param ncols the number of columns.
   *
   * @throws std::invalid_argument if nrows or ncols is 0.
   */
  Matrix(std::size_t nrows, std::size_t ncols) {
    if (!(nrows && ncols))
      throw std::invalid_argument("nrows or ncols cannot be 0.");

    mRows = nrows;
    mCols = ncols;
    mValues = std::vector<T>(mRows * mCols);
  };

  /**
   * @brief The number of rows in this matrix.
   */
  std::size_t RowCount() const { return mRows; };

  /**
   * @brief The number of columns in this matrix.
   */
  std::size_t ColumnCount() const { return mCols; };

  /**
   * @brief Element access.
   *
   * @param r the row.
   * @param c the column.
   * @returns The element at the r'th row and c'th column.
   *
   * @remark This method does not perform any kind of bounds check.
   */
  T &operator()(std::size_t r, std::size_t c) {
    return mValues[mCols * r + c];
  };

  /**
   * @brief Element access.
   *
   * @param r the row.
   * @param c the column.
   * @returns The element at the r'th row and c'th column.
   *
   * @remark This method does not perform any kind of bounds check.
   */
  T operator()(std::size_t r, std::size_t c) const {
    return mValues[mCols * r + c];
  };

  /**
   * @brief Extract a submatrix.
   *
   * Returns a matrix M' of size \f$(i_1 - i_0)\times(j_1 - j_0)\f$.
   *
   * @param row_beg row offset begin.
   * @param row_end row offset end.
   * @param col_beg column offset begin.
   * @param col_end column offset end.
   *
   * @throws std::logic_error offsets are invalid or if the requested
   * submatrix is larger than the matrix.
   */
  Matrix extract_submatrix(std::size_t row_beg, std::size_t row_end,
                           std::size_t col_beg, std::size_t col_end) {
    if (row_beg > row_end || row_end - row_beg > RowCount())
      throw std::logic_error("invalid row offsets");

    if (col_beg > col_end || col_end - col_beg > ColumnCount())
      throw std::logic_error("invalid column offsets");

    auto nrows = row_end - row_beg;
    auto ncols = col_end - col_beg;
    Matrix sub(nrows, ncols);
    for (std::size_t i = 0; i < nrows; ++i) {
      for (std::size_t j = 0; j < ncols; ++j) {
        sub(i, j) = this->operator()(row_beg + i, col_beg + j);
      }
    }
    return sub;
  };

  /**
   * @brief Adds the content of one matrix to another.
   *
   * @param other the other matrix \f$Y\f$.
   * @return This matrix \f$X\f$ updated such that \f$X_{i,j} = X_{i,j} +
   * Y_{i,j}\f$.
   *
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix &AddInto(const Matrix &other) {
    AssertCompatible(other);
    vector::AddInto(mValues, other.mValues);
    return *this;
  };

  /**
   * @brief Adds the content of this matrix to another.
   *
   * @param other the other matrix \f$Y\f$.
   * @return A matrix \f$Z\f$ updated such that \f$Z_{i,j} = X_{i,j} +
   * Y_{i,j}\f$.
   *
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix Add(const Matrix &other) {
    AssertCompatible(other);
    return Matrix(vector::Add(mValues, other.mValues), mRows, mCols);
  };

  /**
   * @brief Subtract the content of one matrix from another.
   *
   * @param other the other matrix \f$Y\f$.
   * @return This matrix \f$X\f$ updated such that \f$X_{i,j} = X_{i,j} -
   * Y_{i,j}\f$.
   *
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix &SubtractInto(const Matrix &other) {
    AssertCompatible(other);
    vector::SubtractInto(mValues, other.mValues);
    return *this;
  };

  /**
   * @brief Subtract the content of one vector from another.
   * @param other the right vector \f$Y\f$.
   * @return A matrix \f$Z\f$ updated such that \f$Z_{i,j} = X_{i,j} -
   * Y_{i,j}\f$.
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix Subtract(const Matrix &other) {
    AssertCompatible(other);
    return Matrix(vector::Subtract(mValues, other.mValues), mRows, mCols);
  }

  /**
   * @brief Multiply the content of one matrix onto another.
   *
   * @param other the other matrix \f$Y\f$.
   * @return This matrix \f$X\f$ updated such that \f$X_{i,j} = X_{i,j} \cdot
   * Y_{i,j}\f$.
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix &MultiplyInto(const Matrix &other) {
    AssertCompatible(other);
    vector::MultiplyInto(mValues, other.mValues);
    return *this;
  };

  /**
   * @brief Multiply the content of one vector onto another.
   * @param other the right vector \f$Y\f$.
   * @return A matrix \f$Z\f$ updated such that \f$Z_{i,j} = X_{i,j} \cdot
   * Y_{i,j}\f$.
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix Multiply(const Matrix &other) {
    AssertCompatible(other);
    return Matrix(vector::Multiply(mValues, other.mValues), mRows, mCols);
  };

  /**
   * @brief Scales this matrix.
   *
   * @param scalar the scalar \f$s\f$.
   * @return This matrix \f$X\f$ updated such that \f$X_{i,j} = s\cdot
   * X_{i,j}\f$.
   */
  Matrix &ScaleBy(const T &scalar) {
    vector::ScaleBy(mValues, scalar);
    return *this;
  };

  /**
   * @brief Scales a matrix.
   *
   * @param scalar the scalar \f$s\f$.
   * @return A matrix \f$Z\f$ updated such that \f$Z_{i,j} = s\cdot X_{i,j}\f$.
   *
   * @throws std::logic_error if \f$X\f$ and \f$Y\f$ does not have the same
   * dimensions.
   */
  Matrix Scale(const T &scalar) {
    return Matrix(vector::Scale(mValues, scalar), mRows, mCols);
  };

  /**
   * @brief Performs a matrix multiplication between matrices.
   *
   * @param other the right hand side, a P x M matrix.
   *
   * @throws std::invalid_argument if ncols of left does not equal nrows of
   * right.
   */
  Matrix MatMul(const Matrix<T> &other) {
    if (ColumnCount() != other.RowCount())
      throw std::invalid_argument(
          "cannot multiply N x P, Q x M matrices when P != Q.");

    const auto n = RowCount();
    const auto p = ColumnCount();
    const auto m = other.ColumnCount();

    Matrix result(n, m);

    for (std::size_t i = 0; i < n; i++)
      for (std::size_t k = 0; k < p; k++)
        for (std::size_t j = 0; j < m; j++)
          result(i, j) += operator()(i, k) * other(k, j);

    return result;
  };

  /**
   * @brief Convert this matrix into something nice and readable.
   * @return a string representation of this matrix
   */
  std::string ToString() const {
    const auto n = ColumnCount();
    const auto m = RowCount();

    // convert all elements to strings and find the widest string in each
    // column.
    std::vector<std::string> items;
    items.reserve(n * m);
    std::vector<std::size_t> fills;
    fills.reserve(m);
    for (std::size_t j = 0; j < m; ++j) {
      auto first = operator()(0, j).ToString();
      auto fill = first.size();
      items.push_back(first);
      for (std::size_t i = 1; i < n; ++i) {
        auto next = operator()(i, j).ToString();
        auto next_fill = next.size();
        if (next_fill > fill) fill = next_fill;
        items.push_back(next);
      }
      // add a bit of extra padding
      fills.push_back(fill + 1);
    }

    std::stringstream ss;
    ss << "\n";
    for (std::size_t i = 0; i < n; ++i) {
      ss << "[";
      for (std::size_t j = 0; j < m; ++j) {
        ss << std::setfill(' ') << std::setw(fills[j]) << items[j * n + i]
           << " ";
      }
      ss << "]";
      if (i < n - 1) ss << "\n";
    }

    return ss.str();
  }

 private:
  Matrix(std::vector<T> values, std::size_t nrows, std::size_t ncols)
      : mRows(nrows), mCols(ncols), mValues(values){};

  void AssertCompatible(const Matrix &other) {
    if (RowCount() != other.RowCount() || ColumnCount() != other.ColumnCount())
      throw std::logic_error("input dimensions mismatch");
  };

  std::size_t mRows;
  std::size_t mCols;
  std::vector<T> mValues;
};

/**
 * @brief Creates a Vandermonde matrix.
 *
 * Creates a <code>alphas.size() * ncols</code> Vandemonde matrix. I.e., a
 * matrix A where A(i,j) = alphas[i]^j.
 *
 * A Vandermonde matrix is particular useful: Given a Vandermonde matrix
 * \f$A\f$ with alphas \f$\vec{\alpha}\f$ and a vector \f$x\f$, the product
 * \f$A\cdot x\f$ corresponds to evaluating the polynomial with coefficients
 * \f$x\f$ on each of the elements in \f$\vec{\alpha}\f$.
 *
 * @param alphas the alphas to use.
 * @param ncols the number of columns.
 * @return a new vandermonde matrix.
 */
template <typename T>
Matrix<T> create_vandermonde_matrix(const std::vector<T> &alphas,
                                    std::size_t ncols) {
  std::size_t nrows = alphas.size();
  Matrix<T> vand(nrows, ncols);
  for (std::size_t i = 0; i < nrows; ++i) {
    auto alpha = alphas[i];
    auto x = T::kOne;
    for (std::size_t j = 0; j < ncols; ++j) {
      vand(i, j) = x;
      x *= alpha;
    }
  }
  return vand;
}

/**
 * @brief Creates a Vandermonde matrix.
 *
 * Creates a Vandermonde matrix where all alphas are a constant offset from
 * alpha.
 *
 * @param base the base for the alphas.
 * @param nrows the number of rows.
 * @param ncols the number of columns.
 */
template <typename T>
Matrix<T> create_vandermonde_matrix(const T &base, std::size_t nrows,
                                    std::size_t ncols) {
  std::vector<T> alphas;
  alphas.reserve(nrows);
  for (std::size_t i = 0; i < nrows; ++i) alphas.emplace_back(base + T(i));
  return create_vandermonde_matrix(alphas, ncols);
}

}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_MATRIX_H
