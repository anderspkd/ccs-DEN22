#ifndef SHR_H
#define SHR_H

#include <vector>

#include "frn/lib/primitives/prg.h"
#include "frn/lib/secret_sharing/rep.h"
#include "frn/util.h"

namespace frn {

/**
 * Type of a replicated share. Identical to std::vector<Field>.
 */
using Shr = frn::lib::secret_sharing::Replicator<Field>::ShareType;

/**
 * Replicated share of double degree.
 */
using ShrD = frn::lib::secret_sharing::Replicator<Field>::ShareType;

// src_a, src_b: indexes of x and y
// dest_c: index of where to place the output
// first_party: first party in D_a cap D_b (used for check)
struct MultEntry {
  unsigned src_a;
  unsigned src_b;
  unsigned dest_c;
  unsigned first_party;
};

enum RecType {
  VALUE,
  HASH,
};

// Indicates the set of parties a given (additive) share must be
// sent to, and if the actual share must be sent, or a hash suffices
struct RecEntry {
  RecType value_or_hash;
  std::vector<unsigned> party_set;
};

/**
 * @brief Create a factory object capable of generating replicated shares.
 * @param n the number of parties to support
 * @return a replicator.
 */
inline frn::lib::secret_sharing::Replicator<Field> CreateReplicator(int n) {
  return frn::lib::secret_sharing::Replicator<Field>(n, (n - 1) / 3);
}

/**
 * @brief A class for performing arithmetic manipulations of shares locally.
 */
class ShrManipulator {
 public:
  /**
   * @brief Create a new manipulator for replicated shares.
   * @param id the ID of this party
   * @param d the threshold
   * @param n the number of parties
   */
  ShrManipulator(std::size_t id, std::size_t d, std::size_t n)
      : mPartyId(id),
        mParties(n),
        mThreshold(d),
        mReplicator(n, d),
        mDoubleReplicator(n, 2 * d),
        mIndexForConstantOps(IndexForConstantOperations()) {
    Init();
  }

  /**
   * @brief Add two shares.
   * @param a the first share
   * @param b the second share
   * @return a sharing of the sum of the inputs.
   */
  Shr Add(const Shr& a, const Shr& b);

  /**
   * @brief Add a constant to share.
   * @param a the share
   * @param c the constant
   * @return a share of a + c.
   */
  Shr AddConstant(const Shr& a, const Field& c);

  /**
   * @brief Add a constant to share.
   * @param a the share
   * @param c the constant
   * @return a share of a + c.
   */
  Shr AddConstant(const Field& c, const Shr& a) { return AddConstant(a, c); };

  /**
   * @brief Subtract two shares.
   * @param a the first share
   * @param b the second share
   * @return a sharing of the difference of the inputs.
   */
  Shr Subtract(const Shr& a, const Shr& b);

  /**
   * @brief Subtract a constant from a share.
   * @param a the share
   * @param c the constant
   * @return a sharing of a - c.
   */
  Shr SubtractConstant(const Shr& a, const Field& c);

  /**
   * @brief Subtract a constant from a share.
   * @param a the share
   * @param c the constant
   * @return a sharing of c - a.
   */
  Shr SubtractConstant(const Field& c, const Shr& a);

  /**
   * @brief Multiply a constant unto a share.
   * @param a the share
   * @param c the constant
   * @return a sharing of a * c.
   */
  Shr MultiplyConstant(const Shr& a, const Field& c);

  /**
   * @brief Multiply a constant unto a share.
   * @param a the share
   * @param c the constant
   * @return a sharing of a * c.
   */
  Shr MultiplyConstant(const Field& c, const Shr& a) {
    return MultiplyConstant(a, c);
  };

  /**
   * @brief Locally multiply two degree d shares and output a degree 2d share.
   * @param a the first share
   * @param b the second share
   * @return a degree 2d share of the product a * b.
   */
  ShrD MultiplyToDoubleDegree(const Shr& a, const Shr& b);

  /**
   * @brief Locally mulitply two degree d shares to obtain an additive share.
   * @param a the first share
   * @param b the second share
   * @return an additive share of the product a * b.
   */
  Field MultiplyToAdditive(const Shr& a, const Shr& b);

  /**
   * s whether the current party is among the first n-2d parties in the
   * intersection between the sets indexed by the inputs a and b, and if so, it
   * returns the index within this party's replicated share of this set of size
   * n-2d
   *
   * @param a local index of input
   * @param b local index of input
   * @return index of the set of size n-2d (within this party's replicated
   * share).
   */
  int ComputeIndexForDoubleMultiplication(std::size_t a, std::size_t b);

  std::vector<MultEntry> GetTableMult() { return mTableMult; }

  std::vector<RecEntry> GetTableRec() { return mTableRec; }

  frn::lib::secret_sharing::Replicator<Field> GetReplicator() {
    return mReplicator;
  }

  frn::lib::secret_sharing::Replicator<Field> GetDoubleReplicator() {
    return mDoubleReplicator;
  }

  /**
   * @brief Returns the size of a share.
   */
  std::size_t ShareSize() const { return mReplicator.ShareSize(); };

 private:
  void Init();

  /**
   * @brief Determines if this party should perform an action during operations
   * on constants.
   *
   * @return An index representing the share element to which a constant should
   * be added or subtracted. -1 is returned if this party does nothing during
   * constant operations.
   */
  int IndexForConstantOperations();

  std::size_t mPartyId;
  std::size_t mParties;
  std::size_t mThreshold;

  // Tables used by the party to determine which shares to multiply
  // (and for the case of 2d-shares, where to store them)
  std::vector<MultEntry> mTableMult;

  // Table used to determine which shares must be sent to which
  // parties when reconstructing, and when do we send full values or
  // hashes.
  std::vector<RecEntry> mTableRec;

  // Auxiliary replicators
  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  frn::lib::secret_sharing::Replicator<Field> mDoubleReplicator;

  int mIndexForConstantOps;
};

}  // namespace frn

#endif  // SHR_H
