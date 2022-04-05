#ifndef _FRN_LIB_NET_CHANNEL_H
#define _FRN_LIB_NET_CHANNEL_H

#include <future>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "frn/lib/net/connector.h"
#include "frn/lib/net/shared_deque.h"

namespace frn::lib {
namespace net {

/**
 * @brief Wraps a connector.
 */
class Channel {
 public:
  virtual ~Channel(){

  };

  /**
   * @brief Set the Connector object that this channel should use.
   */
  Channel(std::unique_ptr<Connector> &connector)
      : mConnector(std::move(connector)){

        };

  /**
   * @brief Open this Channel.
   */
  virtual void Open() { mConnector->Connect(); };

  /**
   * @brief Close this Channel.
   */
  virtual void Close() { mConnector->Close(); }

  /**
   * @brief Send the content of a buffer over the channel.
   * @param buffer a pointer to some bytes
   * @param size how many bytes to send
   */
  virtual void Send(const unsigned char *buffer, std::size_t size) {
    std::size_t bytes_sent = 0;
    while (bytes_sent < size) {
      auto n = mConnector->Send(buffer + bytes_sent, size - bytes_sent);
      bytes_sent += n;
    }
  }

  /**
   * @brief Recveive bytes into a buffer from the channel.
   * @param buffer where to store the received data
   * @param size how many bytes are expected
   */
  virtual void Recv(unsigned char *buffer, std::size_t size) {
    std::size_t bytes_recv = 0;
    while (bytes_recv < size) {
      auto n = mConnector->Recv(buffer + bytes_recv, size - bytes_recv);
      bytes_recv += n;
    }
  }

  /**
   * @brief Get the state of this Channel.
   */
  enum Connector::State State() const { return mConnector->State(); }

  /**
   * @brief Returns a string representation of a channel.
   */
  std::string ToString() const {
    std::stringstream ss;
    ss << "<Channel(" << mConnector->ToString() << ")>";
    return ss.str();
  }

 protected:
  /**
   * @brief Connector.
   */
  std::unique_ptr<Connector> mConnector;
};

/**
 * @brief A channel implementation which perfoms all send operations in a
 * separate thread.
 *
 * A Threadedsenderchannel performs all send operations asynchronously by
 * executing all calls to send on the underlying Connector object in a separate
 * thread.
 */
class AsyncSenderChannel : public Channel {
 public:
  /**
   * @brief Create a new async channel with a connector.
   */
  AsyncSenderChannel(std::unique_ptr<Connector> &connector)
      : Channel(connector){

        };

  void Open();

  void Close();

  void Send(const unsigned char *buffer, std::size_t size);

 private:
  SharedDeque<std::vector<unsigned char>> mSendQueue;
  std::future<void> mSender;
};

}  // namespace net
}  // namespace frn::lib

#endif  // _FRN_LIB_NET_CHANNEL_H
