// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/region/rollback_executor.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <print>
#include <string>
#include <utility>
#include <vector>

#include "core/check.h"
#include "core/cli/colored_prefix.h"
#include "core/core.h"
#include "core/file_util.h"
#include "core/region/processor/chunk_processor.h"
#include "core/region/processor/full_region_processor.h"
#include "core/region/rollback_task.h"

namespace core {

namespace {

struct RollbackContext {
  Dimension dimension;
  RollbackType type;
};

}  // namespace

void RollbackExecutor::init(RollbackConfig&& config) {
  config_ = std::move(config);

  workers_.reserve(static_cast<size_t>(config_.num_threads));
}

void RollbackExecutor::start() {
  dcheck(num_threads_ > 0);

  ColoredPrefix cp;
  cp.init_info();
  std::println("starting rollback...");

  start_time_ = std::chrono::steady_clock::now();

  const RollbackType r = config_.type;
  const bool region = r == RollbackType::Region || r == RollbackType::All;
  const bool entities = r == RollbackType::Entities || r == RollbackType::All;
  const bool poi = r == RollbackType::Poi || r == RollbackType::All;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (region) {
      schedule(config_.dimension, RollbackType::Region);
    }
    if (entities) {
      schedule(config_.dimension, RollbackType::Entities);
    }
    if (poi) {
      schedule(config_.dimension, RollbackType::Poi);
    }
  }
  std::println("scheduled {} regions", region_queue_.size());
  start_workers();
}

void RollbackExecutor::flush() {
  if (stop_ && workers_.empty()) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    stop_.store(true);
  }
  cv_.notify_all();

  for (std::thread& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
  workers_.clear();

  namespace ch = std::chrono;
  const auto end_time = ch::steady_clock::now();
  const auto diff = end_time - start_time_;
  if (diff < ch::seconds(1)) {
    std::println("rollback completed in: {}ms",
                 ch::duration_cast<ch::milliseconds>(diff).count());
  } else {
    const auto s = ch::duration_cast<ch::seconds>(diff);
    const auto ms = ch::duration_cast<ch::milliseconds>(diff - s);
    std::println("rollback completed in: {}.{}s", s.count(), ms.count());
  }
}

void RollbackExecutor::schedule(Dimension dimension, RollbackType type) {
  const i32 min_region_x = *config_.min_x >> 5;
  const i32 max_region_x = *config_.max_x >> 5;
  const i32 min_region_z = *config_.min_z >> 5;
  const i32 max_region_z = *config_.max_z >> 5;

  for (i32 rx = min_region_x; rx <= max_region_x; ++rx) {
    const i32 min_chunk_x = (rx << 5);
    const i32 max_chunk_x = (rx << 5) + 31;
    for (i32 rz = min_region_z; rz <= max_region_z; ++rz) {
      const i32 min_chunk_z = (rz << 5);
      const i32 max_chunk_z = (rz << 5) + 31;

      if (config_.verbose) {
        std::println("scheduling region ({:4}, {:4})", rx, rz);
      }

      const bool fully_contained =
          *config_.min_x <= min_chunk_x && *config_.max_x >= max_chunk_x &&
          *config_.min_z <= min_chunk_z && *config_.max_z >= max_chunk_z;

      region_queue_.push(RollbackTask{
          .region = {.region_x = rx, .region_z = rz},
          .chunk_range = {.min_x = std::max(min_chunk_x, *config_.min_x),
                          .min_z = std::max(min_chunk_z, *config_.min_z),
                          .max_x = std::min(max_chunk_x, *config_.max_x),
                          .max_z = std::min(max_chunk_z, *config_.max_z)},
          .dimension = dimension,
          .type = type,
          .mode =
              fully_contained ? RollbackMode::FullCopy : RollbackMode::Partial,
      });
    }
  }
}

void RollbackExecutor::start_workers() {
  dcheck(0 < config_.num_threads);
  dcheck(config_.num_threads <= static_cast<i32>(std::hardware_concurrency()));
  for (i32 i = 0; i < config_.num_threads; ++i) {
    workers_.emplace_back(&RollbackExecutor::worker_thread, this);
  }
}

void RollbackExecutor::worker_thread() {
  if (config_.verbose) {
    std::println("started worker thread");
  }
  while (true) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return stop_ || !region_queue_.empty(); });

    if (stop_ && region_queue_.empty()) {
      return;
    }

    const RollbackTask task = region_queue_.front();
    region_queue_.pop();
    lock.unlock();

    run_task(task);
  }
}

void RollbackExecutor::run_task(const RollbackTask& task) {
  const i32 rx = task.region.region_x;
  const i32 rz = task.region.region_z;

  std::string src_dir(config_.src_world);
  src_dir.push_back('/');
  src_dir.append(dimension_path_with_slash(task.dimension));
  src_dir.append(type_path(task.type));
  std::string dest_dir(config_.dest_world);
  dest_dir.push_back('/');
  dest_dir.append(dimension_path_with_slash(task.dimension));
  dest_dir.append(type_path(task.type));
  {
    bool dir_exists = true;
    if (!is_dir(src_dir)) {
      std::println(stderr, "directory not found: {}", src_dir);
      dir_exists = false;
    } else if (!is_dir(dest_dir)) {
      std::println(stderr, "directory not found: {}", src_dir);
      dir_exists = false;
    }
    if (!dir_exists) {
      failed_region_count_.fetch_add(1);
      return;
    }
  }

  const std::string filename = std::format("r.{}.{}.mca", rx, rz);
  std::string src_file(std::move(src_dir));
  src_file.push_back('/');
  src_file.append(filename);
  std::string dest_file(std::move(dest_dir));
  dest_file.push_back('/');
  dest_file.append(filename);

  if (!is_file(src_file) || !is_file(dest_file)) {
    if (config_.verbose) {
      std::println("skipped missing region: {}", filename);
    }
    failed_region_count_.fetch_add(1);
    return;
  }

  switch (task.mode) {
    case RollbackMode::FullCopy: {
      // full region copy rollback
      rollback_region(rx, rz, src_file, dest_file);
      break;
    }
    case RollbackMode::Partial: {
      // rollback per chunk: open files and execute chunk rollbacks

      rollback_chunks(rx, rz, task.chunk_range, src_file, dest_file);
      break;
    }
    default: {
      // unreachable
      dcheck(false);
      break;
    }
  }
}

void RollbackExecutor::rollback_region(i32 rx,
                                       i32 rz,
                                       std::string_view src_path,
                                       std::string_view dest_path) {
  FullRegionProcessor region_processor;
  region_processor.init(rx, rz, src_path, dest_path, config_.verbose);
  if (!region_processor.process()) {
    failed_region_count_.fetch_add(1);
  } else {
    successfull_region_count_.fetch_add(1);
  }
}

void RollbackExecutor::rollback_chunks(i32 rx,
                                       i32 rz,
                                       ChunkRange range,
                                       const std::string& src_file,
                                       const std::string& dest_file) {
  std::fstream src(src_file, std::ios::binary | std::ios::in);
  std::fstream dest(dest_file, std::ios::binary | std::ios::in | std::ios::out);

  if (!src) {
    std::println(stderr, "failed to open src file: {}", src_file);
    failed_region_count_.fetch_add(1);
    return;
  } else if (!dest) {
    std::println(stderr, "failed to open dest file: {}", dest_file);
    failed_region_count_.fetch_add(1);
    return;
  }

  ChunkProcessor chunk_processor;
  chunk_processor.init(rx, rz, &src, &dest, config_.verbose);

  i64 failed_local = 0;
  for (i32 cx = range.min_x; cx <= range.max_x; ++cx) {
    for (i32 cz = range.min_z; cz <= range.max_z; ++cz) {
      if (!chunk_processor.process(cx, cz)) {
        ++failed_local;
      }
    }
  }

  const i64 chunks_tried =
      (range.max_x - range.min_x + 1) * (range.max_z - range.min_z + 1);
  dcheck(chunks_tried > 0);
  successfull_chunk_count_.fetch_add(
      static_cast<u64>(chunks_tried - failed_local));
  failed_chunk_count_.fetch_add(static_cast<u64>(failed_local));
}

}  // namespace core
