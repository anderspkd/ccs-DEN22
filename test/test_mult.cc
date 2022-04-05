#include <catch2/catch.hpp>

#include "frn/mult.h"
#include "frn/shr.h"
#include "frn/util.h"
#include "mock_network.h"

struct Helper {
  frn::Correlator corr;
  frn::ShrManipulator manipulator;
  frn::CheckData checkdata;
  std::vector<frn::RandomShare> random_shares;

  Helper(frn::Correlator c, frn::ShrManipulator m, frn::CheckData cd) : corr(c), manipulator(m), checkdata(cd) {};

  frn::Field Prepare(const frn::Shr x, const frn::Shr y) {
    auto r = corr.GenRandomShare();
    random_shares.emplace_back(r);
    return manipulator.MultiplyToAdditive(x, y) - r.add_share;
  };

  frn::Shr AdjustOutput(const frn::Field c) {
    return manipulator.AddConstant(random_shares[0].rep_share, c);
  };
};

std::vector<Helper> GetHelpers(
    unsigned n, unsigned d, frn::lib::secret_sharing::Replicator<frn::Field> rep) {
  std::vector<Helper> helpers;
  for (std::size_t i = 0; i < n; i++) {
    auto correlator = frn::Correlator(i, rep);
    auto manipulator = frn::ShrManipulator(i, d, n);
    auto checkdata = frn::CheckData(d);
    helpers.emplace_back(Helper(correlator, manipulator, checkdata));
  }
  return helpers;
}

// setup the objects used to run the multiplication protocol with a mocked
// network
#define SETUP(_id)                                    \
  std::shared_ptr<frn::MockNetwork> network =         \
      frn::MockNetwork::Create(_id, n);               \
  auto correlator = frn::Correlator(_id, replicator); \
  auto manipulator = frn::ShrManipulator(_id, d, n);  \
  auto checkdata = frn::CheckData(d);		      \
  auto helpers = GetHelpers(n, d, replicator);        \
  std::vector<frn::Field> prepared_shares;            \
  frn::Mult mult_protocol(network, replicator, manipulator, correlator, checkdata)

// prepare to send values from all other parties that are not the run we're
// running
#define PREPARE_SEND_STEP(_id)                                                \
  for (std::size_t i = 0; i < n; i++) {                                       \
    if (i == _id) continue;                                                   \
    auto additive_share_to_send = helpers[i].Prepare(sharesx[i], sharesy[i]); \
    prepared_shares.emplace_back(additive_share_to_send);                     \
    network->SendValuesFrom(i, {additive_share_to_send});                     \
  }

// adjust the output in the same way as the OutputStep method
#define ADJUST_OUTPUT(_id)                          \
  for (std::size_t i = 0; i < n; i++) {             \
    if (i == _id) continue;                         \
    output_shares[i] = helpers[i].AdjustOutput(re); \
  }

TEST_CASE("Secure multiplication") {
  unsigned n = 7;
  unsigned d = (n - 1) / 3;
  frn::lib::primitives::PRG prg;
  frn::Field x(10), y(20);
  frn::Field z = x * y;
  auto replicator = frn::lib::secret_sharing::Replicator<frn::Field>(n, d);
  std::vector<frn::Shr> sharesx = replicator.Share(x, prg);
  std::vector<frn::Shr> sharesy = replicator.Share(y, prg);

  SECTION("p1") {
    // party 1 performs special actions, so test this guy separately.

    unsigned id = 0;
    SETUP(id);

    mult_protocol.Prepare(sharesx[id], sharesy[id]);

    PREPARE_SEND_STEP(id);

    mult_protocol.SendStep();

    // p1 sends 1 thing to itself.
    auto r0 = network->GetValuesReceivedBy(0);
    REQUIRE(r0.size() == 1);
    REQUIRE(r0[0].size() == 1);

    network->Clear();
    mult_protocol.ReconstructionStep();
    // p1 reconstructs some values, and then sends these out to all parties P_i
    // for i < n - t
    auto r1 = network->GetValuesReceivedBy(0);
    REQUIRE(r1.size() == 1);
    REQUIRE(r1[0].size() == 1);
    auto re = r1[0][0];

    for (std::size_t i = 1; i < n; i++) {
      auto ri = network->GetValuesReceivedBy(i);
      if (i < n - d) {
        REQUIRE(ri.size() == 1);
        REQUIRE(ri[0].size() == 1);
        REQUIRE(ri[0][0] == re);
      } else {
        REQUIRE(ri.empty());
      }
    }

    // dont clear network here, since P1 sent a value to itself.
    auto output = mult_protocol.OutputStep();
    REQUIRE(output.size() == 1);
    REQUIRE(network->GetValuesReceivedBy(0).size() == 1);
    REQUIRE(network->GetValuesReceivedBy(0)[0].size() == 1);
    // adjust output for the other parties
    std::vector<frn::Shr> output_shares(n);
    output_shares[id] = output[0];

    ADJUST_OUTPUT(id);

    auto v = replicator.Reconstruct(output_shares);
    REQUIRE(v == z);
  }

  SECTION("p2") {
    unsigned id = 1;
    SETUP(id);

    mult_protocol.Prepare(sharesx[id], sharesy[id]);
    PREPARE_SEND_STEP(id);

    mult_protocol.SendStep();
    auto x = network->GetValuesReceivedBy(0);
    REQUIRE(x.size() == 1);

    auto re = x[0][0];
    for (std::size_t i = 0; i < 2 * d + 1; i++) re += prepared_shares[i];

    network->Clear();
    network->SendValuesFrom(0, {re});

    auto output = mult_protocol.OutputStep();
    REQUIRE(output.size() == 1);

    std::vector<frn::Shr> output_shares(n);
    output_shares[id] = output[0];
    ADJUST_OUTPUT(id);

    auto w = replicator.Reconstruct(output_shares);
    REQUIRE(w == z);
  }

  SECTION("plast") {
    unsigned id = n - 1;
    SETUP(id);

    mult_protocol.Prepare(sharesx[id], sharesy[id]);
    mult_protocol.SendStep();
    auto output = mult_protocol.OutputStep();
    REQUIRE(output.size() == 1);
  }
}
