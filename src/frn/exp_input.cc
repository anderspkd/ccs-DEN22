#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "frn.h"

#define DELIM std::cout << "========================================\n"

#define BASE_PORT 6677
// ID of the party giving inputs
#define INPUTTER 0

inline void Populate(std::vector<frn::Field>& vector) {
  for (std::size_t i = 0; i < vector.size(); i++) vector[i] = frn::Field(i);
}

inline std::size_t ValidateN(const std::size_t n) {
  assert(n > 3 || n < 17);
  return n;
}

inline std::size_t ValidateId(const std::size_t id, const std::size_t n) {
  assert(id < n);
  return id;
}

inline std::size_t ValidateNumberOfInputs(const std::size_t n) {
  assert(n < 1000000);
  return n;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "usage: " << argv[0] << " [N] [id] [number_of_inputs]\n";
    return 0;
  }

  std::size_t n = ValidateN(std::stoul(argv[1]));
  std::size_t t = (n - 1) / 3;
  std::size_t id = ValidateId(std::stoul(argv[2]), n);
  std::size_t number_of_inputs = ValidateNumberOfInputs(std::stoul(argv[3]));

  DELIM;
  std::cout << "Running input benchmark with N " << n << " and inputs "
            << number_of_inputs << "\n";
  DELIM;

  auto network =
      frn::TcpNetwork::CreateWithLocalParties(id, n, BASE_PORT, false);
  network->Connect();

  auto replicator = frn::CreateReplicator(n);
  frn::lib::primitives::PRG prg;
  frn::InputSetup setup(network, replicator, prg);

  START_TIMER(setup);
  auto correlator = setup.Run();
  STOP_TIMER(setup);

  frn::Input input(network, frn::ShrManipulator(id, t, n), correlator);

  if (id == INPUTTER) {
    std::vector<frn::Field> inputs(number_of_inputs);
    Populate(inputs);

    input.Prepare(inputs);

    auto outputs = input.Run();

  } else {
    input.PrepareToReceive(INPUTTER, number_of_inputs);
    auto outputs = input.Run();
  }

  DELIM;
  network->PrintCommunicationSummary();
  network->Close();
}
