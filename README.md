# chrb

Chunk rollback tool for Minecraft

> `chrb` stands for _CHunk Roll Back_

## Overview

`chrb` is a high-performance command-line tool designed to restore specific areas of a Minecraft world from a backup.  

Unlike traditional restoration methods that require replacing entire region or world files, the tool allows you to pinpoint exact chunk coordinates to roll back.  
This is useful for fixing localized griefing, corrupted chunks, or reversing accidental changes in a specific district while keeping the rest of your world's progress intact.

## Key Features

- High Performance: Leverages multi-threaded processing and **hybrid restoration** to handle gigabytes of world data in seconds
  - Hybrid Restoration: Automatically switches between fine grained chunk-level updates and fast region-level file replacement based on the requested area
- Atomic Safety: Uses temporary file staging and atomic renaming to prevent world corruption even if the process is interrupted
- Comprehensive Support: Granular control over different data types, including Terrain (`region`), Entities (`entities`), and Point of Interest (`poi`)

## Installation

(To run, replace `[options]` with proper configuration)

### Download prebuilt binary (Recommended)

Install from [GitHub Release Page](https://github.com/pug523/chrb/releases/latest)

```shell
./chrb [options]
```

### Build manually with xmake
#### Dependencies

- [xmake](https://xmake.io/posts/quickstart-1-installation.html)

#### Installation Workflow

```shell
xmake config -c -y -m release
xmake build chrb

# A: run with xmake
xmake run chrb [options]

# B: install to the directory you like and run it
xmake install --installdir="./install"
./install/bin/chrb [options]

# C: install to your system and run it
sudo xmake install --root # linux / macos
chrb --version # check if installed
chrb [options]

```

## Quick Start

If you haven't installed `chrb`, see [Installation](#installation)  
You need 2 minecraft worlds: the destination world and the source world (your backup).
Hereafter, $c(x, z)$ means that "Chunk coordinates $(x, z)$"

Assumed situation:
```shell
ls ./my_backup
#  entities   poi   region

ls ./my_world
#  advancements     DIM-1              level.dat_old     poi
#  serverconfig     data               DIM1              icon.png
#  region           session.lock       carpet.conf       datapacks
#  entities         level.dat          playerdata        stats
```

### Simplest Usage

```shell
chrb --src ./my_backup --dest ./my_world --min_x -3 --min_z 2 --max_x 5 --max_z 4
```

> [!NOTE]
Chunk range is inclusive, so $c(0,\ 0)$ to $c(2,\ 1)$ will be interpreted as $\{\ c(0, 0),\ c(0, 1),\ c(1, 0),\ c(1, 1),\ c(2, 0),\ c(2, 1)\ \}$

### Customize More
Rollback **only entities** in $c(35, 25)$ to $c(50, 75)$ in the Nether using 8 worker threads with verbose output

```shell
chrb --src ./my_backup --dest ./my_world --dim nether --type=entities --min_x 35 --min_z 25 --max_x 50 --max_z 75 -j 8 --verbose
```

Rollback all(region/entities/poi) in $c(-1, -2)$ to $c(3, 4)$ in the End using 16 worker threads

```shell
chrb --src ./my_backup --dest ./my_world --dim end --type=all --min_x -1 --min_z -2 --max_x 3 --max_z 4 -j 16
```

### Show Help: `chrb -h` or `chrb --help`

- Multiple values specification like a `--type=region,entities` is currently not supported.
```
                     chrb

  =-=-= chunk rollback tool for minecraft =-=-=

Usage: chrb [Options]

Options:
  -s, --src <path>                             source world directory [required]
  -d, --dest <path>                            destination world directory [required]
  -D, --dim <overworld|nether|end>             target dimension [optional, default:overworld]
  -t, --type <region|entities|poi|all>         rollback type [optional, default:all]
  -x, --min_x <n>                              minimum chunk x coordinate [required]
  -X, --max_x <n>                              maximum chunk x coordinate [required]
  -z, --min_z <n>                              minimum chunk z coordinate [required]
  -Z, --max_z <n>                              maximum chunk z coordinate [required]
  -j, --num_threads <n>                        number of worker threads [optional, default:half of num threads on your hardware]
  -b, --bulk_copy                              use bulk copy for full region rollback [optional]
  -V, --verbose                                enable verbose output [optional]
  -h, --help                                   print this help message [optional]
  -v, --version                                print version [optional]
```

## Safety
> [!CAUTION]
Always create backup of your world and ensure the Minecraft server is stopped before running this tool.  
Direct manipulation of MCA files carries inherent risks, and I'm not responsible for any harm caused by this tool.  

## Technical Information

See [ARCHITECTURE.md](./ARCHITECTURE.md).

## Tested Versions

- 1.21.1

## License
Copyright &copy; 2026 pugur  
This project is licensed under the Apache License, Version 2.0 which can be found in the [LICENSE](./LICENSE) file.
