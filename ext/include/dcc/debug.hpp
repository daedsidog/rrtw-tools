#ifndef DCC_DEBUG_HPP
#define DCC_DEBUG_HPP

#include <dcc/logger.hpp>

#ifndef DCC_SRC_STAMP
#define DCC_SRC_STAMP "EYO"
#endif

#ifdef NDEBUG
#define dcc_assert(condition) ((void)0)
#else
#define dcc_assert(condition)                                                  \
  do {                                                                         \
    if (not(condition)) {                                                      \
      flogmsg(                                                                 \
        stderr,                                                                \
        dcc::fmt::format(                                                      \
          "[{}] ({}): ",                                                       \
          dcc::fmt::format(dcc::fmt::fg(dcc::fmt::color::yellow), "ASSERT"),   \
          DCC_SRC_STAMP),                                                      \
        "Assertion {} failed.",                                                \
        dcc::fmt::format(dcc::fmt::fg(dcc::fmt::color::yellow), #condition));  \
      exit(-1);                                                                \
    }                                                                          \
  } while (0)
#endif

#endif
