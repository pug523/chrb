# Mechanism

## ChunkProcessor Mechanism

The `ChunkProcessor` synchronizes specific chunks between source and destination MCA files by directly manipulating the Region File Format.  
See [src/region/chunk_processor.cc](src/region/chunk_processor.cc).

### Processing Logic

For each chunk coordinate $(cx, cz)$, the processor compares the Location Table (the first 4 KiB of a MCA file) of both source and destination to determine the required action:
For each chunk coordinate $(cx, cz)$, the processor compares the Location Table entries to decide the action:

- Deletion: If a chunk exists in the destination but is missing in the source, the processor zeroes out the corresponding 4 bytes entry. This unlinks the chunk, marking its sectors as available.
- Addition: If a chunk is new, the processor reads the source data, finds a suitable gap in the destination using the Sector Map, and appends or inserts the data.
- In-place Overwrite: Used if the new data fits within the currently allocated sectors.
- Relocation: If the data grows, the old sectors are freed, and the chunk is moved to a new offset to prevent overwriting adjacent data.

#### 1. Chunk Deletion
- Condition: Exists in Destination, but missing in Source.
- Action: The processor zeroes out the 4-byte entry in the destination's location table. This effectively "unlinks" the chunk from the world, making the sectors available for future overwrites.

#### 2. Chunk Addition
- Condition: Missing in Destination, but exists in Source.
- Action:
    1. Reads the chunk data from the source file.
    2. Scans the destination file for a gap (free sectors) using a sector map.
    3. If no suitable hole is found, it appends the data to the end of the file.
    4. Updates the destination's location table and sets the current timestamp.

#### 3. Chunk Update (Overwrite vs. Relocation)
- Condition: Exists in both Source and Destination.
- Action:
    - In-place Overwrite: If the new chunk data fits within existing allocated sectors, the data is written directly to the same offset.
    - Relocation: If the new data is larger than the allocated space, the processor:
        1. Marks the old sectors as free.
        2. Finds a new offset that can accommodate the larger size.
        3. Writes the data and updates the location table to point to the new offset.

### Key Components

| Component      | Description                                                                                                   |
| :------------- | :------------------------------------------------------------------------------------------------------------ |
| Location Table | A 4 KiB header where each 4-byte entry contains a 3-byte offset and a 1-byte sector count.                    |
| Sector Map     | A bitset-like structure used to track which 4KiB sectors in the destination file are currently occupied.      |
| Relocation     | A mechanism to prevent file corruption when a chunk grows, ensuring it doesn't overwrite adjacent chunk data. |

### Technical Details

- Sector Size: All operations are aligned to `kSectorSize` (4 KiB).
- Chunk Indexing: Calculated as ($(cx \ \& \ 31) + ((cz \ \& \ 31) \ll 5)$).
- Timestamp: The second 4 KiB page of the MCA file is updated with the current Unix timestamp whenever a chunk is modified.
- I/O Backend: All file access uses fast `MappedFile`.

---

## FullRegionProcessor Mechanism

The `FullRegionProcessor` handles high-efficiency rollbacks when an entire region (32x32 chunks) needs to be restored. Instead of parsing internal MCA structures, it operates at the file-system level.  
See [src/region/full_region_processor.cc](src/region/full_region_processor.cc).

### Processing Logic
- Direct File Replacement: If a region is fully contained within the rollback boundaries, the processor replaces the destination `.mca` file with the source file.
- Atomic Write:
    1. Opens the source MCA via `MappedFile` (mmap).
    2. Creates a `.tmp` file in the destination directory, sized to match the source.
    3. Maps the `.tmp` file and copies data via `memcpy` with `madvise(MADV_SEQUENTIAL)` for prefetch optimization.
    4. Calls `msync(MS_SYNC)` to flush pages to disk before rename.
    5. Renames the `.tmp` file to the final destination filename (atomic on same filesystem).
       - This ensures that a crash during the copy process doesn't leave the destination world with a corrupted or half-written region file.

---

## RollbackExecutor Mechanism

The `RollbackExecutor` is the multi-threaded orchestration engine. It determines the most efficient processing strategy (Full vs. Partial) based on the requested coordinates and manages the worker lifecycle.  
See [src/region/rollback_executor.cc](src/region/rollback_executor.cc).

### 1. Task Scheduling & Optimization
The executor divides the world into "tasks" based on region boundaries ($512 \times 512$ blocks). It automatically selects the optimal `RollbackMode`:

| Mode     | Trigger Condition                                                                | Execution Strategy                                                          |
| :------- | :------------------------------------------------------------------------------- | :-------------------------------------------------------------------------- |
| FullCopy | The requested range completely covers the $32 \times 32$ chunk area of a region. | Uses `FullRegionProcessor` for instant file-level restoration.              |
| Partial  | The requested range only partially overlaps a region.                            | Uses `ChunkProcessor` to surgically update specific chunks within the file. |

### 2. Multi-threaded Architecture
- Worker Pool: Spawns a configurable number of worker threads (up to `std::hardware_concurrency`).
- Thread Safety: Uses a synchronized `std::queue` protected by a `std::mutex` and `std::condition_variable` to distribute tasks.
- Atomic Progress Tracking: Uses `std::atomic<u64>` to track processed chunk counts across all threads without performance bottlenecks.

### 3. Workflow Lifecycle
1. Init: Loads configuration (paths, thread count, coordinates).
2. Start: - Calculates the required regions.
   - Pushes `RollbackTask` objects into the queue.
   - Wakes up worker threads.
3. Execution: Workers pull tasks and route them to either `FullRegionProcessor` or `ChunkProcessor`.
4. Flush/Cleanup: Signals threads to stop and joins them, ensuring all I/O operations are completed before the application exits.

---

## Summary of Execution Flow

1. User defines a 3D box $(\min x, \min z)$ to $(\max X, \max Z)$.
2. RollbackExecutor identifies all affected `.mca` files for `region`, `entities`, and `poi`.
3. Region is fully contained within the box $\implies$ pass to FullRegionProcessor (Fast).
4. Region is not fully contained within the box $\implies$ pass to ChunkProcessor (Precise).
5. Workers perform the binary patching or file replacement in parallel.
