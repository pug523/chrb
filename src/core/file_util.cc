// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/file_util.h"

#ifdef IS_PLAT_WINDOWS
#ifdef _CRT_SECURE_NO_WARNINGS
#undef _CRT_SECURE_NO_WARNINGS
#endif  // _CRT_SECURE_NO_WARNINGS
#ifdef _CRT_NONSTDC_NO_WARNINGS
#undef _CRT_NONSTDC_NO_WARNINGS
#endif  // _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>

#include "core/core.h"

#ifdef IS_PLAT_WINDOWS
#include <io.h>
#include <windows.h>
#define stat _stat
#define close _close
#define read _read
#define write _write
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define ssize_t SSIZE_T
#else
#include <unistd.h>
#endif

#include <string_view>

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
#ifdef IS_PLAT_WINDOWS
  i32 fd;
  errno_t err =
      _sopen_s(&fd, path.data(), O_CREAT | O_WRONLY | O_TRUNC, _SH_DENYNO, 0);
  if (err != 0) {
    return false;
  }
#else
  i32 fd = open(path.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
  if (fd == -1) {
    return false;
  }
  close(fd);
  return true;
}

bool copy_file(const std::string_view from, const std::string_view to) {
#ifdef IS_PLAT_WINDOWS
  i32 source;
  errno_t err = _sopen_s(&source, from.data(), O_RDONLY, _SH_DENYNO, 0);
  if (err != 0) {
    return false;
  }
#else
  i32 source = open(from.data(), O_RDONLY);
#endif
  if (source == -1) {
    return false;
  }

#ifdef IS_PLAT_WINDOWS
  i32 dest;
  errno_t err2 = _sopen_s(&dest, to.data(), O_CREAT | O_WRONLY | O_TRUNC,
                          _SH_DENYNO, S_IRUSR | S_IWUSR);
  if (err2 != 0) {
    return false;
  }
#else
  i32 dest = open(to.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
  if (dest == -1) {
    close(source);
    return false;
  }

  char buffer[4096];
  ssize_t bytes;
  while ((bytes = read(source, buffer, sizeof(buffer))) > 0) {
    if (write(dest, buffer, static_cast<u32>(bytes)) != bytes) {
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

