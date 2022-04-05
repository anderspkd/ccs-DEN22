#ifndef _FRN_LIB_MATH_FPELEMENT_H
#define _FRN_LIB_MATH_FPELEMENT_H

#include <cstring>

#include "frn/lib/math/ring.h"
#include "frn/lib/tools.h"

namespace frn::lib {
namespace math {

/**
 * @brief Finite Field of prime order.
 *
 * FpElement defines a prime order finite field, that is \f$\mathbb{F}_{p}\f$
 * for a prime \f$p\f$. The prime is supplied through a template parameter and
 * currently supports either \f$p=2^{61}-1\f$ or \f$p=2^{127}-1\f$ providing
 * roughly 61 and 127 bits of computation, respectively.
 *
 * @tparam Prime the prime.
 *
 * @see Prime.hpp.
 */
template <typename Prime>
class FpElement : RingElement<FpElement<Prime>> {
 public:
  /**
   * @brief Numeric type of an element. (E.g., an unsigned 64-bit integer).
   */
  using ValueType = typename Prime::ValueType;

  /**
   * @brief Construct a new FpElement from a series of bytes.
   *
   * @tparam Prime definition of a prime.
   * @param buffer a byte pointer.
   *
   * @pre <code>buffer</code> must point to at least
   *   <code>FpElement::byte_size()</code> bytes of memory.
   */
  static FpElement FromBytes(const unsigned char *buffer) {
    return FpElement(*((ValueType *)buffer));
  };

  /**
   * @brief Create an FpElement element from a compile-time constant.
   *
   * @tparam Constant the constant.
   *
   * @pre <code>Constant</code> must be smaller than the prime.
   */
  template <std::size_t K>
  static constexpr FpElement FromConstant() {
    FpElement v;
    static_assert(K < Prime::kPrime, "constant must be less than prime.");
    v.mValue = K;
    return v;
  }

  /**
   * @brief Additive neural element.
   */
  static constexpr FpElement kZero = FpElement::FromConstant<0>();

  /**
   * @brief Multiplicative neutral element.
   */
  static constexpr FpElement kOne = FpElement::FromConstant<1>();

  /**
   * @brief Size of an FpElement element in bytes.
   */
  static constexpr std::size_t ByteSize() { return sizeof(ValueType); };

  /**
   * @brief Size of an FpElement element in bits.
   */
  static constexpr std::size_t BitSize() { return 8 * ByteSize(); };

  // LCOV_EXCL_START

  /**
   * @brief Construct a new FpElement element with value 0.
   */
  constexpr FpElement()
      : mValue(0){

        };

  // LCOV_EXCL_STOP

  /**
   * @brief Construct a new FpElement element from a string
   * <code>std::string</code>.
   *
   * @param value_string a base 10 string.
   */
  explicit FpElement(const std::string value_string) {
    auto const parsed_value = utils::from_string<ValueType>(value_string);
    mValue = Prime::Reduce(parsed_value);
  };

  /**
   * @brief Construct a new FpElement <code>ValueType</code> value.
   */
  explicit FpElement(const ValueType &value) { mValue = Prime::Reduce(value); };

  /**
   * @brief Set element to its additive negative.
   */
  FpElement &Negate() {
    mValue = Prime::Negate(mValue);
    return *this;
  };

  /**
   * @brief Set element to its modular inverse.
   *
   * @throws std::logic_error if this FpElement is 0.
   */
  FpElement &Invert() {
    mValue = Prime::Invert(mValue);
    return *this;
  };

  /**
   * @brief Return the modular inverse of this element.
   *
   * @throws std::logic_error if this FpElement is 0.
   */
  FpElement Inverse() const { return FpElement(Prime::Invert(mValue)); };

  /**
   * @brief Add another element to this.
   */
  FpElement &operator+=(const FpElement &other) {
    mValue = Prime::Add(mValue, other.mValue);
    return *this;
  };

  /**
   * @brief Subtract another element from this.
   */
  FpElement &operator-=(const FpElement &other) {
    mValue = Prime::Subtract(mValue, other.mValue);
    return *this;
  };

  /**
   * @brief Multiply another element onto this.
   */
  FpElement &operator*=(const FpElement &other) {
    mValue = Prime::Multiply(mValue, other.mValue);
    return *this;
  };

  /**
   * @brief Divide another element from this.
   *
   * @remark This does not define an arithmetic division, but rather, it is
   * (roughly) equivalent to <code>(*this) * other.invert()</code>.
   *
   * @throws std::logic_error if other is 0.
   */
  FpElement &operator/=(const FpElement &other) {
    mValue = Prime::Multiply(mValue, Prime::Invert(other.mValue));
    return *this;
  };

  /**
   * @brief compares the lower <code>bit_size()</code> bits of this and other
   * and returns true if they're equal.
   */
  bool Equal(const FpElement &other) const {
    return Prime::Equal(mValue, other.mValue);
  };

  /**
   * @brief compares the lower <code>bit_size()</code> bits of this and other
   * and returns false if they're equal.
   */
  bool NotEqual(const FpElement &other) const { return !Equal(other); };

  /**
   * @brief write this element into a buffer.
   *
   * @param dest the destination buffer.
   *
   * @pre <code>buffer</code> must point to <code>byte_size()</code> bytes of
   * free space.
   */
  void ToBytes(unsigned char *dest) const {
    std::memcpy(dest, &mValue, ByteSize());
  };

  /**
   * @brief << overload.
   */
  friend std::ostream &operator<<(std::ostream &os, const FpElement &element) {
    return os << element.ToString();
  };

  /**
   * @brief Returns a string representation of this element.
   */
  std::string ToString() const {
    return frn::lib::utils::to_string<ValueType>(mValue);
  };

 private:
  ValueType mValue;
};

template <typename Prime>
constexpr FpElement<Prime> FpElement<Prime>::kZero;
template <typename Prime>
constexpr FpElement<Prime> FpElement<Prime>::kOne;

}  // namespace math
}  // namespace frn::lib

#endif  // _FRN_LIB_MATH_FPELEMENT_H
