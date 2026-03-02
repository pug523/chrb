// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/cli/colored_prefix.h"

#include <format>

#include "core/cli/console.h"
#include "core/cli/style_util.h"

namespace core {

void ColoredPrefix::init_info() {
  dcheck(!initialized);
  if (can_use_ansi_escape_sequence(Stream::Stderr)) {
    info_formatted = std::format("{}{}info{}: ", kBold, kBrightGreen, kReset);
  } else {
    info_formatted = "info: ";
  }
  initialized = true;
}

void ColoredPrefix::init_warn() {
  dcheck(!initialized);
  if (can_use_ansi_escape_sequence(Stream::Stderr)) {
    warn_formatted = std::format("{}{}warn{}: ", kBold, kYellow, kReset);
  } else {
    warn_formatted = "warn: ";
  }
  initialized = true;
}

void ColoredPrefix::init_error() {
  dcheck(!initialized);
  if (can_use_ansi_escape_sequence(Stream::Stderr)) {
    constexpr const char* kVividRed = "255;10;10m";
    err_formatted =
        std::format("{}{}{}error{}: ", kBold, kFgRgbPrefix, kVividRed, kReset);
  } else {
    err_formatted = "error: ";
  }
  initialized = true;
}

}  // namespace core
