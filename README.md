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
chrb --src ./my_backup --dest ./my_world --min-x -3 --min-z 2 --max-x 5 --max-z 4
```

> [!NOTE]
Chunk range is inclusive, so $c(0,\ 0)$ to $c(2,\ 1)$ will be interpreted as $\{\ c(0, 0),\ c(0, 1),\ c(1, 0),\ c(1, 1),\ c(2, 0),\ c(2, 1)\ \}$

### Customize More
Rollback **only entities** in $c(35, 25)$ to $c(50, 75)$ in the Nether using 8 worker threads with verbose output

```shell
chrb --src ./my_backup --dest ./my_world --dim nether --type=entities --min-x 35 --min-z 25 --max-x 50 --max-z 75 -j 8 --verbose
```

Rollback all(region/entities/poi) for all chunks specified in `chunks` that satisfy the chunk range constraint ($c(-1, -2)$ to $c(3, 4)$) 

```shell
chrb --src ./my_backup --dest ./my_world --chunks "-300.-300, 0.-1, -2.-1, 3.1" --min-x -5 --min-z -5 --max-x 5 --max-z 5  # This will skip (-300,-300) as it doesn't satisfy chunk range constraint
```

Rollback all(region/entities/poi) in $c(-1, -2)$ to $c(3, 4)$ in the End using 16 worker threads

```shell
chrb --src ./my_backup --dest ./my_world --dim end --type=all --min-x -1 --min-z -2 --max-x 3 --max-z 4 -j 16
```

Same as before, but **not to execute rollback** and check only how it will be executed with current options (dry run)

```shell
chrb --dry-run --src ./my_backup --dest ./my_world --dim end --type=all --min-x -1 --min-z -2 --max-x 3 --max-z 4 -j 16
```

Rollback with toml config file (options can be overwritten by command-line arguments)

```
chrb -c --config-path chrb.toml
```

config file looks like this: (note that keys are in snake_case)
```toml
[rollback_config]

src_world = "/path/to/my_backup"
dest_world = "/path/to/my_world"
dimension = "nether"
rollback_type = "all"
color = "auto"
src_world_structure = "old"
dest_world_structure = "old"
chunks = [
  [3, 4],
  [-35, -24],
  [5, 9],
  [-15, -24],
]
min_x = -25
max_x = 150
min_z = -25
max_z = 30
num_threads = 8
```

### Show Help: `chrb -h` or `chrb --help`

- Multiple values specification like a `--type=region,entities` is currently not supported.
```
                     chrb

  =-=-= chunk rollback tool for minecraft =-=-=

Usage: chrb [Options]

Options:
  -c, --config                                                 enable config toml file
      --config-path <path>                                     config file path (default: chrb.toml)
  -s, --src <path>                                             source world directory [required]
  -d, --dest <path>                                            destination world directory [required]
  -D, --dim <overworld|nether|end>                             target dimension (default: overworld)
  -t, --type <region|entities|poi|all>                         rollback type (default: all)
      --color <auto|always|never>                              color mode (default: auto)
  -w, --src-world-structure <auto|old|new|paper>               directory structure of source world. (old: DIM-1/, new: nether/, paper: world_nether/DIM-1/) (default: auto)
  -W, --dest-world-structure <auto|old|new|paper>              directory structure of dest world. (old: DIM-1/, new: nether/, paper: world_nether/DIM-1/) (default: auto)
  -C, --chunks <"x0.z0, x1.z1, ...">                           chunk positions to rollback (e.g. "10.20, -5.15")
  -x, --min-x <n>                                              minimum chunk x coordinate constraint
  -X, --max-x <n>                                              maximum chunk x coordinate constraint
  -z, --min-z <n>                                              minimum chunk z coordinate constraint
  -Z, --max-z <n>                                              maximum chunk z coordinate constraint
  -j, --num-threads <n>                                        number of worker threads (default: half of num threads on your hardware)
  -a, --allow-whole-rollback                                   allow whole world rollback when neither chunk range nor chunks are specified
  -b, --bulk-copy                                              use bulk copy for full region rollback
      --dry-run                                                only print scheduled chunks without executing rollback
  -S, --silent                                                 disable information logging
  -V, --verbose                                                enable verbose output
  -h, --help                                                   print this help message
  -v, --version                                                print version
```

## Installation

(To run, replace `[options]` with proper configuration)

### Download prebuilt binary (Recommended)

Simply download it from [GitHub Release Page](https://github.com/pug523/chrb/releases/latest) and extract it to the current directory. 
You can run it as follows:
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
# you can run it as follows:
./install/bin/chrb [options]

# C: install to your system and run it
sudo xmake install --root # linux / macos
chrb --version # check if installed
# you can run it as follows:
chrb [options]

```

## Safety
> [!CAUTION]
Always create backup of your world and ensure the Minecraft server is stopped before running this tool.  
Direct manipulation of MCA files carries inherent risks, and I'm not responsible for any harm caused by this tool.  

## Technical Information

See [ARCHITECTURE.md](./ARCHITECTURE.md).

## Tested Environments

- 1.19.4 - Paper
- 1.21.1 - Fabric
- 1.21.11 - Fabric
- 26.1 - Fabric

## License
Copyright &copy; 2026 pugur  
This project is licensed under the Apache License, Version 2.0 which can be found in the [LICENSE](./LICENSE) file.
