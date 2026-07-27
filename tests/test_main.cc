// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "catch2/catch_session.hpp"
#include "fpag/base/console.h"
#include "fpag/base/debug/signal_handler.h"
#include "fpag/base/debug/terminate_handler.h"
#include "fpag/base/exit_handler.h"
#include "fpag/base/numeric.h"
#include "fpag/base/profiler/profile_scope.h"
#include "fpag/base/profiler/profiler.h"
#include "fpag/base/profiler/time_trace_formatter.h"

#define CATCH_CONFIG_RUNNER

void init() {
  base::register_console();
  base::register_exit_handler();
  base::register_terminate_handler();
  base::register_signal_handlers();

  base::Profiler::global().start();
}

void clean_up() {
  base::Profiler::global().stop();
  base::TimeTraceFormatter::write_to_file("test_time_trace.json",
                                          base::Profiler::global().events());
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

