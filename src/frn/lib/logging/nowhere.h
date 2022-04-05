#ifndef _FRN_LIB_LOGGING_VOIDLOGGER_H
#define _FRN_LIB_LOGGING_VOIDLOGGER_H

#include <string>

#include "frn/lib/logging/logger.h"

namespace frn::lib {
namespace logging {

/**
 * @brief Logger implementation which doesn't log anything.
 *
 * VoidLogger only provides dummy implementations of Logger and so is useful if
 * one wishes to turn of logging at runtime.
 */
class VoidLogger : public Logger {
 public:
  std::string ToString() const { return "VoidLogger"; };

 protected:
  /**
   * @brief Does nothing.
   */
  void LogBegin(Level level) { (void)level; };

  /**
   * @brief Does nothing.
   */
  void Log(const std::string &thing) { (void)thing; };

  /**
   * @brief Does nothing
   */
  void LogEnd(Level level) { (void)level; };
};

}  // namespace logging
}  // namespace frn::lib

#endif  // _FRN_LIB_LOGGING_VOIDLOGGER_HPP
