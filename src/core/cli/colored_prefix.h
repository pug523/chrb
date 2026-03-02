// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>
#include <string_view>

#include "core/check.h"

namespace core {

class ColoredPrefix {
 public:
  ColoredPrefix() = default;
  ~ColoredPrefix() = default;

  ColoredPrefix(const ColoredPrefix&) = delete;
  ColoredPrefix& operator=(const ColoredPrefix&) = delete;

  ColoredPrefix(ColoredPrefix&&) noexcept = default;
  ColoredPrefix& operator=(ColoredPrefix&&) noexcept = default;

  void init_info();
  void init_warn();
  void init_error();

  inline std::string_view info() const {
    dcheck(initialized);
    return std::string_view{info_formatted};
  }
  inline std::string_view warn() const {
    dcheck(initialized);
    return std::string_view{warn_formatted};
  }
  inline std::string_view err() const {
    dcheck(initialized);
    return std::string_view{err_formatted};
  }

 private:
  std::string info_formatted;
  std::string warn_formatted;
  std::string err_formatted;
  bool initialized = false;
};

}  // namespace core
