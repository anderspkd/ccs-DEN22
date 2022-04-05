#ifndef _FRN_LIB_LOGGING_STDOUTLOGGER_H
#define _FRN_LIB_LOGGING_STDOUTLOGGER_H

#include <chrono>
#include <iomanip>
#include <iostream>

#include "frn/lib/logging/logger.h"

namespace frn::lib {
namespace logging {

//! indicator for logged info.
#define INFO_BEGIN "[ ] "

//! indicator for logged warnings.
#define WARN_BEGIN "[W] "

//! indicator for logged errors.
#define ERROR_BEGIN "[E] "

//! format string for timestamps.
#define TIMESTAMP_FMT "%T"

/**
 * @brief Logger implementation which logs to <code>stdout</code>.
 *
 * This Logger writes all information to <code>stdout</code> using
 * <code>std::cout</code>. The logger can optionally be made to include a
 * timestamp in all calls.
 *
 * Example output:
 *
 * <code>
 *  auto logger = frn::lib::log::create_logger<StdoutLogger>(); <br>
 *  logger->info("hey!");
 * </code>
 *
 * will print:
 *
 * <code>[ ] hey!</code>
 *
 * <code>
 *  auto logger = frn::lib::log::create_logger<StdoutLogger>(true); <br>
 *  logger->warn("uh-oh"); <br>
 *  logger->error("dead");
 * </code>
 *
 * will output:
 *
 * <code>[W] [HH:MM:SS] uh-oh</code><br>
 * <code>[E] [HH:MM:SS] dead</code>
 */
class StdoutLogger : public Logger {
 public:
  /**
   * @brief Constructs a new StdoutLogger.
   *
   * @param with_timestamps if true, timestamps are included in the output.
   */
  StdoutLogger(bool with_timestamps = false)
      : mIncludeTimestamp(with_timestamps){};

  std::string ToString() const {
    if (mIncludeTimestamp) return "StdoutLogger(with timestamps)";
    return "StdoutLogger";
  }

 protected:
  void LogBegin(Level level) {
    switch (level) {
      case Level::eInfo:
        std::cout << INFO_BEGIN;
        break;
      case Level::eWarn:
        std::cout << WARN_BEGIN;
        break;
      case Level::eError:
        std::cout << ERROR_BEGIN;
        break;
    }

    if (mIncludeTimestamp) InsertTimestamp();
  };

  void Log(const std::string &thing) { std::cout << thing; };

  void LogEnd(Level) { std::cout << "\n"; };

 private:
  /**
   * @brief Prints a timestamp
   */
  void InsertTimestamp() const {
    using std_clock = std::chrono::system_clock;
    auto now = std_clock::to_time_t(std_clock::now());
    std::cout << "[" << std::put_time(std::localtime(&now), TIMESTAMP_FMT)
              << "] ";
  };

  /**
   * @brief If true, timestamps are included in the output.
   */
  bool mIncludeTimestamp;
};

}  // namespace logging
}  // namespace frn::lib

#endif  // _FRN_LIB_LOGGING_STDOUTLOGGER_H
