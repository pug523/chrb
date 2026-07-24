// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/logger.h"

#include "fpag/base/console.h"
#include "fpag/logging/sync/sync_logger.h"
#include "fpag/mem/page_allocator.h"

namespace core {

Logger logger;

void init_logger(base::ColorStyle color_style) {
  logger.init(logging::StdoutSink(
      static_cast<char*>(mem::allocate_pages(mem::kPageSize)), mem::kPageSize,
      color_style, true));
}

}  // namespace core
