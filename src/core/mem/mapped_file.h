// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstring>
#include <string>
#include <string_view>

#include "core/check.h"
#include "core/core.h"

namespace core {

class MappedFile {
 public:
  MappedFile() = default;
  ~MappedFile() { close(); }

  MappedFile(const MappedFile&) = delete;
  MappedFile& operator=(const MappedFile&) = delete;

  MappedFile(MappedFile&&) = default;
  MappedFile& operator=(MappedFile&&) = default;

  bool open(std::string_view path, size_t min_size = 0);
  void close();

  bool resize(size_t new_size);

  inline void read(size_t offset, void* dst, size_t len) const {
    dcheck(offset + len <= size_);
    std::memcpy(dst, data_ + offset, len);
  }

  inline void write(size_t offset, const void* src, size_t len) {
    dcheck(offset + len <= size_);
    std::memcpy(data_ + offset, src, len);
  }

  inline u8* data() { return data_; }
  inline size_t size() const { return size_; }
  inline bool is_open() const { return data_ != nullptr; }

 private:
  u8* data_ = nullptr;
  size_t size_ = 0;
  std::string path_;

#ifdef IS_PLAT_WINDOWS
  void* file_handle_ = reinterpret_cast<void*>(-1);  // INVALID_HANDLE_VALUE
  void* mapping_handle_ = nullptr;
#else
  i32 fd_ = -1;
#endif
};

}  // namespace core
