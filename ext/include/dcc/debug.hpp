#ifndef DCC_DEBUG_HPP
#define DCC_DEBUG_HPP

#include <dcc/logger.hpp>

#ifdef NDEBUG
#define dcc_assert(condition) ((void)0)
#else
#define dcc_assert(condition)                                                  \
  do {                                                                         \
    if (not(condition)) {                                                      \
      logfat("Assertion {} failed.",                                           \
             dcc::fmt::format(dcc::fmt::fg(dcc::fmt::color::yellow), #condition)); \
      exit(-1);                                                                \
    }                                                                          \
  } while (0)
#endif

#endif
