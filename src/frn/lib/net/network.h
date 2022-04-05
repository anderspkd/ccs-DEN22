#ifndef _FRN_LIB_NET_NETWORK_H
#define _FRN_LIB_NET_NETWORK_H

#include <memory>
#include <stdexcept>
#include <vector>

#include "frn/lib/logging/logger.h"
#include "frn/lib/net/channel.h"
#include "frn/lib/net/connector.h"

namespace frn::lib {
namespace net {

/**
 * @brief A network of n peers.
 */
class Network {
 public:
  /**
   * @brief All ports are a positive offset of this value.
   */
  static constexpr int kBasePort = 9876;

  /**
   * @brief Builder used to create a Network.
   */
  class Builder;

  /**
   * @brief Valid connection types.
   */
  enum class TransportType {
    //! Channels are connected via. TCP
    eTcp,

    //! Dummy TransportType. Used in testing.
    eFake
  };

  /**
   * @brief Connect the network to all peers.
   */
  void Connect() {
    for (std::size_t i = 0; i < mSize; ++i) {
      LOG_INFO(mLogger, "connect %", mChannels[i]->ToString());
      mChannels[i]->Open();
    }
  };

  /**
   * @brief Close the network.
   */
  void Close() {
    for (std::size_t i = 0; i < mSize; ++i) {
      LOG_INFO(mLogger, "closing %", mChannels[i]->ToString());
      mChannels[i]->Close();
    }
  };

  /**
   * @brief Send some bytes to a particular party.
   * @param id the identity of the receiver
   * @param data a pointer to the data to send
   * @param size how many bytes to send
   */
  virtual void SendTo(int id, const unsigned char *data, std::size_t size) {
    mChannels[id]->Send(data, size);
  };

  /**
   * @brief Broadcast some bytes to all parties.
   * @param data the data to send
   * @param size how many bytes to send
   */
  virtual void Broadcast(const unsigned char *data, std::size_t size) {
    for (std::size_t i = 0; i < mSize; ++i) {
      SendTo(i, data, size);
    }
  };

  /**
   * @brief Recveive from a particular party.
   * @param id the identity of the sender
   * @param buffer where to store the received data
   * @param size how many bytes are expected to be received
   */
  virtual void RecvFrom(int id, unsigned char *buffer, std::size_t size) {
    mChannels[id]->Recv(buffer, size);
  };

  /**
   * @brief Return the amount of peers in the network.
   */
  std::size_t Size() const { return mSize; };

  /**
   * @brief Return the identifier for the local peer.
   */
  int LocalPeerId() const { return mId; };

 private:
  /**
   * @brief Create a network.
   *
   * @param id the identifier of the local peer.
   * @param n the total number of nodes in the network.
   * @param ttype enum denoting how connections are established.
   * @param channels channels between the different peers.
   * @param logger the logger to use.
   */
  Network(int id, std::size_t n, TransportType ttype,
          std::vector<std::unique_ptr<Channel>> &channels,
          std::shared_ptr<logging::Logger> logger)
      : mId(id),
        mSize(n),
        mTransportType(ttype),
        mChannels(std::move(channels)),
        mLogger(std::move(logger)){};

 protected:
  int mId = 0;
  std::size_t mSize = 0;
  TransportType mTransportType;
  std::vector<std::unique_ptr<Channel>> mChannels;
  std::shared_ptr<logging::Logger> mLogger;
};

constexpr int Network::kBasePort;

}  // namespace net

}  // namespace frn::lib

#endif  // _FRN_LIB_NET_NETWORK_H
