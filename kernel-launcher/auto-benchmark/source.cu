#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cinttypes>
#include <iostream>

// start of shared definitions block
#define SHARED_DEFINITIONS
//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = 0xffffffffffff;

__device__ inline void setSeed(u64* rand, u64 value){ *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline i32 next(u64* rand, const i32 bits){ *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48; return (i32)((i64)*rand >> (48 - bits)); }
__device__ inline i32 nextInt(u64* rand, const i32 n){ if ((n-1 & n) == 0) {u64 x = n * (u64)next(rand, 31); return (i32)((i64)x >> 31);} else {return (i32)(next(rand, 31) % n);} }
__device__ inline float nextFloat(u64* rand){ return next(rand, 24) / (float)(1 << 24); }; 
__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline i32 nextIntNoAdvance(u64 *rand, const i32 n) {if ((n-1 & n) == 0) {u64 x = n * *rand; return (i32)((i64)x >> 31);} else {return (i32)(*rand % n);}} 
__device__ inline void write_result(u64 input_seed, u64 *result_array, u32 *result_count) {result_array[atomicAdd(result_count, 1)] = input_seed;}

// end of shared definitions block

#define CUDA_CHECK(ans) do { gpuAssert((ans), __FILE__, __LINE__); } while(false)
void gpuAssert(cudaError_t code, const char *file, int line) {
    if (code != cudaSuccess) {
        std::cerr << "CUDA error: " << cudaGetErrorString(code) << " at " << file << ":" << line << std::endl;
        exit(1);
    }
}

struct LaunchParameters {
    std::string kernel_name;
    std::vector<u32> kernel_shared_memory;
    u64 threads_total;
    u64 threads_per_batch;
    u32 threads_per_block;
    u32 device_id;
    i32 start_batch;
    i32 end_batch;
};
struct KernelMemory {
    u32* d_shared_mem_contents;
    u32 shared_mem_contents_length;
    u32 shared_mem_bytes;
    u64* d_result_array;
    u32* d_result_count;

    KernelMemory(const LaunchParameters& lp) {
        shared_mem_contents_length = lp.kernel_shared_memory.size();
        shared_mem_bytes = lp.kernel_shared_memory.size() * sizeof(u32);
        cudaMalloc(&d_shared_mem_contents, shared_mem_bytes);
        cudaMalloc(&d_result_array, 16384 * sizeof(u64));
        cudaMalloc(&d_result_count, sizeof(u32));
        cudaMemcpy(d_shared_mem_contents, lp.kernel_shared_memory.data(), shared_mem_bytes, cudaMemcpyHostToDevice);
    }
    ~KernelMemory() {
        cudaFree(d_shared_mem_contents);
        cudaFree(d_result_array);
        cudaFree(d_result_count);
    }
};
struct BenchmarkResults {
    std::string kernel_name;
    bool success;
    float ms_per_batch;
    float ms_total_estimate;
};

typedef void (*launch_function)(const LaunchParameters&, const KernelMemory&, u32, u64);

void launch_configured_kernel(launch_function lf, const LaunchParameters& lp, const KernelMemory& mem, bool print_results) {
    u64 h_result_array[16384];
    const u32 num_blocks = lp.threads_per_batch / lp.threads_per_block;
    for (u32 b = lp.start_batch; b < lp.end_batch; b++) {
        u32 h_result_count = 0;
        CUDA_CHECK(cudaMemcpy(mem.d_result_count, &h_result_count, sizeof(u32), cudaMemcpyHostToDevice));
        lf(lp, mem, num_blocks, b*lp.threads_per_batch);
        CUDA_CHECK(cudaDeviceSynchronize());

        CUDA_CHECK(cudaMemcpy(&h_result_count, mem.d_result_count, sizeof(u32), cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h_result_array, mem.d_result_array, h_result_count * sizeof(u64), cudaMemcpyDeviceToHost));
        
        if (!print_results) continue;
        for (u32 i = 0; i < h_result_count; i++) {
            std::cout << h_result_array[i] << '\n';
        }
        std::cout << std::flush;
    }
}


namespace kernel0 {
#ifndef SHARED_DEFINITIONS
//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = 0xffffffffffff;

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
}__global__ void kernel_1(u64* result_array, u32* result_count, u32* shared_mem_contents, u32 shared_mem_contents_length, u64 offset) {
    extern __shared__ u32 data[1024 * 16];
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


} //namespace
namespace kernel0 {
void launch(const LaunchParameters& lp, const KernelMemory& mem, u32 num_blocks, u64 offset) 
{
    kernel_1<<< num_blocks, lp.threads_per_block >>> (
        mem.d_result_array, mem.d_result_count, mem.d_shared_mem_contents, mem.shared_mem_contents_length, offset
    );
}} //namespace

int main() {
    LaunchParameters config = {"kernel_1", std::vector<u32>(), 68719476736ULL, 4294967296ULL, 256U, 0U, 0, 16};
    {uint32_t shmem[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,22,22,22,22,22,16,16,16,16,16,24,24,24,24,24,15,15,15,15,15,12,12,12,12,12,14,14,14,14,14,1,11,20,4294967295,25,25,5,5,5,2,2,3,3,3,1,1,5,5,5,5,1,3,3,1,1,5,1,3,3,1,1,5,1,3,3,1,1,5,1,3,3,1,1,4,4,4,4,4,3,3,3,1,1,2,1,4,4,4,4,3,3,1,1,1,4,4,4,4,3,1,3,3,1,1,1,4,4,4,4,3,3,1,1,1}; for (int k = 0; k < 479; k++) config.kernel_shared_memory.push_back(shmem[k]);}
    const KernelMemory mem(config);
    launch_configured_kernel(kernel0::launch, config, mem, true);
    return 0;
}
