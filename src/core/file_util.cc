// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/file_util.h"

// std::filesystem is suck

#include <fcntl.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/check.h"
#include "core/core.h"

#if defined(IS_PLAT_LINUX)
#include <asm/unistd_64.h>
// NOLINTNEXTLINE
#include <dirent.h>
#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#elif defined(IS_PLAT_MACOS)
// NOLINTNEXTLINE
#include <dirent.h>
#include <unistd.h>
#elif defined(IS_PLAT_WINDOWS)
#include <io.h>
#include <windows.h>
#define stat _stat
#define close _close
#define read _read
#define write _write
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define ssize_t SSIZE_T
#endif

namespace core {

namespace {

inline bool exists_as(const std::string_view path, u32 mask) {
  struct stat info;
  if (stat(path.data(), &info) != 0) {
    return false;
  }
  return (info.st_mode & mask) != 0;
}

#ifdef IS_PLAT_LINUX
// define linux_dirent64 manually as it is not in glibc headers
struct linux_dirent64 {
  u64 d_ino;
  i64 d_off;
  u16 d_reclen;
  unsigned char d_type;
  char d_name[1];
};
#endif

i64 walk_recursive(const std::string& path, std::vector<std::string>* out) {
  i64 count = 0;
#if defined(IS_PLAT_LINUX)
  const i32 fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
  if (fd == -1) {
    return 0;
  }

  alignas(linux_dirent64) char buf[16384];
  while (true) {
    const i64 nread = syscall(SYS_getdents64, fd, buf, sizeof(buf));
    if (nread <= 0) {
      break;
    }

    for (i64 bpos = 0; bpos < nread;) {
      const linux_dirent64* d = reinterpret_cast<linux_dirent64*>(buf + bpos);
      const char* name = d->d_name;

      if (name[0] == '.' &&
          (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
        bpos += d->d_reclen;
        continue;
      }

      if (d->d_type == DT_DIR) {
        std::string sub_path = path;
        if (sub_path.back() != '/') {
          sub_path.push_back('/');
        }
        sub_path.append(name);
        count += walk_recursive(sub_path, out);
      } else if (d->d_type == DT_REG) {
        ++count;
        if (out) {
          std::string file_path = path;
          if (file_path.back() != '/') {
            file_path.push_back('/');
          }
          file_path.append(name);
          out->push_back(std::move(file_path));
        }
      }
      bpos += d->d_reclen;
    }
  }
  close(fd);

#elif defined(IS_PLAT_WINDOWS)
  // narrow -> wide conversion via WinAPI to handle non-ASCII paths correctly
  const int wlen =
      MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
  std::wstring wpath(wlen, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath.data(), wlen);
  // remove null terminator added by MultiByteToWideChar, then append wildcard
  if (!wpath.empty() && wpath.back() == L'\0') {
    wpath.pop_back();
  }
  if (!wpath.empty() && wpath.back() != L'\\' && wpath.back() != L'/') {
    wpath.push_back(L'\\');
  }
  wpath.append(L"*");

  WIN32_FIND_DATAW find_data;
  HANDLE hFind =
      FindFirstFileExW(wpath.c_str(), FindExInfoBasic, &find_data,
                       FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
  if (hFind == INVALID_HANDLE_VALUE) {
    return 0;
  }

  do {
    const wchar_t* wname = find_data.cFileName;
    if (wname[0] == L'.' &&
        (wname[1] == L'\0' || (wname[1] == L'.' && wname[2] == L'\0'))) {
      continue;
    }

    // wide -> narrow for sub_path construction
    const int nlen = WideCharToMultiByte(CP_UTF8, 0, wname, -1, nullptr, 0,
                                         nullptr, nullptr);
    std::string name_narrow(nlen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wname, -1, name_narrow.data(), nlen,
                        nullptr, nullptr);
    if (!name_narrow.empty() && name_narrow.back() == '\0') {
      name_narrow.pop_back();
    }

    std::string sub_path = path;
    if (sub_path.back() != '/' && sub_path.back() != '\\') {
      sub_path.push_back('/');
    }
    sub_path.append(name_narrow);

    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      count += walk_recursive(sub_path, out);
    } else {
      ++count;
      if (out) {
        out->push_back(sub_path);
      }
    }
  } while (FindNextFileW(hFind, &find_data));
  FindClose(hFind);

#else
  // standard POSIX fallback using readdir
  DIR* dp = opendir(path.c_str());
  if (!dp) {
    return 0;
  }

  struct dirent* entry;
  while ((entry = readdir(dp))) {
    const char* name = entry->d_name;
    if (name[0] == '.' &&
        (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
      continue;
    }

    if (entry->d_type == DT_DIR) {
      std::string sub_path = path;
      if (sub_path.back() != '/') {
        sub_path.push_back('/');
      }
      sub_path.append(name);
      count += walk_recursive(sub_path, out);
    } else if (entry->d_type == DT_REG) {
      ++count;
      if (out) {
        std::string file_path = path;
        if (file_path.back() != '/') {
          file_path.push_back('/');
        }
        file_path.append(name);
        out->push_back(std::move(file_path));
      }
    }
  }
  closedir(dp);
#endif

  return count;
}

#if defined(IS_PLAT_LINUX)

// thin syscall wrappers without liburing dependency
inline int io_uring_setup(u32 entries, struct io_uring_params* p) {
  return static_cast<int>(syscall(__NR_io_uring_setup, entries, p));
}

inline int io_uring_enter(int ring_fd,
                          u32 to_submit,
                          u32 min_complete,
                          u32 flags) {
  return static_cast<int>(syscall(__NR_io_uring_enter, ring_fd, to_submit,
                                  min_complete, flags, nullptr, 0));
}

// copy_file_range(2): kernel-to-kernel copy, no userland buffer
// falls back to pread/pwrite on exdev / enosys
bool copy_file_range_full(int src_fd, int dest_fd, off_t total) {
  off_t off_in = 0;
  off_t off_out = 0;
  while (off_in < total) {
    const ssize_t copied =
        copy_file_range(src_fd, &off_in, dest_fd, &off_out,
                        static_cast<size_t>(total - off_in), 0);
    if (copied < 0) {
      if (errno == EXDEV || errno == ENOSYS || errno == EOPNOTSUPP) {
        // cross-device or unsupported fs: fallback to pread/pwrite loop
        // 128 KiB
        constexpr size_t kBuf = 1 << 17;
        alignas(4096) static thread_local char buf[kBuf];
        while (off_in < total) {
          const ssize_t r =
              pread(src_fd, buf,
                    std::min<size_t>(kBuf, static_cast<size_t>(total - off_in)),
                    off_in);
          if (r <= 0) {
            return false;
          }
          ssize_t written = 0;
          while (written < r) {
            const ssize_t w =
                pwrite(dest_fd, buf + written, static_cast<size_t>(r - written),
                       off_out + written);
            if (w <= 0) {
              return false;
            }
            written += w;
          }
          off_in += r;
          off_out += r;
        }
        return true;
      }
      return false;
    }
    if (copied == 0) {
      break;  // eof
    }
  }
  return true;
}
struct IoUring {
  int ring_fd = -1;
  u32 sq_entries = 0;

  // sq
  u8* sq_ring = nullptr;
  size_t sq_ring_sz = 0;
  u32* sq_head = nullptr;
  u32* sq_tail = nullptr;
  u32* sq_mask = nullptr;
  u32* sq_array = nullptr;
  struct io_uring_sqe* sqes = nullptr;
  size_t sqes_sz = 0;

  // cq
  u8* cq_ring = nullptr;  // may alias sq_ring if IORING_FEAT_SINGLE_MMAP
  size_t cq_ring_sz = 0;
  u32* cq_head = nullptr;
  u32* cq_tail = nullptr;
  u32* cq_mask = nullptr;
  struct io_uring_cqe* cqes_ptr = nullptr;

  bool valid() const { return ring_fd >= 0; }

  bool init(u32 entries) {
    struct io_uring_params p{};
    ring_fd = io_uring_setup(entries, &p);
    if (ring_fd < 0) {
      return false;
    }
    sq_entries = p.sq_entries;

    // map sq ring
    sq_ring_sz = p.sq_off.array + p.sq_entries * sizeof(u32);
    sq_ring = static_cast<u8*>(mmap(nullptr, sq_ring_sz, PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_POPULATE, ring_fd,
                                    IORING_OFF_SQ_RING));
    if (sq_ring == MAP_FAILED) {
      sq_ring = nullptr;
      return false;
    }

    sq_head = reinterpret_cast<u32*>(sq_ring + p.sq_off.head);
    sq_tail = reinterpret_cast<u32*>(sq_ring + p.sq_off.tail);
    sq_mask = reinterpret_cast<u32*>(sq_ring + p.sq_off.ring_mask);
    sq_array = reinterpret_cast<u32*>(sq_ring + p.sq_off.array);

    // map sqe array
    sqes_sz = p.sq_entries * sizeof(struct io_uring_sqe);
    sqes = static_cast<struct io_uring_sqe*>(
        mmap(nullptr, sqes_sz, PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQES));
    if (sqes == MAP_FAILED) {
      sqes = nullptr;
      return false;
    }

    // map cq ring (may share mapping with sq ring)
    if (p.features & IORING_FEAT_SINGLE_MMAP) {
      cq_ring = sq_ring;
      cq_ring_sz = 0;  // no separate unmap needed
    } else {
      cq_ring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
      cq_ring = static_cast<u8*>(
          mmap(nullptr, cq_ring_sz, PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_CQ_RING));
      if (cq_ring == MAP_FAILED) {
        cq_ring = nullptr;
        return false;
      }
    }

    cq_head = reinterpret_cast<u32*>(cq_ring + p.cq_off.head);
    cq_tail = reinterpret_cast<u32*>(cq_ring + p.cq_off.tail);
    cq_mask = reinterpret_cast<u32*>(cq_ring + p.cq_off.ring_mask);
    cqes_ptr = reinterpret_cast<struct io_uring_cqe*>(cq_ring + p.cq_off.cqes);

    return true;
  }

  ~IoUring() {
    if (sqes && sqes != MAP_FAILED) {
      munmap(sqes, sqes_sz);
    }
    if (cq_ring && cq_ring != MAP_FAILED && cq_ring != sq_ring) {
      munmap(cq_ring, cq_ring_sz);
    }
    if (sq_ring && sq_ring != MAP_FAILED) {
      munmap(sq_ring, sq_ring_sz);
    }
    if (ring_fd >= 0) {
      ::close(ring_fd);
    }
  }

  // submit one IORING_OP_READ or IORING_OP_WRITE SQE
  // returns false if the ring is full (caller should drain first)
  bool push_rw(u8 opcode,
               int fd,
               void* buf,
               u32 len,
               u64 offset,
               u64 user_data) {
    const u32 tail = *sq_tail;
    const u32 head = __atomic_load_n(sq_head, __ATOMIC_ACQUIRE);
    if (tail - head >= sq_entries) {
      return false;  // ring full
    }
    const u32 idx = tail & *sq_mask;
    struct io_uring_sqe* sqe = &sqes[idx];
    memset(sqe, 0, sizeof(*sqe));
    sqe->opcode = opcode;
    sqe->fd = fd;
    sqe->addr = reinterpret_cast<u64>(buf);
    sqe->len = len;
    sqe->off = offset;
    sqe->user_data = user_data;
    sq_array[idx] = idx;
    __atomic_store_n(sq_tail, tail + 1, __ATOMIC_RELEASE);
    return true;
  }

  // drain at least `min_completions` CQEs, call cb(user_data, res) for each
  template <typename Fn>
  void drain(u32 min_completions, Fn&& cb) {
    u32 completed = 0;
    while (true) {
      const u32 head = *cq_head;
      const u32 tail = __atomic_load_n(cq_tail, __ATOMIC_ACQUIRE);
      if (head == tail) {
        if (completed >= min_completions) {
          break;
        }
        // wait for at least one completion
        io_uring_enter(ring_fd, 0, 1, IORING_ENTER_GETEVENTS);
        continue;
      }
      const struct io_uring_cqe& cqe = cqes_ptr[head & *cq_mask];
      cb(cqe.user_data, cqe.res);
      __atomic_store_n(cq_head, head + 1, __ATOMIC_RELEASE);
      ++completed;
    }
  }

  int submit(u32 n) { return io_uring_enter(ring_fd, n, 0, 0); }
};

struct CopySlot {
  int src_fd = -1;
  int dest_fd = -1;
  off_t total = 0;      // file size
  off_t rd_off = 0;     // next read offset
  off_t wr_off = 0;     // next write offset
  size_t file_idx = 0;  // index into the caller's srcs/dests arrays
  bool failed = false;

  // intermediate buffer — allocated once, reused across reads
  // 256 KiB per slot
  static constexpr size_t kChunk = 1 << 18;
  u8* buf = nullptr;

  // is a read in-flight, is a write in-flight?
  bool rd_inflight = false;
  bool wr_inflight = false;
  // how many bytes the last read returned (to be written next)
  i32 rd_bytes = 0;

  void close_fds() {
    if (src_fd >= 0) {
      ::close(src_fd);
      src_fd = -1;
    }
    if (dest_fd >= 0) {
      ::close(dest_fd);
      dest_fd = -1;
    }
  }
};

// encode (slot index, is_write) into a u64 user_data tag
inline u64 make_tag(u32 slot_idx, bool is_write) {
  return (static_cast<u64>(slot_idx) << 1) | (is_write ? 1u : 0u);
}
inline u32 tag_slot(u64 tag) {
  return static_cast<u32>(tag >> 1);
}
inline bool tag_is_write(u64 tag) {
  return (tag & 1u) != 0;
}

#endif  // IS_PLAT_LINUX

}  // namespace

bool is_dir(const std::string_view path) {
  return exists_as(path, S_IFDIR);
}

bool is_file(const std::string_view path) {
  return exists_as(path, S_IFREG);
}

bool create_file(const std::string_view path) {
#ifdef IS_PLAT_WINDOWS
  i32 fd;
  errno_t err =
      _sopen_s(&fd, path.data(), O_CREAT | O_WRONLY | O_TRUNC, _SH_DENYNO, 0);
  if (err != 0) {
    return false;
  }
#else
  i32 fd = open(path.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
  if (fd == -1) {
    return false;
  }
  close(fd);
  return true;
}

bool copy_file(const std::string_view from, const std::string_view to) {
#ifdef IS_PLAT_WINDOWS
  i32 source;
  errno_t err = _sopen_s(&source, from.data(), O_RDONLY, _SH_DENYNO, 0);
  if (err != 0) {
    return false;
  }
#else
  const i32 source = open(from.data(), O_RDONLY);
#endif
  if (source == -1) {
    return false;
  }

#ifdef IS_PLAT_WINDOWS
  i32 dest;
  errno_t err2 = _sopen_s(&dest, to.data(), O_CREAT | O_WRONLY | O_TRUNC,
                          _SH_DENYNO, S_IRUSR | S_IWUSR);
  if (err2 != 0) {
    close(source);
    return false;
  }
#else
  const i32 dest =
      open(to.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
  if (dest == -1) {
    close(source);
    return false;
  }

#if defined(IS_PLAT_LINUX)
  // sendfile copies entirely in kernel space — no userland buffer round-trip
  struct stat src_stat;
  if (fstat(source, &src_stat) == -1) {
    close(source);
    close(dest);
    return false;
  }

  off_t offset = 0;
  const off_t total = src_stat.st_size;
  bool ok = true;
  while (offset < total) {
    const ssize_t sent =
        sendfile(dest, source, &offset, static_cast<size_t>(total - offset));
    if (sent <= 0) {
      ok = false;
      break;
    }
  }
  close(source);
  close(dest);
  return ok;

#else
  char buffer[4096];
  ssize_t bytes;
  while ((bytes = read(source, buffer, sizeof(buffer))) > 0) {
    if (write(dest, buffer, static_cast<u32>(bytes)) != bytes) {
      close(source);
      close(dest);
      return false;
    }
  }

  close(source);
  close(dest);
  return bytes == 0;
#endif
}

bool rename_file(const std::string_view from, const std::string_view to) {
  return rename(from.data(), to.data()) == 0;
}

i64 count_files(const std::string_view dir) {
  dcheck(!dir.empty());
  dcheck(is_dir(dir));
  return walk_recursive(std::string(dir), nullptr);
}

std::vector<std::string> list_files(const std::string_view dir) {
  dcheck(!dir.empty());
  dcheck(is_dir(dir));
  std::vector<std::string> files;
  walk_recursive(std::string(dir), &files);
  return files;
}

#if defined(IS_PLAT_LINUX)

std::vector<u64> async_copy_files(const std::vector<std::string>& srcs,
                                  const std::vector<std::string>& dests,
                                  u32 queue_depth) {
  dcheck(srcs.size() == dests.size());

  const size_t n = srcs.size();
  // failure bitmask: one bit per file, packed into u64 words
  const size_t words = (n + 63) / 64;
  std::vector<u64> failed_mask(words, 0u);

  if (n == 0) {
    return failed_mask;
  }

  // mark a file index as failed in the bitmask
  auto mark_failed = [&](size_t i) {
    failed_mask[i / 64] |= (u64{1} << (i % 64));
  };

  // first, attempt copy_file_range for every file
  // this is a pure kernel-space copy and is the fastest path on most
  // same-filesystem setups (e.g. ext4 / xfs reflinks)
  // we do this synchronously but without userland i/o — the kernel handles it
  //
  // if all files succeed via copy_file_range we return immediately without
  // ever touching io_uring, keeping overhead minimal for the common case
  bool any_cfr_failed = false;
  std::vector<bool> cfr_ok(n, false);

  for (size_t i = 0; i < n; ++i) {
    const int src_fd = open(srcs[i].c_str(), O_RDONLY | O_NOATIME);
    if (src_fd < 0) {
      mark_failed(i);
      any_cfr_failed = true;
      continue;
    }
    struct stat st{};
    if (fstat(src_fd, &st) < 0) {
      ::close(src_fd);
      mark_failed(i);
      any_cfr_failed = true;
      continue;
    }
    const int dest_fd =
        open(dests[i].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest_fd < 0) {
      ::close(src_fd);
      mark_failed(i);
      any_cfr_failed = true;
      continue;
    }
    if (st.st_size == 0) {
      // empty file: nothing to copy, just close
      ::close(src_fd);
      ::close(dest_fd);
      cfr_ok[i] = true;
      continue;
    }

    // try copy_file_range; fallback to pread/pwrite inside if needed
    if (copy_file_range_full(src_fd, dest_fd, st.st_size)) {
      cfr_ok[i] = true;
    } else {
      mark_failed(i);
      any_cfr_failed = true;
    }
    ::close(src_fd);
    ::close(dest_fd);
  }

  if (!any_cfr_failed) {
    // all files copied successfully via copy_file_range
    return failed_mask;
  }

  // fallback path: use io_uring READ+WRITE pairs to asynchronously copy the
  // files that copy_file_range could not handle (cross-device, etc.)
  //
  // design:
  //   queue_depth  slots are kept active simultaneously, each slot owns:
  //     - src_fd / dest_fd
  //     - a 256 KiB heap buffer
  //     - state machine: idle → read_inflight → write_inflight → next_chunk
  //
  //   SQE user_data encodes (slot_index << 1 | is_write) so completions can
  //   be dispatched back to the right slot
  //
  //   we submit as many SQEs as will fit in one io_uring_enter call, then
  //   drain completions
  //   The loop exits when every file has been fully
  //   transferred or marked failed
  IoUring ring;
  if (!ring.init(queue_depth * 2)) {
    // io_uring unavailable (old kernel or permissions) — do sequential fallback
    for (size_t i = 0; i < n; ++i) {
      if (cfr_ok[i]) {
        continue;
      }
      if (!copy_file(srcs[i], dests[i])) {
        mark_failed(i);
      } else {
        // clear any previous failure mark from the cfr attempt
        failed_mask[i / 64] &= ~(u64{1} << (i % 64));
      }
    }
    return failed_mask;
  }

  // collect the indices that still need copying
  std::vector<size_t> pending;
  pending.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    if (!cfr_ok[i]) {
      pending.push_back(i);
    }
  }

  // allocate slots
  const u32 num_slots =
      std::min<u32>(queue_depth, static_cast<u32>(pending.size()));
  std::vector<CopySlot> slots(num_slots);
  for (auto& s : slots) {
    s.buf =
        static_cast<u8*>(mmap(nullptr, CopySlot::kChunk, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (s.buf == MAP_FAILED) {
      s.buf = nullptr;
    }
  }

  size_t next_pending = 0;  // index into `pending`
  u32 inflight = 0;         // SQEs in flight

  // assign a file to a slot and issue the first read sqe
  // returns false if slot init fails (marks the file as failed)
  auto assign_slot = [&](u32 slot_idx) -> bool {
    if (next_pending >= pending.size()) {
      return false;
    }
    CopySlot& s = slots[slot_idx];
    const size_t fi = pending[next_pending++];
    s.file_idx = fi;
    s.rd_off = 0;
    s.wr_off = 0;
    s.failed = false;
    s.rd_inflight = false;
    s.wr_inflight = false;
    s.rd_bytes = 0;

    s.src_fd = open(srcs[fi].c_str(), O_RDONLY | O_NOATIME);
    if (s.src_fd < 0) {
      mark_failed(fi);
      s.failed = true;
      return false;
    }
    struct stat st{};
    if (fstat(s.src_fd, &st) < 0) {
      mark_failed(fi);
      s.close_fds();
      s.failed = true;
      return false;
    }
    s.total = st.st_size;

    s.dest_fd = open(dests[fi].c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR);
    if (s.dest_fd < 0) {
      mark_failed(fi);
      s.close_fds();
      s.failed = true;
      return false;
    }

    if (s.total == 0) {
      // nothing to copy
      s.close_fds();
      // clear any old failure
      failed_mask[fi / 64] &= ~(u64{1} << (fi % 64));
      return false;  // slot is done immediately, re-assign
    }

    if (!s.buf) {
      // no buffer: fall back to synchronous copy_file for this file
      const bool ok = copy_file_range_full(s.src_fd, s.dest_fd, s.total);
      s.close_fds();
      if (!ok) {
        mark_failed(fi);
      } else {
        failed_mask[fi / 64] &= ~(u64{1} << (fi % 64));
      }
      s.failed = true;  // slot done (not actually failed, just done)
      return false;
    }

    // issue first read SQE
    const u32 to_read =
        static_cast<u32>(std::min<off_t>(CopySlot::kChunk, s.total - s.rd_off));
    if (!ring.push_rw(IORING_OP_READ, s.src_fd, s.buf, to_read,
                      static_cast<u64>(s.rd_off), make_tag(slot_idx, false))) {
      // ring full; caller will retry after draining
      next_pending--;  // undo advance
      s.close_fds();
      s.failed = true;
      return false;
    }
    s.rd_inflight = true;
    ++inflight;
    return true;
  };

  // prime the slots
  for (u32 s = 0; s < num_slots; ++s) {
    while (!assign_slot(s) && next_pending < pending.size()) {
      // slot was empty-file or buffer-alloc-failed; keep trying next file
    }
  }

  // submit whatever is in the ring
  if (inflight > 0) {
    ring.submit(inflight);
  }

  // main completion loop
  while (inflight > 0) {
    ring.drain(1, [&](u64 user_data, i32 res) {
      --inflight;
      const u32 si = tag_slot(user_data);
      const bool is_write = tag_is_write(user_data);
      CopySlot& s = slots[si];

      if (s.failed) {
        return;
      }

      if (is_write) {
        // write completed
        s.wr_inflight = false;
        if (res < 0) {
          mark_failed(s.file_idx);
          s.close_fds();
          s.failed = true;
          return;
        }
        s.wr_off += res;

        if (s.wr_off >= s.total) {
          // file fully written
          s.close_fds();
          failed_mask[s.file_idx / 64] &=
              ~(u64{1} << (s.file_idx % 64));  // clear failure

          // assign next pending file to this slot
          while (next_pending < pending.size() && !assign_slot(si)) {}
          if (next_pending > 0 && slots[si].rd_inflight) {
            ring.submit(1);
            ++inflight;  // we added a new SQE; account for it
          }
          return;
        }

        // more to write — if we have fresh read data, write the next slice
        // (read and write are already serialised per-slot: we only issue a
        // write after read completes, and only issue the next read after
        // write completes to keep ordering simple)
        if (s.rd_off < s.total && !s.rd_inflight) {
          const u32 to_read = static_cast<u32>(
              std::min<off_t>(CopySlot::kChunk, s.total - s.rd_off));
          if (ring.push_rw(IORING_OP_READ, s.src_fd, s.buf, to_read,
                           static_cast<u64>(s.rd_off), make_tag(si, false))) {
            s.rd_inflight = true;
            ring.submit(1);
            ++inflight;
          } else {
            // ring temporarily full: will re-try on next iteration
          }
        }
      } else {
        // read completed
        s.rd_inflight = false;
        if (res <= 0) {
          if (res < 0) {
            mark_failed(s.file_idx);
            s.close_fds();
            s.failed = true;
          }
          return;
        }
        s.rd_bytes = res;
        s.rd_off += res;

        // issue write SQE for the bytes just read
        if (ring.push_rw(IORING_OP_WRITE, s.dest_fd, s.buf,
                         static_cast<u32>(s.rd_bytes),
                         static_cast<u64>(s.wr_off), make_tag(si, true))) {
          s.wr_inflight = true;
          ring.submit(1);
          ++inflight;
        } else {
          // ring full — synchronously write to avoid deadlock
          ssize_t written = 0;
          while (written < s.rd_bytes) {
            const ssize_t w = pwrite(s.dest_fd, s.buf + written,
                                     static_cast<size_t>(s.rd_bytes - written),
                                     s.wr_off + written);
            if (w <= 0) {
              mark_failed(s.file_idx);
              s.close_fds();
              s.failed = true;
              return;
            }
            written += w;
          }
          s.wr_off += s.rd_bytes;
          if (s.wr_off >= s.total) {
            s.close_fds();
            failed_mask[s.file_idx / 64] &= ~(u64{1} << (s.file_idx % 64));
            while (next_pending < pending.size() && !assign_slot(si)) {}
            if (slots[si].rd_inflight) {
              ring.submit(1);
              ++inflight;
            }
          }
        }
      }
    });
  }

  // cleanup buffers
  for (auto& s : slots) {
    s.close_fds();
    if (s.buf && s.buf != MAP_FAILED) {
      munmap(s.buf, CopySlot::kChunk);
    }
  }

  return failed_mask;
}

#endif  // IS_PLAT_LINUX

}  // namespace core

