// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/process_chunk.h"

#include <fstream>
#include <print>
#include <string>
#include <utility>
#include <vector>

#include "core/build_sector_map.h"
#include "core/check.h"
#include "core/core.h"
#include "core/file_util.h"
#include "core/find_free_sector.h"
#include "core/location.h"

namespace core {

namespace {

inline i32 chunk_index(i32 chunk_x, i32 chunk_z) {
  const i32 local_z = chunk_x & 31;
  const i32 local_x = chunk_z & 31;
  return local_z + (local_x << 5);
}

}  // namespace

void process_chunk(i32 cx,
                   i32 cz,
                   const std::string& src_dir,
                   const std::string& dest_dir) {
  const i32 region_x = cx >> 5;
  const i32 region_z = cz >> 5;

  const std::string filename =
      "r." + std::to_string(region_x) + '.' + std::to_string(region_z) + ".mca";

  const std::string src_file = src_dir + '/' + filename;
  const std::string dest_file = dest_dir + '/' + filename;

  if (!is_file(src_file) || !is_file(dest_file)) {
    std::println("skipped missing region: {}", filename);
    return;
  }

  std::fstream src(src_file, std::ios::binary | std::ios::in);
  std::fstream dest(dest_file, std::ios::binary | std::ios::in | std::ios::out);

  if (!src || !dest) {
    return;
  }

  const i32 idx = chunk_index(cx, cz);

  const LocationEntry src_loc = read_location(src, idx);
  const LocationEntry dest_loc = read_location(dest, idx);

  if (src_loc.offset == 0 && dest_loc.offset == 0) {
    // src and dest do not exist: chunk missing
    std::println("ignored chunk {},{} because both src and dest did not exist",
                 cx, cz);
    return;
  } else if (src_loc.offset == 0 && dest_loc.offset != 0) {
    // src not exists and dest exists: delete the chunk
    dest.seekp(idx * 4);
    constexpr u8 kZero[4] = {0, 0, 0, 0};
    dest.write(reinterpret_cast<const char*>(kZero), 4);
    std::println("deleted chunk {},{} because src did not exist", cx, cz);
    return;
  } else if (src_loc.offset != 0 && dest_loc.offset == 0) {
    const size_t bytes = src_loc.sectors * kSectorSize;
    std::vector<char> buffer(bytes);

    src.seekg(src_loc.offset * kSectorSize);
    src.read(buffer.data(), static_cast<std::streamsize>(bytes));

    const std::vector<bool> used = build_sector_map(dest);

    i32 new_offset = find_free_sector(used, src_loc.sectors);

    if (new_offset < 0) {
      dest.seekp(0, std::ios::end);
      new_offset = static_cast<int>(dest.tellp()) / kSectorSize;
    }

    dest.seekp(new_offset * kSectorSize);
    dest.write(buffer.data(), static_cast<std::streamsize>(bytes));

    // update location table
    u8 loc[4];
    loc[0] = static_cast<u8>((new_offset >> 16) & 0xFF);
    loc[1] = static_cast<u8>((new_offset >> 8) & 0xFF);
    loc[2] = static_cast<u8>(new_offset & 0xFF);
    loc[3] = static_cast<u8>(src_loc.sectors);

    dest.seekp(idx * 4);
    dest.write(reinterpret_cast<const char*>(loc), 4);

    // update timestamp
    const u32 now = static_cast<u32>(time(nullptr));
    const u8 ts[4] = {u8(now >> 24), u8(now >> 16), u8(now >> 8), u8(now)};

    dest.seekp(4096 + idx * 4);
    dest.write(reinterpret_cast<const char*>(ts), 4);

    std::println("added chunk {},{}", cx, cz);
    return;
  } else if (src_loc.offset != 0 && dest_loc.offset != 0) {
    const std::streamsize src_bytes = src_loc.sectors * kSectorSize;
    // const std::streamsize dest_bytes = dest_loc.sectors * kSectorSize;

    std::vector<char> buffer(static_cast<size_t>(src_bytes));

    src.seekg(src_loc.offset * kSectorSize);
    src.read(buffer.data(), src_bytes);

    if (src_loc.sectors <= dest_loc.sectors) {
      // can be overwritten as is
      dest.seekp(dest_loc.offset * kSectorSize);
      dest.write(buffer.data(), src_bytes);

      std::println("overwritten chunk {},{}", cx, cz);
    } else {
      // relocation
      std::vector<bool> used = build_sector_map(dest);

      // free old sectors
      for (u8 s = 0; s < dest_loc.sectors; ++s) {
        if (dest_loc.offset + s < used.size()) {
          used[dest_loc.offset + s] = false;
        }
      }

      std::streamoff new_offset = find_free_sector(used, src_loc.sectors);

      if (new_offset < 0) {
        dest.seekp(0, std::ios::end);
        new_offset = dest.tellp() / kSectorSize;
      }

      // write to new location
      dest.seekp(new_offset * kSectorSize);
      dest.write(buffer.data(), src_bytes);

      // update location table
      u8 loc[4];
      loc[0] = static_cast<u8>((new_offset >> 16) & 0xFF);
      loc[1] = static_cast<u8>((new_offset >> 8) & 0xFF);
      loc[2] = static_cast<u8>(new_offset & 0xFF);
      loc[3] = static_cast<u8>(src_loc.sectors);

      dest.seekp(idx * 4);
      dest.write(reinterpret_cast<char*>(loc), 4);

      std::println("relocated chunk {},{}", cx, cz);
    }

    // update timestamp
    const uint32_t now = static_cast<u32>(time(nullptr));
    u8 ts[4] = {u8(now >> 24), u8(now >> 16), u8(now >> 8), u8(now)};

    dest.seekp(4096 + idx * 4);
    dest.write(reinterpret_cast<char*>(ts), 4);
  } else {
    // unreachable
    dcheck(false);
  }
}

}  // namespace core
