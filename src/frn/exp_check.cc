#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "frn.h"

#define DELIM std::cout << "========================================\n"
#define BASE_PORT 6677

inline std::size_t ValidateN(const std::size_t n) {
  assert(n > 3 || n < 17);
  return n;
}

inline std::size_t ValidateId(const std::size_t id, const std::size_t n) {
  assert(id < n);
  return id;
}

inline std::size_t ValidateNumberOfMults(const std::size_t n) {
  assert(n < 100000);
  return n;
}

inline void FakeInputs(
    std::vector<frn::Shr>& xs, std::vector<frn::Shr>& ys, std::size_t n,
    unsigned id,
    const frn::lib::secret_sharing::Replicator<frn::Field>& replicator) {
  xs.reserve(n);
  ys.reserve(n);
  // same prg for all parties, so shares make sense.
  frn::lib::primitives::PRG prg;
  for (std::size_t i = 0; i < n; i++) {
    auto shares_xs = replicator.Share(frn::Field(i + 1), prg);
    auto shares_ys = replicator.Share(frn::Field(i + 2), prg);
    xs.emplace_back(shares_xs[id]);
    ys.emplace_back(shares_ys[id]);
  }
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "usage: " << argv[0] << " [N] [id] [number_of_mults]\n";
    return 0;
  }

  std::size_t n = ValidateN(std::stoul(argv[1]));
  std::size_t t = (n - 1) / 3;
  std::size_t id = ValidateId(std::stoul(argv[2]), n);
  std::size_t number_of_mults = ValidateNumberOfMults(std::stoul(argv[3]));

  DELIM;
  std::cout << "Running multiplication benchmark with N " << n << " and #mults "
            << number_of_mults << "\n";
  DELIM;

  auto replicator = frn::CreateReplicator(n);
  auto correlator = frn::Correlator(id, replicator);
  auto manipulator = frn::ShrManipulator(id, t, n);

  std::vector<frn::Shr> xs;
  std::vector<frn::Shr> ys;
  FakeInputs(xs, ys, number_of_mults, id, replicator);

  auto network =
      frn::TcpNetwork::CreateWithLocalParties(id, n, BASE_PORT, false);
  network->Connect();

  auto check_data = frn::CheckData(t);
  frn::Mult mult_protocol(network, replicator, manipulator, correlator,
                          check_data);

  mult_protocol.Prepare(xs, ys);

  auto output = mult_protocol.Run();
  (void)output;

  assert(check_data.counter == number_of_mults);

  frn::Check check_protocol(network, replicator, manipulator, check_data);
  check_protocol.ComputeRandomCoefficients();
  check_protocol.PrepareLinearCombinations();
  check_protocol.PrepareMsgs();
  check_protocol.ReconstructMsgs();

  DELIM;
  network->PrintCommunicationSummary();
  network->Close();
}
