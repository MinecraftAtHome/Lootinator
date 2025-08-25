# LSM - Loot Kernel Assembly

## Warning: WIP
This document is a work-in-progress, so things will change dramatically.

## Purpose
LSM is an intermediate language used for translation of loot table constraints to efficient CUDA kernels searching for loot seeds that satisfy those constraints. The main purpose of LSM is to divide the translation process into two manageable steps: translating loot constraints to LSM, and then translating LSM to the final CUDA source. This document will describe the syntax and technical details of the language.

## Block Instructions

Block instructions define special properties of the code they wrap. Each block instruction must be colon-terminated and followed by a C-style block of code wrapped in parentheses: `{}`. The available block instructions are:

`pool [pool_idx:uint32]; {...}`\
Declares which pool of the loot table the sequence of code is targetting. `pool_idx` is a positive integer representing the loot pool's index in the loot table. For a loot table with N loot pools, the legal values of `pool_idx` are 0, 1, 2, ..., N-1. The first loot pool's index is 0.

`roll [roll_count:uint32 | 'natural']; {...}`\
May only be defined inside a `pool` block. Declares that all instructions inside the block aggregate the results of `roll_count` rolls of the corresponding loot pool, or the natural, potentially pseudo-random number of rolls specified in the loot pool if the `natural` keyword is provided instead.

`lcg-fork-range [start_advancement:int32] [end_advancement:int32] [step_size:int32]; {...}`\
Forks the current LCG state: declares that the sequence of instructions inside the block should be executed multiple times in a loop, starting with a copy of the previously used LCG advanced by `start_advancement` steps (inclusive), and ending with `end_advancement` steps (inclusive). Every consecutive iteration will start with an LCG advanced by `step_size` more states than the previous one.\
For example, `lcg-fork-range -2 -10 -1; {...}` would make the first iteration of `{...}` start with an LCG advanced backwards by 2 states, the second one: by 3 states, and so on. The last iteration would start with a backward advancement of 10 states.