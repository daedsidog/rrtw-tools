#ifndef DCC_LOGGER_HPP
#define DCC_LOGGER_HPP

#include <dcc/fmt/color.h>
#include <dcc/fmt/format.h>
#include <string>

#ifndef __FILE_STEM__
#define __FILE_STEM__ __FILE__
#endif
#ifndef __FILE_BASE__
#define __FILE_BASE__ __FILE__
#endif
#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#ifdef NDEBUG
#define LOGSRC __FILE_STEM__
#else
#define LOGSRC __FILE_BASE__ ":" STRINGIFY(__LINE__)
#endif

#define MAX_STAMP_STRLEN 64
#define CRT_STAMP                                                              \
  dcc::fmt::styled("FATAL", dcc::fmt::emphasis::bold |                         \
                              dcc::fmt::fg(dcc::fmt::color::red))
#define ERR_STAMP dcc::fmt::styled("ERROR", dcc::fmt::fg(dcc::fmt::color::red))
#define WAR_STAMP                                                              \
  dcc::fmt::styled("WARNING", dcc::fmt::fg(dcc::fmt::color::yellow))
#define INF_STAMP dcc::fmt::styled("INFO", dcc::fmt::fg(dcc::fmt::color::white))
#define DBG_STAMP                                                              \
  dcc::fmt::styled("DEBUG", dcc::fmt::fg(dcc::fmt::color::orange))
#define SRC_STAMP dcc::fmt::styled(LOGSRC, dcc::fmt::fg(dcc::fmt::color::gray))

#define logerr(...)                                                            \
  flogmsg(stderr, dcc::fmt::format("[{}] ({}): ", ERR_STAMP, SRC_STAMP),       \
          __VA_ARGS__)
#define logwar(...)                                                            \
  flogmsg(stderr, dcc::fmt::format("[{}] ({}): ", WAR_STAMP, SRC_STAMP),       \
          __VA_ARGS__)
#define logfat(...)                                                            \
  flogmsg(stderr, dcc::fmt::format("[{}] ({}): ", CRT_STAMP, SRC_STAMP),       \
          __VA_ARGS__)
#define loginf(...)                                                            \
  flogmsg(stdout, dcc::fmt::format("[{}] ({}): ", INF_STAMP, SRC_STAMP),       \
          __VA_ARGS__)

#ifndef NDEBUG
#define logdbg(...)                                                            \
  flogmsg(stdout, dcc::fmt::format("[{}] ({}): ", DBG_STAMP, SRC_STAMP),       \
          __VA_ARGS__)
#else
#define logdbg(...) (void)(0)
#endif

namespace dcc {

  void _append_timestamp(FILE* buf);
  void _write_lock();
  void _write_unlock();

  template <typename... T>
  void flogmsg(FILE* buf, std::string_view prefix,
               fmt::format_string<T...> format, T... args) {

#ifndef DCC_DISABLE_LOGGER
    _write_lock();
    _append_timestamp(buf);
    fprintf(buf, "%s", prefix.data());
    fmt::print(buf, format, std::forward<T>(args)...);
    fprintf(buf, "\n");
    _write_unlock();
#endif
  }

  template <typename... T>
  void logmsg(fmt::format_string<T...> format, T... args) {
    flogmsg(stdout, "", format, std::forward<T>(args)...);
  }

  FILE* get_current_log_file(const char* dir);

  namespace sgr {

    template <typename T> std::string file(T t) {
      return fmt::format("{}", fmt::styled(t, fmt::emphasis::underline));
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

}; // namespace dcc

#endif
