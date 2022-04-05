#ifndef CORR_H
#define CORR_H

#include <memory>

#include "frn/shr.h"

namespace frn {

struct ZeroShare {
  // Additive share. Parties above P_2d+1 get zero
  Field add_share;
  // Replicated shares of each additive share
  std::vector<Shr> rep_add_shares;
};

struct RandomShare {
  // Replicated share in [r]_d
  Shr rep_share;
  // Additive share in <r>_2d. Parties above P_2d+1 get zero
  Field add_share;
  // Replicated shares of each additive share
  std::vector<Shr> rep_add_shares;
};

/**
 * @brief A class to produce and store correlated randomness
 */
class Correlator {
 public:
  /**
   * @brief Create a new Correlator instance
   * @param id the ID of this party
   * @param replicator the "global" replicator to use
   */
  Correlator(unsigned id,
             frn::lib::secret_sharing::Replicator<Field> const& replicator)
      : mReplicator(replicator),
        mId(id),
        mThreshold(replicator.Threshold()),
        mSize(replicator.Size()),
        mOwnPRGs(mReplicator.AdditiveShareSize()),
        mRandPRGs(2 * mThreshold + 1) {
    Init();
  };

  /**
   * Returns additive shares among parties P_1...P_2d+1 of 0, together
   * with replicated shares of each additive share
   */
  ZeroShare GenZeroShare();

  /**
   * As above but all shares set to zero
   */
  ZeroShare GenZeroShareDummy();

  /**
   * Returns additive shares among parties P_1...P_2d+1 of 0, together
   * with replicated shares of each additive share
   */
  RandomShare GenRandomShare();

  /**
   * As above but all shares set to zero
   */
  RandomShare GenRandomShareDummy();

  // Setters for the PRGs

  void SetOwnPRGs(std::vector<frn::lib::primitives::PRG> PRGs) { mOwnPRGs = PRGs; };
  void SetRandPRGs(std::vector<frn::lib::primitives::PRG> PRGs, unsigned idx) {
    mRandPRGs[idx] = PRGs;
  };

 private:
  void Init();

  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  unsigned mId;
  std::size_t mThreshold;
  std::size_t mSize;

  // PRGs used for own additive shares
  // These are seeded with the additive shares sent by Pi
  // len = AdditiveShareSize
  std::vector<frn::lib::primitives::PRG> mOwnPRGs;
  // PRGs used for random shares. Each vector of PRGs is seeded with
  // the replicated shares of the key that each Pj distributed len =
  // 2*d+1
  std::vector<std::vector<frn::lib::primitives::PRG>> mRandPRGs;
  // PRGs used for zero shares
  std::vector<frn::lib::primitives::PRG> mZeroPRGs;
};

}  // namespace frn

#endif  // CORR_H
