// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/file_util.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>

#ifdef IS_PLAT_WINDOWS
#include <io.h>
#include <windows.h>
#define stat _stat
#define open _open
#define close _close
#define read _read
#define write _write
#else
#include <unistd.h>
#endif

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

bool create_file(const std::string_view path) {
  i32 fd = open(path.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return false;
  }
  close(fd);
  return true;
}

bool copy_file(const std::string_view from, const std::string_view to) {
  i32 source = open(from.data(), O_RDONLY);
  if (source == -1) {
    return false;
  }

  i32 dest = open(to.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  if (dest == -1) {
    close(source);
    return false;
  }

  char buffer[4096];
  ssize_t bytes;
  while ((bytes = read(source, buffer, sizeof(buffer))) > 0) {
    if (write(dest, buffer, static_cast<size_t>(bytes)) != bytes) {
      close(source);
      close(dest);
      return false;
    }
  }

  close(source);
  close(dest);
  return bytes == 0;
}

bool rename_file(const std::string_view from, const std::string_view to) {
  return rename(from.data(), to.data()) == 0;
}

}  // namespace core
