// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/driver.h"
#include "core/cli/console.h"

int main(int argc, char** argv) {
  core::register_console();

  return rollback(argc, argv);
}
