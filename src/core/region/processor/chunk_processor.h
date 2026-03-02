// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <fstream>
#include <string_view>

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

  void init(i32 rx,
            i32 rz,
            std::fstream* src,
            std::fstream* dest,
            bool verbose);

  bool process(i32 cx, i32 cz);

 private:
  i32 chunk_index(i32 chunk_x, i32 chunk_z);

  i32 rx_;
  i32 rz_;
  std::fstream* src_ = nullptr;
  std::fstream* dest_ = nullptr;
  bool verbose_ = false;
};

}  // namespace core
