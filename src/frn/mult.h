#ifndef MULT_H
#define MULT_H

#include <memory>

#include "frn/corr.h"
#include "frn/network.h"
#include "frn/shr.h"

namespace frn {

struct AddAndMsgs {
  Field add_share;
  std::vector<Shr> msgs;
};

struct CheckData {
  // The shares the given party sent to P1 across the
  // multiplications
  std::vector<Field> shares_sent_to_p1;
  // For each party, the shares P1 received across all
  // multiplications
  std::vector<std::vector<Field>> shares_recv_by_p1;
  // Reconstructions received from P1
  std::vector<Field> values_recv_from_p1;
  // For each mult and for each party in U, rep share of msg^i
  std::vector<std::vector<Shr>> msgs;
  // Counter
  std::size_t counter = 0;

  CheckData(unsigned threshold) { shares_recv_by_p1.resize(2 * threshold + 1); }
};

/**
 * @brief The inteface of the Mult protocol.
 */
class Mult {
 public:
  /**
   * @brief Create a new mult protocol instance.
   * @param network: an object for talking with other parties
   * @param replicator: an object for talking with other parties
   * @param manipulator: a manipulator to compute on shares
   * @param correlator: object for obtaining correlated data
   */
  Mult(std::shared_ptr<Network> network,
       const frn::lib::secret_sharing::Replicator<Field>& replicator,
       const ShrManipulator& manipulator, const Correlator& correlator,
       CheckData& cd)
      : mNetwork(network),
        mReplicator(replicator),
        mId(network->Id()),
        mThreshold(replicator.Threshold()),
        mSize(network->Size()),
        // TODO Manipulator already has a replicator
        // TODO pass by reference the replicator(s) to the manipulators
        mManipulator(manipulator),
        mCorrelator(correlator),
        mCount(0),
        mCheckData(&cd) {
    mSharesRecvByP1.resize(2 * mThreshold + 1);
    // TODO Provide a default size for internal containers.
  };

  /**
   * @brief Indicates that we wish to multiply two shared values.
   * @param ShareX replicated share of first factor
   * @param ShareX replicated share of second factor
   */
  void Prepare(const Shr shares_x, const Shr shares_y) {
    RandomShare randomShares = mCorrelator.GenRandomShare();
    mRandomShares.emplace_back(randomShares);
    AddAndMsgs output = MultiplyToAddAndMsgs(shares_x, shares_y, randomShares);

    mSharesToSendP1.emplace_back(output.add_share);

    // Append check data
    mCheckData->shares_sent_to_p1.emplace_back(output.add_share);
    mCheckData->msgs.emplace_back(output.msgs);

    ++mCount;
  };

  void Prepare(const std::vector<Shr>& xs, const std::vector<Shr>& ys) {
    START_TIMER(prepare);
    // assumes xs and ys have the same size.
    for (std::size_t i = 0; i < xs.size(); i++) Prepare(xs[i], ys[i]);
    STOP_TIMER(prepare);
  };

  /**
   * @brief Run the multiplication protocol.
   * @return secret shares of each party's input
   */
  std::vector<Shr> Run() {
    mCheckData->counter += mCount;
    SendStep();
    if (mId == 0) ReconstructionStep();
    return OutputStep();
  };

  /**
   * @brief P1 receives shares from all P_i with i < 2T+1.
   */
  void SendStep();

  /**
   * @brief P1 reconstructs and sends out the result.
   */
  void ReconstructionStep();

  /**
   * @brief Parties adjust their local shares to get a share of the output
   */
  std::vector<Shr> OutputStep();

 private:
  std::shared_ptr<Network> mNetwork;
  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  unsigned mId;
  std::size_t mThreshold;
  std::size_t mSize;
  ShrManipulator mManipulator;
  Correlator mCorrelator;
  std::size_t mCount;

  // vector with the random shares used for the multiplications
  std::vector<RandomShare> mRandomShares;
  // vector with additive shares sent to P1
  std::vector<Field> mSharesToSendP1;
  // vector of length 2d+1 where each entry is the vector of additive
  // shares that P1 receives from each party
  std::vector<std::vector<Field>> mSharesRecvByP1;
  // vector of the reconstructed values received from P1
  std::vector<Field> mValuesRecvFromP1;
  // vector of the reconstructed values that P1 sent
  std::vector<Field> mValuesSentFromP1;

  CheckData * mCheckData;

  AddAndMsgs MultiplyToAddAndMsgs(const Shr& a, const Shr& b,
                                  const RandomShare& randomShares) {
    mRandomShares.emplace_back(randomShares);

    // Initialize output
    AddAndMsgs output;
    output.add_share = Field(0);
    output.msgs = std::vector<Shr>(
        2 * mThreshold + 1,
        Shr(mManipulator.GetDoubleReplicator().ShareSize(), Field(0)));

    Field prod;

    for (MultEntry tuple : mManipulator.GetTableMult()) {
      unsigned src_a = tuple.src_a;
      unsigned src_b = tuple.src_b;
      unsigned dest = tuple.dest_c;
      unsigned first_party = tuple.first_party;

      prod = a[src_a] * b[src_b];
      output.msgs[first_party][dest] += prod;
      if (mId == first_party) output.add_share += prod;
    }

    // Subtract random keys
    output.add_share -= randomShares.add_share;

    // TODO subtract random key from Msgs
    return output;
  }
};

}  // namespace frn

#endif  // INPUT_H
