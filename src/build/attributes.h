// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include "build/build_config.h"

#if CHRB_BUILD_FLAG(IS_COMPILER_GCC)
#define CHRB_ASSUME(expr) __builtin_assume(static_cast<bool>(expr))
#elif CHRB_BUILD_FLAG(IS_COMPILER_MSVC)
#define CHRB_ASSUME(expr) __assume(static_cast<bool>(expr))
#else
#define CHRB_ASSUME(expr)
#endif

#if CHRB_BUILD_FLAG(IS_COMPILER_GCC)
#define CHRB_COLD [[gnu::cold]]
#elif CHRB_BUILD_FLAG(IS_COMPILER_MSVC)
#define CHRB_COLD
#else
#define CHRB_COLD
#endif

#if CHRB_BUILD_FLAG(IS_COMPILER_GCC)
#define CHRB_NOINLINE __attribute__((noinline))
#elif CHRB_BUILD_FLAG(IS_COMPILER_MSVC)
#define CHRB_NOINLINE __declspec(noinline)
#else
#define CHRB_NOINLINE
#endif

#if CHRB_BUILD_FLAG(IS_COMPILER_GCC)
#define CHRB_VISIBLE __attribute__((visibility("default")))
#elif CHRB_BUILD_FLAG(IS_COMPILER_MSVC)
#define CHRB_VISIBLE __declspec(dllexport)
#else
#define CHRB_VISIBLE
#endif
