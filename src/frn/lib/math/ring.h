#ifndef _FRN_LIB_MATH_RINGELEMENT_H
#define _FRN_LIB_MATH_RINGELEMENT_H

#include <type_traits>

namespace frn::lib {
namespace math {

/**
 * @brief An abstract RingElement.
 *
 * RingElement provides definitions of various operators for ring elements
 * through the Barton-Nackman idiom (and is similar in spirit to
 * boost::operators).
 *
 * Additionally, RingElement is used as a "tag" by various classes (e.g.,
 * <code>Matrix</code> only works if instantiated with a ring element).
 *
 * Children of RingElement must implement the methods and operators below:
 * - <code>+=</code>
 * - <code>-=</code>
 * - <code>*=</code>
 * - <code>/=</code>
 * - <code>negate</code>
 * - <code>equal</code>
 * - <code>not_equal</code>
 *
 * @tparam T the type of the derived class.
 */
template <typename T>
struct RingElement {
  /**
   * @brief Add two elements and return their sum.
   */
  friend T operator+(const T &lhs, const T &rhs) {
    T temp(lhs);
    return temp += rhs;
  };

  /**
   * @brief Subtract two elements and return their difference.
   */
  friend T operator-(const T &lhs, const T &rhs) {
    T temp(lhs);
    return temp -= rhs;
  };

  /**
   * @brief Return the negation of an element.
   */
  friend T operator-(const T &elem) {
    T temp(elem);
    return temp.Negate();
  };

  /**
   * @brief Multiply two elements and return their product.
   */
  friend T operator*(const T &lhs, const T &rhs) {
    T temp(lhs);
    return temp *= rhs;
  };

  /**
   * @brief Divide two elements and return their quotient.
   */
  friend T operator/(const T &lhs, const T &rhs) {
    T temp(lhs);
    return temp /= rhs;
  };

  /**
   * @brief Compare two elements for equality.
   */
  friend bool operator==(const T &lhs, const T &rhs) { return lhs.Equal(rhs); };

  /**
   * @brief Compare two elements for inequality.
   */
  friend bool operator!=(const T &lhs, const T &rhs) {
    return lhs.NotEqual(rhs);
  };
};

/**
 * @brief Use to ensure a template parameter is a ring.
 *
 * enable_if_ring will check if its first parameter is a ring (i.e., it inherits
 * from RingElement) and if so, set <code>type</code> to be V. Otherwise it
 * fails to compile.
 */
template <typename T, typename V>
struct EnableIfRing {
  //! type when T is a ring.
  using Type =
      typename std::enable_if<std::is_base_of<RingElement<T>, T>::value,
                              V>::type;
};

/**
 * @brief Helper macro. Compiles if type is inherits from RingElement.
 */
#define VALID_IF_RING(type) typename EnableIfRing<type, bool>::Type = true

/**
 * @brief Helper for is_ring.
 */
template <typename T, typename V>
using RingType = typename EnableIfRing<T, V>::Type;

}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_RINGELEMENT_H
