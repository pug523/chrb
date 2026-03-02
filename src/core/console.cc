// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/console.h"

#include "core/check.h"

#ifdef IS_PLAT_WINDOWS
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace core {

namespace {

bool check_ansi_sequence_available(Stream stream) {
#ifdef IS_PLAT_WINDOWS
  // check if GetStdHandle() returns terminal handle
  DWORD handle = 0;
  if (stream == Stream::Stdout) {
    handle = STD_OUTPUT_HANDLE;
  } else if (stream == Stream::Stderr) {
    handle = STD_ERROR_HANDLE;
  }
  const HANDLE h_out = GetStdHandle(handle);
  if (h_out == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD dw_mode = 0;
  if (!GetConsoleMode(h_out, &dw_mode)) {
    return false;
  }

  // enable virtual terminal processing
  dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(h_out, dw_mode)) {
    // cannot use ansi sequence if fail to enable virtual terminal
    return false;
  }
  return true;
#else
  if (stream == Stream::Stdout) {
    return isatty(STDOUT_FILENO);
  } else if (stream == Stream::Stderr) {
    return isatty(STDERR_FILENO);
  }
  dcheck(false);
  return false;
#endif
}

}  // namespace

bool can_use_ansi_escape_sequence(Stream stream) {
  static const bool out = check_ansi_sequence_available(Stream::Stdout);
  static const bool err = check_ansi_sequence_available(Stream::Stderr);
  switch (stream) {
    case Stream::Stdout: return out;
    case Stream::Stderr: return err;
    default: dcheck(false); return false;
  }
}

void register_console() {
  // set console mode to utf8
#ifdef IS_PLAT_WINDOWS
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#endif
  // call these once to initialize the static variable
  can_use_ansi_escape_sequence(Stream::Stdout);
  can_use_ansi_escape_sequence(Stream::Stderr);
}

}  // namespace core
