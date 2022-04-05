#ifndef MOCK_NETWORK_H
#define MOCK_NETWORK_H

#include <memory>
#include <queue>
#include <vector>

#include "frn/network.h"

namespace frn {

/**
 * A mock network for testing which inspecting exactly what a particular party
 * sends, as well as specifying exactly what a particular party is supposed to
 * receive.
 */
class MockNetwork final : public Network {
 public:
  /**
   * @brief Create a new mock network for testing.
   * @param id the ID of the local party
   * @param n the total number of parties
   * @return a pointer to a new network object
   */
  static std::shared_ptr<MockNetwork> Create(unsigned id, std::size_t n) {
    return std::shared_ptr<MockNetwork>(new MockNetwork(id, n));
  };

  // remove default constructor to prevent construting invalid network objects.
  MockNetwork() = delete;

  /**
   * @brief Fakes sends field elements from some party to this party.
   * @param id the sender
   * @param values the elements to send
   */
  void SendValuesFrom(unsigned id, const std::vector<Field>& values);

  /**
   * @brief Fakes sends shares from some party to this party.
   * @param id the sender
   * @param shares the shares to send
   */
  void SendSharesFrom(unsigned id, const std::vector<Shr>& shares);

  /**
   * @brief Fakes sends data from some party to this party.
   * @param id the sender
   * @param data the data to send
   */
  void SendBytesFrom(unsigned id, const std::vector<unsigned char>& data);

  /**
   * @brief Get the field elements that this party sent to some other party.
   * @param id the ID of the receiver
   * @return the elements sent to ID
   */
  std::vector<std::vector<Field>> GetValuesReceivedBy(unsigned id) {
    return mValues[id];
  };

  /**
   * @brief Get the shares that this party sent to some other party.
   * @param id the ID of the receiver
   * @return the shares sent to ID
   */
  std::vector<std::vector<Shr>> GetSharesReceivedBy(unsigned id) {
    return mShares[id];
  };

  /**
   * @brief Get the bytes that this party sent to some other party.
   * @param id the ID of the receiver
   * @return the bytes sent to ID
   */
  std::vector<std::vector<unsigned char>> GetDataReceivedBy(unsigned id) {
    return mData[id];
  };

  /**
   * @brief Clears all buffers
   */
  void Clear() {

#define CLEAR_QUEUE(_q)\
    while (!_q.empty()) _q.pop()

    for (std::size_t i = 0; i < this->Size(); i++) {
      mValues[i].clear();
      mShares[i].clear();
      mData[i].clear();
      CLEAR_QUEUE(mValuesOut[i]);
      CLEAR_QUEUE(mSharesOut[i]);
      CLEAR_QUEUE(mDataOut[i]);
    }
#undef CLEAR_QUEUE
  };

  void Send(unsigned id, const std::vector<Field>& values) override;
  void SendShares(unsigned id, const std::vector<Shr>& shares) override;
  void SendBytes(unsigned id, const std::vector<unsigned char>& data) override;
  std::vector<Field> Recv(unsigned id, std::size_t n) override;
  std::vector<Shr> RecvShares(unsigned id, std::size_t n) override;
  std::vector<unsigned char> RecvBytes(unsigned id, std::size_t n) override;

 private:
  MockNetwork(unsigned id, std::size_t n);

  // buffers for outgoing messages
  std::vector<std::vector<std::vector<Field>>> mValues;
  std::vector<std::vector<std::vector<Shr>>> mShares;
  std::vector<std::vector<std::vector<unsigned char>>> mData;

  // buffers for incoming messages
  std::vector<std::queue<Field>> mValuesOut;
  std::vector<std::queue<Shr>> mSharesOut;
  std::vector<std::queue<unsigned char>> mDataOut;
};

}  // namespace frn

#endif  // MOCK_NETWORK_H
