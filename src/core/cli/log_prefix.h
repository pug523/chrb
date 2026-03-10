// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/core.h"

namespace core {

enum class ColorMode : u8 {
  Auto,
  Always,
  Never,
  Unknown,
};

inline ColorMode str_to_color_mode(const std::string_view s) {
  // only supports lower case
  if (s == "auto") {
    return ColorMode::Auto;
  } else if (s == "always") {
    return ColorMode::Always;
  } else if (s == "never") {
    return ColorMode::Never;
  } else {
    return ColorMode::Unknown;
  }
}

enum class LogLevel : u8 {
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  MaxValue = Fatal,
};

std::string_view log_prefix(ColorMode mode, LogLevel level);

inline std::string_view debug_prefix(ColorMode mode) {
  return log_prefix(mode, LogLevel::Debug);
}
inline std::string_view info_prefix(ColorMode mode) {
  return log_prefix(mode, LogLevel::Info);
}
inline std::string_view warn_prefix(ColorMode mode) {
  return log_prefix(mode, LogLevel::Warn);
}
inline std::string_view error_prefix(ColorMode mode) {
  return log_prefix(mode, LogLevel::Error);
}
inline std::string_view fatal_prefix(ColorMode mode) {
  return log_prefix(mode, LogLevel::Fatal);
}

}  // namespace core
