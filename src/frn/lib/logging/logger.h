#ifndef _FRN_LIB_LOGGING_LOGGER_H
#define _FRN_LIB_LOGGING_LOGGER_H

#include <memory>
#include <sstream>

namespace frn::lib {
namespace logging {

#if 1
//! Log some info. Use this macro to toggle logging at compile time.
#define LOG_INFO(logger, ...) (logger)->Info(__VA_ARGS__)
//! Log a warning. Use this macro to toggle logging at compile time.
#define LOG_WARN(logger, ...) (logger)->Warn(__VA_ARGS__)
//! Log an error. Use this macro to toggle logging at compile time.
#define LOG_ERROR(logger, ...) (logger)->Error(__VA_ARGS__)
#else
#define LOG_INFO(logger, ...)
#define LOG_WARN(logger, ...)
#define LOG_ERROR(logger, ...)
#endif

/**
 * @brief Logging interface.
 *
 * Logger defines a very simple interface with three distinct levels and with
 * support for string interpolation. For example:
 *
 * <code>
 *  std::shared_ptr<frn::lib::log::Logger> logger = ... <br>
 *  logger->info("hello"); <br>
 *  logger->warn("some int x=%", 27);
 * </code>
 *
 * Interpolation is possible by placing a <code>%</code> in the format string
 * and then providing the thing to be interpolated as an additional argument. No
 * guarantee is given with regards to the behavior for ill-formed
 * calls. Interpolation uses <code>std::stringstream</code> under the hood, so
 * all format arguments must support a <code>&lt;&lt;</code> style overload.
 *
 * To implement a custom logger, inherit from Logger and implement:
 *
 * - <code>log_begin(Logger::Level level)</code>. This method is called
 *   immediately logging of the actual message happens. The <code>level</code>
 *   argument denotes the level of log call and is one of
 *   <code>Logger:Level::INFO</code>, <code>Logger::Level::WARN</code> or
 *   <code>Logger::Level::ERROR</code>.
 *
 * - <code>log(const std::string& thing)</code>. This method is called with the
 *   message to be logged. The string passed to <code>log</code> is string after
 *   interpolation.
 *
 * - <code>log_end(Logger::Level level)</code>. Called after
 *   <code>log</code>. The argument will be the same as the one provided to
 *   <code>log_begin</code>.
 */
class Logger {
 public:
  /**
   * @brief Supported logging levels.
   */
  enum class Level { eInfo, eWarn, eError };

  /**
   * @brief Call to log useful information.
   *
   * @param message a format string.
   * @param args arguments for the format string.
   */
  template <typename... Ts>
  void Info(const char *message, Ts &&...args) {
    LogBegin(Level::eInfo);
    DoLog(message, std::forward<Ts>(args)...);
    LogEnd(Level::eInfo);
  }

  /**
   * @brief Call to log a warning.
   *
   * @param message a format string.
   * @param args arguments for the format string.
   */
  template <typename... Ts>
  void Warn(const char *message, Ts &&...args) {
    LogBegin(Level::eWarn);
    DoLog(message, std::forward<Ts>(args)...);
    LogEnd(Level::eWarn);
  }

  /**
   * @brief Call to log an error.
   *
   * @param message a format string.
   * @param args arguments for the format string.
   */
  template <typename... Ts>
  void Error(const char *message, Ts &&...args) {
    LogBegin(Level::eError);
    DoLog(message, std::forward<Ts>(args)...);
    LogEnd(Level::eError);
  }

  /**
   * @brief Return a string representation of this logger.
   */
  virtual std::string ToString() const = 0;

 protected:
  /**
   * @brief Called just before <code>log(const std::string& thing)</code>.
   *
   * @param level the logging level of the caller.
   */
  virtual void LogBegin(Level level) = 0;

  /**
   * @brief Called with the message to be logged.
   *
   * @param thing the thing to be logged.
   */
  virtual void Log(const std::string &thing) = 0;

  /**
   * @brief Called just after <code>log(const std::string& thing)</code>.
   *
   * @param level the logging level of the caller.
   */
  virtual void LogEnd(Level level) = 0;

 private:
  template <typename... Ts>
  void DoLog(const char *format, Ts &&...args) {
    std::stringstream ss;
    DoLog(ss, format, std::forward<Ts>(args)...);
    Log(ss.str());
  }

  // Recursion base case. Add the rest of the format string to the stream.
  void DoLog(std::stringstream &acc, const char *format) { acc << format; };

  // Run through the format string and replace all occurances of '%' with
  // extra arguments.
  template <typename T, typename... Ts>
  void DoLog(std::stringstream &acc, const char *format, T &&current,
             Ts &&...args) {
    for (; *format != '\0'; format++) {
      if (*format == '%') {
        acc << current;
        DoLog(acc, format + 1, std::forward<Ts>(args)...);
        return;
      }
      acc << *format;
    }
  }
};

/**
 * @brief Helper for creating a logger object.
 *
 * @tparam LoggerType the type of the logger.
 * @tparam Ts the type pack of arguments to the logger.
 * @param args arguments for concrete logger.
 */
template <typename LoggerType, typename... Ts>
std::shared_ptr<LoggerType> create_logger(Ts &&...args) {
  return std::make_shared<LoggerType>(args...);
}

}  // namespace logging
}  // namespace frn::lib

#endif  // _FRN_LIB_LOGGING_LOGGER_H
