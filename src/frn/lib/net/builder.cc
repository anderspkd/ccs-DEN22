#include "frn/lib/net/builder.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "frn/lib/logging/logger.h"
#include "frn/lib/logging/nowhere.h"
#include "frn/lib/net/channel.h"
#include "frn/lib/net/connector.h"
#include "frn/lib/net/network.h"
#include "frn/lib/net/shared_deque.h"
#include "frn/lib/net/sysi.h"
#include "frn/lib/tools.h"

using Connector = frn::lib::net::Connector;
using LocalConnector = frn::lib::net::LocalConnector;
using Channel = frn::lib::net::Channel;
using SystemInterface = frn::lib::net::SystemInterface;
using TCPClientConnector = frn::lib::net::TCPClientConnector;
using TCPServerConnector = frn::lib::net::TCPServerConnector;
using AsyncSenderChannel = frn::lib::net::AsyncSenderChannel;
using Network = frn::lib::net::Network;

static inline std::unique_ptr<Connector> make_local_connector() {
  auto buffer = LocalConnector::MakeBuffer();
  return std::make_unique<LocalConnector>(buffer, buffer);
}

using PortSupplier = std::function<int(bool is_server, int id)>;

static inline std::vector<std::unique_ptr<Channel>> create_tcp_channels(
    int local_id, std::size_t size, PortSupplier get_port,
    const std::vector<std::string>& ips) {
  std::vector<std::unique_ptr<Channel>> channels;
  channels.reserve(size);

  std::shared_ptr<SystemInterface> system = std::make_shared<SystemInterface>();

  for (std::size_t i = 0; i < size; ++i) {
    // this line is missed by gcov for some reason.
    // LCOV_EXCL_START
    std::unique_ptr<Connector> connector;
    // LCOV_EXCL_STOP
    if ((int)i == local_id) {
      connector = make_local_connector();
    } else if ((int)i < local_id) {
      auto port = get_port(false, i);
      connector = std::make_unique<TCPClientConnector>(system, port, ips[i]);
    } else {  // i > local_id
      auto port = get_port(true, i);
      connector = std::make_unique<TCPServerConnector>(system, port);
    }
    channels.emplace_back(std::make_unique<AsyncSenderChannel>(connector));
  }
  return channels;
}

Network frn::lib::net::Network::Builder::Build() const {
  if (!mLocalPeerId) throw std::logic_error("identifier not set");

  if (!mSize) throw std::logic_error("network size not provided");

  if (!mTransportType) throw std::logic_error("transport type not specified");

  std::shared_ptr<logging::Logger> logger =
      mLogger.value_or(std::make_shared<logging::VoidLogger>());

  auto ttype = mTransportType.value();

  if (ttype == Network::TransportType::eTcp) {
    int id = mLocalPeerId.value();
    std::size_t n = mSize.value();

    std::vector<std::string> ips(n);
    if (mAllLocal) {
      for (auto& ip : ips) ip = "0.0.0.0";
    } else {
      ips = mIps.value();
    }

    int base_port = mBasePort.value_or(Network::kBasePort);
    auto port_picker = [&base_port, &id, &n](bool is_server, int other_id) {
      const auto server_port = id * n + other_id;
      const auto client_port = other_id * n + id;
      return base_port + (is_server ? server_port : client_port);
    };
    auto channels = create_tcp_channels(id, n, port_picker, ips);

    return Network(id, n, ttype, channels, logger);
  }

  throw std::logic_error("unknown transport type");
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::TransportType(
    Network::TransportType ttype) {
  mTransportType = ttype;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::LocalPeerId(int id) {
  if (id < 0) throw std::logic_error("identifier cannot be negative");
  if (mSize && (std::size_t)id >= mSize.value())
    throw std::logic_error(
        "identifier must be strictly less than network size");
  mLocalPeerId = id;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::Size(std::size_t n) {
  if (mLocalPeerId && (std::size_t)mLocalPeerId.value() >= n)
    throw std::logic_error("identifier is larger than network size");
  mSize = n;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::BasePort(int port) {
  if (invalid_port(port)) throw std::logic_error("port outside allowed range");
  mBasePort = port;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::ConnectionFile(
    std::string filename) {
  std::ifstream file(filename);
  if (!file.is_open()) throw std::logic_error("could not open connection file");

  std::string line;
  std::size_t n = 0;
  std::vector<std::string> ips;

  while (std::getline(file, line)) {
    ips.push_back(line);
    ++n;
  }

  if (!n) throw std::logic_error("no IPs in provided connection file");

  if (mSize && n != mSize.value())
    throw std::logic_error(
        "number of entries in connection file "
        "does not match network size");

  mIps = ips;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::AllPartiesLocal() {
  mAllLocal = true;
  return *this;
}

frn::lib::net::Network::Builder& frn::lib::net::Network::Builder::Logger(
    std::shared_ptr<logging::Logger> logger) {
  using LoggerPtr = std::shared_ptr<logging::Logger>;
  mLogger = std::make_optional<LoggerPtr>(logger);
  return *this;
}
