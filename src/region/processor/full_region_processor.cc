// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/processor/full_region_processor.h"

#include <sys/types.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "build/build_config.h"
#include "core/file_util.h"
#include "core/logger.h"
#include "core/mem/mapped_file.h"
#include "region/mca_util.h"
#include "region/rollback_config.h"

#if CHRB_BUILD_FLAG(IS_OS_WIN)
#include <windows.h>
#elif CHRB_BUILD_FLAG(IS_OS_POSIX)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#else
#error "Unsupported platform"
#endif

#if CHRB_BUILD_FLAG(IS_OS_WIN)
#include <functional>
#include <vector>

#include "core/check.h"
#endif

namespace region {

void FullRegionProcessor::init(RollbackConfig* config) {
  config_ = config;
  DCHECK(config_);
}

#if CHRB_BUILD_FLAG(IS_OS_LINUX)
size_t FullRegionProcessor::process_batch(
    const std::vector<std::string>& srcs,
    const std::vector<std::string>& dsts,
    u32 queue_depth,
    const std::function<void(size_t)>& success_cb,
    const std::function<void(size_t)>& failure_cb) {
  DCHECK(srcs.size() == dsts.size());

  if (srcs.empty()) {
    return 0;
  }

  // async_copy_files returns a bitmask: bit set = failed
  const std::vector<u64> failed_mask =
      core::async_copy_files(srcs, dsts, queue_depth);

  size_t ok = 0;
  for (size_t i = 0; i < srcs.size(); ++i) {
    if ((failed_mask[i / 64] >> (i % 64)) & u64{1}) {
      failure_cb(i);
    } else {
      success_cb(i);
      ++ok;
    }
  }
  return ok;
}
#endif

bool FullRegionProcessor::process_one(const i32 rx,
                                      const i32 rz,
                                      const std::string_view src,
                                      const std::string_view dest) {
  core::MappedFile src_map;
  if (!src_map.open(src, kMcaHeaderSize)) [[unlikely]] {
    core::logger.error("failed to open src: {} (in r.{}.{})", src, rx, rz);
    return false;
  }

  const size_t file_size = src_map.size();

  std::string tmp{dest};
  tmp.append(".tmp");

  {
#if CHRB_BUILD_FLAG(IS_OS_WIN)
    void* fh =
        CreateFileA(tmp.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fh == INVALID_HANDLE_VALUE) [[unlikely]] {
      core::logger.error("failed to create tmp: {} (in r.{}.{})", tmp, rx, rz);
      return false;
    }
    CloseHandle(fh);
#elif CHRB_BUILD_FLAG(IS_OS_POSIX)
    const i32 fd = ::open(tmp.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) [[unlikely]] {
      core::logger.error("failed to create tmp: {} (in r.{}.{})", tmp, rx, rz);
      return false;
    }
    if (ftruncate(fd, static_cast<off_t>(file_size)) < 0) [[unlikely]] {
      ::close(fd);
      core::logger.error("failed to resize tmp: {} (in r.{}.{})", tmp, rx, rz);
      return false;
    }
    ::close(fd);
#endif
  }

  core::MappedFile tmp_map;
  if (!tmp_map.open(tmp, file_size)) [[unlikely]] {
    core::logger.error("failed to open tmp: {} (in r.{}.{})", tmp, rx, rz);
    return false;
  }

#if CHRB_BUILD_FLAG(IS_OS_POSIX)
  madvise(src_map.data(), file_size, MADV_SEQUENTIAL);
  madvise(tmp_map.data(), file_size, MADV_SEQUENTIAL);
#endif

  std::memcpy(tmp_map.data(), src_map.data(), file_size);

#if CHRB_BUILD_FLAG(IS_OS_POSIX)
  // flush explicitly before renaming for clush tolerance
  msync(tmp_map.data(), file_size, MS_SYNC);
#endif

  tmp_map.close();
  src_map.close();

  if (!core::rename_file(tmp, dest)) [[unlikely]] {
    core::logger.error("failed to rename {} to {} (in r.{}.{})", tmp, dest, rx,
                       rz);
    return false;
  }

  return true;
}

}  // namespace region
