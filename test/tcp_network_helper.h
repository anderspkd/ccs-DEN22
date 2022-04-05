#include "frn/tcp_network.h"
#include "frn/util.h"

/**
 * @brief Initialize parties.
 * @param __n the number of parties.
 */
#define CREATE_PARTIES(__n, __base_port)                             \
  const std::size_t __nparties = __n;                                \
  std::vector<std::shared_ptr<frn::TcpNetwork>> __networks;          \
  std::vector<unsigned> __ids;                                       \
  std::vector<std::thread> __parties;                                \
  for (std::size_t __i = 0; __i < __n; __i++) {                      \
    __ids.emplace_back(__i);                                         \
    __networks.emplace_back(frn::TcpNetwork::CreateWithLocalParties( \
        __i, __n, __base_port, __i == 0));                           \
  }

/**
 * @brief Define how a particular player should act.
 *
 * BEGIN_PLAYER_DEF(id) only works in conjuction if END_PLAYER_DEF(id) is called
 * immediately afterwards. Moreover, using this macro for the same id more than
 * once will likely result in undefined behavior.
 *
 * Intended use is something like:
 *
 * <code>
 * BEGIN_PLAYER_DEF(0) {
 *   std::cout << "hello from " << my_id << "\n";  // will print "hello from 0"
 *   network->SendBytes(7, {0, 1, 2, 3, 4, 5});
 *   auto r = network->RecvBytes(5, 10);
 * }
 * END_PLAYER_DEF(0);
 * </code>
 *
 * The above snippet defines a party with id 0 who first sends 6 bytes to party
 * 7 and then receives 10 bytes from party 5.
 *
 * BEGIN_PLAYER_DEF(id) defines two variables that are available between
 * BEGIN_PLAYER_DEF(id) and END_PLAYER_DEF(id):
 *  - unsigned my_id: This player's id.
 *  - std::shared_ptr<TcpNetwork> network: The network object for this player.
 *
 * @param __id the ID of the player.
 */
#define BEGIN_PLAYER_DEF(__id)      \
  do {                              \
    unsigned __my_id = __ids[__id]; \
  __parties.emplace_back([&, __my_id]() {       \
  unsigned my_id = __my_id;                     \
  auto network = __networks[my_id];             \
  network->Connect();
#define END_PLAYER_DEF(__id) \
  });                        \
  }                          \
  while (0)

/**
 * @brief Cleanup networks and parties.
 *
 * The output of any compution as specified by {BEGIN,END}_PLAYER_DEF(id) should
 * first checked after CLEANUP() has been called. This macro makes sure that all
 * threads have been stopped and that all network connections have been closed.
 */
#define CLEANUP()                                        \
  do {                                                   \
    for (std::size_t __i = 0; __i < __nparties; __i++) { \
      __parties[__i].join();                             \
    }                                                    \
    for (std::size_t __i = 0; __i < __nparties; __i++) { \
      __networks[__i]->PrintCommunicationSummary();      \
      __networks[__i]->Close();                          \
    }                                                    \
  } while (0)
