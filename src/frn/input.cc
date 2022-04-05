#include "frn/input.h"

std::vector<std::vector<frn::Shr>> frn::Input::Run() {
  START_TIMER(Input_send);
  for (std::size_t i = 0; i < mSize; i++) {
    if (mSharesToDistibute.size()) {
      // not a proper broadcast.
      mNetwork->Send(i, mSharesToDistibute);
    }
  }
  STOP_TIMER(Input_send);

  START_TIMER(Input_recv_add_constant);
  std::vector<std::vector<Shr>> output(mSize);
  for (std::size_t i = 0; i < mSize; i++) {
    std::vector<frn::Shr> masked_shares = mSharesToReceive[i];
    std::vector<frn::Field> masked = mNetwork->Recv(i, masked_shares.size());

    for (std::size_t j = 0; j < masked.size(); j++) {
      output[i].emplace_back(
          mManipulator.AddConstant(masked_shares[j], masked[j]));
    }
  }
  STOP_TIMER(Input_recv_add_constant);

  return output;
}
