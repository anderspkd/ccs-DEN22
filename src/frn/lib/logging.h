#ifndef _FRN_LIB_LOGGING_H
#define _FRN_LIB_LOGGING_H

#include "frn/lib/logging/logger.h"
#include "frn/lib/logging/nowhere.h"
#include "frn/lib/logging/stdout.h"

// frn::lib
namespace frn::lib {

/**
 * @brief Logging.
 *
 * All loggers in frn::lib implement the interface defined by Logger and provides
 * three ways to log information:
 * - <code>INFO</code>, for logging normal information.
 * - <code>WARN</code>, for logging warnings.
 * - <code>ERROR</code>, for logging errors.
 *
 * There is currently no difference in behaviour between logging an error and
 * logging information.
 *
 * frn::lib provides two concrete loggers which can be used: VoidLogger, which
 * doesn't log anything (i.e., turns off logging) and StdoutLogger which logs
 * everything to <code>stdout</code>.
 *
 * frn::lib users can write their own logger, by deriving from Logger and
 * implementing the required methods themselves.
 */
namespace logging {}  // namespace logging

}  // namespace frn::lib

#endif  // _FRN_LIB_LOGGING_H
