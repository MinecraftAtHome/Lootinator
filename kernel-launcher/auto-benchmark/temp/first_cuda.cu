#ifndef SHARED_DEFINITIONS
//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

__device__ inline void setSeed(u64* rand, u64 value){ *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline i32 next(u64* rand, const i32 bits){ *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48; return (i32)((i64)*rand >> (48 - bits)); }
__device__ inline i32 nextInt(u64* rand, const i32 n){ if ((n-1 & n) == 0) {u64 x = n * (u64)next(rand, 31); return (i32)((i64)x >> 31);} else {return (i32)(next(rand, 31) % n);} }
__device__ inline float nextFloat(u64* rand){ return next(rand, 24) / (float)(1 << 24); }; 
__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline i32 nextIntNoAdvance(u64 *rand, const i32 n) {if ((n-1 & n) == 0) {u64 x = n * *rand; return (i32)((i64)x >> 31);} else {return (i32)(*rand % n);}} 
__device__ inline void write_result(u64 input_seed, u64 *result_array, u32 *result_count) {result_array[atomicAdd(result_count, 1)] = input_seed;}
//@SharedDefinitionsEnd
#endif
__device__ bool forward_filter(u64 loot_seed, u32 data[]) {
{
i32 local_constraints[1] = {0};
i32 rolls = nextIntBounded(&loot_seed, 4,8);
for (i32 roll = 0; roll < rolls; roll++) {
int item = data[0+ nextInt(&loot_seed, 398)];
switch (item) {
case 8: { //minecraft:obsidian
i32 item_count = 1;item_count = 1 + nextInt(&loot_seed, 2);break;}
case 0: { //minecraft:flint
i32 item_count = 1;item_count = 1 + nextInt(&loot_seed, 4);break;}
case 18: { //minecraft:iron_nugget
i32 item_count = 1;item_count = 9 + nextInt(&loot_seed, 10);break;}
case 17: { //minecraft:flint_and_steel
i32 item_count = 1;break;}
case 21: { //minecraft:fire_charge
i32 item_count = 1;break;}
case 3: { //minecraft:golden_apple
i32 item_count = 1;break;}
case 5: { //minecraft:gold_nugget
i32 item_count = 1;item_count = 4 + nextInt(&loot_seed, 21);break;}
case 9: { //minecraft:golden_sword
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 10);
u32 max_level = data[401+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 4: { //minecraft:golden_axe
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 9);
u32 max_level = data[411+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 10: { //minecraft:golden_hoe
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 6);
u32 max_level = data[420+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 7: { //minecraft:golden_shovel
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 6);
u32 max_level = data[426+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 23: { //minecraft:golden_pickaxe
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 6);
u32 max_level = data[432+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 19: { //minecraft:golden_boots
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 12);
u32 max_level = data[438+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 13: { //minecraft:golden_chestplate
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 9);
u32 max_level = data[450+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
if (enchantment == 4 && level == 3) local_constraints[0] += item_count;break;}
case 2: { //minecraft:golden_helmet
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 11);
u32 max_level = data[459+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 6: { //minecraft:golden_leggings
i32 item_count = 1;i32 enchantment = nextInt(&loot_seed, 9);
u32 max_level = data[470+ enchantment];
i32 level = nextIntBounded(&loot_seed, 1, max_level);
break;}
case 22: { //minecraft:glistering_melon_slice
i32 item_count = 1;item_count = 4 + nextInt(&loot_seed, 9);break;}
case 16: { //minecraft:golden_horse_armor
i32 item_count = 1;break;}
case 24: { //minecraft:light_weighted_pressure_plate
i32 item_count = 1;break;}
case 15: { //minecraft:golden_carrot
i32 item_count = 1;item_count = 4 + nextInt(&loot_seed, 9);break;}
case 12: { //minecraft:clock
i32 item_count = 1;break;}
case 14: { //minecraft:gold_ingot
i32 item_count = 1;item_count = 2 + nextInt(&loot_seed, 7);break;}
case 1: { //minecraft:bell
i32 item_count = 1;break;}
case 11: { //minecraft:enchanted_golden_apple
i32 item_count = 1;break;}
case 20: { //minecraft:gold_block
i32 item_count = 1;item_count = 1 + nextInt(&loot_seed, 2);break;}
}}
if (!(local_constraints[0]>=4)) return false;
}
{
i32 rolls = nextIntBounded(&loot_seed, 1,1);
for (i32 roll = 0; roll < rolls; roll++) {
int item = data[398+ nextInt(&loot_seed, 3)];
switch (item) {
case -1: { //
i32 item_count = 1;break;}
case 25: { //minecraft:lodestone
i32 item_count = 1;item_count = 1 + nextInt(&loot_seed, 2);break;}
}}
}
return true;
}extern "C" __global__ void kernel_1(u64* result_array, u32* result_count, u32* shared_mem_contents, u32 shared_mem_contents_length, u64 offset) {
    extern __shared__ u32 data[];
    if (threadIdx.x < shared_mem_contents_length) {
        for (int i = threadIdx.x; i < shared_mem_contents_length; i += blockDim.x) {
            data[i] = shared_mem_contents[i];
        }
    }
    __syncthreads();

    u64 input_seed = (u64)blockIdx.x * blockDim.x + threadIdx.x + offset;
    
    if (forward_filter(input_seed, data)) {
        write_result(input_seed ^ JRAND_MULTIPLIER, result_array, result_count);
    }
}

