#include <catch2/catch.hpp>

#include "frn/input.h"
#include "frn/shr.h"
#include "frn/util.h"
#include "mock_network.h"
#include "tcp_network_helper.h"

void Prepare(std::shared_ptr<frn::MockNetwork> network, unsigned id, unsigned n,
             frn::lib::secret_sharing::Replicator<frn::Field> replicator) {
  frn::lib::primitives::PRG prg;
  for (std::size_t i = 0; i < n; i++) {
    if (i == id) continue;
    auto s = frn::GetRandomElement(prg);
    auto shr = replicator.Share(s, prg);
    network->SendSharesFrom(i, {shr[id]});
  }
}

TEST_CASE("input") {
  unsigned id = 1;
  unsigned n = 7;
  unsigned d = (n - 1) / 3;
  frn::lib::primitives::PRG prg;

  SECTION("send") {
    auto replicator = frn::CreateReplicator(n);
    auto network = frn::MockNetwork::Create(id, n);
    frn::InputSetup setup(network, replicator, prg);
    Prepare(network, id, n, replicator);
    auto corr = setup.Run();

    network->Clear();

    frn::Input input(network, frn::ShrManipulator(id, d, n), corr);
    frn::Field secret0(1234);
    frn::Field secret1(4443);
    input.Prepare(secret0);
    input.Prepare(secret1);
    auto shares = input.Run();
    REQUIRE(shares.size() == n);
    for (std::size_t i = 0; i < n; i++) {
      if (i == id)
        REQUIRE(shares[i].size() == 2);
      else
        REQUIRE(shares[i].size() == 0);
    }
  }
}

#define PRINT(s) \
  if (my_id == 0) std::cout << s << "\n"

TEST_CASE("input real") {
  unsigned n = 4;
  unsigned d = (n - 1) / 3;
  (void)d;

  CREATE_PARTIES(n, 12000);

  std::vector<frn::Shr> output_shares(n);
  frn::Field secret;

  BEGIN_PLAYER_DEF(3) {
    auto rep = frn::CreateReplicator(n);
    unsigned char seed[frn::lib::primitives::PRG::SeedSize()] = {0};
    seed[0] = (unsigned char)my_id;
    frn::lib::primitives::PRG prg (seed);
    frn::InputSetup setup(network, rep, prg);
    auto corr = setup.Run();
    frn::Input input(network, frn::ShrManipulator(my_id, d, n), corr);
    secret = frn::Field(123456);
    input.Prepare(secret);
    auto shares = input.Run();
    REQUIRE(shares.size() == n);
    REQUIRE(shares[3].size() == 1);
    output_shares[3] = shares[3][0];
  }
  END_PLAYER_DEF(3);

  for (std::size_t i = 0; i < n; i++) {
    if (i == 3) continue;
    BEGIN_PLAYER_DEF(i) {
      auto rep = frn::CreateReplicator(n);
      unsigned char seed[frn::lib::primitives::PRG::SeedSize()] = {0};
      seed[0] = (unsigned char)my_id;
      frn::lib::primitives::PRG prg (seed);
      frn::InputSetup setup(network, rep, prg);
      auto corr = setup.Run();
      frn::Input input(network, frn::ShrManipulator(my_id, d, n), corr);
      input.PrepareToReceive(3);
      auto shares = input.Run();
      REQUIRE(shares.size() == n);
      REQUIRE(shares[3].size() == 1);
      output_shares[my_id] = shares[3][0];
    }
    END_PLAYER_DEF(i);
  }

  CLEANUP();

  auto rep = frn::CreateReplicator(n);
  auto secret0 = rep.Reconstruct(output_shares);
  REQUIRE(secret == secret0);
}
