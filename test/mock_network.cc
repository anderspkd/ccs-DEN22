#include "mock_network.h"

frn::MockNetwork::MockNetwork(unsigned id, std::size_t n) : Network(id, n) {
  mValues.resize(n);
  mShares.resize(n);
  mData.resize(n);
  mValuesOut.resize(n);
  mSharesOut.resize(n);
  mDataOut.resize(n);
}

void frn::MockNetwork::Send(unsigned id,
                            const std::vector<frn::Field>& values) {
  mValues[id].emplace_back(values);
  if (id == Id()) {
    SendValuesFrom(Id(), values);
  }
}

void frn::MockNetwork::SendShares(unsigned id,
                                  const std::vector<frn::Shr>& shares) {
  mShares[id].emplace_back(shares);
  if (id == Id()) {
    SendSharesFrom(Id(), shares);
  }
}

void frn::MockNetwork::SendBytes(unsigned id,
                                 const std::vector<unsigned char>& data) {
  mData[id].emplace_back(data);
  if (id == Id()) {
    SendBytesFrom(Id(), data);
  }
}

std::vector<frn::Field> frn::MockNetwork::Recv(unsigned id, std::size_t n) {
  std::vector<Field> received;
  auto& buffer = mValuesOut[id];
  for (std::size_t i = 0; i < n; i++) {
    if (buffer.empty()) break;
    received.emplace_back(buffer.front());
    buffer.pop();
  }
  return received;
}

std::vector<frn::Shr> frn::MockNetwork::RecvShares(unsigned id, std::size_t n) {
  std::vector<Shr> received;
  auto& buffer = mSharesOut[id];
  for (std::size_t i = 0; i < n; i++) {
    if (buffer.empty()) break;
    received.emplace_back(buffer.front());
    buffer.pop();
  }
  return received;
}

std::vector<unsigned char> frn::MockNetwork::RecvBytes(unsigned id,
                                                       std::size_t n) {
  std::vector<unsigned char> received;
  auto& buffer = mDataOut[id];
  for (std::size_t i = 0; i < n; i++) {
    if (buffer.empty()) break;
    received.emplace_back(buffer.front());
    buffer.pop();
  }
  return received;
}

void frn::MockNetwork::SendValuesFrom(unsigned id,
                                      const std::vector<frn::Field>& values) {
  for (auto& v : values) mValuesOut[id].push(v);
}

void frn::MockNetwork::SendSharesFrom(unsigned id,
                                      const std::vector<frn::Shr>& shares) {
  for (auto& shr : shares) mSharesOut[id].push(shr);
}

void frn::MockNetwork::SendBytesFrom(unsigned id,
                                     const std::vector<unsigned char>& data) {
  for (auto& x : data) mDataOut[id].push(x);
}
