// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <new>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "core/core.h"
#include "core/region/rollback_config.h"
#include "core/region/rollback_task.h"
#include "core/region/rollback_type.h"

namespace core {

enum class RollbackResult : u8 {
  // for static_cast<i32>
  Success = 0,
  Failed = 1,
};

class RollbackExecutor {
 public:
  RollbackExecutor() = default;
  ~RollbackExecutor() { flush(); }

  RollbackExecutor(const RollbackExecutor&) = delete;
  RollbackExecutor& operator=(const RollbackExecutor&) = delete;

  RollbackExecutor(RollbackExecutor&&) noexcept = delete;
  RollbackExecutor& operator=(RollbackExecutor&&) noexcept = delete;

  void init(RollbackConfig&& config);

  void start();
  void flush();

  inline u64 successfull_region_count() const {
    return successfull_region_count_;
  }
  inline u64 failed_region_count() const { return failed_region_count_; }
  inline u64 successfull_chunk_count() const {
    return successfull_chunk_count_;
  }
  inline u64 failed_chunk_count() const { return failed_chunk_count_; }

 private:
  void schedule(Dimension dimension, RollbackType type);

  void start_workers();

  void worker_thread();

  void run_task(const RollbackTask& task);

  void rollback_region(i32 rx,
                       i32 rz,
                       std::string_view src_file,
                       std::string_view dest_file);

  void rollback_chunks(i32 rx,
                       i32 rz,
                       ChunkRange range,
                       const std::string& src_file,
                       const std::string& dest_file);

  std::queue<RollbackTask> region_queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<std::thread> workers_;
  RollbackConfig config_;
  std::chrono::time_point<std::chrono::steady_clock> start_time_;

  static constexpr size_t kAlignSize =
      std::hardware_destructive_interference_size;

  alignas(kAlignSize) std::atomic<u64> successfull_region_count_ = 0;
  alignas(kAlignSize) std::atomic<u64> failed_region_count_ = 0;
  alignas(kAlignSize) std::atomic<u64> successfull_chunk_count_ = 0;
  alignas(kAlignSize) std::atomic<u64> failed_chunk_count_ = 0;
  alignas(kAlignSize) std::atomic<bool> stop_ = false;

  // a region contains 1024 (= 32 * 32) chunks
  // static constexpr const u64 kChunksPerRegion = 1024;
};

}  // namespace core
