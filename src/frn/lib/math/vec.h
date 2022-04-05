#ifndef _FRN_LIB_MATH_VECTOR_H
#define _FRN_LIB_MATH_VECTOR_H

#include <cstring>
#include <stdexcept>
#include <vector>

#include "frn/lib/math/ring.h"

namespace frn::lib {
namespace math {
namespace vector {

/**
 * @brief Computes the inner product between two vectors.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return \f$\sum_ix_i\cdot y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, T> Dot(const std::vector<T> &left, const std::vector<T> &right) {
  auto n = left.size();
  if (n != right.size())
    throw std::logic_error("cannot Dot vectors with different sizes");

  auto lit = left.begin();
  auto rit = right.begin();

  T result;

  while (n-- > 0) result += *(lit++) * *(rit++);

  return result;
}

/**
 * @brief Adds the content of one vector to another.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return the vector \f$x\f$ updated such that \f$x_i = x_i + y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T> &> AddInto(std::vector<T> &left,
                                      const std::vector<T> &right) {
  auto n = left.size();
  if (n != right.size())
    throw std::logic_error("addition of vectors with different sizes");

  auto lit = left.begin();
  auto rit = right.begin();

  while (n-- > 0) *(lit++) += *(rit++);

  return left;
}

/**
 * @brief Adds two vectors and returns the result.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return a vector \f$z\f$ such that \f$z_i = x_i + y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T>> Add(const std::vector<T> &left,
                                const std::vector<T> &right) {
  std::vector<T> temp(left);
  return AddInto(temp, right);
}

/**
 * @brief Subtracts the content of one vector from another.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return the vector \f$x\f$ updated such that \f$x_i = x_i - y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T> &> SubtractInto(std::vector<T> &left,
                                           const std::vector<T> &right) {
  auto n = left.size();
  if (n != right.size())
    throw std::logic_error("subtraction of vectors with different sizes");

  auto lit = left.begin();
  auto rit = right.begin();

  while (n-- > 0) *(lit++) -= *(rit++);

  return left;
}

/**
 * @brief Subtract two vectors and return the result.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return a vector \f$z\f$ such that \f$z_i = x_i - y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T>> Subtract(const std::vector<T> &left,
                                     const std::vector<T> &right) {
  std::vector<T> temp(left);
  return SubtractInto(temp, right);
}

/**
 * @brief Multiply the content of one vector unto another.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return the vector \f$x\f$ updated such that \f$x_i = x_i \cdot y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T> &> MultiplyInto(std::vector<T> &left,
                                           const std::vector<T> &right) {
  auto n = left.size();
  if (n != right.size())
    throw std::logic_error(
        "entry-wise multiplication of vectors with different sizes");

  auto lit = left.begin();
  auto rit = right.begin();

  while (n-- > 0) *(lit++) *= *(rit++);

  return left;
}

/**
 * @brief Multiply two vectors and return the result.
 *
 * @param left the left vector \f$x\f$.
 * @param right the right vector \f$y\f$.
 * @return a vector \f$z\f$ such that \f$z_i = x_i \cdot y_i\f$.
 *
 * @throws std::logic_error if \f$x\f$ and \f$y\f$ are not the same length.
 */
template <typename T>
RingType<T, std::vector<T>> Multiply(const std::vector<T> &left,
                                     const std::vector<T> &right) {
  std::vector<T> temp(left);
  return MultiplyInto(temp, right);
}

/**
 * @brief Scales a vector by a constant.
 *
 * @param vector the vector \f$v\f$.
 * @param scalar the scalar \f$x\f$.
 * @return the vector \f$v\f$ updated such that \f$v_i = x\cdot v_i\f$.
 */
template <typename T>
RingType<T, std::vector<T> &> ScaleBy(std::vector<T> &vector, const T &scalar) {
  auto n = vector.size();
  auto it = vector.begin();

  while (n-- > 0) *(it++) *= scalar;

  return vector;
}

/**
 * @brief Scales a vector by a constant.
 *
 * @param vector the vector \f$v\f$.
 * @param scalar the scalar \f$x\f$.
 * @return a vector \f$z\f$ updated such that \f$z_i = x\cdot v_i\f$.
 */
template <typename T>
RingType<T, std::vector<T>> Scale(const std::vector<T> &vector,
                                  const T &scalar) {
  std::vector<T> temp(vector);
  return ScaleBy(temp, scalar);
}

/**
 * @brief Writes the content of a vector to a buffer.
 *
 * @param buffer the desination buffer.
 * @param vector the vector.
 *
 * @remark assumes that <code>buffer</code> points to enough space to hold the
 * content of vector.
 */
template <typename T>
RingType<T, void> ToBytes(unsigned char *buffer, const std::vector<T> &vector) {
  std::memcpy(buffer, vector.data(), vector.size() * T::ByteSize());
}

/**
 * @brief Reads a vector from a buffer.
 *
 * @param buffer the buffer with the serialized vector.
 * @param size the number of elements to read.
 * @return the deserialized vector.
 */
template <typename T>
RingType<T, std::vector<T>> FromBytes(const unsigned char *buffer,
                                      std::size_t size) {
  std::vector<T> items;
  items.reserve(size);
  for (std::size_t i = 0; i < size; i++)
    items.push_back(T::FromBytes(buffer + i * T::ByteSize()));

  return items;
}

/**
 * @brief Compute the sum over a vector of values.
 *
 * @param values the vector \f$V\f$ to sum over.
 * @return \f$\sum V_i\f$.
 */
template <typename T>
RingType<T, T> Sum(const std::vector<T> &values) {
  T result;
  for (const auto &v : values) result += v;
  return result;
}

}  // namespace vector
}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_VECTOR_H
