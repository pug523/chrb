// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string_view>

#include "core/core.h"

namespace core {

class FullRegionProcessor {
 public:
  FullRegionProcessor() = default;
  ~FullRegionProcessor() = default;

  FullRegionProcessor(const FullRegionProcessor&) = delete;
  FullRegionProcessor& operator=(const FullRegionProcessor&) = delete;

  FullRegionProcessor(FullRegionProcessor&&) noexcept = default;
  FullRegionProcessor& operator=(FullRegionProcessor&&) noexcept = default;

  void init(i32 rx,
            i32 rz,
            std::string_view src,
            std::string_view dest,
            bool verbose);

  bool process();

 private:
  i32 rx_;
  i32 rz_;
  std::string_view src_;
  std::string_view dest_;
  bool verbose_;
};

}  // namespace core
