<p align="center">
  <img align="center" alt="Lootinator logo of a Minecraft chest" src="https://github.com/user-attachments/assets/b6332fb1-26c8-4cda-acb3-eb919dae957a"></img>
</p>
<h1 align="center"><b>Lootinator</b></h1>
<p align="center">
  <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/MinecraftAtHome/Lootinator?style=for-the-badge"></img>
  <img alt="GitHub top language" src="https://img.shields.io/github/languages/top/MinecraftAtHome/Lootinator?style=for-the-badge">
  <img alt="GitHub Downloads" src="https://img.shields.io/github/downloads/MinecraftAtHome/Lootinator/total?style=for-the-badge">
  <img alt="GitHub License" src="https://img.shields.io/github/license/MinecraftAtHome/Lootinator?style=for-the-badge">
</p>
<p align="center">
  GPU accelerated Minecraft loot finding
</p>

## About

Lootinator finds _loot seeds_ - the internal seeds Minecraft uses to determine chest loot.

Lootinator does not find loot seeds directly, instead it generates optimized CUDA kernels that can be compiled and run on any NVIDIA graphics card from 2012 onwards.

## Prerequisites

- **NVIDIA GPU** - required to run the generated kernel.
- **NVIDIA CUDA Toolkit** - provides `nvcc` for compiling the kernel. Download from <https://developer.nvidia.com/cuda-downloads>.
- **To build from source:** CMake 3.11+, a C++11 compiler (MSVC/Visual Studio on Windows, GCC/Clang on Linux), and Git.

> Pre-built binaries for Windows and Linux are on the [releases page](https://github.com/MinecraftAtHome/Lootinator/releases) - no compiler needed.

## Getting Started

### 1. Download or build the binary

#### Option A

Download a pre-built Windows or Linux `lootinator-cli` binary from the [releases page](https://github.com/MinecraftAtHome/Lootinator/releases).

#### Option B

Clone the repository to build the `lootinator-cli` binary from source.

```bash
git clone --recurse-submodules https://github.com/MinecraftAtHome/Lootinator.git
cd Lootinator
```

### 2. Prepare your inputs

You need two JSON files:

- **Loot table** for the chest type you are targeting - all versions available at <https://mcasset.cloud/latest>.
- **Constraint file** describing the items you want in the chest - schema described [below](#constraint-file-schema).

### 3. Generate the CUDA kernel

```bash
./lootinator-cli --loot-table <file_path.json> --constraint-file <file_path.json> -o <filepath> [-sc] [-sk] [-v <version>]
```

This produces a `.cu` source file.

### 4. Compile the kernel

```bash
nvcc output.cu -o loot_finder
```

### 5. Run

```bash
./loot_finder
```

A full example of the CLI can be found [below](#usage-example).

## CLI Reference

### Flags

| Flag                     | Description                                                                                 |
| ------------------------ | ------------------------------------------------------------------------------------------- |
| `-sc`, `--seedcracking`  | Only accept the **exact** chest contents specified in the constraint file (no extra items). |
| `-sk`, `--single-kernel` | Generate the predicted best kernel instead of benchmarking all possible kernels.            |
| `-v`, `--version`        | Target Minecraft version, e.g. `1.16` or `latest`.                                          |
| `--list-versions`        | List all supported Minecraft versions.                                                      |

### Constraint file schema

```json
[
  {
    "item": "<minecraft:item_name>",
    "range": {
      "min": "<min_count>",
      "max": "<max_count>"
    },
    "slot": "<slot_index_0_to_27>",
    "attributes": [
      {
        "type": "<enchant_name>",
        "level": "<enchant_level>"
      }
    ]
  }
]
[
    {
        // the name of the item you are targetting, must be prefixed with minecraft:
        "item": "minecraft:<item_name>",

        // the minimum and maximum item count for this constraint
        "range": {
            "min": <min_count>,
            "max": <max_count>,
        },

        // the slot of the item between 0-27 (currently ignored)
        "slot": <slot_index>,

        // optional attribute data for enchantments
		"attributes": [
			{
                "type": "<enchant_name>", // e.g. efficiency
                "level": <enchant_level> // number between 1-5 (optional)
            }
		]
    }
]
```

## Features

### CLI

- Support for Minecraft 1.13 and above (experimental).
- Support for any loot table defined as a JSON file (experimental).
- Two operation modes:
  - **Seedfinding** (default) - find chests containing at least the specified items.
  - **Seedcracking** (`-sc`) - find chests with the exact items specified.
- Built-in automated kernel benchmarking (disable with `-sk`).

### API

- Supports all operations provided by the CLI.
- See `lootinator/include/lootinator.h` for available functions.

### Coming Soon

- **Web interface** - run Lootinator directly in the browser using WebGPU.
- **Native GUI** - a native desktop application for Windows and Linux.

## Issues

If you have any issues, please report them at <https://github.com/MinecraftAtHome/Lootinator/issues>.

You can also join the `#lootinator` channel in the [Minecraft@Home Discord server](https://discord.gg/QmE5zeWVBA).

## Usage Example

Find ruined portal chests containing at least 4 enchanted golden apples in version 26.1.

**Loot table:** <https://mcasset.cloud/26.1.2/data/minecraft/loot_table/chests/ruined_portal.json>

**Constraint file** (`constraints_4_notches.json`):

```json
[
  {
    "item": "minecraft:enchanted_golden_apple",
    "range": { "min": 4, "max": 10000 },
    "slot": 0
  }
]
```

**Generate:**

```bash
./lootinator-cli --loot-table ruined_portal.json --constraint-file constraints_4_notches.json -o four_notches.cu -v 26.1
```

**Compile:**

```bash
nvcc four_notches.cu -o four_notches
```

**Run:**

```bash
./four_notches
```
