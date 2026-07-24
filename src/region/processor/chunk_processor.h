// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>

#include "core/mem/mapped_file.h"
#include "fpag/base/numeric.h"
#include "region/location.h"
#include "region/rollback_config.h"

namespace region {

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
            core::MappedFile* src,
            core::MappedFile* dest,
            RollbackConfig* config);

  // TODO: error handling is maybe needed?
  void process(i32 cx, i32 cz);

 private:
  void ignore_chunk(const i32 cx, const i32 cz);
  void add_chunk(const i32 cx,
                 const i32 cz,
                 const i32 index,
                 const LocationEntry src_loc);
  void delete_chunk(const i32 cx, const i32 cz, const i32 index);
  void replace_chunk(const i32 cx,
                     const i32 cz,
                     const i32 index,
                     const LocationEntry src_loc,
                     const LocationEntry dest_loc);

  i32 chunk_index(i32 chunk_x, i32 chunk_z);
  void update_location_table(size_t index, u8 sectors, i32 new_offset);
  void update_timestamp(size_t index);

  i32 rx_;
  i32 rz_;
  core::MappedFile* src_ = nullptr;
  core::MappedFile* dest_ = nullptr;
  RollbackConfig* config_ = nullptr;
};

}  // namespace region
