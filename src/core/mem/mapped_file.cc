// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/mem/mapped_file.h"

#include <print>

#include "core/cli/log_prefix.h"

#ifdef IS_PLAT_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace core {

bool MappedFile::open(std::string_view path, size_t min_size) {
  close();
  path_ = path;

#ifdef IS_PLAT_WINDOWS
  file_handle_ =
      CreateFileA(path_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }

  LARGE_INTEGER file_size;
  if (!GetFileSizeEx(file_handle_, &file_size)) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }
  size_ = static_cast<size_t>(file_size.QuadPart);

  if (min_size > 0 && size_ < min_size) {
    LARGE_INTEGER new_size;
    new_size.QuadPart = static_cast<LONGLONG>(min_size);
    if (!SetFilePointerEx(file_handle_, new_size, nullptr, FILE_BEGIN) ||
        !SetEndOfFile(file_handle_)) {
      CloseHandle(file_handle_);
      file_handle_ = INVALID_HANDLE_VALUE;
      return false;
    }
    size_ = min_size;
  }

  mapping_handle_ =
      CreateFileMappingA(file_handle_, nullptr, PAGE_READWRITE, 0, 0, nullptr);
  if (!mapping_handle_) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }

  data_ = static_cast<u8*>(
      MapViewOfFile(mapping_handle_, FILE_MAP_ALL_ACCESS, 0, 0, 0));
  if (!data_) {
    CloseHandle(mapping_handle_);
    mapping_handle_ = nullptr;
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }

#else
  fd_ = ::open(path_.c_str(), O_RDWR);
  if (fd_ < 0) {
    std::println(stderr, "{}failed to open the file: {}", error_prefix(), path);
    return false;
  }

  struct stat st;
  if (fstat(fd_, &st) < 0) {
    ::close(fd_);
    fd_ = -1;
    std::println(stderr, "{}failed to get the attributes for the file: {}",
                 error_prefix(), path);
    return false;
  }
  size_ = static_cast<size_t>(st.st_size);

  if (min_size > 0 && size_ < min_size) {
    if (ftruncate(fd_, static_cast<off_t>(min_size)) < 0) {
      ::close(fd_);
      fd_ = -1;
      std::println(stderr, "{}failed to truncate the file: {}", error_prefix(),
                   path);
      return false;
    }
    size_ = min_size;
  }

  data_ = static_cast<u8*>(
      mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  if (data_ == MAP_FAILED) {
    data_ = nullptr;
    ::close(fd_);
    fd_ = -1;
    std::println(stderr, "{}failed to map the file to memory: {}",
                 error_prefix(), path);
    return false;
  }
#endif

  return true;
}

void MappedFile::close() {
  if (!data_) {
    return;
  }
#ifdef IS_PLAT_WINDOWS
  UnmapViewOfFile(data_);
  CloseHandle(mapping_handle_);
  CloseHandle(file_handle_);
#else
  munmap(data_, size_);
  ::close(fd_);
#endif
  data_ = nullptr;
}

bool MappedFile::resize(size_t new_size) {
  if (!is_open()) {
    return false;
  }

#ifdef IS_PLAT_WINDOWS
  UnmapViewOfFile(data_);
  data_ = nullptr;
  CloseHandle(mapping_handle_);
  mapping_handle_ = nullptr;

  LARGE_INTEGER li;
  li.QuadPart = static_cast<LONGLONG>(new_size);
  if (!SetFilePointerEx(file_handle_, li, nullptr, FILE_BEGIN) ||
      !SetEndOfFile(file_handle_)) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    size_ = 0;
    return false;
  }
  size_ = new_size;

  mapping_handle_ =
      CreateFileMappingA(file_handle_, nullptr, PAGE_READWRITE, 0, 0, nullptr);
  if (!mapping_handle_) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    size_ = 0;
    return false;
  }

  data_ = static_cast<u8*>(
      MapViewOfFile(mapping_handle_, FILE_MAP_ALL_ACCESS, 0, 0, 0));
  if (!data_) {
    CloseHandle(mapping_handle_);
    mapping_handle_ = nullptr;
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
    size_ = 0;
    return false;
  }

#else
  munmap(data_, size_);
  data_ = nullptr;

  if (ftruncate(fd_, static_cast<off_t>(new_size)) < 0) {
    ::close(fd_);
    fd_ = -1;
    size_ = 0;
    return false;
  }
  size_ = new_size;

  data_ = static_cast<u8*>(
      mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  if (data_ == MAP_FAILED) {
    data_ = nullptr;
    ::close(fd_);
    fd_ = -1;
    size_ = 0;
    return false;
  }
#endif

  return true;
}

}  // namespace core
