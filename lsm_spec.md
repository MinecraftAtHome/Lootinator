# LSM - Loot Kernel Assembly

## Warning: WIP
This document is a work-in-progress, so things will change dramatically.

## Purpose
LSM is an intermediate language used for translation of loot table constraints to efficient CUDA kernels searching for loot seeds that satisfy those constraints. The main purpose of LSM is to divide the translation process into two manageable steps: translating loot constraints to LSM, and then translating LSM to the final CUDA source. This document will describe the syntax and technical details of the language.

## About LSM Code
Each LSM program maps to a single CUDA kernel. A single thread of the resulting kernel checks a range of LCG states at some defined point in the loot generation process, and outputs all loot seeds satisfying the generated conditions to a global buffer. LSM hides away many aspects of the structure and technical realisation of the final CUDA kernel, most notably declarations and access to variables and arrays, or the implementation of loot functions.\
Operations on the LCG state unrelated with loot generation are stated explicitly in LSM code, but they avoid references to physical variables in favor of a mechanism further referred to as **context**.\
The default context of every LSM program contains the structure of the loot table for which constraints need to be satisfied. It also contains a single LCG state variable, initially set to the unique thread identifier of the CUDA kernel. The default context can be modified by **block instructions** and the `filter-on` **regular instruction**. Modifications of the context may include changes to the LCG state (`filter-on`, `lcg-fork-range`), or specification of additional properties (`pool`, `roll`).\
Any operation that would implicitly use variables in the final CUDA kernel uses the current context in LSM. For example, `lcg-advance 2;` uses the LCG of the context defined for the point where the instruction is located.

## Block Instructions
Block instructions define special properties or structure of the code they wrap. The aggregated properties of all blocks wrapping an LSM instruction are always part of the context for that instruction. Each block instruction must be colon-terminated and followed by a C-style block of code wrapped in parentheses: `{}`. The available block instructions are:

`pool [pool_idx:uint32]; {...}`\
Declares which pool of the loot table the sequence of code is targetting. `pool_idx` is a positive integer representing the loot pool's index in the loot table. For a loot table with N loot pools, the legal values of `pool_idx` are 0, 1, 2, ..., N-1. The first loot pool's index is 0.

`roll [roll_count:uint32 | 'natural']; {...}`\
May only be defined inside a `pool` block. Declares that all instructions inside the block aggregate the results of `roll_count` rolls of the corresponding loot pool, or the natural, potentially pseudo-random number of rolls specified in the loot pool if the `natural` keyword is provided instead. `natural` rolls use the LCG defined of the `roll` instruction's context.

`lcg-fork-range [start_advancement:int32] [end_advancement:int32] [step_size:int32]; {...}`\
Forks the current LCG state: declares that the sequence of instructions inside the block should be executed multiple times in a loop, starting with a copy of the previously used LCG advanced by `start_advancement` steps (inclusive), and ending with `end_advancement` steps (inclusive). Every consecutive iteration will start with an LCG advanced by `step_size` more states than the previous one.\
For example, `lcg-fork-range -2 -10 -1; {...}` would make the first iteration of `{...}` start with an LCG advanced backwards by 2 states, the second one: by 3 states, and so on. The last iteration would start with a backward advancement of 10 states.

## Named Values
In LSM, values related directly with loot generation must be stored with additional context, as named values. Each named value follows the format: `name(value)`. Below is the list of all named values recognized by LSM:

`item(type:string)`\
Defines an item. `type` must be the full Minecraft item id, e.g. `minecraft:iron_ingot` and not the shortened form `iron_ingot`.

`item_count(count:uint32)`\
Defines an item count. Depending on the context, this can mean both an aggregated count, or the count in a single loot roll. `item_count(0)` denotes "no instance of an item" and can be used for negative filtering.

`enchantment(type:string)`\
Defines an enchantment. `type` must be the full Minecraft enchantment id, e.g. `minecraft:protection` and not the shortened form `protection`.

`enchantment_level(level:uint32)`\
Defines an enchantment level. `enchantment_level(0)` denotes "no instance of an enchantment" and can be used for negative filtering.

## Regular Instructions
Unlike block instructions, regular instructions always represent a single action or sequence of actions that will be performed by the kernel using the current context.