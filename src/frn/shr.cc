#include "frn/shr.h"

frn::Shr frn::ShrManipulator::Add(const frn::Shr& a, const frn::Shr& b) {
  Shr r;
  r.reserve(a.size());
  for (std::size_t i = 0; i < a.size(); i++) {
    r.emplace_back(a[i] + b[i]);
  }
  return r;
}

frn::Shr frn::ShrManipulator::AddConstant(const frn::Shr& a,
                                          const frn::Field& c) {
  if (mIndexForConstantOps == -1) return a;
  Shr r;
  r = a;
  r[mIndexForConstantOps] += c;
  return r;
}

frn::Shr frn::ShrManipulator::Subtract(const frn::Shr& a, const frn::Shr& b) {
  Shr r;
  r.reserve(a.size());
  for (std::size_t i = 0; i < a.size(); i++) {
    r.emplace_back(a[i] - b[i]);
  }
  return r;
}

frn::Shr frn::ShrManipulator::SubtractConstant(const frn::Shr& a,
                                               const frn::Field& c) {
  if (mIndexForConstantOps == -1) return a;
  Shr r;
  r = a;
  r[mIndexForConstantOps] -= c;
  return r;
}

frn::Shr frn::ShrManipulator::SubtractConstant(const frn::Field& c,
                                               const frn::Shr& a) {
  Shr r;
  r.reserve(a.size());
  for (const auto& s : a) r.emplace_back(-s);
  if (mIndexForConstantOps == -1) return r;
  r[mIndexForConstantOps] += c;
  return r;
}

frn::Shr frn::ShrManipulator::MultiplyConstant(const frn::Shr& a,
                                               const frn::Field& c) {
  Shr r;
  r.reserve(a.size());
  for (const auto& s : a) r.emplace_back(c * s);
  return r;
}

frn::ShrD frn::ShrManipulator::MultiplyToDoubleDegree(const frn::Shr& a,
                                                      const frn::Shr& b) {
  std::vector<Field> c((int)mDoubleReplicator.ShareSize(), Field(0));

  for (const auto& triple : mTableMult) {
    int i = triple.src_a;
    int j = triple.src_b;
    int idx_in_double_rep_share = triple.dest_c;
    c[idx_in_double_rep_share] += a[i] * b[j];
  }
  return c;
}

frn::Field frn::ShrManipulator::MultiplyToAdditive(const frn::Shr& a,
                                                   const frn::Shr& b) {
  Field c(0);

  for (const auto& tuple : mTableMult) {
    int i = tuple.src_a;
    int j = tuple.src_b;
    if (mPartyId == tuple.first_party) c += a[i] * b[j];
  }
  return c;
}

#define INDEX_SHARE_FOR_CNST 0

int frn::ShrManipulator::IndexForConstantOperations() {
  auto id = mPartyId;
  auto set = mReplicator.IndexSetFor(id);

  // Check if the special index is in set

  auto it = find(set.begin(), set.end(), INDEX_SHARE_FOR_CNST);
  int idx = -1;
  // If element was found
  if (it != set.end()) {
    // calculating the desired index
    idx = it - set.begin();
  }
  return idx;
}

void frn::ShrManipulator::Init() {
  // Precompute mTableMult

  for (unsigned a = 0; a < mReplicator.ShareSize(); ++a) {
    for (unsigned b = 0; b < mReplicator.ShareSize(); ++b) {
      // We convert the input indexes from local to global
      int a_ = mReplicator.IndexSetFor(mPartyId)[a];
      int b_ = mReplicator.IndexSetFor(mPartyId)[b];

      // We fetch the corresponding input sets
      std::vector<int> SetA = mReplicator.Combination(a_);
      std::vector<int> SetB = mReplicator.Combination(b_);

      // Compute the intersection
      std::vector<int> intersection;
      frn::lib::secret_sharing::Intersection(SetA, SetB,
                                        [&intersection, SetA](unsigned i) {
                                          intersection.emplace_back(SetA[i]);
                                        });

      // Take the first n-2d elements of the intersection
      intersection.resize(mParties - 2 * mThreshold);

      // Get the index of this set with the replicator of double degree
      int target_set = mDoubleReplicator.RevComb(intersection);

      // Check if the current party owns this additive share
      auto index_set = mDoubleReplicator.IndexSetFor(mPartyId);
      auto it = find(index_set.begin(), index_set.end(), target_set);
      int idx = -1;
      if (it != index_set.end()) {
        // calculating the desired index
        idx = it - index_set.begin();
        if (idx != -1)
          mTableMult.emplace_back(
              MultEntry{a, b, (unsigned)idx, (unsigned)intersection[0]});
      }
    }
  }

  // precompute mTableRec
  // We use the double-replicator since this will be used to reconstruct a degree-2d sharing
  RecEntry entry;

  for (unsigned shr_id = 0; shr_id < mDoubleReplicator.ShareSize(); shr_id++) {
    // We convert the input index from local to global
    int shr_id_ = mDoubleReplicator.IndexSetFor(mPartyId)[shr_id];

    // We fetch the corresponding set of parties
    std::vector<int> Set = mDoubleReplicator.Combination(shr_id_);

    // We let party_set to be these parties NOT in Set
    for (int party_id = 0; (unsigned)party_id < mParties; party_id++) {
      auto it = find(Set.begin(), Set.end(), party_id);
      // If party_id is NOT in Set
      if (it == Set.end()) {
	entry.party_set.emplace_back(party_id);
      } 
    }

    // We determine if we send full value or hash .This is done by
    // checking if the given party is the first in the set
    if (mPartyId == (unsigned)Set[0]) {
      entry.value_or_hash = VALUE;
    } else {
      entry.value_or_hash = HASH;
    }
    mTableRec.emplace_back(entry);
  }
}

int frn::ShrManipulator::ComputeIndexForDoubleMultiplication(std::size_t a,
                                                             std::size_t b) {
  // We convert the input indexes from local to global
  int a_ = mReplicator.IndexSetFor(mPartyId)[a];
  int b_ = mReplicator.IndexSetFor(mPartyId)[b];

  // We fetch the corresponding input sets
  std::vector<int> SetA = mReplicator.Combination(a_);
  std::vector<int> SetB = mReplicator.Combination(b_);

  // Compute the intersection
  std::vector<int> intersection;
  frn::lib::secret_sharing::Intersection(SetA, SetB, [&intersection, SetA](int i) {
    intersection.emplace_back(SetA[i]);
  });

  // Take the first n-2d elements of the intersection
  intersection.resize(mParties - 2 * mThreshold);

  // Get the index of this set with the replicator of double degree
  int target_set = mDoubleReplicator.RevComb(intersection);

  // Check if the current party owns this additive share
  auto index_set = mDoubleReplicator.IndexSetFor(mPartyId);
  auto it = find(index_set.begin(), index_set.end(), target_set);
  int idx;

  // If element was found
  if (it != index_set.end()) {
    // calculating the desired index
    idx = it - index_set.begin();
  } else {
    // If the element is not
    // present in the vector
    idx = -1;
  }
  return idx;
}
