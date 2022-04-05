#ifndef _FRN_LIB_UTILS_TOOLS_H
#define _FRN_LIB_UTILS_TOOLS_H

#include <array>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

/**
 * @brief defines how many elements to print when converting a std::vector to a
 * string.
 */
#define VEC_MAX_PRINT_N 5

namespace frn::lib {
namespace utils {

/**
 * @brief Convenience macro to enable templated functions based on some
 * expression.
 */
#define VALID_IF(expr) std::enable_if_t<(expr), bool> = true

/**
 * @brief Convert a value to a <code>std::string</code> representation.
 *
 * @tparam T the type of the argument,
 *
 * @param value the value to convert to a string.
 */
template <typename T>
std::string to_string(const T &value);

/**
 * @brief Convert a list of elements to a string representation.
 */
template <typename It>
std::string to_string(It it, std::size_t size) {
  std::stringstream ss;
  auto const n = size >= VEC_MAX_PRINT_N ? VEC_MAX_PRINT_N : size;

  ss << "#[";
  for (std::size_t i = 0; i < n; ++i) {
    ss << (*it++);
    if (i < n - 1) ss << ", ";
  }
  if (size == VEC_MAX_PRINT_N + 1)
    ss << ", ... (1 more element)";
  else if (size > VEC_MAX_PRINT_N)
    ss << ", ... (" << (size - VEC_MAX_PRINT_N) << " more elements)";
  ss << "]";
  return ss.str();
}

/**
 * @brief std::vector to string.
 */
template <typename T>
std::string to_string(const std::vector<T> &vector) {
  return to_string<decltype(vector.begin())>(vector.begin(), vector.size());
}

/**
 * @brief std::array to string.
 */
template <typename T, std::size_t K>
std::string to_string(const std::array<T, K> &array) {
  return to_string<decltype(array.begin())>(array.begin(), K);
}

/**
 * @brief Convert an <code>std::string</code> to a value.
 *
 * @tparam T the type of the converted string.
 *
 * @param str the string, assumed to represent a value in base 10.
 */
template <typename T>
T from_string(const std::string &str) {
  T v = 0;
  for (std::size_t i = 0; i < str.size(); i++) {
    const char c = str[i];
    v *= 10;
    v += c - '0';
  }
  return v;
}

/**
 * @brief Convert an array of bytes to a hex string.
 *
 * @param bytes a byte pointer.
 * @param length the number of bytes to convert.
 */
std::string bytes2hex(const unsigned char *bytes, std::size_t length);

}  // namespace utils
}  // namespace frn::lib

#endif  // _FRN_LIB_UTILS_TOOLS_H
