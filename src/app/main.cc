// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/driver.h"
#include "fpag/base/console.h"
#include "fpag/base/debug/signal_handler.h"
#include "fpag/base/debug/terminate_handler.h"
#include "fpag/base/exit_handler.h"

int main(int argc, char** argv) {
  base::register_console();
  base::register_exit_handler();
  base::register_terminate_handler();
  base::register_signal_handlers();

  return app::rollback(argc, argv);
}
