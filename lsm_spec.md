# LSM - Loot Kernel Assembly

## Warning: WIP
This document is a work-in-progress, so things will change dramatically.

## Purpose
LSM is an intermediate language used for translation of loot table constraints to efficient CUDA kernels searching for loot seeds that satisfy those constraints. The main purpose of LSM is to divide the translation process into two manageable steps: translating loot constraints to LSM, and then translating LSM to the final CUDA source. This document will describe the syntax and technical details of the language.

## Block Instructions

Block instructions define special properties of the code they wrap. Each block instruction is colon-terminated, and followed by a C-style block of code wrapped in parenthesis: `{}`. The available block instructions are:

`pool [pool_idx:uint32]; {...}`\
Declares which pool of the loot table the sequence of code is targetting. `pool_idx` is a positive integer representing the loot pool's index in the loot table. For a loot table with N loot pools, the legal values of `pool_idx` are 0, 1, 2, ..., N-1. The first loot pool's index is 0.

`roll [roll_count:uint32 | 'natural']; {...}`\
May only be defined inside a `pool` block.

`lcg-advance-range [start_advancement:int32] [end_advancement:int32] [step_size:int32]; {...}`\