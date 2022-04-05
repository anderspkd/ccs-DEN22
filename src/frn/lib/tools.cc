#include "frn/lib/tools.h"

#include <iomanip>
#include <sstream>
#include <string>

//! frn::lib
namespace frn::lib {

// utils
namespace utils {

using std::string;
using std::stringstream;

/**
 * @brief Specialization of to_string for unsinged 64-bit ints.
 */
template <>
string to_string(const std::uint64_t& v) {
  stringstream ss;
  ss << v;
  return ss.str();
}

/**
 * @brief Specialization of to_string for unsigned 128-bit ints.
 */
template <>
string to_string(const __uint128_t& v) {
  if (!v) return "0";

  stringstream ss;
  __uint128_t w = v;
  while (w) {
    ss << ((int)(w % 10));
    w = w / 10;
  }
  const string number_str = ss.str();
  return string(number_str.rbegin(), number_str.rend());
}

/**
 * @brief Convert an array of bytes to a hex string.
 */
string bytes2hex(const unsigned char* bytes, size_t length) {
  stringstream ss;
  ss << std::hex << std::setfill('0') << std::uppercase;
  for (size_t i = 0; i < length; i++) {
    ss << std::setw(2) << (int)bytes[i];
    if (i < length - 1) ss << " ";
  }
  return ss.str();
}

}  // namespace utils

}  // namespace frn::lib
