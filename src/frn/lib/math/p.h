#ifndef _FRN_LIB_MATH_PRIMES_H
#define _FRN_LIB_MATH_PRIMES_H

#include <cstdint>
#include <type_traits>

#include "frn/lib/math/arithmetic.h"

namespace frn::lib {
namespace math {

/**
 * @brief An abstract prime definition.
 *
 * Prime encapsulates a collection of static methods which together defines the
 * operations on integers modulo some prime number. I.e., a Finite Field of
 * prime order.
 *
 * @tparam Definition a class defining the prime number.
 * @tparam U the type used to represent elements of the field.
 * @tparam S same as U, but signed.
 */
template <typename Details, typename U, typename S>
struct Prime {
  /**
   * @brief Perform a reduction modulo the prime.
   *
   * @param x the value to reduce.
   */
  static U Reduce(const U &x) {
    return details::red(x, static_cast<U>(Details::kPrime));
  };

  /**
   * @brief Add two elements in the field.
   *
   * @param x the first value.
   * @param y the second value.
   */
  static U Add(const U &x, const U &y) {
    return details::addm(x, y, static_cast<U>(Details::kPrime));
  };

  /**
   * @brief Subtract two elements in the field.
   *
   * @param x the first value.
   * @param y the second value.
   */
  static U Subtract(const U &x, const U &y) {
    return details::subm(x, y, static_cast<U>(Details::kPrime));
  };

  /**
   * @brief Multiply two elements of in field.
   *
   * @param x the first value.
   * @param y the second value.
   */
  static U Multiply(const U &x, const U &y) {
    return details::mulm(x, y, static_cast<U>(Details::kPrime));
  }

  /**
   * @brief Find the additive inverse of an element in the field.
   *
   * @param v the value to negate.
   */
  static U Negate(const U &v) {
    return details::neg(v, static_cast<U>(Details::kPrime));
  };

  /**
   * @brief Find multiplicative inverse of an element in the field.
   *
   * @param v the value to invert.
   *
   * @throws std::runtime_error if <code>v</code> is 0.
   */
  static U Invert(const U &v) {
    return details::invp<U, S>(v, static_cast<U>(Details::kPrime));
  };

  /**
   * @brief Compare two elements of the field.
   *
   * @param x the first value.
   * @param y the second value.
   */
  static bool Equal(const U &x, const U &y) { return details::eq(x, y); };
};

/**
 * @brief 61-bit Mersenne Prime \f$p=2^{61}-1\f$.
 */
struct Mp61 : public Prime<Mp61, std::uint64_t, std::int64_t> {
  /**
   * @brief A Mp61 element is an unsigned long.
   */
  using ValueType = std::uint64_t;

  /**
   * @brief The prime.
   */
  static constexpr ValueType kPrime = 0x1FFFFFFFFFFFFFFF;
};

/**
 * @brief 127-bit Mersenne Prime \f$p=2^{127}-1\f$.
 */
struct Mp127 : public Prime<Mp127, __uint128_t, __int128_t> {
  /**
   * @brief A Mp127 element is an unsigned 128-bit integer.
   *
   * @remark probably only works on GCC and a 64 bit machine.
   */
  using ValueType = __uint128_t;

  /**
   * @brief The prime.
   */
  static constexpr ValueType kPrime =
      (((ValueType)0x7FFFFFFFFFFFFFFF) << 64) | 0xFFFFFFFFFFFFFFFF;
};

}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_PRIMES_H
