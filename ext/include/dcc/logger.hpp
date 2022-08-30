#ifndef DCC_LOGGER_HPP
#define DCC_LOGGER_HPP

#include <dcc/fmt/color.h>
#include <dcc/fmt/format.h>
#include <string>

#ifdef _WIN32
#include <io.h>
#define dcc_fileno(stream) (_fileno(stream))
#define dcc_isatty(fd) (_isatty(fd))
#elif __unix__
#define dcc_fileno(stream) (fileno(stream))
#define dcc_isatty(fd) (isatty(fd))
#else // POSIX
#include <io.h>
#define dcc_fileno(stream) (_fileno(stream))
#define dcc_isatty(fd) (_isatty(fd))
#endif

#define dcc_stringify_detail(x) #x
#define dcc_stringify(x) dcc_stringify_detail(x)

#if defined(__FILE_STEM__) && defined(__FILE_BASE__)
#ifdef NDEBUG
#define DCC_LOGSRC __FILE_STEM__
#else
#define DCC_LOGSRC __FILE_BASE__ ":" dcc_stringify(__LINE__)
#endif
#endif

#define dcc_logmsg(...) dcc::flogmsg(stdout, "", __VA_ARGS__)

#ifdef DCC_LOGSRC
#define DCC_SRC_STAMP dcc::sgr::file(DCC_LOGSRC)
#define dcc_logerr(...)                                                        \
  dcc::flogmsg(                                                                \
    stderr,                                                                    \
    dcc::fmt::format("[{}] ({}):", dcc::detail::error_stamp, DCC_SRC_STAMP),   \
    __VA_ARGS__)
#define dcc_logwar(...)                                                        \
  dcc::flogmsg(                                                                \
    stderr,                                                                    \
    dcc::fmt::format("[{}] ({}):", dcc::detail::warning_stamp, DCC_SRC_STAMP), \
    __VA_ARGS__)
#define dcc_logfat(...)                                                        \
  dcc::flogmsg(                                                                \
    stderr,                                                                    \
    dcc::fmt::format("[{}] ({}):", dcc::detail::fatal_stamp, DCC_SRC_STAMP),   \
    __VA_ARGS__)
#define dcc_loginf(...)                                                        \
  dcc::flogmsg(                                                                \
    stdout,                                                                    \
    dcc::fmt::format("[{}] ({}):", dcc::detail::info_stamp, DCC_SRC_STAMP),    \
    __VA_ARGS__)

#ifndef NDEBUG
#define dcc_logdbg(...)                                                        \
  dcc::flogmsg(                                                                \
    stdout,                                                                    \
    dcc::fmt::format("[{}] ({}):", dcc::detail::debug_stamp, DCC_SRC_STAMP),   \
    __VA_ARGS__);
#else
#define dcc_logdbg(...) (void)(0)
#endif

#else
#define dcc_logerr(...)                                                        \
  dcc::flogmsg(stderr, dcc::fmt::format("[{}]:", dcc::detail::error_stamp),    \
               __VA_ARGS__)
#define dcc_logwar(...)                                                        \
  dcc::flogmsg(stderr, dcc::fmt::format("[{}]:", dcc::detail::warning_stamp),  \
               __VA_ARGS__)
#define dcc_logfat(...)                                                        \
  dcc::flogmsg(stderr, dcc::fmt::format("[{}]:", dcc::detail::fatal_stamp),    \
               __VA_ARGS__)
#define dcc_loginf(...)                                                        \
  dcc::flogmsg(stdout, dcc::fmt::format("[{}]:", dcc::detail::info_stamp),     \
               __VA_ARGS__)

#ifndef NDEBUG
#define dcc_logdbg(...)                                                        \
  dcc::flogmsg(stdout, dcc::fmt::format("[{}]:", dcc::detail::debug_stamp),    \
               __VA_ARGS__)
#else
#define dcc_logdbg(...) (void)(0)
#endif
#endif

namespace dcc {

  namespace detail {

    constexpr size_t date_strlen = 32;
    constexpr auto fatal_stamp =
      fmt::styled("FATAL", fmt::emphasis::bold | fmt::fg(fmt::color::red));
    constexpr auto error_stamp = fmt::styled("ERROR", fmt::fg(fmt::color::red));
    constexpr auto warning_stamp =
      fmt::styled("WARNING", fmt::fg(fmt::color::yellow));
    constexpr auto info_stamp = fmt::styled("INFO", fmt::fg(fmt::color::white));
    constexpr auto debug_stamp =
      fmt::styled("DEBUG", fmt::fg(fmt::color::orange));

  }; // namespace detail

  namespace sgr {

    void clear(std::string&);

    size_t length(std::string_view);

    template <typename T> std::string good(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::green)));
    }

    template <typename T> std::string bad(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::red)));
    }

    template <typename T> std::string file(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::dark_gray)));
    }

    template <typename T> std::string unique(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::cyan)));
    }

    template <typename T> std::string problem(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::yellow)));
    }

    template <typename T> std::string semiunique(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::fg(fmt::color::bisque)));
    }

  }; // namespace sgr

  namespace logger {

    void use_timestamps(bool);
    bool use_timestamps();

  }; // namespace logger

  void _write_lock();
  void _write_unlock();
  void _clear_sgr(std::string&);

  template <typename... T>
  void flogmsg(FILE* buf, std::string_view prefix,
               const fmt::format_string<T...>& format, T&&... args) {
#ifndef DCC_DISABLE_LOGGER
    std::string msg;
    if (logger::use_timestamps()) {
      char date[detail::date_strlen];
      time_t t = time(0);
      struct tm* tm;
      tm = gmtime(&t);
      strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);
      msg = fmt::format("[{}] {}{}", fmt::format(fg(fmt::color::gray), date),
                        prefix, fmt::format(format, std::forward<T>(args)...));
    } else
      msg = fmt::format("{} {}", prefix,
                        fmt::format(format, std::forward<T>(args)...));

    // Check whether or not the standard output buffers are being redirected. If
    // they are, we should clear the message of any SGR codes. We should also do
    // this in the case the buffer we print to isn't a console buffer.
    if (buf == stdout or buf == stderr) {
      if (not dcc_isatty(dcc_fileno(buf)))
        sgr::clear(msg);
    } else
      sgr::clear(msg);
    _write_lock();
    fmt::print(buf, "{}\n", msg);
    _write_unlock();
#endif
  }

  FILE* get_current_log_file(const char* dir);

}; // namespace dcc

#endif
