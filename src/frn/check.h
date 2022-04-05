#ifndef CHECK_H
#define CHECK_H

#include <memory>

#include "frn/corr.h"
#include "frn/lib/primitives/hash.h"
#include "frn/lib/primitives/prg.h"
#include "frn/mult.h"
#include "frn/network.h"
#include "frn/shr.h"

namespace frn {

struct CompressedCheckData {
  // The shares the given party sent to P1 across the
  // multiplications
  Field shares_sent_to_p1;
  // For each party, the shares P1 received across all
  // multiplications
  std::vector<Field> shares_recv_by_p1;
  // Reconstructions received from P1
  Field values_recv_from_p1;
  // For each party, rep share of msg^i
  std::vector<Shr> msgs;

  CompressedCheckData(ShrManipulator m)
      : shares_sent_to_p1(0), values_recv_from_p1(0) {
    shares_recv_by_p1.resize(2 * m.GetReplicator().Threshold() + 1);
    // Initialize msgs to zero shares so that we can use them as accumulators
    msgs.resize(2 * m.GetReplicator().Threshold() + 1,
                Shr(m.GetDoubleReplicator().ShareSize(), Field(0)));
  }
};

/**
 * @brief The inteface of the Check protocol.
 */
class Check {
 public:
  /**
   * @brief Create a new mult protocol instance.
   */
  Check(std::shared_ptr<Network> network,
        const frn::lib::secret_sharing::Replicator<Field>& replicator,
        const ShrManipulator& manipulator, CheckData& cd)
      : mNetwork(network),
        mReplicator(replicator),
        mId(network->Id()),
        mThreshold(replicator.Threshold()),
        mSize(network->Size()),
        mManipulator(manipulator),
        mCount(cd.counter),
        mCheckData(cd),
        mCompressedCD(mManipulator),
        mValuesToSend(mSize),
        mDigestsToSend(mSize),
        mValuesReceived(mSize),
        mDigestsReceived(mSize){};

  // Omitted for now
  void SetupPRG();

  void ComputeRandomCoefficients() {
    START_TIMER(RandCoeff);
    Field coeff;
    unsigned char buf[Field::ByteSize()];

    for (unsigned mult_idx = 0; mult_idx < mCheckData.counter; mult_idx++) {
      mPRG.Next(buf, Field::ByteSize());
      coeff = Field::FromBytes(buf);
      mRandomCoefficients.emplace_back(coeff);
    }
    STOP_TIMER(RandCoeff);
  };

  // At the end of this call: Pi for 0<i<2d+1 populate the
  // compressed shares_sent_to_p1 and values_recv_from_p1, while P0
  // populates the compressed shares_recv_by_p1
  void PrepareLinearCombinations() {
    START_TIMER(LinearComb);

    // this assumes 2d+1 = n-d <> n=3d+1 (so U=T)
    if ((0 < mId) && (mId < 2 * mThreshold - 1)) {
      for (unsigned mult_idx = 0; mult_idx < mCheckData.counter; mult_idx++) {

        mCompressedCD.shares_sent_to_p1 +=
            mRandomCoefficients[mult_idx] * mCheckData.shares_sent_to_p1[mult_idx];
        mCompressedCD.values_recv_from_p1 +=
            mRandomCoefficients[mult_idx] * mCheckData.values_recv_from_p1[mult_idx];
      }
    }
    else if (mId == 0) {
      for (unsigned mult_idx = 0; mult_idx < mCheckData.counter; mult_idx++) {
        for (unsigned party_idx = 0; party_idx < 2 * mThreshold + 1;
             party_idx++) {
          mCompressedCD.shares_recv_by_p1[party_idx] +=
              mRandomCoefficients[mult_idx] * mCheckData.shares_recv_by_p1[party_idx][mult_idx];
        }
      }
    }
    STOP_TIMER(LinearComb);
  };

  // Omitted for now
  void AgreeOnTranscript();

  void PrepareMsgs() {
    START_TIMER(PrepareMsgs);
    // Compress msgs
    for (unsigned mult_idx = 0; mult_idx < mCheckData.counter; mult_idx++) {
      for (unsigned party_idx = 0; party_idx < 2 * mThreshold + 1;
           party_idx++) {
        Shr shr = mManipulator.MultiplyConstant(mRandomCoefficients[mult_idx],
                                          mCheckData.msgs[mult_idx][party_idx]);
        mCompressedCD.msgs[party_idx] =
            mManipulator.Add(mCompressedCD.msgs[party_idx], shr);
      }
    }

    // Prepare reconstruction
    const auto& TableRec = mManipulator.GetTableRec();

    for (unsigned shr_id = 0;
         shr_id < mManipulator.GetDoubleReplicator().ShareSize(); shr_id++) {
      // VALUES to send
      if (TableRec[shr_id].value_or_hash == VALUE) {
        for (unsigned recv_idx : TableRec[shr_id].party_set) {
          std::vector<Field> batched_add_share;
          for (unsigned i = 0; i < 2 * mThreshold + 1; i++) {
            batched_add_share.emplace_back(mCompressedCD.msgs[i][shr_id]);
          }
          // Append to mValuesToSend
          mValuesToSend[recv_idx].insert(mValuesToSend[recv_idx].end(),
                                         batched_add_share.begin(),
                                         batched_add_share.end());
        }
      }
      // HASHES to send
      // TODO we don't currently compute hashes. Simply send dummy field
      // elements
      else if (TableRec[shr_id].value_or_hash == HASH) {
        for (unsigned recv_idx : TableRec[shr_id].party_set) {
          std::vector<Field> batched_add_share;
          Field hash;
          for (unsigned i = 0; i < 2 * mThreshold + 1; i++) {
            batched_add_share.emplace_back(mCompressedCD.msgs[i][shr_id]);
          }
          // Here: compute hash of batched_add_share
          hash = Field(0);
          mDigestsToSend[recv_idx].emplace_back(hash);
        }
      }
    }
    STOP_TIMER(PrepareMsgs);
  };

  void ReconstructMsgs() {
    START_TIMER(ReconstructMsgs);
    std::vector<unsigned char> buffer(4);
    for (std::size_t recv_id = 0; recv_id < mSize; ++recv_id) {
      // Send length
      std::uint32_t size = mValuesToSend[recv_id].size();
      std::memcpy(buffer.data(), (unsigned char*)&size, 4);
      mNetwork->SendBytes(recv_id, buffer);

      // Send values
      mNetwork->Send(recv_id, mValuesToSend[recv_id]);

      // Send length
      size = mDigestsToSend[recv_id].size();
      std::memcpy(buffer.data(), (unsigned char*)&size, 4);
      mNetwork->SendBytes(recv_id, buffer);

      // Send hashes
      mNetwork->Send(recv_id, mDigestsToSend[recv_id]);
    }

    for (std::size_t sender_id = 0; sender_id < mSize; ++sender_id) {
      // Receive length
      std::uint32_t size;
      size = *(std::uint32_t*)mNetwork->RecvBytes(sender_id, 4).data();
      // Receive values
      mNetwork->Recv(sender_id, size);

      // Receive length
      size = *(std::uint32_t*)mNetwork->RecvBytes(sender_id, 4).data();
      // Receive hashes
      mNetwork->Recv(sender_id, size);
    }
    STOP_TIMER(ReconstructMsgs);
  };

 private:
  std::shared_ptr<Network> mNetwork;
  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  unsigned mId;
  std::size_t mThreshold;
  std::size_t mSize;
  ShrManipulator mManipulator;

  std::size_t mCount;
  CheckData mCheckData;
  // Used for sampling random values for the check
  // TODO protocol for obtaining it
  frn::lib::primitives::PRG mPRG;
  std::vector<Field> mRandomCoefficients;
  CompressedCheckData mCompressedCD;

  std::vector<std::vector<Field>> mValuesToSend;
  std::vector<std::vector<Field>> mDigestsToSend;
  std::vector<std::vector<Field>> mValuesReceived;
  std::vector<std::vector<Field>> mDigestsReceived;
};

}  // namespace frn

#endif  // CHECK_H
