// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "app/driver.h"
#include "fpag/debug/exit_handler.h"
#include "fpag/debug/signal_handler.h"
#include "fpag/debug/terminate_handler.h"
#include "fpag/term/console.h"

int main(int argc, char** argv) {
  term::register_console();
  debug::register_exit_handler();
  debug::register_terminate_handler();
  debug::register_signal_handlers();

  return app::rollback(argc, argv);
}
