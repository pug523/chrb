// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "core/core.h"

namespace region {

class FullRegionProcessor {
 public:
  FullRegionProcessor() = default;
  ~FullRegionProcessor() = default;

  FullRegionProcessor(const FullRegionProcessor&) = delete;
  FullRegionProcessor& operator=(const FullRegionProcessor&) = delete;

  FullRegionProcessor(FullRegionProcessor&&) noexcept = default;
  FullRegionProcessor& operator=(FullRegionProcessor&&) noexcept = default;

  void init(bool verbose);

#ifdef IS_PLAT_LINUX
  size_t process_batch(const std::vector<std::string>& srcs,
                       const std::vector<std::string>& dsts,
                       u32 queue_depth,
                       const std::function<void(size_t)>& success_cb,
                       const std::function<void(size_t)>& failure_cb);
#endif

  bool process_one(const i32 rx,
                   const i32 rz,
                   const std::string_view src,
                   const std::string_view dest);

 private:
  bool verbose_;
};

}  // namespace region
