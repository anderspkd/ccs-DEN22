#ifndef _FRN_LIB_NET_NETWORKBUILDER_H
#define _FRN_LIB_NET_NETWORKBUILDER_H

#include <functional>
#include <optional>
#include <vector>

#include "frn/lib/logging/logger.h"
#include "frn/lib/net/network.h"

namespace frn::lib {
namespace net {

/**
 * @brief Builder for Network.
 */
class Network::Builder {
 public:
  /**
   * @brief Construct a new builder.
   * @param ttype the transport type to use for the final Network.
   */
  Builder(){

  };

  /**
   * @brief Finalize the build process.
   *
   * Constructs the Network using values previously passed to the builder. Any
   * inconsitencies that wheren't caught earlier will result in an exception
   * being thrown here.
   *
   * @return A new <code>Network</code> object.
   * @throws std::logic_error if invalid parameters where passed during
   *   building.
   */
  virtual Network Build() const;

  /**
   * @brief Build the network as a shared pointer.
   * @return A new <code>Network</code> object that can be shared.
   */
  std::shared_ptr<Network> BuildShared() const {
    return std::make_shared<Network>(Build());
  };

  /**
   * @brief Specify the transport type of the network.
   * @param ttype the transport type.
   */
  Builder &TransportType(Network::TransportType ttype);

  /**
   * @brief Set the identifier of the local peer.
   *
   * This method is mandatory and specifies the identity of the local party.
   *
   * @param id the identifier of the local peer.
   * @throws std::logic_error if <code>id</code> is negative, or if
   *   <code>size</code> was previously called with an argument <code>n</code>
   *   that was less than or equal to <code>id</code>
   */
  Builder &LocalPeerId(int id);

  /**
   * @brief Set the size of the network.
   *
   * Specifies the size of the final network. That is, the number of parties
   * (the local party included).
   *
   * @param n the size of the network.
   * @throws std::logic_error if a previously set <code>id</code> was greater
   *   or equal to <code>n</code>.
   */
  Builder &Size(std::size_t n);

  /**
   * @brief Set the base port.
   *
   * @param port other peers ports are offsets of this one.
   * @remark has no effect if TransportType is set to <code>MEMORY</code>.
   */
  Builder &BasePort(int port);

  /**
   * @brief Read connection information from a file.
   *
   * The file pointed to by <code>filename</code> are assumed by have a single
   * IP address per line, and to have as many lines as the size of the desired
   * network.
   *
   * @param filename file with connection information.
   */
  Builder &ConnectionFile(std::string filename);

  /**
   * @brief All parties run locally.
   */
  Builder &AllPartiesLocal();

  /**
   * @brief Set the logger to be used.
   */
  Builder &Logger(std::shared_ptr<logging::Logger> logger);

 protected:
  /**
   * @brief The transport type of the network we're building.
   */
  std::optional<Network::TransportType> mTransportType;

  /**
   * @brief The identifier of the party owning this network.
   */
  std::optional<int> mLocalPeerId;

  /**
   * @brief Size of the network.
   */
  std::optional<std::size_t> mSize;

  /**
   * @brief Base port. All ports are offsets of this.
   */
  std::optional<int> mBasePort;

  /**
   * @brief If true, all parties reside on the same machine.
   */
  bool mAllLocal = false;

  /**
   * @brief Addresses for remote peers.
   */
  std::optional<std::vector<std::string>> mIps;

  /**
   * @brief The logger.
   */
  std::optional<std::shared_ptr<logging::Logger>> mLogger;
};

}  // namespace net
}  // namespace frn::lib

#endif  // _FRN_LIB_NET_NETWORKBUILDER_H
