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

`pool pool_idx:uint32; {...}`\
Declares which pool of the loot table the sequence of code is targetting. `pool_idx` is a positive integer representing the loot pool's index in the loot table. For a loot table with N loot pools, the legal values of `pool_idx` are 0, 1, 2, ..., N-1. The first loot pool's index is 0.

`roll roll_count:[uint32 | 'natural']; {...}`\
May only be defined inside a `pool` block. Declares that all instructions inside the block aggregate the results of `roll_count` rolls of the corresponding loot pool, or the natural, potentially pseudo-random number of rolls specified in the loot pool if the `natural` keyword is provided instead. `natural` rolls use the LCG defined of the `roll` instruction's context.

`lcg-fork-range start_advancement:int32 end_advancement:int32 step_size:int32; {...}`\
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

## The `filter-on` instruction
`filter-on` is an optional instruction that modifies the way the initial LCG state for the default context is calculated. It may only be declared once per LSM program, as the first instruction inside the first `pool` block in the code. TODO perhaps we should change this to be independent of pools and operate on raw PRNG functions?

## Regular Instructions
Unlike block instructions, regular instructions always represent a single action or sequence of actions that will be performed by the kernel using data represented by the current context.
Below is a list of all regular instructions allowed by LSM: (TODO organize, maybe group some of these?)

`lcg-advance state_count:int32`\
Advances the current context's LCG by `state_count` states.

`count-items {item1:item(), item2:item(), ...}`\
May only be located inside a `roll` block. Instructs the LSM assembler to aggregate the item counts for the provided item types.

`count-enchanted-items {item1:item(), item2:item(), ...}`\
May only be located inside a `roll` block. Instructs the LSM assembler to aggregate the item counts of distinct enchanted items for the provided item types.

`assert properties:{[item() | enchantment() | enchantment_level()], ...} operator:['==' | '!=' | '>=' | '>' | '<=' | '<'] r_property:[item_count() | enchantment_level()]`\
*NOTE: decided to drop the -eq, -ge, -le suffix from assert in favor of the operator argument, can change back if needed.*\
Performs a context-based assertion that the `r_property` value(s) of the item(s) matching the specified `properties` satisfies the given `r_property` value when compared using `operator`. If the assertion fails, a context-based fail action is performed. In the default context, the fail action terminates the current thread. However, in `lcg-fork-range` blocks, the fail action is treated as an instruction to jump to the next iteration of the lcg advancement range (same effect `continue` has on a C `for` loop).\
For clarification, below are some code examples of assertions and their effects:

```
pool 0;
{
    roll natural;
    {
        count-items item(minecraft:iron_ingot);
        assert item(minecraft:iron_ingot) >= item_count(14);
    }
}
```
This simple example would produce code that generates the first loot pool of a loot table and checks if at least 14 iron ingot items were rolled in total.

```
pool 0;
{
    roll natural;
    {
        assert item(minecraft:gold_ingot) == item_count(0);
        assert item(minecraft:diamond) == item_count(0);

        count-items item(minecraft:iron_ingot);
        assert item(minecraft:iron_ingot) >= item_count(14);
        assert item(minecraft:iron_ingot) <= item_count(16);
    }
}
```
In this example, apart from filtering for 14-16 iron ingots, the resulting code would also make sure no gold ingots or diamonds were generated. Note the use of assertions on item counts without the aggregating `count-items` statement - this would be reflected as a repeated assertion for each of the individual rolled entries. This is further illustrated in the example below.

```
pool 0;
{
    roll natural;
    {
        assert item(minecraft:gold_ingot) > item_count(10);
        assert item(minecraft:diamond) > item_count(4);
    }
}
```
This largely impractical example would produce code searching for LCG states where **every single roll** of the first loot pool yields either more than 10 gold ingots, more than 4 diamonds, or any other item.\
**TODO is this actually the behavior we want for these assert statements?**