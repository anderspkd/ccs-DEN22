#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <memory>

#include "frn/input_corr.h"
#include "frn/network.h"
#include "frn/shr.h"

namespace frn {

/**
 * @brief The inteface of the Input protocol.
 */
class Input {
 public:
  /**
   * @brief Create a new input protocol instance.
   * @param network an object for talking with other parties
   */
  Input(std::shared_ptr<Network> network, ShrManipulator manipulator,
        InputSetup::Correlator correlator)
      : mNetwork(network),
        mManipulator(manipulator),
        mCorrelator(correlator),
        mId(network->Id()),
        mSize(network->Size()) {
    mSharesToReceive.resize(mSize);
    mSharesToDistibute.reserve(10000);
  };

  /**
   * @brief Indicate that we wish to input a value.
   * @param secret the value we wish to input
   */
  void Prepare(const Field& secret) {
    auto mask = mCorrelator.GetMask();
    mSharesToDistibute.emplace_back(secret - mask);
    PrepareToReceive(mId);
  };

  void Prepare(const std::vector<Field>& secrets) {
    for (const auto& secret : secrets) Prepare(secret);
  };

  /**
   * @brief Indicate that we expect to receive shares from some other party
   * @param id the ID of the party performing an input
   */
  void PrepareToReceive(unsigned id) {
    mSharesToReceive[id].emplace_back(mCorrelator.GetMaskShare(id));
  };

  void PrepareToReceive(unsigned id, std::size_t n) {
    for (std::size_t i = 0; i < n; i++) PrepareToReceive(id);
  };

  /**
   * @brief Run the input protocol.
   * @return secret shares of each party's input
   */
  std::vector<std::vector<Shr>> Run();

 private:
  std::shared_ptr<Network> mNetwork;
  ShrManipulator mManipulator;
  InputSetup::Correlator mCorrelator;
  unsigned mId;
  std::size_t mSize;
  std::vector<std::vector<Shr>> mSharesToReceive;
  std::vector<Field> mSharesToDistibute;
};

}  // namespace frn

#endif  // INPUT_H
