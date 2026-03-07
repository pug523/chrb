// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstdlib>
#include <print>

namespace core {

[[noreturn]] inline void dcheck_fail(const char* expr,
                                     const char* file,
                                     int line,
                                     const char* func) {
  std::print(stderr, "DCHECK failed: {}\n  at {}:{} ({})\n", expr, file, line,
             func);
  std::abort();
}

}  // namespace core

#ifdef DEBUG
#define dcheck(expr)                                            \
  do {                                                          \
    if (__builtin_expect(!(expr), 0)) {                         \
      ::core::dcheck_fail(#expr, __FILE__, __LINE__, __func__); \
    }                                                           \
  } while (false)
#else
#define dcheck(expr) \
  do {               \
    (void)(expr);    \
  } while (false)
#endif
