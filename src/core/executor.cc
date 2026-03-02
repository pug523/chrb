// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/executor.h"

#include <sys/stat.h>

#include <print>

#include "core/dimension.h"
#include "core/file_util.h"
#include "core/parse_args.h"
#include "core/process_chunk.h"
#include "core/rollback_config.h"

namespace core {

i32 execute_rollback(i32 argc, char** argv) {
  RollbackConfig config{};
  const ArgStatus result = parse_args(argc, argv, &config);
  if (result == ArgStatus::PrintHelp || result == ArgStatus::PrintVersion) {
    return 0;
  } else if (result != ArgStatus::Success) {
    return static_cast<i32>(result);
  }

  if (config.src_world.back() != '/') {
    config.src_world.push_back('/');
  }
  config.src_world.append(dimension_path(config.dimension));
  config.src_world.append(config.type_str);

  if (config.dest_world.back() != '/') {
    config.dest_world.push_back('/');
  }
  config.dest_world.append(dimension_path(config.dimension));
  config.dest_world.append(config.type_str);

  if (!is_dir(config.src_world) || !is_dir(config.dest_world)) {
    std::println(stderr, "invalid world structure.\n");
    return 1;
  }

  u32 chunk_cnt = 0;
  for (i32 cx = *config.min_x; cx <= *config.max_x; ++cx) {
    for (i32 cz = *config.min_z; cz <= *config.max_z; ++cz) {
      ++chunk_cnt;
      process_chunk(cx, cz, config.src_world, config.dest_world);
    }
  }

  std::println("processed {} chunks", chunk_cnt);
  std::println("completed!");
  return 0;
}

}  // namespace core
