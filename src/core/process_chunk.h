// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>

#include "core/core.h"

namespace core {

class ChunkProcessor {
 public:
  ChunkProcessor() = default;
  ~ChunkProcessor() = default;

  ChunkProcessor(const ChunkProcessor&) = delete;
  ChunkProcessor& operator=(const ChunkProcessor&) = delete;

  ChunkProcessor(ChunkProcessor&&) noexcept = default;
  ChunkProcessor& operator=(ChunkProcessor&&) noexcept = default;

  void process(i32 cx, i32 cz);

  std::string_view src_dir;
  std::string_view dest_dir;
};

void process_chunk(i32 cx,
                   i32 cz,
                   const std::string& src_dir,
                   const std::string& dest_dir);

}  // namespace core
