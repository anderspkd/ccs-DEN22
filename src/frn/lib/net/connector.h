#ifndef _FRN_LIB_NET_CONNECTOR_H
#define _FRN_LIB_NET_CONNECTOR_H

#include <algorithm>
#include <cstring>
#include <memory>
#include <sstream>
#include <vector>

#include "frn/lib/net/shared_deque.h"
#include "frn/lib/net/sysi.h"
#include "frn/lib/tools.h"

/**
 * @brief Timeout between each connection attempt.
 */
#ifndef CLIENT_CONNECT_SLEEP
#define CLIENT_CONNECT_SLEEP 300ms
#endif

namespace frn::lib {
namespace net {

/**
 * @brief Keeps track of a, and facilitates communication on, a socket.
 *
 * A Connector object is a somewhat thin wrapper around a socket and abstracts
 * away details related to setting up a two-way connection between two peers.
 *
 * The Connector class itself is not meant to be instantiated, but defines the
 * interface against which a Connector is used.
 */
class Connector {
 public:
  /**
   * @brief The valid states of a Connector.
   */
  enum class State {
    //! State of the connector after initilizations
    eIdle,
    //! State of the connector after a connection have been established
    eActive,
    //! State of the connector after a connection have been closed
    eClosed,
    //! State of the connector if a critical error happens
    eError,
    //! Dummy
    eUnknown
  };

  // LCOV_EXCL_START

  /**
   * @brief destructor.
   */
  virtual ~Connector(){

  };

  // LCOV_EXCL_STOP

  /**
   * @brief Establish a Connection to a another peer.
   *
   * This function calls <code>establish_connection</code>.
   */
  virtual void Connect() {
    EstablishConnection();
    this->mState = State::eActive;
  };

  /**
   * @brief Close connection.
   */
  virtual void Close() {
    TeardownConnection();
    this->mState = State::eClosed;
  };

  /**
   * @brief Send data to another peer.
   *
   * @param buffer a pointer to the data to be sent.
   * @param size the amount of data to be sent.
   * @return the number of bytes sent.
   */
  virtual std::int64_t Send(const unsigned char *buffer, std::size_t size) = 0;

  /**
   * @brief Recv data from another peer.
   *
   * @param buffer a pointer to where recieved data is stored.
   * @param size the amount of data that is expected to be received.
   * @return the number of bytes received.
   */
  virtual std::int64_t Recv(unsigned char *buffer, std::size_t size) = 0;

  /**
   * @brief Returns a string representation of this Connector.
   */
  virtual std::string ToString() const = 0;

  /**
   * @brief Read the current state of the Connector.
   */
  State State() const { return mState; };

 protected:
  /**
   * @brief Establish connection to a remote peer.
   */
  virtual void EstablishConnection() = 0;

  /**
   * @brief Teardown a connection.
   */
  virtual void TeardownConnection() = 0;

  /**
   * @brief Current state.
   */
  enum State mState = State::eIdle;
};

}  // namespace net

namespace utils {

/**
 * @brief to_string specialization for Connector states.
 */
template <>
std::string to_string(const enum net::Connector::State &state);

}  // namespace utils

namespace net {

/**
 * @brief A Connector for talking locally through shared buffers.
 */
class LocalConnector : public Connector {
 public:
  /**
   * @brief Type of the data buffer that is used to store messages.
   */
  using BufferType = SharedDeque<std::vector<unsigned char>>;

  /**
   * @brief Shared pointer to data buffer type.
   */
  using BufferPtr = std::shared_ptr<BufferType>;

  /**
   * @brief Helper to create a new pointer to a buffer.
   */
  static BufferPtr MakeBuffer() { return std::make_shared<BufferType>(); };

  /**
   * @brief Construct a new LocalConnector with a provided in and out buffer.
   *
   * This constructs a new LocalConnector where outgoing messages are placed
   * in the first parameter, and incomming messages are written to the second.
   *
   * @param out buffer for placing outgoing messages.
   * @param in buffer for reading incoming messages.
   */
  LocalConnector(BufferPtr &out, BufferPtr &in)
      : mOutgoing(out), mIncoming(in) {
    if (!(mOutgoing && mIncoming))
      throw std::logic_error("buffers cannot be null");
  };

  std::int64_t Send(const unsigned char *buffer, std::size_t size) {
    mOutgoing->PushBack(std::vector<unsigned char>(buffer, buffer + size));
    return size;
  };

  std::int64_t Recv(unsigned char *buffer, std::size_t size) {
    auto data = mIncoming->Front();
    auto actual_size = std::min(data.size(), size);
    std::memcpy(buffer, data.data(), actual_size);
    mIncoming->PopFront();
    return actual_size;
  };

  /**
   * @brief Returns <code>"LocalConnector()"</code>.
   */
  std::string ToString() const { return "LocalConnector()"; };

 private:
  void EstablishConnection(){};

  void TeardownConnection(){};

  BufferPtr mOutgoing = 0;
  BufferPtr mIncoming = 0;
};

/**
 * @brief Shared code connectors based on TCP sockets.
 */
class TCPConnector : public Connector {
 public:
  /**
   * @brief Disallow default construction.
   */
  TCPConnector() = delete;

  std::int64_t Send(const unsigned char *buffer, std::size_t size) override;

  std::int64_t Recv(unsigned char *buffer, std::size_t size) override;

 protected:
  /**
   * @brief Constructor
   * @param system access to system calls
   */
  TCPConnector(std::shared_ptr<SystemInterface> system)
      : mSystem(system){

        };

  void TeardownConnection() override;

  /**
   * @brief Converts C style errors to <code>std::system_error</code>.
   */
  void set_error_and_throw(std::string error_message) {
    mState = Connector::State::eError;
    auto err = mSystem->get_errno();
    throw std::system_error(err, std::generic_category(),
                            error_message.c_str());
  };

  /**
   * @brief Socket.
   */
  int mSocket = 0;

  /**
   * @brief Pointer to a class handling system calls.
   */
  std::shared_ptr<SystemInterface> mSystem;
};

/**
 * @brief Returns true if a port is valid.
 */
constexpr inline bool invalid_port(int port) {
  return port < 1025 || port > 65536;
}

/**
 * @brief A TCP Connector which connects as a client.
 */
class TCPClientConnector : public TCPConnector {
 public:
  /**
   * @brief Constructor.
   *
   * @param system pointer for accessing system calls.
   * @param port the port of the remote host.
   * @param hostname the IP address of the remote host.
   */
  TCPClientConnector(std::shared_ptr<SystemInterface> system, int port,
                     std::string hostname);

  /**
   * @brief Returns <code>"TCPClientConnector(state = ..., server =
   * ...)"</code>.
   */
  std::string ToString() const {
    std::stringstream ss;
    ss << "TCPClientConnector(state = " << utils::to_string(State())
       << ", server = " << mHostname << ":" << mPort << ")";
    return ss.str();
  };

 private:
  void EstablishConnection();

  int mPort = 0;
  std::string mHostname = "";
};

/**
 * @brief A TCP Connector which listens for one incoming client.
 */
class TCPServerConnector : public TCPConnector {
 public:
  /**
   * @brief Constructor.
   * @param system pointer for accessing system calls.
   * @param port the port on which to listen for a connection.
   */
  TCPServerConnector(std::shared_ptr<SystemInterface> system, int port);

  /**
   * @brief Returns <code>"TCPServerConnector(state = ..., port = ...)"</code>.
   */
  std::string ToString() const {
    std::stringstream ss;
    ss << "TCPServerConnector(state = " << utils::to_string(State())
       << ", port = " << mPort << ")";
    return ss.str();
  };

 private:
  void EstablishConnection();

  int mPort = 0;
};

}  // namespace net
}  // namespace frn::lib

#endif  // _FRN_LIB_NET_CONNECTOR_H
