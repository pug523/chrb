// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "catch2/catch_session.hpp"
#include "fpag/base/numeric.h"
#include "fpag/debug/exit_handler.h"
#include "fpag/debug/profiler/profile_scope.h"
#include "fpag/debug/profiler/profiler.h"
#include "fpag/debug/profiler/time_trace_formatter.h"
#include "fpag/debug/signal_handler.h"
#include "fpag/debug/terminate_handler.h"
#include "fpag/term/console.h"

#define CATCH_CONFIG_RUNNER

void init() {
  term::register_console();
  debug::register_exit_handler();
  debug::register_terminate_handler();
  debug::register_signal_handlers();

  debug::Profiler::global().start();
}

void clean_up() {
  debug::Profiler::global().stop();
  debug::TimeTraceFormatter::write_to_file(
      "test_time_trace.json", debug::Profiler::global().copy_events());
}

i32 run_tests(i32 argc, char** argv) {
  PROFILE_FUNCTION();
  Catch::Session session;

  const i32 return_code = session.applyCommandLine(argc, argv);
  if (return_code != 0) {
    return return_code;
  }

  return session.run();
}

i32 main(i32 argc, char** argv) {
  init();

  const i32 result = run_tests(argc, argv);

  clean_up();

  return result;
}

