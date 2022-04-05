#include <catch2/catch.hpp>

#include "frn/corr.h"
#include "frn/shr.h"
#include "frn/util.h"

using namespace frn;

TEST_CASE("Dummy correlation") {
  unsigned n = 10;
  unsigned d = (n - 1) / 3;
  frn::lib::secret_sharing::Replicator<Field> replicator(n, d);
  std::vector<Correlator> correlators;
  for (std::size_t i = 0; i < n; i++) {
    // All correlators use the same replicator for simplicity
    correlators.emplace_back(Correlator(i, replicator));
  }

  // Check shares of zero
  std::vector<ZeroShare> zShares;
  for (std::size_t i=0; i<n; i++) {
    zShares.emplace_back(correlators[i].GenZeroShareDummy());
  }

  // Check additive share of zero
  Field recAdd(0);
  for (std::size_t i = 0; i < 2 * d + 1; i++) {
    recAdd += zShares[i].add_share;
  }
  REQUIRE(recAdd == Field(0));

  // Check replicated shares of the additive shares
  std::vector<std::vector<Shr>> zRepAddShares;
  zRepAddShares.resize(2 * d + 1);
  for (std::size_t i = 0; i < 2 * d + 1; i++) {
    for (std::size_t j = 0; j < n; j++) {
      zRepAddShares[i].emplace_back((zShares[j].rep_add_shares)[i]);
    }
    REQUIRE(replicator.ErrorDetection(zRepAddShares[i]) ==
            zShares[i].add_share);
  }

  // Check replicated shares of random value
  std::vector<RandomShare> rShares;
  for (std::size_t i=0; i<n; i++) {
    rShares.emplace_back(correlators[i].GenRandomShareDummy());
  }

  std::vector<Shr> rRepShares;
  for (std::size_t i=0; i<n; i++) {
      rRepShares.emplace_back(rShares[i].rep_share);
    }

  // Check replicated shares are consistent
  Field rValue = replicator.ErrorDetection(rRepShares);

  // Check additive shares reconstruct to the same secret
  Field rAdditive(0);
  for (std::size_t i = 0; i < 2 * d + 1; i++) {
    recAdd += rShares[i].add_share;
  }
  REQUIRE(rAdditive == rValue);

  // Check all rep shares of the additive shares work
  std::vector<std::vector<Shr>> rRepAddShares;
  rRepAddShares.resize(2 * d + 1);
  for (std::size_t i = 0; i < 2 * d + 1; i++) {
    for (std::size_t j = 0; j < n; j++) {
      rRepAddShares[i].emplace_back((rShares[j].rep_add_shares)[i]);
    }
    REQUIRE(replicator.ErrorDetection(rRepAddShares[i]) ==
            rShares[i].add_share);
  }
}


TEST_CASE("Real random correlation") {
  // Here we exploit the fact that all the PRGs are initialized to
  // default, which corresponds to keys of 0s, which, coincidentally,
  // constitute consistent shares.

  unsigned n = 10;
  unsigned d = (n-1)/3;
  frn::lib::secret_sharing::Replicator<Field> replicator(n, d);
  std::vector<Correlator> correlators;
  for (std::size_t i=0; i<n; i++) {
    // All correlators use the same replicator for simplicity
    correlators.emplace_back(Correlator(i, replicator));
  };

  // Check replicated shares of random value
  std::vector<RandomShare> rShares;
  for (std::size_t i=0; i<n; i++) {
    rShares.emplace_back(correlators[i].GenRandomShare());
  }

  std::vector<Shr> rRepShares;
  for (std::size_t i=0; i<n; i++) {
      rRepShares.emplace_back(rShares[i].rep_share);
    }

  // Check replicated shares are consistent
  Field rValue = replicator.ErrorDetection(rRepShares);

  // Check additive shares reconstruct to the same secret
  Field rAdditive(0);
  for (std::size_t i=0; i<2*d+1; i++) {
    rAdditive += rShares[i].add_share;
  }
  REQUIRE(rAdditive == rValue);

  // Check all rep shares of the additive shares work
  std::vector<std::vector<Shr>> rRepAddShares;
  rRepAddShares.resize(2*d+1);
  for (std::size_t i=0; i<2*d+1; i++) {
    for (std::size_t j=0; j<n; j++) {
      rRepAddShares[i].emplace_back((rShares[j].rep_add_shares)[i]);
    }
    REQUIRE(replicator.ErrorDetection(rRepAddShares[i]) == rShares[i].add_share);
  }
}
