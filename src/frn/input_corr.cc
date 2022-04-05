#include "frn/input_corr.h"
#include "frn/lib/secret_sharing/additive.h"

#include <algorithm>
#include <iostream>

frn::Field frn::GetRandomElement(frn::lib::primitives::PRG& prg) {
  unsigned char buffer[Field::ByteSize()] = {0};
  prg.Next(buffer, Field::ByteSize());
  return Field::FromBytes(buffer);
}

frn::lib::primitives::PRG frn::FieldElementToPrg(const frn::Field& element) {
  // Have to pick the larger of either the PRGs seed size, or the size of a
  // field element. We need to write a field element, but read a seed, so the
  // buffer have to accommodate both sizes. Extra zeros in the seed is fine.
  unsigned char buffer[std::max(frn::lib::primitives::PRG::SeedSize(),
                                       frn::Field::ByteSize())] = {0};
  element.ToBytes(buffer);
  return frn::lib::primitives::PRG(buffer);
}

frn::InputSetup::Correlator frn::InputSetup::Run() {
  auto k = GetRandomElement(mPrg);
  auto copy = mPrg;
  auto size = mNetwork->Size();
  auto shr_k = mReplicator.Share(k, mPrg);
  auto add_k = frn::lib::secret_sharing::ShareAdditive(k, size, copy);

  for (std::size_t i = 0; i < size; i++) {
    mNetwork->SendShares(i, {shr_k[i]});
  }

  std::vector<Shr> shares_k(size);
  for (std::size_t i = 0; i < size; i++) {
    shares_k[i] = mNetwork->RecvShares(i, 1)[0];
  }

  std::vector<frn::lib::primitives::PRG> my_prg;
  my_prg.reserve(add_k.size());
  for (const auto& s: add_k)
    my_prg.emplace_back(FieldElementToPrg(s));

  auto share_size = mReplicator.ShareSize();
  std::vector<std::vector<frn::lib::primitives::PRG>> other_prgs;
  other_prgs.reserve(size);
  for (std::size_t i = 0; i < size; i++) {
    std::vector<frn::lib::primitives::PRG> prgs_for_i;
    prgs_for_i.reserve(share_size);
    auto share_from_i = shares_k[i];
    for (const auto& v : share_from_i) {
      prgs_for_i.emplace_back(FieldElementToPrg(v));
    }
    other_prgs.emplace_back(prgs_for_i);
  }

  return Correlator(other_prgs, my_prg, share_size);
}
