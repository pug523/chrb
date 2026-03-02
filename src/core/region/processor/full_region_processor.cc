// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/processor/full_region_processor.h"

#include <print>
#include <string>
#include <string_view>

#include "core/check.h"
#include "core/cli/colored_prefix.h"
#include "core/file_util.h"

namespace core {

void FullRegionProcessor::init(i32 rx,
                               i32 rz,
                               std::string_view src,
                               std::string_view dest,
                               bool verbose) {
  rx_ = rx;
  rz_ = rz;
  src_ = src;
  dest_ = dest;
  verbose_ = verbose;

  dcheck(src_);
  dcheck(dest_);
}

bool FullRegionProcessor::process() {
  std::string tmp{src_};
  tmp.append(".tmp");

  if (!copy_file(src_, tmp)) [[unlikely]] {
    ColoredPrefix cp;
    cp.init_error();
    std::println(stderr, "{}failed to copy {} to {} (in {},{})", cp.err(), src_,
                 tmp, rx_, rz_);
    return false;
  }
  if (!rename_file(tmp, dest_)) [[unlikely]] {
    ColoredPrefix cp;
    cp.init_error();
    std::println(stderr, "{}failed to rename {} to {} (in {},{})", cp.err(),
                 tmp, dest_, rx_, rz_);
    return false;
  }

  return true;
}

}  // namespace core
