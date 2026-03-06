// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>

#include "core/core.h"
#include "core/mem/mapped_file.h"

namespace core {

class ChunkProcessor {
 public:
  ChunkProcessor() = default;
  ~ChunkProcessor() = default;

  ChunkProcessor(const ChunkProcessor&) = delete;
  ChunkProcessor& operator=(const ChunkProcessor&) = delete;

  ChunkProcessor(ChunkProcessor&&) noexcept = default;
  ChunkProcessor& operator=(ChunkProcessor&&) noexcept = default;

  void init(i32 rx, i32 rz, MappedFile* src, MappedFile* dest, bool verbose);

  // TODO: error handling is maybe needed?
  void process(i32 cx, i32 cz);

 private:
  i32 chunk_index(i32 chunk_x, i32 chunk_z);
  void update_location_table(size_t index, u8 sectors, i32 new_offset);
  void update_timestamp(size_t index);

  i32 rx_;
  i32 rz_;
  MappedFile* src_ = nullptr;
  MappedFile* dest_ = nullptr;
  bool verbose_ = false;
};

}  // namespace core
