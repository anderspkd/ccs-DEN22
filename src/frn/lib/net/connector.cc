#include "frn/lib/net/connector.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <thread>

template <>
std::string frn::lib::utils::to_string(const enum net::Connector::State& state) {
  switch (state) {
    case net::Connector::State::eIdle:
      return "IDLE";
      break;
    case net::Connector::State::eActive:
      return "ACTIVE";
      break;
    case net::Connector::State::eClosed:
      return "CLOSED";
      break;
    case net::Connector::State::eError:
      return "ERROR";
      break;
    default:
      return "???";
  }
}

void frn::lib::net::TCPConnector::TeardownConnection() {
  auto err = mSystem->close(mSocket);
  if (err < 0) set_error_and_throw("error while closing socket");
}

std::int64_t frn::lib::net::TCPConnector::Send(const unsigned char* buffer,
                                          std::size_t size) {
  size_t rem = size;
  size_t offset = 0;

  while (rem > 0) {
    auto n = mSystem->write(mSocket, buffer + offset, rem);

    if (n < 0) set_error_and_throw("write failed");

    rem -= n;
    offset += n;
  }

  return size;
}

std::int64_t frn::lib::net::TCPConnector::Recv(unsigned char* buffer,
                                          std::size_t size) {
  size_t rem = size;
  size_t offset = 0;

  while (rem > 0) {
    auto n = mSystem->read(mSocket, buffer + offset, rem);

    // other end disconnected.
    if (!n) break;
    if (n < 0) set_error_and_throw("recv failed");

    rem -= n;
    offset += n;
  }

  return size;
}

using SystemInterface = frn::lib::net::SystemInterface;
using Connector = frn::lib::net::Connector;

static inline int read_hostname(std::shared_ptr<SystemInterface> system,
                                std::string hostname,
                                struct sockaddr_in* addr) {
  auto err = system->inet_to_int(AF_INET, hostname.c_str(), &(addr->sin_addr));
  if (err == 0) throw std::runtime_error("invalid hostname");
  return err;
}

frn::lib::net::TCPClientConnector::TCPClientConnector(
    std::shared_ptr<SystemInterface> system, int port, std::string hostname)
    : TCPConnector(system), mPort(port), mHostname(hostname) {
  if (invalid_port(mPort)) throw std::runtime_error("invalid port");

  // attempt to read the address to catch errors during object construction
  // instead of later.
  struct sockaddr_in addr;
  if (read_hostname(mSystem, mHostname, &addr) == -1)
    set_error_and_throw("hostname");

  mState = Connector::State::eIdle;
}

void frn::lib::net::TCPClientConnector::EstablishConnection() {
  using namespace std::chrono_literals;

  int sock = mSystem->create_socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0) set_error_and_throw("could not acquire socket");

  // TODO: TCP_NODELAY, TCP_CORK ?

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(mPort);

  if (read_hostname(mSystem, mHostname, &addr) == -1)
    set_error_and_throw("hostname");

  while (true) {
    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      std::this_thread::sleep_for(CLIENT_CONNECT_SLEEP);
    } else {
      break;
    }
  }

  mSocket = sock;
}

frn::lib::net::TCPServerConnector::TCPServerConnector(
    std::shared_ptr<SystemInterface> system, int port)
    : TCPConnector(system), mPort(port) {
  if (invalid_port(mPort)) throw std::runtime_error("invalid port");

  mState = Connector::State::eIdle;
}

void frn::lib::net::TCPServerConnector::EstablishConnection() {
  int ssock = mSystem->create_socket(AF_INET, SOCK_STREAM, 0);

  if (ssock < 0) set_error_and_throw("could not acquire socket");

  int opt = 1;
  auto options = SO_REUSEADDR | SO_REUSEPORT;

  auto err = mSystem->set_socket_options(ssock, SOL_SOCKET, options, &opt,
                                         sizeof(opt));
  if (err < 0) set_error_and_throw("could not set options on socket");

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  addr.sin_port = htons(mPort);

  struct sockaddr* addr_ptr = (struct sockaddr*)&addr;

  if (mSystem->bind(ssock, addr_ptr, sizeof(addr)) < 0)
    set_error_and_throw("bind");

  if (mSystem->listen(ssock, 1) < 0) set_error_and_throw("listen");

  auto addrsize = sizeof(addr);
  int sock = mSystem->accept(ssock, addr_ptr, (socklen_t*)&addrsize);

  if (sock < 0) set_error_and_throw("could not accept connection from client");

  mSocket = sock;
}
