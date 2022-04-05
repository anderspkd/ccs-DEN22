#include "frn/mult.h"

void frn::Mult::SendStep() {
  START_TIMER(SendStep_send);
  // Send shares to P1
  if (mId < 2 * mThreshold + 1) {
    mNetwork->Send(0, mSharesToSendP1);
  }
  STOP_TIMER(SendStep_send);
  START_TIMER(SendStep_receive);
  // P1 receives the shares
  if (mId == 0) {
    for (std::size_t i = 0; i < 2 * mThreshold + 1; ++i) {
      mSharesRecvByP1[i] = mNetwork->Recv(i, mCount);

      // Append these shares to shares_recv_by_p1[i]
      mCheckData->shares_recv_by_p1[i].insert(mCheckData->shares_recv_by_p1[i].end(),
					     mSharesRecvByP1[i].begin(),
					     mSharesRecvByP1[i].end());

      // mCheckData.insert_shares_recv_by_P1(i, mSharesRecvByP1)
    }
  }
  STOP_TIMER(SendStep_receive);
}

void frn::Mult::ReconstructionStep() {
  START_TIMER(ReconstructionStep);
  // P1 reconstructs the xy-r's
  mValuesSentFromP1.resize(mCount);
  for (std::size_t mult_id = 0; mult_id < mCount; ++mult_id) {
    mValuesSentFromP1[mult_id] = Field(0);
    for (std::size_t party_id = 0; party_id < 2 * mThreshold + 1; ++party_id) {
      mValuesSentFromP1[mult_id] += mSharesRecvByP1[party_id][mult_id];
    }
  }

  // P1 sends the reconstructions to parties in T=1...n-d
  for (std::size_t party_id = 0; party_id < mSize - mThreshold; ++party_id) {
    mNetwork->Send(party_id, mValuesSentFromP1);
  }
  STOP_TIMER(ReconstructionStep);
}

std::vector<frn::Shr> frn::Mult::OutputStep() {
  START_TIMER(OutputStep_receive);
  // Parties in T receive the message from P1
  if (mId < mSize - mThreshold) {
    mValuesRecvFromP1 = mNetwork->Recv(0, mCount);

    // Append this to CheckData
    mCheckData->values_recv_from_p1.insert(mCheckData->values_recv_from_p1.end(),
					   mValuesRecvFromP1.begin(),
					   mValuesRecvFromP1.end());
  } else {
    // Other parties can pretend they received 0 from P1
    // This doesn't matter as they don't do anything when adding the constant
    mValuesRecvFromP1 = std::vector<Field>(mCount, Field(0));
  }
  STOP_TIMER(OutputStep_receive);

  START_TIMER(OutputStep_add_constant);
  // All parties compute the resulting shares
  std::vector<Shr> output;
  output.resize(mCount);
  for (std::size_t mult_id = 0; mult_id < mCount; ++mult_id) {
    output[mult_id] = mManipulator.AddConstant(mRandomShares[mult_id].rep_share,
                                               mValuesRecvFromP1[mult_id]);
  }
  STOP_TIMER(OutputStep_add_constant);
  return output;
}
