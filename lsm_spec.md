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

## The property syntax
In LSM, every single item property such as an item name, an enchantment name, an enchantment level, or any other literal (constant value) gets stored directly, without any additional wrapping syntax. As an example, `protection 3` represents the protection III enchantment, and `diamond_pickaxe efficiency 5` represents a diamond pickaxe enchanted with efficiency V. 

## Block Instructions
Block instructions define special properties or structure of the code they wrap. The aggregated properties of all blocks wrapping an LSM instruction are always part of the context for that instruction. Each block instruction must be followed by a C-style block of code wrapped in curly parentheses: `{}`. The available block instructions are:

`pool pool_idx:uint32 {...}`\
Declares which pool of the loot table the sequence of code is targetting. `pool_idx` is a positive integer representing the loot pool's index in the loot table. For a loot table with N loot pools, the legal values of `pool_idx` are 0, 1, 2, ..., N-1. The first loot pool's index is 0.

`roll roll_count:[uint32] {...}`\
May only be defined inside a `pool` block. Declares that all instructions inside the block aggregate the results of `roll_count` rolls of the corresponding loot pool, or the natural, potentially pseudo-random number of rolls specified in the loot pool if `0` is provided instead. `roll 0` **does not advance the context's LCG state**; instead, a suitable `nextInt` output value is computed based on the **current LCG state**. To emulate regular generation of a loot pool, the following structure should be used:
```
pool 0
{
    lcg-advance 1;
    roll 0
    {
        //...
    }
}
```

`case item:[property] {...}`\
Must be defined inside a `roll` block. `case` instructions are core building blocks of each LSM program, and contain instructions on how the final kernel should handle every single item type.   

`lcg-fork-range start_advancement:int32 end_advancement:int32 step_size:int32 {...}`\
Forks the current LCG state: declares that the sequence of instructions inside the block should be executed multiple times in a loop, starting with a copy of the previously used LCG advanced by `start_advancement` steps (inclusive), and ending with `end_advancement` steps (inclusive). Every consecutive iteration will start with an LCG advanced by `step_size` more states than the previous one.\
For example, `lcg-fork-range -2 -10 -1; {...}` would make the first iteration of `{...}` start with an LCG advanced backwards by 2 states, the second one: by 3 states, and so on. The last iteration would start with a backward advancement of 10 states.

## The `filter-on` instruction
`filter-on` is an optional instruction that modifies the way the initial LCG state for the default context is calculated. It may only be declared once per LSM program, as the very first instruction. Other use of the instruction is not allowed and will result in compilation errors. 

`filter-on` takes a list of range constraints
on `nextInt(n)` calls, given as 3-element tuples as follows:\
`filter-on [[bound1:uint32 min1:uint32 max1:uint32], [bound2:uint32 min2:uint32 max2:uint32], ...];`

Each tuple `[bound min max]` represents the following constraint:\
`min <= nextInt(bound) <= max`\
If any given constraint is not satisfiable (i.e. `min > max` or `min >= bound`), the code will fail to compile.

`filter-on` will always translate to exactly one specific LCG state reversal method, depending on the number of parameters:
- state prediction if 1 range constraint with a non-power-of-two `bound` was provided,
- advanced reversal if multiple such constraints were given.


## Regular Instructions
Unlike block instructions, regular instructions always represent a single action or sequence of actions that will be performed by the kernel using data represented by the current context.
Below is a list of all regular instructions allowed by LSM: (TODO organize, maybe group some of these?)

`lcg-advance state_count:int32;`\
Advances the current context's LCG by `state_count` states.

`lcg-reset;`\
Resets the current context's LCG state to its initial state. 

`succeed;`\
Indicates that the current state of the current context's LCG satisfies all the requirements. This instruction will produce code that appends the corresponding 48-bit loot seed to a global result buffer. 

`fail;`\
Indicates that the current state of the current context's LCG cannot produce the desired results. This will compile to an always-false assertion.

`apply-function pool_id:[uint32] entry_id:[uint32] function_id:[uint32]`\
Instructs the compiler to emit code to call a specific loot function. The loot function is defined objectively by the parameters. For example, `apply-function 0 1 1;` would call the second function of the second entry of the first loot pool. Whether to store the results of the loot function or not is left as a task for the compiler.

`assert properties:[property, ...] operator:['==' | '!=' | '>=' | '>' | '<=' | '<'] item_count:[uint32_t];`\
Must be placed inside a case block.
Performs a context-based assertion that the number of items matches `item_count` using `operator`. If the assertion fails, a context-based fail action is performed. In the default context, the fail action terminates the current thread. However, in `lcg-fork-range` blocks, the fail action is treated as an instruction to jump to the next iteration of the lcg advancement range (same effect `continue` has on a C `for` loop).\

`assert-pool properties:[property, ...] operator:['==' | '!=' | '>=' | '>' | '<=' | '<'] item_count:[uint32_t];`\
Works the same as `assert` but treats the item count tested against the target value `item_count` as the final item count aggregated on a per-pool basis.

For clarification, below are some code examples of assertions and their interpretation:

```
pool 0
{
    roll 0
    {
        case iron_ingot
        {
            assert-pool >= 14;  
        }
        //...
    }
    lcg-reset;
    succeed;
}
```
This simple example would produce code that generates the first loot pool of a loot table and checks if at least 14 iron ingot items were rolled in total.

```
pool 0
{
    roll 0
    {
        case gold_ingot diamond
        {
            fail;
        }
        case iron_ingot
        {
            assert-pool >= 14;
            assert-pool <= 16;
        }
        //...
    }
    lcg-reset;
    succeed;
}
```
In this example, apart from filtering for 14-16 iron ingots, the resulting code would also make sure no gold ingots or diamonds were generated.

```
pool 0
{
    roll natural
    {
        case gold_ingot
        {
            assert > 10;
        }
        case diamond
        {
            assert > 4;
        }
        //...
    }
    lcg-reset;
    succeed;
}
```
This largely impractical example would produce code searching for LCG states where **every single roll** of the first loot pool yields either more than 10 gold ingots, more than 4 diamonds, or any other item.

`test-layout [slot1:uint32, slot2:uint32, ..., slot27:uint32];`
Generates the full contents of the chest loot table and tests whether the layout of item counts matches the one provided. This is a really slow operation and should only be executed once all other assertions have passed, or preferably not executed at all if the other assertions have sufficient filtering strength. For clarification, below is an example `test-layout` instruction along with the arrangement of items it would test for:
```
test-layout
    0, 0, 3, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 2, 0, 0, 1, 0, 7, 
    5, 1, 0, 2, 0, 0, 0, 0, 0;

```
|   |   |   |   |   |   |   |   |   |   
|---|---|---|---|---|---|---|---|---|
| - | - | 3 | - | - | 1 | 1 | - | - |
| - | - | - | 2 | - | - | 1 | - | 7 |   
| 5 | 1 | - | 2 | - | - | - | - | - |   


## Code Examples

```
pool 0
{
    lcg-advance 1;
    roll natural;
    {
        case golden_pickaxe
        {
            apply-function 0 5 0;
            pool-assert efficiency 5 >= 2;
            pool-assert efficiency 3 >= 1;
        }
        case flint_and_steel
        {
            pool-assert >= 1; // would translate to boolean flag being set
        }
        case golden_nugget iron_nugget obsidian
        {
            lcg-advance 1;
        }
        case golden_sword
        {
            apply-function 0 6 0;
        }
        case golden_axe
        {
            apply-function 0 7 0;
        }
        //...
        case gold_block enchanted_golden_apple bell
        {
            fail; // for lootseed cracking or specific kinds of lootseed finding
        }
    }
    lcg-reset;
    succeed;
}
```
