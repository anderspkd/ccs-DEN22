#include "frn/lib/math/arithmetic.h"

#include <cstdint>
#include <utility>

#include "frn/lib/tools.h"

using u64 = std::uint64_t;
using u128 = __uint128_t;

static inline u64 mod_mul_mersenne61(const u64& x, const u64& y, const u64& n) {
  u128 z = (u128)x * y;
  u64 a = z >> 61;
  u64 b = (u64)z;

  a |= b >> 61;
  b &= n;

  return frn::lib::math::details::addm(a, b, n);
}

struct u256 {
  u128 high;
  u128 low;
};

//  https://cp-algorithms.com/algebra/montgomery_multiplication.html
static inline u256 mul_u128(const u128 x, const u128 y) {
  u64 a = x >> 64, b = x;
  u64 c = y >> 64, d = y;
  // (a*2^64 + b) * (c*2^64 + d) =
  // (a*c) * 2^128 + (a*d + b*c)*2^64 + (b*d)
  u128 ac = (u128)a * c;
  u128 ad = (u128)a * d;
  u128 bc = (u128)b * c;
  u128 bd = (u128)b * d;

  u128 carry = (u128)(u64)ad + (u128)(u64)bc + (bd >> 64u);
  u128 high = ac + (ad >> 64u) + (bc >> 64u) + (carry >> 64u);
  u128 low = (ad << 64u) + (bc << 64u) + bd;

  return {high, low};
}

static inline u128 mod_mul_mersenne127(const u128& x, const u128& y,
                                       const u128& n) {
  u256 z = mul_u128(x, y);
  u128 a = z.high << 1;
  u128 b = z.low;

  a |= b >> 127;
  b &= n;

  u128 c = a + b;
  if (c >= n) c -= n;
  return c;
}

template <>
u64 frn::lib::math::details::mulm(const u64& x, const u64& y, const u64& n) {
  return mod_mul_mersenne61(x, y, n);
}

template <>
u128 frn::lib::math::details::mulm(const u128& x, const u128& y, const u128& n) {
  return mod_mul_mersenne127(x, y, n);
}
