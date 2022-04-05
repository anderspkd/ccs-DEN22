#include "frn/corr.h"

frn::ZeroShare frn::Correlator::GenZeroShareDummy() {
  ZeroShare output;

  // Set the additive share to 0
  output.add_share = Field(0);

  // Replicated share with zeros
  std::vector<Field> ZeroRepShare(mReplicator.ShareSize(), Field(0));

  // Fill in the vector of replicated shares with zero rep shares
  for (std::size_t i = 0; i < 2 * mThreshold + 1; i++) {
    output.rep_add_shares.emplace_back(ZeroRepShare);
  }

  return output;
}


frn::RandomShare frn::Correlator::GenRandomShareDummy() {
  RandomShare output;

  // Set the additive share to 0
  output.add_share = Field(0);

  // Replicated share with zeros
  std::vector<Field> ZeroRepShare(mReplicator.ShareSize(), Field(0));

  // Set the replicated share to 0
  output.rep_share = ZeroRepShare;

  // Fill in the vector of replicated shares with zero rep shares
  for (std::size_t i = 0; i < 2 * mThreshold + 1; i++) {
    output.rep_add_shares.emplace_back(ZeroRepShare);
  }
  return output;
}


frn::RandomShare frn::Correlator::GenRandomShare() {
  RandomShare output;
  auto element_size = Field::ByteSize();
  std::vector<unsigned char> buf(element_size);

  // Set the additive share
  output.add_share = Field(0);

  // Only parties in U have additive shares
  if (mId < 2*mThreshold+1) {
    // Get the additive share by adding the PRGs obtained when Pi
    // shared its own key
    for (unsigned i=0; i < mReplicator.AdditiveShareSize(); i++){
      mOwnPRGs[i].Next(buf);
      output.add_share += Field::FromBytes(buf.data());
    };
  };

  // Set the replicated share of each additive share
  // and of the secret
  output.rep_share.resize(mReplicator.ShareSize());
  output.rep_add_shares.resize(2*mThreshold+1);

  for (unsigned shr_idx = 0; shr_idx < mReplicator.ShareSize(); shr_idx++) {
    output.rep_share[shr_idx] = Field(0);

    for (unsigned idx_in_U = 0; idx_in_U < 2*mThreshold+1; idx_in_U++) {
      mRandPRGs[idx_in_U][shr_idx].Next(buf);
      output.rep_add_shares[idx_in_U].emplace_back(Field::FromBytes(buf.data()));
      output.rep_share[shr_idx] += output.rep_add_shares[idx_in_U][shr_idx];
    }
  }

  return output;
}


void frn::Correlator::Init() {
  // Initializes the internal PRGs to default
  std::vector<frn::lib::primitives::PRG> PRGs(mReplicator.ShareSize());
  for (unsigned j=0; j < 2*mThreshold+1; j++) {
      SetRandPRGs(PRGs, j);
  };

}
