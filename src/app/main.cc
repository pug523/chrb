// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/cli/console.h"
#include "core/driver.h"

int main(int argc, char** argv) {
  core::register_console();

  return core::rollback(argc, argv);
}
