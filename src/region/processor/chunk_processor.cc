// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/processor/chunk_processor.h"

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <print>
#include <vector>

#include "core/check.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "core/mem/mapped_file.h"
#include "region/build_sector_map.h"
#include "region/find_free_sector.h"
#include "region/location.h"

namespace core {

void ChunkProcessor::init(i32 rx,
                          i32 rz,
                          MappedFile* src,
                          MappedFile* dest,
                          bool verbose) {
  rx_ = rx;
  rz_ = rz;
  src_ = src;
  dest_ = dest;
  verbose_ = verbose;

  dcheck(src_);
  dcheck(dest_);
  dcheck(src_->is_open());
  dcheck(dest_->is_open());
}

void ChunkProcessor::process(i32 cx, i32 cz) {
  const i32 index = chunk_index(cx, cz);

  const LocationEntry src_loc = read_location(*src_, index);
  const LocationEntry dest_loc = read_location(*dest_, index);

  if (src_loc.offset == 0 && dest_loc.offset == 0) {
    // src and dest do not exist: chunk missing
    if (verbose_) {
      std::println(
          "{}c({:4}, {:4}) ignored because both src and dest data did not "
          "exist",
          debug_prefix(), cx, cz);
    }
    return;
  } else if (src_loc.offset == 0 && dest_loc.offset != 0) {
    // src does not exist and dest exists: delete the chunk
    constexpr const u8 kZero[4] = {0, 0, 0, 0};
    dest_->write(static_cast<size_t>(index) * 4, kZero, 4);
    if (verbose_) {
      std::println("{}c({:4}, {:4}) deleted because src did not exist",
                   debug_prefix(), cx, cz);
    }
    return;
  } else if (src_loc.offset != 0 && dest_loc.offset == 0) {
    const size_t bytes = src_loc.sectors * kSectorSize;
    std::vector<char> buffer(bytes);
    src_->read(static_cast<size_t>(src_loc.offset) * kSectorSize, buffer.data(),
               bytes);

    const std::vector<bool> used = build_sector_map(*dest_);
    i32 new_offset = find_free_sector(used, src_loc.sectors);

    if (new_offset < 0) {
      new_offset = static_cast<i32>(dest_->size() / kSectorSize);
      dest_->resize(dest_->size() + bytes);
    }

    dest_->write(static_cast<size_t>(new_offset) * kSectorSize, buffer.data(),
                 bytes);

    update_location_table(static_cast<size_t>(index), src_loc.sectors,
                          new_offset);
    update_timestamp(static_cast<size_t>(index));

    if (verbose_) {
      std::println("{}c({:4}, {:4}) added", debug_prefix(), cx, cz);
    }
    return;
  } else if (src_loc.offset != 0 && dest_loc.offset != 0) {
    const size_t src_bytes = src_loc.sectors * kSectorSize;
    std::vector<u8> buffer(src_bytes);
    src_->read(static_cast<size_t>(src_loc.offset) * kSectorSize, buffer.data(),
               src_bytes);

    if (src_loc.sectors <= dest_loc.sectors) [[likely]] {
      // can be overwritten as is
      dest_->write(static_cast<size_t>(dest_loc.offset) * kSectorSize,
                   buffer.data(), src_bytes);

      // no need to update location table

      if (verbose_) {
        std::println("{}c({:4}, {:4}) overwritten", debug_prefix(), cx, cz);
      }
    } else {
      // relocation
      std::vector<bool> used = build_sector_map(*dest_);

      // free old sectors
      for (u8 s = 0; s < dest_loc.sectors; ++s) {
        const size_t sector_index = static_cast<size_t>(dest_loc.offset) + s;
        if (sector_index < used.size()) {
          used[sector_index] = false;
        }
      }

      i32 new_offset = find_free_sector(used, src_loc.sectors);

      if (new_offset < 0) {
        new_offset = static_cast<i32>(dest_->size() / kSectorSize);
        dest_->resize(dest_->size() + src_bytes);
      }

      // write to new location
      dest_->write(static_cast<size_t>(new_offset) * kSectorSize, buffer.data(),
                   src_bytes);

      update_location_table(static_cast<size_t>(index), src_loc.sectors,
                            new_offset);

      if (verbose_) {
        std::println("{}c({:4}, {:4}) relocated", debug_prefix(), cx, cz);
      }
    }

    update_timestamp(static_cast<size_t>(index));
    return;
  } else {
    // unreachable
    dcheck(false);
    return;
  }
}

inline i32 ChunkProcessor::chunk_index(i32 chunk_x, i32 chunk_z) {
  const i32 local_x = chunk_x & 31;
  const i32 local_z = chunk_z & 31;
  return local_x + (local_z << 5);
}

void ChunkProcessor::update_location_table(size_t index,
                                           u8 sectors,
                                           i32 new_offset) {
  u8 loc[4];
  loc[0] = static_cast<u8>((new_offset >> 16) & 0xFF);
  loc[1] = static_cast<u8>((new_offset >> 8) & 0xFF);
  loc[2] = static_cast<u8>(new_offset & 0xFF);
  loc[3] = sectors;
  dest_->write(index * 4, loc, 4);
}

void ChunkProcessor::update_timestamp(size_t index) {
  const uint32_t now = static_cast<u32>(time(nullptr));
  u8 ts[4] = {
      u8(now >> 24),
      u8(now >> 16),
      u8(now >> 8),
      u8(now),
  };
  dest_->write(4096 + index * 4, ts, 4);
}

}  // namespace core
