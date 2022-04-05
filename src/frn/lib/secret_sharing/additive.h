#ifndef _FRN_LIB_SECRET_SHARING_ADDITIVE_H
#define _FRN_LIB_SECRET_SHARING_ADDITIVE_H

#include <stdexcept>
#include <vector>

#include "frn/lib/math/vec.h"
#include "frn/lib/math/mat.h"
#include "frn/lib/primitives/prg.h"

namespace frn::lib {
namespace secret_sharing {

/**
 * @brief Create an additive sharing.
 *
 * @param secret the secret \f$s\f$ to share.
 * @param n the number of shares to create.
 * @param prg the pseudorandom generator used to generate random shares.
 * @returns a \f$(n-1,n)\f$-sharing of \f$s\f$.
 */
template <typename T>
std::vector<T> ShareAdditive(const T &secret, std::size_t n,
                             primitives::PRG &prg) {
  if (!n)
    throw std::invalid_argument("cannot create additive sharing for 0 people");

  auto element_size = T::ByteSize();
  std::vector<T> shares(n);
  std::vector<unsigned char> buf(element_size * n);
  prg.Next(buf);

  for (std::size_t i = 0; i < n - 1; ++i) {
    const auto share = T::FromBytes(buf.data() + (i * element_size));
    shares[i] = share;
  }

  shares[n - 1] = secret - math::vector::Sum(shares);

  return shares;
}

}  // namespace secret_sharing
}  // namespace frn::lib

#endif  // _FRN_LIB_SECRET_SHARING_ADDITIVE_H
