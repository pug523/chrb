// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/file_util.h"

#include <sys/stat.h>

#include <string_view>

#include "core/core.h"

namespace core {

namespace {

inline bool exists_as(const std::string_view path, u32 mask) {
  struct stat info;
  if (stat(path.data(), &info) != 0) {
    return false;
  }
  return (info.st_mode & mask) != 0;
}

}  // namespace

bool is_dir(const std::string_view path) {
  return exists_as(path, S_IFDIR);
}

bool is_file(const std::string_view path) {
  return exists_as(path, S_IFREG);
}

}  // namespace core
