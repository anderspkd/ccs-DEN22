#ifndef _FRN_LIB_MATH_ARITHMETIC_H
#define _FRN_LIB_MATH_ARITHMETIC_H

#include <stdexcept>
#include <type_traits>

#include "frn/lib/tools.h"

namespace frn::lib {
namespace math {
namespace details {

/**
 * @brief Equality of arguments of integral type.
 */
template <typename T, VALID_IF(std::is_integral_v<T>)>
inline bool eq(const T &x, const T &y) {
  return x == y;
}

/**
 * @brief reduction modulo n.
 */
template <typename T, VALID_IF(std::is_integral_v<T>)>
inline T red(const T &x, const T &n) {
  return x % n;
}

/**
 * @brief \f$x + y \mod n\f$.
 */
template <typename T, VALID_IF(std::is_integral_v<T>)>
inline T addm(const T &x, const T &y, const T &n) {
  auto const z = x + y;
  return z >= n ? z - n : z;
}

/**
 * @brief \f$x - y \mod n\f$.
 */
template <typename T, VALID_IF(std::is_integral_v<T>)>
inline T subm(const T &x, const T &y, const T &n) {
  auto const x_ = x < y ? x + n : x;
  return x_ - y;
}

/**
 * @brief \f$-v \mod n\f$.
 */
template <typename T, VALID_IF(std::is_integral_v<T>)>
inline T neg(const T &v, const T &n) {
  return v ? n - v : v;
}

/**
 * @brief Finds \f$t\f$ such that \f$v \cdot t \equiv 1 \mod n\f$.
 *
 * @see
 * https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm#Modular_integers
 *
 * @throws std::logic_error if input is 0 or if the value was not invertible.
 */
template <typename T, typename S, VALID_IF(std::is_integral_v<T>)>
inline T invp(const T &v, const T &n) {
#define PAR_ASSIGN(v1, v2, q) \
  do {                            \
    const auto __temp = v2;       \
    v2 = v1 - q * __temp;         \
    v1 = __temp;                  \
  } while (0)

  if (v == 0) throw std::logic_error("0 is not invertible mod p.");

  S t = 0;
  S newt = 1;
  S r = n;
  S newr = v;

  while (newr != 0) {
    const S q = r / newr;
    PAR_ASSIGN(t, newt, q);
    PAR_ASSIGN(r, newr, q);
  }

  // LCOV_EXCL_START
  if (r != 1) {
    throw std::logic_error("non-invertible non-zero element encountered.");
  }
  // LCOV_EXCL_STOP

#undef PAR_ASSIGN

  if (t < 0) t = t + n;

  return static_cast<T>(t);
}

/**
 * @brief Finds \f$t\f$ such that \f$v \cdot t \equiv 1 \mod 2^{K}\f$.
 *
 * @see https://marc-b-reynolds.github.io/math/2017/09/18/ModInverse.html
 *
 * @throws std::logic_error if input was even (i.e., not invertible).
 */
template <std::size_t K, typename T, VALID_IF(K <= 128)>
inline T inv2(const T &v) {
  if (!(v & 1)) {
    throw std::logic_error("even numbers are not invertible mod 2^K.");
  }

  static constexpr T two = static_cast<T>(2);
  static constexpr T three = static_cast<T>(3);

  std::size_t bit_counter = 5;

  auto z = ((v * three) ^ 2);
  while (bit_counter <= K) {
    z *= two - v * z;
    bit_counter *= 2;
  }

  return z;
}

template <std::size_t K, typename T, VALID_IF(K > 128)>
inline T inv2(const T &v) {
  (void)v;
  throw std::runtime_error("unsupported operation.");
}

/**
 * @brief \f$x \cdot y \mod n\f$.
 */
template <typename T>
T mulm(const T &x, const T &y, const T &n);

}  // namespace details
}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_ARITHMETIC_H
