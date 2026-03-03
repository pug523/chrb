// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/core.h"

namespace core {

enum class LogLevel : u8 {
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  MaxValue = Fatal,
};

std::string_view log_prefix(LogLevel level);

inline std::string_view debug_prefix() {
  return log_prefix(LogLevel::Debug);
}
inline std::string_view info_prefix() {
  return log_prefix(LogLevel::Info);
}
inline std::string_view warn_prefix() {
  return log_prefix(LogLevel::Warn);
}
inline std::string_view error_prefix() {
  return log_prefix(LogLevel::Error);
}
inline std::string_view fatal_prefix() {
  return log_prefix(LogLevel::Fatal);
}

}  // namespace core
