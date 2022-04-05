#ifndef NETWORK_H
#define NETWORK_H

#include <memory>
#include <vector>

#include "frn/shr.h"
#include "frn/util.h"

namespace frn {

/**
 * @brief Network interface.
 */
class Network {
 public:
  /**
   * @brief Get the ID of this party.
   */
  unsigned Id() const { return mId; }

  /**
   * @brief Get the size of the network, i.e., the number of parties
   */
  std::size_t Size() const { return mSize; }

  /**
   * @brief Send a vector of field elements to another party.
   * @param id the ID of the remote party
   * @param values the field elements to send
   */
  virtual void Send(unsigned id, const std::vector<Field>& values) = 0;

  /**
   * @brief Send a vector of shares to another party.
   * @param id the ID of the remote party
   * @param shares the shares to send
   */
  virtual void SendShares(unsigned id, const std::vector<Shr>& shares) = 0;

  /**
   * @brief Send a vector of bytes to another party.
   * @param id the ID of the remote party
   * @param data the bytes to send
   */
  virtual void SendBytes(unsigned id,
                         const std::vector<unsigned char>& data) = 0;

  /**
   * @brief Receive a vector of field elements from a remote party
   * @param id the ID of the sender
   * @param n the number of elements to receive
   * @return the received elements.
   */
  virtual std::vector<Field> Recv(unsigned id, std::size_t n) = 0;

  /**
   * @brief Receive a vector of shares from a remote party
   * @param id the ID of the sender
   * @param n the number of shares to receive
   * @return the received shares.
   */
  virtual std::vector<Shr> RecvShares(unsigned id, std::size_t n) = 0;

  /**
   * @brief Receive a vector of bytes from a remote party
   * @param id the ID of the sender
   * @param n the number of bytes to receive
   * @return the received bytes.
   */
  virtual std::vector<unsigned char> RecvBytes(unsigned id, std::size_t n) = 0;

 protected:
  /**
   * @brief Construct a new network
   * @param id the identity of this party
   * @param n the number of parties
   */
  Network(unsigned id, std::size_t n) : mId(id), mSize(n){};

 private:
  unsigned mId;
  std::size_t mSize;
};

}  // namespace frn

#endif  // NETWORK_H
