// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/logger.h"

#include "fpag/logging/sync/sync_logger.h"
#include "fpag/mem/page_allocator.h"
#include "fpag/term/console.h"

namespace core {

Logger logger;

void init_logger(term::ColorStyle color_style) {
  logger.init(logging::StdoutSink(
      static_cast<char*>(mem::allocate_pages(mem::page_size())),
      mem::page_size(), color_style, true));
}

}  // namespace core
