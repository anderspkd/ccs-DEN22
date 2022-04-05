#ifndef _FRN_LIB_NET_SYSTEMINTERFACE_H
#define _FRN_LIB_NET_SYSTEMINTERFACE_H

#include <sys/socket.h>
#include <unistd.h>

#include <cstddef>

namespace frn::lib {
namespace net {

/**
 * @brief Provides access to system calls.
 *
 * SystemInterface is the single entry point to system calls that are needed for
 * working with sockets. By inheriting from SystemInterface and overriding a
 * call, it is possible to test code that would be executed in case a system
 * call fails.
 */
class SystemInterface {
 public:
  // LCOV_EXCL_START

  /**
   * @brief destructor.
   */
  virtual ~SystemInterface(){

  };

  // LCOV_EXCL_STOP

  /**
   * @brief Man errno.
   */
  virtual int get_errno();

  /**
   * @brief Man 2 close.
   */
  virtual int close(int fd);

  /**
   * @brief Man 3 inet_pton.
   */
  virtual int inet_to_int(int af, const char *src, void *dst);

  /**
   * @brief Man 2 socket.
   */
  virtual int create_socket(int domain, int type, int protocol);

  /**
   * @brief Man 2 write.
   */
  virtual ssize_t write(int fd, const void *buf, std::size_t count);

  /**
   * @brief Man 2 read.
   */
  virtual ssize_t read(int fd, void *buf, std::size_t count);

  /**
   * @brief Man 2 getsockopt.
   */
  virtual int set_socket_options(int sockfd, int level, int optname,
                                 const void *optval, socklen_t optlen);

  /**
   * @brief Man 2 bind.
   */
  virtual int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

  /**
   * @brief Man 2 listen.
   */
  virtual int listen(int sockfd, int backlog);

  /**
   * @brief Man 2 accept.
   */
  virtual int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
};

}  // namespace net
}  // namespace frn::lib

#endif  // _FRN_LIB_NET_SYSTEMINTERFACE_H
