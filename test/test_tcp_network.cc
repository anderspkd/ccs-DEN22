#include <catch2/catch.hpp>
#include <thread>

#include "frn/mult.h"
#include "frn/shr.h"
#include "frn/tcp_network.h"
#include "frn/util.h"
#include "tcp_network_helper.h"

TEST_CASE("net") {
  const std::size_t n = 4;

  // setup n parties.
  CREATE_PARTIES(n, 10000);

  // parties P1, P2, P3 all send [a, b, c, i] where i is their id to P0.
  for (std::size_t i = 1; i < n; i++) {
    BEGIN_PLAYER_DEF(i) {
      network->SendBytes(0, {'a', 'b', 'c', (unsigned char)my_id});
    }
    END_PLAYER_DEF(i);
  }

  bool received_abc_and_id = true;

  // P0 receives 4 bytes from P1, P2 and P3 and checks that it matches the
  // expected value.
  BEGIN_PLAYER_DEF(0) {
    for (std::size_t i = 1; i < n; i++) {
      auto r = network->RecvBytes(i, 4);
      std::cout << "got " << r.size() << " bytes from " << i << "\n";
      std::cout << "content: " << r[0] << r[1] << r[2]
                << ", sender: " << (int)r[3] << "\n";
      received_abc_and_id &=
          std::vector<unsigned char>{'a', 'b', 'c', (unsigned char)i} == r;
    }
  }
  END_PLAYER_DEF(0);

  // join threads and networks.
  CLEANUP();

  // check that P0 received the expected data.
  REQUIRE(received_abc_and_id);
}

TEST_CASE("mult") {
  const std::size_t n = 7;
  const std::size_t d = (n - 1) / 3;
  frn::lib::primitives::PRG prg;
  frn::Field x(100);
  frn::Field y(200);
  auto rep = frn::lib::secret_sharing::Replicator<frn::Field>(n, d);
  auto shr_xs = rep.Share(x, prg);
  auto shr_ys = rep.Share(y, prg);

  CREATE_PARTIES(n, 11000);

  std::vector<frn::Shr> output_shares(n);

  for (std::size_t i = 0; i < n; i++) {
    BEGIN_PLAYER_DEF(i) {
      auto corr = frn::Correlator(my_id, rep);
      auto mani = frn::ShrManipulator(my_id, d, n);
      auto checkdata = frn::CheckData(d);
      frn::Mult multp(network, rep, mani, corr, checkdata);

      auto shr_x = shr_xs[my_id];
      auto shr_y = shr_ys[my_id];

      multp.Prepare(shr_x, shr_y);
      auto r = multp.Run();
      REQUIRE(r.size() == 1);
      output_shares[my_id] = r[0];
    }
    END_PLAYER_DEF(i);
  }

  CLEANUP();

  auto w = rep.Reconstruct(output_shares);
  REQUIRE(w == x * y);
}
