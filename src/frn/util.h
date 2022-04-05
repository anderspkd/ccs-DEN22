#ifndef UTIL_H
#define UTIL_H

#include <memory>

#include "frn/lib/logging.h"
#include "frn/lib/math/fp.h"
#include "frn/lib/math/p.h"
#include "frn/lib/primitives/hash.h"

#define QUOTE(x) #x

#define START_TIMER(name) \
  auto start##name = std::chrono::high_resolution_clock::now()

#define STOP_TIMER(name)                                                       \
  auto stop##name = std::chrono::high_resolution_clock::now();                 \
  auto duration##name = std::chrono::duration_cast<std::chrono::microseconds>( \
      stop##name - start##name);                                               \
  std::cout << QUOTE(name) << ": " << duration##name.count() << " us\n"

namespace frn {

/**
 * Some field defintion. Mostly used in tests.
 */
using Field = frn::lib::math::FpElement<frn::lib::math::Mp61>;

/**
 * Hash function.
 */
using Hash = frn::lib::primitives::Hash<frn::lib::primitives::SHA3_256>;

/**
 * Logger type.
 */
using Logger = frn::lib::logging::Logger;

/**
 * @brief Create a logger that writes to stdout.
 * @return a logger.
 */
inline std::shared_ptr<Logger> CreateLogger() {
  return frn::lib::logging::create_logger<frn::lib::logging::StdoutLogger>(
      true);
}

}  // namespace frn

#endif  // UTIL_H
