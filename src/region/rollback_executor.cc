// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "region/rollback_executor.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <mutex>
#include <print>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "core/check.h"
#include "core/cli/log_prefix.h"
#include "core/core.h"
#include "core/file_util.h"
#include "core/mem/mapped_file.h"
#include "region/dimension.h"
#include "region/processor/chunk_processor.h"
#include "region/processor/full_region_processor.h"
#include "region/rollback_config.h"
#include "region/rollback_task.h"
#include "region/rollback_type.h"

namespace region {

void RollbackExecutor::init(RollbackConfig&& config) {
  config_ = std::move(config);
  workers_.reserve(static_cast<size_t>(config_.num_threads));
}

void RollbackExecutor::start() {
  dcheck(config_.num_threads > 0);

  std::println("{}starting rollback...", core::info_prefix());

  start_time_ = std::chrono::steady_clock::now();

  const RollbackType r = config_.type;
  const bool do_region = r == RollbackType::Region || r == RollbackType::All;
  const bool do_entities =
      r == RollbackType::Entities || r == RollbackType::All;
  const bool do_poi = r == RollbackType::Poi || r == RollbackType::All;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (do_region) {
      schedule(config_.dimension, RollbackType::Region);
    }
    if (do_entities) {
      schedule(config_.dimension, RollbackType::Entities);
    }
    if (do_poi) {
      schedule(config_.dimension, RollbackType::Poi);
    }
  }
  std::println("{}scheduled {} regions", core::info_prefix(),
               region_queue_.size());

#ifdef IS_PLAT_LINUX
  if (config_.bulk_copy) {
    run_full_copy_batch();
  }
#endif

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
  const auto elapsed = ch::steady_clock::now() - start_time_;
  if (elapsed < ch::seconds(1)) {
    std::println("{}rollback completed in: {}ms", core::info_prefix(),
                 ch::duration_cast<ch::milliseconds>(elapsed).count());
  } else {
    const auto s = ch::duration_cast<ch::seconds>(elapsed);
    const auto ms = ch::duration_cast<ch::milliseconds>(elapsed - s);
    std::println("{}rollback completed in: {}.{}s", core::info_prefix(),
                 s.count(), ms.count());
  }
}

void RollbackExecutor::schedule(Dimension dimension, RollbackType type) {
  const i32 min_region_x = *config_.min_x >> 5;
  const i32 max_region_x = *config_.max_x >> 5;
  const i32 min_region_z = *config_.min_z >> 5;
  const i32 max_region_z = *config_.max_z >> 5;

  const i64 estimated_region_count =
      (max_region_x - min_region_x + 1) * (max_region_z - min_region_z + 1);
  dcheck(estimated_region_count > 0);

  std::string mca_dir = config_.src_world;
  mca_dir.append(dimension_path_with_slash(dimension)).append(type_path(type));
  const i64 files_on_disk = core::count_files(mca_dir);

  if (estimated_region_count < files_on_disk) {
    schedule_for_each_region_requested(min_region_x, max_region_x, min_region_z,
                                       max_region_z, dimension, type);
  } else {
    schedule_for_each_region_in_mca_dir(min_region_x, max_region_x,
                                        min_region_z, max_region_z, dimension,
                                        type);
  }
}

void RollbackExecutor::schedule_for_each_region_requested(
    const i32 min_region_x,
    const i32 max_region_x,
    const i32 min_region_z,
    const i32 max_region_z,
    const Dimension dimension,
    const RollbackType type) {
  for (i32 rx = min_region_x; rx <= max_region_x; ++rx) {
    const i32 min_chunk_x = (rx << 5);
    const i32 max_chunk_x = (rx << 5) + 31;
    for (i32 rz = min_region_z; rz <= max_region_z; ++rz) {
      const i32 min_chunk_z = (rz << 5);
      const i32 max_chunk_z = (rz << 5) + 31;

      if (config_.verbose) {
        std::println("{}scheduling region r({:4}, {:4})", core::debug_prefix(),
                     rx, rz);
      }

      const bool fully_contained =
          *config_.min_x <= min_chunk_x && *config_.max_x >= max_chunk_x &&
          *config_.min_z <= min_chunk_z && *config_.max_z >= max_chunk_z;

      region_queue_.push(RollbackTask{
          .region = {.x = rx, .z = rz},
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

void RollbackExecutor::schedule_for_each_region_in_mca_dir(
    const i32 min_rx,
    const i32 max_rx,
    const i32 min_rz,
    const i32 max_rz,
    const Dimension dimension,
    const RollbackType type) {
  std::string mca_dir = config_.src_world;
  mca_dir.append(dimension_path_with_slash(dimension)).append(type_path(type));

  for (const std::string& file : core::list_files(mca_dir)) {
    i32 rx = 0;
    i32 rz = 0;
    if (!parse_region_filename(file, &rx, &rz)) {
      continue;
    }

    if (rx < min_rx || rx > max_rx || rz < min_rz || rz > max_rz) {
      continue;
    }

    const i32 min_cx = rx << 5;
    const i32 max_cx = (rx << 5) + 31;
    const i32 min_cz = rz << 5;
    const i32 max_cz = (rz << 5) + 31;

    if (config_.verbose) {
      std::println("{}scheduling region r({:4}, {:4})", core::debug_prefix(),
                   rx, rz);
    }

    const bool fully_contained =
        *config_.min_x <= min_cx && *config_.max_x >= max_cx &&
        *config_.min_z <= min_cz && *config_.max_z >= max_cz;

    region_queue_.push(RollbackTask{
        .region = {.x = rx, .z = rz},
        .chunk_range = {.min_x = std::max(min_cx, *config_.min_x),
                        .min_z = std::max(min_cz, *config_.min_z),
                        .max_x = std::min(max_cx, *config_.max_x),
                        .max_z = std::min(max_cz, *config_.max_z)},
        .dimension = dimension,
        .type = type,
        .mode =
            fully_contained ? RollbackMode::FullCopy : RollbackMode::Partial,
    });
  }
}

#ifdef IS_PLAT_LINUX
void RollbackExecutor::run_full_copy_batch() {
  // drain full-copy tasks; leave partial tasks in the queue
  std::vector<RollbackTask> full_tasks;
  {
    std::queue<RollbackTask> remaining;
    while (!region_queue_.empty()) {
      RollbackTask t = region_queue_.front();
      region_queue_.pop();
      if (t.mode == RollbackMode::FullCopy) {
        full_tasks.push_back(t);
      } else {
        remaining.push(t);
      }
    }
    region_queue_ = std::move(remaining);
  }

  if (full_tasks.empty()) {
    return;
  }

  // validate paths; build flat src/dest vectors for process_batch
  std::vector<std::string> srcs;
  std::vector<std::string> dests;
  std::vector<size_t> task_idx;  // copy index -> full_tasks index

  srcs.reserve(full_tasks.size());
  dests.reserve(full_tasks.size());
  task_idx.reserve(full_tasks.size());

  for (size_t i = 0; i < full_tasks.size(); ++i) {
    const RollbackTask& task = full_tasks[i];
    std::string src, dest;

    if (!build_region_paths(task, &src, &dest)) {
      failed_region_count_.fetch_add(1);
      std::unique_lock<std::mutex> lock(failed_regions_mutex_);
      failed_regions_.emplace_back(task.region.x, task.region.z);
      continue;
    }
    if (!core::is_file(src) || !core::is_file(dest)) {
      if (config_.verbose) {
        std::println("{}skipped missing region: r.{}.{}.mca",
                     core::debug_prefix(), task.region.x, task.region.z);
      }
      continue;
    }

    srcs.push_back(std::move(src));
    dests.push_back(std::move(dest));
    task_idx.push_back(i);
  }

  if (srcs.empty()) {
    return;
  }

  std::println("{}bulk-copying {} full region(s)...", core::info_prefix(),
               srcs.size());

  const auto on_success = [&](size_t ci) {
    successfull_region_count_.fetch_add(1);
    if (config_.verbose) {
      const auto& t = full_tasks[task_idx[ci]];
      std::println("{}copied region r({:4}, {:4})", core::debug_prefix(),
                   t.region.x, t.region.z);
    }
  };

  const auto on_failure = [&](size_t ci) {
    const auto& t = full_tasks[task_idx[ci]];
    std::println(stderr, "{}failed to copy region r({}, {})",
                 core::error_prefix(), t.region.x, t.region.z);
    failed_region_count_.fetch_add(1);
    std::unique_lock<std::mutex> lock(failed_regions_mutex_);
    failed_regions_.emplace_back(t.region.x, t.region.z);
  };

  FullRegionProcessor processor;
  processor.init(config_.verbose);
  const u32 queue_depth = static_cast<u32>(std::min<size_t>(srcs.size(), 64));
  processor.process_batch(srcs, dests, queue_depth, on_success, on_failure);
}
#endif

void RollbackExecutor::start_workers() {
  dcheck(0 < config_.num_threads);
  dcheck(config_.num_threads <=
         static_cast<i32>(std::thread::hardware_concurrency()));

  if (region_queue_.empty()) {
    return;
  }

  for (i32 i = 0; i < config_.num_threads; ++i) {
    workers_.emplace_back(&RollbackExecutor::worker_thread, this);
    if (config_.verbose) {
      std::println("{}started worker thread (id: {})", core::debug_prefix(), i);
    }
  }
  if (config_.verbose) {
    std::println("{}started all {} worker threads", core::debug_prefix(),
                 config_.num_threads);
  }
}

void RollbackExecutor::worker_thread() {
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
  const i32 rx = task.region.x;
  const i32 rz = task.region.z;

  std::string src_dir(config_.src_world);
  src_dir.append(dimension_path_with_slash(task.dimension))
      .append(type_path(task.type))
      .push_back('/');
  std::string dest_dir(config_.dest_world);
  dest_dir.append(dimension_path_with_slash(task.dimension))
      .append(type_path(task.type))
      .push_back('/');
  {
    bool dir_exists = true;
    if (!core::is_dir(src_dir)) {
      std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                   src_dir);
      dir_exists = false;
    } else if (!core::is_dir(dest_dir)) {
      std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                   dest_dir);
      dir_exists = false;
    }
    if (!dir_exists) {
      failed_region_count_.fetch_add(1);
      return;
    }
  }

  const std::string filename = std::format("r.{}.{}.mca", rx, rz);
  std::string src_file(std::move(src_dir));
  src_file.append(filename);
  std::string dest_file(std::move(dest_dir));
  dest_file.append(filename);

  if (!core::is_file(src_file) || !core::is_file(dest_file)) {
    if (config_.verbose) {
      std::println("{}skipped missing region: r.{}.{}.mca",
                   core::debug_prefix(), task.region.x, task.region.z);
    }
    // this is not an error
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
  region_processor.init(config_.verbose);
  if (!region_processor.process_one(rx, rz, src_path, dest_path)) {
    failed_region_count_.fetch_add(1);
    std::unique_lock<std::mutex> lock(failed_regions_mutex_);
    failed_regions_.emplace_back(rx, rz);
    lock.unlock();
  } else {
    successfull_region_count_.fetch_add(1);
  }
}

void RollbackExecutor::rollback_chunks(i32 rx,
                                       i32 rz,
                                       ChunkRange range,
                                       const std::string& src_file,
                                       const std::string& dest_file) {
  constexpr size_t kMcaHeaderSize = 8192;
  core::MappedFile src;
  src.open(src_file, kMcaHeaderSize);
  core::MappedFile dest;
  dest.open(dest_file, kMcaHeaderSize);

  {
    bool failed_open = false;
    if (!src.is_open()) {
      std::println(stderr, "{}failed to open src file", core::error_prefix());
      failed_open = true;
    }
    if (!dest.is_open()) {
      std::println(stderr, "{}failed to open dest file", core::error_prefix());
      failed_open = true;
    }
    if (failed_open) {
      failed_region_count_.fetch_add(1);
      return;
    }
  }

  ChunkProcessor chunk_processor;
  chunk_processor.init(rx, rz, &src, &dest, config_.verbose);

  // i64 failed_local = 0;
  for (i32 cx = range.min_x; cx <= range.max_x; ++cx) {
    for (i32 cz = range.min_z; cz <= range.max_z; ++cz) {
      chunk_processor.process(cx, cz);
      // if (!chunk_processor.process(cx, cz)) {
      //   ++failed_local;
      // }
    }
  }

  const i64 chunks_tried =
      (range.max_x - range.min_x + 1) * (range.max_z - range.min_z + 1);
  dcheck(chunks_tried > 0);
  successfull_chunk_count_.fetch_add(static_cast<u64>(chunks_tried));
  // successfull_chunk_count_.fetch_add(
  //     static_cast<u64>(chunks_tried - failed_local));
  // failed_chunk_count_.fetch_add(static_cast<u64>(failed_local));
}

bool RollbackExecutor::build_region_paths(const RollbackTask& task,
                                          std::string* out_src,
                                          std::string* out_dest) {
  const std::string suffix =
      std::string(dimension_path_with_slash(task.dimension))
          .append(type_path(task.type));

  std::string src = std::string(config_.src_world) + '/' + suffix;
  std::string dest = std::string(config_.dest_world) + '/' + suffix;

  if (!core::is_dir(src)) {
    std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                 src);
    return false;
  }
  if (!core::is_dir(dest)) {
    std::println(stderr, "{}directory not found: {}", core::error_prefix(),
                 dest);
    return false;
  }

  const std::string filename =
      std::format("r.{}.{}.mca", task.region.x, task.region.z);
  src.push_back('/');
  src.append(filename);
  dest.push_back('/');
  dest.append(filename);

  *out_src = std::move(src);
  *out_dest = std::move(dest);
  return true;
}

// static
bool RollbackExecutor::parse_region_filename(const std::string& path,
                                             i32* out_rx,
                                             i32* out_rz) {
  // find the last '/' or '\' to isolate the filename
  const size_t slash = path.find_last_of("/\\");
  const size_t name_start = (slash == std::string::npos) ? 0 : slash + 1;
  const std::string_view name(path.data() + name_start,
                              path.size() - name_start);

  // expect "r.<x>.<z>.mca"
  if (name.size() < 9 || name[0] != 'r' || name[1] != '.') {
    return false;
  }

  const size_t dot1 = name.find('.', 2);
  if (dot1 == std::string_view::npos) {
    return false;
  }
  const size_t dot2 = name.find('.', dot1 + 1);
  if (dot2 == std::string_view::npos) {
    return false;
  }
  // must end with ".mca"
  if (name.substr(dot2 + 1) != "mca") {
    return false;
  }

  const std::string_view x_str = name.substr(2, dot1 - 2);
  const std::string_view z_str = name.substr(dot1 + 1, dot2 - dot1 - 1);

  // simple integer parse (avoids std::stoi and locale overhead)
  auto parse_int = [](std::string_view s, i32* out) -> bool {
    if (s.empty()) {
      return false;
    }
    bool neg = false;
    size_t i = 0;
    if (s[0] == '-') {
      neg = true;
      ++i;
    }
    if (i == s.size()) {
      return false;
    }
    i32 v = 0;
    for (; i < s.size(); ++i) {
      if (s[i] < '0' || s[i] > '9') {
        return false;
      }
      v = v * 10 + (s[i] - '0');
    }
    *out = neg ? -v : v;
    return true;
  };

  return parse_int(x_str, out_rx) && parse_int(z_str, out_rz);
}

}  // namespace region
