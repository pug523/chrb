// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/processor/chunk_processor.h"

#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/check.h"
#include "core/core.h"
#include "core/file_util.h"
#include "core/region/build_sector_map.h"
#include "core/region/find_free_sector.h"
#include "core/region/location.h"

namespace core {

void ChunkProcessor::init(i32 rx,
                          i32 rz,
                          std::fstream* src,
                          std::fstream* dest,
                          bool verbose) {
  rx_ = rx;
  rz_ = rz;
  src_ = src;
  dest_ = dest;
  verbose_ = verbose;

  dcheck(src_);
  dcheck(dest_);
  dcheck(*src_);
  dcheck(*dest_);
}

bool ChunkProcessor::process(i32 cx, i32 cz) {
  const i32 idx = chunk_index(cx, cz);

  const LocationEntry src_loc = read_location(*src_, idx);
  const LocationEntry dest_loc = read_location(*dest_, idx);

  if (src_loc.offset == 0 && dest_loc.offset == 0) {
    // src and dest do not exist: chunk missing
    if (verbose_) {
      std::println(
          "({:4}, {:4}) was ignored because both src and dest did not exist",
          cx, cz);
    }
    return true;
  } else if (src_loc.offset == 0 && dest_loc.offset != 0) {
    // src not exists and dest exists: delete the chunk
    dest_->seekp(idx * 4);
    constexpr u8 kZero[4] = {0, 0, 0, 0};
    dest_->write(reinterpret_cast<const char*>(kZero), 4);
    if (verbose_) {
      std::println("({:4}, {:4}) was deleted because src did not exist", cx,
                   cz);
    }
    return true;
  } else if (src_loc.offset != 0 && dest_loc.offset == 0) {
    const size_t bytes = src_loc.sectors * kSectorSize;
    std::vector<char> buffer(bytes);

    src_->seekg(src_loc.offset * kSectorSize);
    src_->read(buffer.data(), static_cast<std::streamsize>(bytes));

    const std::vector<bool> used = build_sector_map(*dest_);

    i32 new_offset = find_free_sector(used, src_loc.sectors);

    if (new_offset < 0) {
      dest_->seekp(0, std::ios::end);
      new_offset = static_cast<int>(dest_->tellp()) / kSectorSize;
    }

    dest_->seekp(new_offset * kSectorSize);
    dest_->write(buffer.data(), static_cast<std::streamsize>(bytes));

    // update location table
    u8 loc[4];
    loc[0] = static_cast<u8>((new_offset >> 16) & 0xFF);
    loc[1] = static_cast<u8>((new_offset >> 8) & 0xFF);
    loc[2] = static_cast<u8>(new_offset & 0xFF);
    loc[3] = static_cast<u8>(src_loc.sectors);

    dest_->seekp(idx * 4);
    dest_->write(reinterpret_cast<const char*>(loc), 4);

    // update timestamp
    const u32 now = static_cast<u32>(time(nullptr));
    const u8 ts[4] = {u8(now >> 24), u8(now >> 16), u8(now >> 8), u8(now)};

    dest_->seekp(4096 + idx * 4);
    dest_->write(reinterpret_cast<const char*>(ts), 4);

    if (verbose_) {
      std::println("({:4}, {:4}) was added", cx, cz);
    }
    return true;
  } else if (src_loc.offset != 0 && dest_loc.offset != 0) {
    const std::streamsize src_bytes = src_loc.sectors * kSectorSize;
    // const std::streamsize dest_bytes = dest_loc.sectors * kSectorSize;

    std::vector<char> buffer(static_cast<size_t>(src_bytes));

    src_->seekg(src_loc.offset * kSectorSize);
    src_->read(buffer.data(), src_bytes);

    if (src_loc.sectors <= dest_loc.sectors) {
      // can be overwritten as is
      dest_->seekp(dest_loc.offset * kSectorSize);
      dest_->write(buffer.data(), src_bytes);

      if (verbose_) {
        std::println("({:4}, {:4}) was overwritten", cx, cz);
      }
    } else {
      // relocation
      std::vector<bool> used = build_sector_map(*dest_);

      // free old sectors
      for (u8 s = 0; s < dest_loc.sectors; ++s) {
        if (dest_loc.offset + s < used.size()) {
          used[dest_loc.offset + s] = false;
        }
      }

      std::streamoff new_offset = find_free_sector(used, src_loc.sectors);

      if (new_offset < 0) {
        dest_->seekp(0, std::ios::end);
        new_offset = dest_->tellp() / kSectorSize;
      }

      // write to new location
      dest_->seekp(new_offset * kSectorSize);
      dest_->write(buffer.data(), src_bytes);

      // update location table
      u8 loc[4];
      loc[0] = static_cast<u8>((new_offset >> 16) & 0xFF);
      loc[1] = static_cast<u8>((new_offset >> 8) & 0xFF);
      loc[2] = static_cast<u8>(new_offset & 0xFF);
      loc[3] = static_cast<u8>(src_loc.sectors);

      dest_->seekp(idx * 4);
      dest_->write(reinterpret_cast<char*>(loc), 4);

      if (verbose_) {
        std::println("({:4}, {:4}) was relocated", cx, cz);
      }
    }

    // update timestamp
    const uint32_t now = static_cast<u32>(time(nullptr));
    u8 ts[4] = {u8(now >> 24), u8(now >> 16), u8(now >> 8), u8(now)};

    dest_->seekp(4096 + idx * 4);
    dest_->write(reinterpret_cast<char*>(ts), 4);
    return true;
  } else {
    // unreachable
    dcheck(false);
    return true;
  }
}

inline i32 ChunkProcessor::chunk_index(i32 chunk_x, i32 chunk_z) {
  const i32 local_z = chunk_x & 31;
  const i32 local_x = chunk_z & 31;
  return local_z + (local_x << 5);
}

}  // namespace core
