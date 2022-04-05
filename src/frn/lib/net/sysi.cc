#include "frn/lib/net/sysi.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>

int frn::lib::net::SystemInterface::get_errno() { return errno; }

int frn::lib::net::SystemInterface::close(int fd) { return ::close(fd); }

int frn::lib::net::SystemInterface::inet_to_int(int af, const char* src,
                                           void* dest) {
  return ::inet_pton(af, src, dest);
}

int frn::lib::net::SystemInterface::create_socket(int domain, int type,
                                             int protocol) {
  return ::socket(domain, type, protocol);
}

ssize_t frn::lib::net::SystemInterface::write(int fd, const void* buf,
                                         std::size_t count) {
  return ::write(fd, buf, count);
}

ssize_t frn::lib::net::SystemInterface::read(int fd, void* buf, std::size_t count) {
  return ::read(fd, buf, count);
}

int frn::lib::net::SystemInterface::set_socket_options(int sockfd, int level,
                                                  int optname,
                                                  const void* optval,
                                                  socklen_t optlen) {
  return ::setsockopt(sockfd, level, optname, optval, optlen);
}

int frn::lib::net::SystemInterface::bind(int sockfd, const struct sockaddr* addr,
                                    socklen_t addrlen) {
  return ::bind(sockfd, addr, addrlen);
}

int frn::lib::net::SystemInterface::listen(int sockfd, int backlog) {
  return ::listen(sockfd, backlog);
}

int frn::lib::net::SystemInterface::accept(int sockfd, struct sockaddr* addr,
                                      socklen_t* addrlen) {
  return ::accept(sockfd, addr, addrlen);
}
