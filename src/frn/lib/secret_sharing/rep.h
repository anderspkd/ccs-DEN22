#ifndef _FRN_LIB_SECRET_SHARING_REPLICATED_H
#define _FRN_LIB_SECRET_SHARING_REPLICATED_H

#include <functional>
#include <stdexcept>
#include <map>

#include "frn/lib/math/vec.h"
#include "frn/lib/math/mat.h"
#include "frn/lib/primitives/prg.h"
#include "frn/lib/secret_sharing/additive.h"

namespace frn::lib {
namespace secret_sharing {

/**
 * @brief m-choose-k.
 */
constexpr std::size_t Binom(const std::size_t m, const std::size_t k) {
  std::size_t end = k > m - k ? m - k : k;
  std::size_t top = 1;
  std::size_t bot = 1;
  for (std::size_t i = 1; i <= end; ++i) {
    top *= (m + 1 - i);
    bot *= i;
  }
  return top / bot;
}

/**
 * @brief Computes the next lexicographic combination.
 *
 * @tparam Set the type of the set in which combinations are stored.
 * @param c the current combination.
 * @param m the size of the set to pick from.
 * @param k the number of elements to pick.
 * @return true if a next combination existed and false otherwise.
 */
template <typename Set>
constexpr bool NextCombination(Set &c, int m, int k) {
  for (int i = k - 1; i >= 0; --i) {
    if (c[i] < m - k + i) {
      c[i]++;
      for (int j = i + 1; j < k; ++j) c[j] = c[j - 1] + 1;
      return true;
    }
  }
  return false;
}

/**
 * @brief Return the n'th m-choose-k combination lexicographically.
 *
 * @tparam Set the type of the set in which combinations are stored.
 * @param c the place to store the n'th combination.
 * @param n the index of the combination.
 * @param m the size of the set to pick from.
 * @remark c also determines the number of elements to pick.
 */
template <typename Set>
constexpr void NthCombination(Set &c, int n, int m) {
  std::size_t k = c.size();
  for (std::size_t i = 0; i < k; ++i) c[i] = i;

  while (n-- > 0 && NextCombination(c, m, k))
    ;
}

/**
 * @brief Compute the intersection of two index sets.
 *
 * Given two sets \f$X\f$ and \f$Y\f$, this function finds the indices of the
 * elements of \f$X\f$ that also exists in \f$Y\f$. Note that this means the
 * function is not symmetric.
 *
 * For example, on input the two lists \f$X=(1,2,3)\f$ and \f$Y=(3,4,5)\f$ this
 * function returns \f$(2)\f$ if called with \f$X\f$ as the first argument, and
 * \f$(0)\f$ if called with \f$Y\f$ as the first argument.
 *
 * @tparam Set the type of the set. Must support iteration.
 * @param set the set.
 * @param other the other set.
 * @param cb callback which is called with indices of elements in
 *        <code>set</code> which also appear in <code>other</code>.
 */
template <typename Set>
void Intersection(const Set &set, const Set &other,
                  std::function<void(int)> cb) {
  auto first1 = set.begin();
  auto last1 = set.end();
  auto first2 = other.begin();
  auto last2 = other.end();
  int i = 0;

  // https://en.cppreference.com/w/cpp/algorithm/set_intersection with a twist.
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2) {
      ++first1;
      ++i;
    } else {
      if (!(*first2 < *first1)) {
        cb(i);
      }
      ++first2;
    }
  }
}

/**
 * @brief Compute the difference between two sets.
 *
 * Similar to <code>intersection</code>. Returns the indices of the first
 * argument which does not appear in the second.
 *
 * @tparam Set the type of the set. Must support iteration.
 * @param set the set.
 * @param other the other set.
 * @param cb callback which is called with indices of elements in
 *        <code>set</code> which that do not appear in <code>other</code>.
 */
template <typename Set>
void Difference(const Set &set, const Set &other, std::function<void(int)> cb) {
  auto first1 = set.begin();
  auto last1 = set.end();
  auto first2 = other.begin();
  auto last2 = other.end();
  int i = 0;

  // https://en.cppreference.com/w/cpp/algorithm/set_difference with a twist.
  while (first1 != last1) {
    if (first2 == last2) {
      while (first1 != last1) {
        cb(i++);
        ++first1;
      }
      return;
    }

    if (*first1 < *first2) {
      cb(i++);
      ++first1;
    } else {
      if (!(*first2 < *first1)) {
        ++first1;
        ++i;
      }
      ++first2;
    }
  }
}

/**
 * @brief A factory for working with replicated shares.
 *
 * @tparam T the type of the ring over which shares are defined.
 */
template <typename T>
class Replicator {
 public:
  //! Type of an index set.
  using IndexSet = std::vector<int>;

  //! Type of a replicated share.
  using ShareType = std::vector<T>;

  //! Type of the elements inside each share.
  using ValueType = T;

  /**
   * @brief Create a new Replicator.
   * @param n the number of shares that can be created
   * @param t the privacy threshold
   */
  Replicator(std::size_t n, std::size_t t)
      : mSize(n),
        mThreshold(t),
        mShareSize(Binom(n - 1, t)),
        mAdditiveShareSize(Binom(n, t)) {
    if (mSize <= mThreshold)
      throw std::invalid_argument("privacy threshold cannot be larger than n");
    if (!mThreshold)
      throw std::invalid_argument("privacy threshold cannot be 0");
    Init();
  };

  // getters

  /**
   * @brief Returns the number of shares this replicator can create.
   * @return the number of shares supported.
   */
  std::size_t Size() const { return mSize; };

  /**
   * @brief Returns the privacy threshold of this replicator.
   * @return the privacy threshold.
   */
  std::size_t Threshold() const { return mThreshold; };

  /**
   * @brief Returns the total number of additive shares used when creating a
   * secret sharing.
   * @return the number of additive shares.
   */
  std::size_t AdditiveShareSize() const { return mAdditiveShareSize; };

  /**
   * @brief The size of an individual share.
   * @return the number of elements in an individual share.
   */
  std::size_t ShareSize() const { return mShareSize; };

  /**
   * @brief The size of a share in bytes.
   * @return the size of a share in bytes.
   */
  std::size_t ShareSizeBytes() const {
    return ShareSize() * ValueType::ByteSize();
  };

  /**
   * @brief Returns the combination corresponding to the given index
   *
   * @param idx the index to query
   * @return (sorted) combination corresponding to this index
   */
  std::vector<int> Combination(std::size_t idx) const { return mCombinations[idx]; };

  /**
   * @brief Returns the index corresponding to the given combination
   *
   * @param (sorted) combination to query
   * @return index corresponding to this combination
   */
  int RevComb(std::vector<int> combination) const { return mRevComb.at(combination); };

  /**
   * @brief Returns the index set for a particular replicated share.
   *
   * Each replicated share has an index set associated with it, telling us which
   * of the total shares are included in this particular share. This information
   * is useful, as it indirectly tells us what values a particular replicated
   * share is missing.
   *
   * @param id the replicated share index.
   * @return the index set for a replicated share.
   */
  IndexSet IndexSetFor(std::size_t id) const { return mLookup[id]; };

  /**
   * @brief Number of elements which differ between two shares.
   * @return Number of elements which differ between two shares.
   */
  std::size_t DifferenceSize() const { return mDifferenceSize; };

  // utility

  /**
   * @brief Read a single share from a byte pointer.
   * @param buffer a pointer to some bytes
   * @return a replicated share.
   */
  ShareType ShareFromBytes(const unsigned char *buffer) const {
    return frn::lib::math::vector::FromBytes<ValueType>(buffer, ShareSize());
  };

  /**
   * @brief Read a collection of shares from a list of bytes.
   * @param buffer a pointer to some bytes
   * @param amount how many shares to read
   * @return a collection of replicated shares.
   */
  std::vector<ShareType> SharesFromBytes(const unsigned char *buffer,
                                         std::size_t amount) const {
    std::vector<ShareType> shares;
    shares.reserve(amount);
    std::size_t offset = 0;
    while (amount-- > 0) {
      shares.emplace_back(ShareFromBytes(buffer + offset));
      offset += ShareSizeBytes();
    }
    return shares;
  }

  /**
   * @brief Convert a replicated share to bytes.
   * @param share the replicated share to serialize
   * @param buffer where to store the serialized share
   */
  void ShareToBytes(const ShareType &share, unsigned char *buffer) const {
    frn::lib::math::vector::ToBytes(buffer, share);
  };

  /**
   * @brief Serialize a collection of replicated shares.
   * @param shares the shares to serialize
   * @param buffer where to store the serialized shares
   */
  void SharesToBytes(const std::vector<ShareType> &shares,
                     unsigned char *buffer) const {
    std::size_t offset = 0;
    for (const auto &share : shares) {
      ShareToBytes(share, buffer + offset);
      offset += ShareSizeBytes();
    }
  }

  /**
   * @brief Create a replicated sharing of a secret.
   * @param secret the secret to share
   * @param prg where to get random data from
   * @return a list of shares.
   */
  std::vector<ShareType> Share(const ValueType &secret,
                               frn::lib::primitives::PRG &prg) const;

  /**
   * @brief Create a list of replicated shares.
   * @param secrets the secrets to share
   * @param prg where to get random data
   * @return a collection of shares.
   */
  std::vector<std::vector<ShareType>> Share(
      const std::vector<ValueType> &secrets, frn::lib::primitives::PRG &prg) const {
    std::vector<std::vector<ShareType>> all_shares;
    all_shares.resize(Size());
    bool first = true;
    const auto n = secrets.size();
    for (const auto &secret : secrets) {
      std::vector<ShareType> shares = Share(secret, prg);
      std::size_t id = 0;
      for (const auto &share : shares) {
        if (first) all_shares[id].reserve(n);
        all_shares[id++].emplace_back(share);
      }
      first = false;
    }
    return all_shares;
  };

  /**
   * @brief Reconstructs secret from a list of replicated shares,
   * assuming shares are consistent
   *
   * @param list of replicated shares
   * @return secret
   */
  ValueType Reconstruct(const std::vector<ShareType> &shares) const {
    std::vector<std::vector<ValueType>> redundant = ComputeRedundantAddShares(shares);
    ValueType secret(0);
    std::vector<ValueType> additive_shares;
    additive_shares.reserve(mAdditiveShareSize);

    for (std::size_t i = 0; i < mAdditiveShareSize; ++i) {
      secret += redundant[i][0];
    }
    return secret;
  };

  /**
   * @brief Reconstructs secret from a list of replicated shares,
   * aborting if shares are inconsistent. Requires d<n/2.
   *
   * @param list of replicated shares
   * @return secret
   */
  ValueType ErrorDetection(const std::vector<ShareType> &shares) const {
    std::vector<std::vector<ValueType>> redundant = ComputeRedundantAddShares(shares);
    ValueType secret(0);
    std::vector<ValueType> additive_shares;
    additive_shares.reserve(mAdditiveShareSize);

    ValueType comparison;
    for (std::size_t i = 0; i < mAdditiveShareSize; ++i) {
      // Check that all received shares are equal
      comparison = redundant[i][0];
      for (auto elt : redundant[i]){
	if (elt != comparison) throw std::runtime_error("Inconsistent shares");
      }
      secret += comparison;
    }
    return secret;
  };

  /**
   * @brief Reconstructs secret from a list of replicated shares,
   * aborting if shares are inconsistent. Requires d<n/3.
   *
   * @param list of replicated shares
   * @return secret
   */
  ValueType ErrorCorrection(const std::vector<ShareType> &shares) const;

 private:
  void Init();

  /**
   * @brief Helper that takes a vector of shares and returns an array
   * containing the copies of each additive share. Used for
   * reconstruction
   */
  std::vector<std::vector<ValueType>> ComputeRedundantAddShares(const std::vector<ShareType> &shares) const {
  std::vector<std::vector<ValueType>> redundant;
  redundant.resize(mAdditiveShareSize);
  for (std::size_t i = 0; i < mAdditiveShareSize; ++i) {
    redundant[i].reserve(mSize - mThreshold);
  }

  for (std::size_t party_idx = 0; party_idx < mSize; ++party_idx) {
    for (std::size_t j = 0; j < mShareSize; ++j) {
      std::size_t shr_idx = mLookup[party_idx][j];
      redundant[shr_idx].emplace_back(shares[party_idx][j]);
    }
  }

  return redundant;
  };

  /**
   * @brief Number of shares.
   */
  std::size_t mSize;

  /**
   * @brief Privacy threshold.
   */
  std::size_t mThreshold;

  /**
   * @brief Size of an individual share.
   */
  std::size_t mShareSize;

  /**
   * @brief Total number of additivate shares used to create a secret sharing.
   */
  std::size_t mAdditiveShareSize;

  /**
   * @brief A precomputed table with all the combinations
   */
  std::vector<std::vector<int>> mCombinations;

  /**
   * @brief A map (dictionary) with the reverse of mCombinations, that
   * is, it finds the index corresponding to a given (ordered)
   * combination
   */
  std::map<std::vector<int>,int> mRevComb;

  /**
   * @brief A precomputed table with information about which share contains
   * which values.
   */
  std::vector<IndexSet> mLookup;

  /**
   * @brief Number of elements that one share has that another is missing.
   */
  std::size_t mDifferenceSize;
};

template <typename T>
void Replicator<T>::Init() {
  auto k = mSize - mThreshold;
  auto m = mSize;
  std::vector<int> combination(k);
  NthCombination<IndexSet>(combination, 0, m);
  mLookup.resize(mSize);
  for (std::size_t i = 0; i < mSize; ++i) {
    mLookup[i].reserve(mShareSize);
  }

  int share_idx = 0;
  mCombinations.resize(mAdditiveShareSize);
  do {
    // Fill in mCombinations
    mCombinations[share_idx] = combination;
    // Fill in mRevComb
    mRevComb.insert(std::pair<std::vector<int>, int>(combination, share_idx));
    for (auto party_idx : combination)
      mLookup[party_idx].emplace_back(share_idx);
    share_idx++;
  } while (NextCombination<IndexSet>(combination, m, k));

  std::size_t d = 0;
  Difference(mLookup[0], mLookup[1], [&d](int idx) {
    (void)idx;
    d++;
  });
  mDifferenceSize = d;
}

template <typename T>
auto Replicator<T>::Share(const ValueType &secret, primitives::PRG &prg) const
    -> std::vector<ShareType> {
  std::vector<ValueType> additive_shares =
      ShareAdditive(secret, mAdditiveShareSize, prg);
  std::vector<ShareType> shares;
  shares.reserve(mSize);
  for (std::size_t i = 0; i < mSize; ++i) {
    ShareType share;
    share.reserve(mShareSize);
    auto is = IndexSetFor(i);
    for (auto index : is) {
      share.emplace_back(additive_shares[index]);
    }
    shares.emplace_back(share);
  }
  return shares;
}

}  // namespace secret_sharing
}  // namespace frn::lib

#endif  // _FRN_LIB_SECRET_SHARING_REPLICATED_H
