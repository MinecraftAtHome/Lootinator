#include <cstdio>

#ifndef SHARED_DEFINITIONS
//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = 0xffffffffffff;

__device__ inline void setSeed(u64 *rand, u64 value) { *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline i32 next(u64 *rand, const i32 bits)
{
    *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
    return (i32)((i64)*rand >> (48 - bits));
}
__device__ inline i32 nextInt(u64 *rand, const i32 n)
{
    if ((n - 1 & n) == 0)
    {
        u64 x = n * (u64)next(rand, 31);
        return (i32)((i64)x >> 31);
    }
    else
    {
        return (i32)(next(rand, 31) % n);
    }
}
__device__ inline float nextFloat(u64 *rand) { return next(rand, 24) / (float)(1 << 24); };
__device__ inline i32 nextIntBounded(u64 *rand, const i32 min, const i32 max)
{
    if (min >= max)
    {
        return min;
    }
    return nextInt(rand, max - min + 1) + min;
}
__device__ inline i32 nextIntNoAdvance(u64 *rand, const i32 n)
{
    if ((n - 1 & n) == 0)
    {
        u64 x = n * *rand;
        return (i32)((i64)x >> 31);
    }
    else
    {
        return (i32)(*rand % n);
    }
}
__device__ inline void write_result(u64 input_seed, u64 *result_array, u32 *result_count) { result_array[atomicAdd(result_count, 1)] = input_seed; }
//@SharedDefinitionsEnd
#endif
__device__ bool forward_filter(uint64_t loot_seed)
{
    extern __shared__ u32 data[];
    {
        i32 local_constraints[1] = {0};
        i32 rolls = nextIntBounded(&loot_seed, 4, 8);
        for (i32 roll = 0; roll < rolls; roll++)
        {
            int item = data[0 + nextInt(&loot_seed, 398)];
            switch (item)
            {
            case 8:
            {
                i32 item_count = 1;
                item_count = 1 + nextInt(&loot_seed, 2);
                local_constraints[0] += item_count;
                break;
            }
            case 0:
            {
                i32 item_count = 1;
                item_count = 1 + nextInt(&loot_seed, 4);
                break;
            }
            case 18:
            {
                i32 item_count = 1;
                item_count = 9 + nextInt(&loot_seed, 10);
                break;
            }
            case 17:
            {
                i32 item_count = 1;
                break;
            }
            case 21:
            {
                i32 item_count = 1;
                break;
            }
            case 3:
            {
                i32 item_count = 1;
                break;
            }
            case 5:
            {
                i32 item_count = 1;
                item_count = 4 + nextInt(&loot_seed, 21);
                break;
            }
            case 9:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 10);
                u32 max_level = data[401 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 4:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 9);
                u32 max_level = data[411 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 10:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 6);
                u32 max_level = data[420 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 7:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 6);
                u32 max_level = data[426 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 23:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 6);
                u32 max_level = data[432 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 19:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 12);
                u32 max_level = data[438 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 13:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 9);
                u32 max_level = data[450 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 2:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 11);
                u32 max_level = data[459 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 6:
            {
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 9);
                u32 max_level = data[470 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                break;
            }
            case 22:
            {
                i32 item_count = 1;
                item_count = 4 + nextInt(&loot_seed, 9);
                break;
            }
            case 16:
            {
                i32 item_count = 1;
                break;
            }
            case 24:
            {
                i32 item_count = 1;
                break;
            }
            case 15:
            {
                i32 item_count = 1;
                item_count = 4 + nextInt(&loot_seed, 9);
                break;
            }
            case 12:
            {
                i32 item_count = 1;
                break;
            }
            case 14:
            {
                i32 item_count = 1;
                item_count = 2 + nextInt(&loot_seed, 7);
                break;
            }
            case 1:
            {
                i32 item_count = 1;
                break;
            }
            case 11:
            {
                i32 item_count = 1;
                break;
            }
            case 20:
            {
                i32 item_count = 1;
                item_count = 1 + nextInt(&loot_seed, 2);
                break;
            }
            }
        }
        if (!(local_constraints[0] >= 10))
            return false;
    }
    {
        i32 rolls = nextIntBounded(&loot_seed, 1, 1);
        for (i32 roll = 0; roll < rolls; roll++)
        {
            int item = data[398 + nextInt(&loot_seed, 3)];
            switch (item)
            {
            case -1:
            {
                i32 item_count = 1;
                break;
            }
            case 25:
            {
                i32 item_count = 1;
                item_count = 1 + nextInt(&loot_seed, 2);
                break;
            }
            }
        }
    }
    return true;
}
__global__ void kernel_1(u64 *result_array, u32 *result_count, u32 *shared_mem_contents, u32 shared_mem_contents_length, u64 offset)
{
    extern __shared__ u32 data[];
    if (threadIdx.x < shared_mem_contents_length)
    {
        for (int i = threadIdx.x; i < shared_mem_contents_length; i += blockDim.x)
        {
            data[i] = shared_mem_contents[i];
        }
    }
    __syncthreads();

    u64 input_seed = (u64)blockIdx.x * blockDim.x + threadIdx.x + offset;

    if (forward_filter(input_seed))
    {
        printf("%llu\n", input_seed);
        //write_result(input_seed, result_array, result_count);
    }
}
