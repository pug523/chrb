// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/processor/full_region_processor.h"

#include <sys/types.h>

#include <cstddef>
#include <cstring>
#include <print>
#include <string>
#include <string_view>

#include "core/core.h"

#ifdef IS_PLAT_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "core/check.h"
#include "core/cli/log_prefix.h"
#include "core/file_util.h"
#include "core/mem/mapped_file.h"

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
  MappedFile src_map;
  if (!src_map.open(src_)) [[unlikely]] {
    std::println(stderr, "{}failed to open src: {} (in {},{})", error_prefix(),
                 src_, rx_, rz_);
    return false;
  }

  const size_t file_size = src_map.size();

  std::string tmp{dest_};
  tmp.append(".tmp");

  {
#ifdef IS_PLAT_WINDOWS
    void* fh =
        CreateFileA(tmp.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fh == INVALID_HANDLE_VALUE) [[unlikely]] {
      std::println(stderr, "{}failed to create tmp: {} (in {},{})",
                   error_prefix(), tmp, rx_, rz_);
      return false;
    }
    CloseHandle(fh);
#else
    const i32 fd = ::open(tmp.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) [[unlikely]] {
      std::println(stderr, "{}failed to create tmp: {} (in {},{})",
                   error_prefix(), tmp, rx_, rz_);
      return false;
    }
    if (ftruncate(fd, static_cast<off_t>(file_size)) < 0) [[unlikely]] {
      ::close(fd);
      std::println(stderr, "{}failed to resize tmp: {} (in {},{})",
                   error_prefix(), tmp, rx_, rz_);
      return false;
    }
    ::close(fd);
#endif
  }

  MappedFile tmp_map;
  if (!tmp_map.open(tmp, file_size)) [[unlikely]] {
    std::println(stderr, "{}failed to open tmp: {} (in {},{})", error_prefix(),
                 tmp, rx_, rz_);
    return false;
  }

#ifndef IS_PLAT_WINDOWS
  madvise(src_map.data(), file_size, MADV_SEQUENTIAL);
  madvise(tmp_map.data(), file_size, MADV_SEQUENTIAL);
#endif

  std::memcpy(tmp_map.data(), src_map.data(), file_size);

#ifndef IS_PLAT_WINDOWS
  // flush explicitly before renaming for clush tolerance
  msync(tmp_map.data(), file_size, MS_SYNC);
#endif

  tmp_map.close();
  src_map.close();

  if (!rename_file(tmp, dest_)) [[unlikely]] {
    std::println(stderr, "{}failed to rename {} to {} (in {},{})",
                 error_prefix(), tmp, dest_, rx_, rz_);
    return false;
  }

  return true;
}

}  // namespace core
