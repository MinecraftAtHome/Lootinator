#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// start of shared definitions block
#define SHARED_DEFINITIONS
//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

__device__ inline void setSeed(u64* rand, u64 value) {
	*rand = (value ^ JRAND_MULTIPLIER) & MASK_48;
}
__device__ inline i32 next(u64* rand, const i32 bits) {
	*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
	return (i32)((i64)*rand >> (48 - bits));
}
__device__ inline i32 nextInt(u64* rand, const i32 n) {
	if ((n - 1 & n) == 0) {
		u64 x = n * (u64)next(rand, 31);
		return (i32)((i64)x >> 31);
	} else {
		return (i32)(next(rand, 31) % n);
	}
}
__device__ inline float nextFloat(u64* rand) {
	return next(rand, 24) / (float)(1 << 24);
};
__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {
	if (min >= max) {
		return min;
	}
	return nextInt(rand, max - min + 1) + min;
}
__device__ inline i32 nextIntNoAdvance(u64* rand, const i32 n) {
	if ((n - 1 & n) == 0) {
		u64 x = n * *rand;
		return (i32)((i64)x >> 31);
	} else {
		return (i32)(*rand % n);
	}
}
__device__ inline void write_result(u64 input_seed, u64* result_array, u32* result_count) {
	result_array[atomicAdd(result_count, 1)] = input_seed;
}

__device__ void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
	const u32 enchantability = array_pointer[0];
	const u32 minLevel = array_pointer[1];
	const u32 maxLevel = array_pointer[2];

	// calculate effective level
	i32 level = minLevel;
	if (minLevel != maxLevel) {
		level += nextInt(rand, maxLevel - minLevel + 1);
	}
	const i32 delta = enchantability / 4 + 1;
	level += 1 + nextInt(rand, delta) + nextInt(rand, delta);
	const float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0F) * 0.15F;
	level = floor(((float)level + (float)level * amplifier) + 0.5F);

	u32 nGroups = array_pointer[3 + level];
	if (nGroups == 0) {
		return;
	}
	*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;

	while (nextInt(rand, 50) <= level) {
		nGroups--;
		if (nGroups == 0) {
			break;
		}
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
		level /= 2;
	}
}

// end of shared definitions block

#define CUDA_CHECK(ans)                                                                            \
	do {                                                                                           \
		gpuAssert((ans), __FILE__, __LINE__);                                                      \
	} while (false)
void gpuAssert(cudaError_t code, const char* file, int line) {
	if (code != cudaSuccess) {
		std::cerr << "CUDA error: " << cudaGetErrorString(code) << " at " << file << ":" << line
				  << std::endl;
		exit(1);
	}
}

struct ConfiguredKernel {
	std::string kernel_name;
	std::vector<u32> shared_memory;
	u64 total_threads;
	u64 threads_per_batch;
	u32 threads_per_block;
	u32 device_id;
	i32 start_batch;
	i32 end_batch;
	u32 max_results;

	// -------------------
	u32* d_shared_mem_contents;
	u32 shared_mem_contents_length;
	u32 shared_mem_bytes;
	u64* d_result_array;
	u32* d_result_count;

	void init_memory() {
		shared_mem_contents_length = shared_memory.size();
		shared_mem_bytes = shared_memory.size() * sizeof(u32);
		cudaMalloc(&d_shared_mem_contents, shared_mem_bytes);
		cudaMalloc(&d_result_array, max_results * sizeof(u64));
		cudaMalloc(&d_result_count, sizeof(u32));
		cudaMemcpy(
			d_shared_mem_contents, shared_memory.data(), shared_mem_bytes, cudaMemcpyHostToDevice);
	}

	~ConfiguredKernel() {
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

typedef std::vector<ConfiguredKernel*> KernelPipeline;
typedef void (*launch_function)(const KernelPipeline&, u32, u64);

void launch_configured_kernel(
	launch_function lf, const KernelPipeline& pipeline, bool print_results) {
	u64* h_result_array = (u64*)malloc(pipeline.back()->max_results * sizeof(u64));
	const u32 num_blocks = pipeline[0]->threads_per_batch / pipeline[0]->threads_per_block;

	for (u32 b = pipeline[0]->start_batch; b < pipeline[0]->end_batch; b++) {
		u32 h_result_count = 0;

		for (const auto& ck : pipeline) {
			CUDA_CHECK(cudaMemcpy(
				ck->d_result_count, &h_result_count, sizeof(u32), cudaMemcpyHostToDevice));
		}
		lf(pipeline, num_blocks, b * pipeline[0]->threads_per_batch);
		CUDA_CHECK(cudaDeviceSynchronize());

		CUDA_CHECK(cudaMemcpy(
			&h_result_count, pipeline.back()->d_result_count, sizeof(u32), cudaMemcpyDeviceToHost));
		CUDA_CHECK(cudaMemcpy(h_result_array,
			pipeline.back()->d_result_array,
			h_result_count * sizeof(u64),
			cudaMemcpyDeviceToHost));

		if (!print_results)
			continue;
		for (u32 i = 0; i < h_result_count; i++) {
			std::cout << h_result_array[i] << '\n';
		}
		std::cout << std::flush;
	}

	delete[] h_result_array;
}

namespace kernel0 {
#ifndef SHARED_DEFINITIONS
	//@SharedDefinitionsStart
	typedef unsigned int u32;
	typedef int i32;
	typedef unsigned long long u64;
	typedef long long i64;

#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

	__device__ inline void setSeed(u64* rand, u64 value) {
		*rand = (value ^ JRAND_MULTIPLIER) & MASK_48;
	}
	__device__ inline i32 next(u64* rand, const i32 bits) {
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
		return (i32)((i64)*rand >> (48 - bits));
	}
	__device__ inline i32 nextInt(u64* rand, const i32 n) {
		if ((n - 1 & n) == 0) {
			u64 x = n * (u64)next(rand, 31);
			return (i32)((i64)x >> 31);
		} else {
			return (i32)(next(rand, 31) % n);
		}
	}
	__device__ inline float nextFloat(u64* rand) {
		return next(rand, 24) / (float)(1 << 24);
	};
	__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {
		if (min >= max) {
			return min;
		}
		return nextInt(rand, max - min + 1) + min;
	}
	__device__ inline i32 nextIntNoAdvance(u64* rand, const i32 n) {
		if ((n - 1 & n) == 0) {
			u64 x = n * *rand;
			return (i32)((i64)x >> 31);
		} else {
			return (i32)(*rand % n);
		}
	}
	__device__ inline void write_result(u64 input_seed, u64* result_array, u32* result_count) {
		result_array[atomicAdd(result_count, 1)] = input_seed;
	}

	__device__ void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
		const u32 enchantability = array_pointer[0];
		const u32 minLevel = array_pointer[1];
		const u32 maxLevel = array_pointer[2];

		// calculate effective level
		i32 level = minLevel;
		if (minLevel != maxLevel) {
			level += nextInt(rand, maxLevel - minLevel + 1);
		}
		const i32 delta = enchantability / 4 + 1;
		level += 1 + nextInt(rand, delta) + nextInt(rand, delta);
		const float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0F) * 0.15F;
		level = floor(((float)level + (float)level * amplifier) + 0.5F);

		u32 nGroups = array_pointer[3 + level];
		if (nGroups == 0) {
			return;
		}
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;

		while (nextInt(rand, 50) <= level) {
			nGroups--;
			if (nGroups == 0) {
				break;
			}
			*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
			level /= 2;
		}
	}

//@SharedDefinitionsEnd
#endif
	__device__ bool forward_filter(u64 loot_seed, u32 data[]) {
		{
			i32 local_constraints[3] = {0};
			i32 rolls = nextIntBounded(&loot_seed, 5, 10);
			for (i32 roll = 0; roll < rolls; roll++) {
				int item = nextInt(&loot_seed, 86);
				u32 entry_data = data[0 + item * 3]; // min_max_count__counter_index__enchantment_count
				u64 enchantment_mask = data[0 + item * 3 + 1];

				u32 item_idx = entry_data >> 24; // [8b][6b][6b][4b][8b]
				u32 min_count = (entry_data >> 18) & 0x3f;
				u32 max_count = (entry_data >> 12) & 0x3f;
				u32 counter_idx = (entry_data >> 8) & 0xf;
				
				if (!(enchantment_mask & 1)) {
					enchantment_mask |= (static_cast<u64>(data[0 + item * 3 + 2]) << 32);
					// enchantment_mask >>= 1;

					u32 enchantment_count = entry_data & 0xff; // [8b]
					i32 enchant_id =
						enchantment_count != 0 ? nextInt(&loot_seed, enchantment_count) : 64;

					bool r = ((enchantment_mask>>1) & (1 << enchant_id));
					u64 m = !r - 1;
					loot_seed = (loot_seed * (1 | (25214903917 & m)) + (11 & m)) & MASK_48;
				}
				i32 item_count = nextIntBounded(&loot_seed, min_count, max_count);
				if (((u64)4) & (((u32)1) << item_idx)) {
					loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
				}

				if (enchantment_mask & 1) { 
					enchant_with_levels_function(&loot_seed, &(data[enchantment_mask >> 1]));
				}
				local_constraints[counter_idx] += item_count;
			}
			if (!(local_constraints[1] >= 7))
				return false;
			if (!(local_constraints[2] == 1))
				return false;
		}
		return true;
	}
	extern "C" __global__ void kernel_1(
		u64* result_array, u32* result_count, u32* shared_mem_contents, u64 offset) {
		__shared__ u32 data[689];
		if (threadIdx.x < 689) {
			for (int i = threadIdx.x; i < 689; i += blockDim.x) {
				data[i] = shared_mem_contents[i];
			}
		}
		__syncthreads();

		u64 input_seed = (u64)blockIdx.x * blockDim.x + threadIdx.x + offset;

		if (forward_filter(input_seed, data)) {
			write_result(input_seed ^ JRAND_MULTIPLIER, result_array, result_count);
		}
	}
#ifndef SHARED_DEFINITIONS
	//@SharedDefinitionsStart
	typedef unsigned int u32;
	typedef int i32;
	typedef unsigned long long u64;
	typedef long long i64;

#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

	__device__ inline void setSeed(u64* rand, u64 value) {
		*rand = (value ^ JRAND_MULTIPLIER) & MASK_48;
	}
	__device__ inline i32 next(u64* rand, const i32 bits) {
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
		return (i32)((i64)*rand >> (48 - bits));
	}
	__device__ inline i32 nextInt(u64* rand, const i32 n) {
		if ((n - 1 & n) == 0) {
			u64 x = n * (u64)next(rand, 31);
			return (i32)((i64)x >> 31);
		} else {
			return (i32)(next(rand, 31) % n);
		}
	}
	__device__ inline float nextFloat(u64* rand) {
		return next(rand, 24) / (float)(1 << 24);
	};
	__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {
		if (min >= max) {
			return min;
		}
		return nextInt(rand, max - min + 1) + min;
	}
	__device__ inline i32 nextIntNoAdvance(u64* rand, const i32 n) {
		if ((n - 1 & n) == 0) {
			u64 x = n * *rand;
			return (i32)((i64)x >> 31);
		} else {
			return (i32)(*rand % n);
		}
	}
	__device__ inline void write_result(u64 input_seed, u64* result_array, u32* result_count) {
		result_array[atomicAdd(result_count, 1)] = input_seed;
	}

	__device__ void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
		const u32 enchantability = array_pointer[0];
		const u32 minLevel = array_pointer[1];
		const u32 maxLevel = array_pointer[2];

		// calculate effective level
		i32 level = minLevel;
		if (minLevel != maxLevel) {
			level += nextInt(rand, maxLevel - minLevel + 1);
		}
		const i32 delta = enchantability / 4 + 1;
		level += 1 + nextInt(rand, delta) + nextInt(rand, delta);
		const float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0F) * 0.15F;
		level = floor(((float)level + (float)level * amplifier) + 0.5F);

		u32 nGroups = array_pointer[3 + level];
		if (nGroups == 0) {
			return;
		}
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;

		while (nextInt(rand, 50) <= level) {
			nGroups--;
			if (nGroups == 0) {
				break;
			}
			*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
			level /= 2;
		}
	}

//@SharedDefinitionsEnd
#endif
	__device__ bool forward_filter_full(u64 loot_seed, u32 data[]) {
		{
			i32 local_constraints[2] = {0};
			i32 rolls = nextIntBounded(&loot_seed, 5, 10);
			for (i32 roll = 0; roll < rolls; roll++) {
				int item = data[0 + nextInt(&loot_seed, 86)];
				switch (item) {
					case 0: { // minecraft:enchanted_golden_apple
						i32 item_count = 1;
						item_count = 1 + nextInt(&loot_seed, 2);
						local_constraints[0] += item_count;
						break;
					}
					case 1: { // minecraft:music_disc_otherside
						break;
					}
					case 2: { // minecraft:compass
						break;
					}
					case 3: { // minecraft:sculk_catalyst
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 4: { // minecraft:name_tag
						break;
					}
					case 5: { // minecraft:diamond_hoe
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						// ENCHANT WITH LEVELS >:)
						enchant_with_levels_function(&loot_seed, &(data[166]));
						break;
					}
					case 6: { // minecraft:lead
						break;
					}
					case 7: { // minecraft:diamond_horse_armor
						break;
					}
					case 8: { // minecraft:leather
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 9: { // minecraft:music_disc_13
						break;
					}
					case 10: { // minecraft:music_disc_cat
						break;
					}
					case 11: { // minecraft:diamond_leggings
						// ENCHANT WITH LEVELS >:)
						enchant_with_levels_function(&loot_seed, &(data[234]));
						break;
					}
					case 12: { // minecraft:book
						u32 eid = nextInt(&loot_seed, 1);
						bool r = (0b1 & (1 << eid));
						uint64_t m = !r - 1;
						loot_seed = (loot_seed * (1 | (25214903917 & m)) + (11 & m)) & MASK_48;
						break;
					}
					case 13: { // minecraft:sculk
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 14: { // minecraft:sculk_sensor
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 15: { // minecraft:candle
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 16: { // minecraft:amethyst_shard
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 17: { // minecraft:experience_bottle
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 18: { // minecraft:glow_berries
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 19: { // minecraft:iron_leggings
						// ENCHANT WITH LEVELS >:)
						enchant_with_levels_function(&loot_seed, &(data[303]));
						break;
					}
					case 20: { // minecraft:echo_shard
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 21: { // minecraft:disc_fragment_5
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 22: { // minecraft:potion
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 23: { // minecraft:book
						i32 item_count = 1;
						i32 enchantment = nextInt(&loot_seed, 40);
						u32 max_level = data[358 + enchantment];
						i32 level = nextIntBounded(&loot_seed, 1, max_level);
						if (enchantment == 14 && level == 3)
							local_constraints[1] += item_count;
						break;
					}
					case 24: { // minecraft:book
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 25: { // minecraft:bone
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 26: { // minecraft:soul_torch
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					case 27: { // minecraft:coal
						loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
						break;
					}
					default: {
						return false;
					}
				}
			}
			if (!(local_constraints[0] >= 7))
				return false;
			if (!(local_constraints[1] == 1))
				return false;
		}
		return true;
	}
	extern "C" __global__ void kernel_2(u64* result_array, u32* result_count,
		u32* shared_mem_contents, u64* kernel_1_out, u32 kernel_1_count) {
		__shared__ u32 data[398];
		if (threadIdx.x < 398) {
			for (int i = threadIdx.x; i < 398; i += blockDim.x) {
				data[i] = shared_mem_contents[i];
			}
		}
		__syncthreads();

		u32 index = blockIdx.x * blockDim.x + threadIdx.x;
		if (index >= kernel_1_count)
			return;
		u64 internal_loot_seed = kernel_1_out[index] ^ JRAND_MULTIPLIER;

		if (forward_filter_full(internal_loot_seed, data)) {
			write_result(internal_loot_seed ^ JRAND_MULTIPLIER, result_array, result_count);
		}
	}

} // namespace kernel0
namespace kernel0 {
	void launch(const KernelPipeline& pipeline, u32 num_blocks, u64 offset) {
		kernel_1<<<num_blocks, pipeline[0]->threads_per_block, pipeline[0]->shared_mem_bytes>>>(
			pipeline[0]->d_result_array,
			pipeline[0]->d_result_count,
			pipeline[0]->d_shared_mem_contents,
			offset);

		CUDA_CHECK(cudaDeviceSynchronize());

		u32 result_count;
		CUDA_CHECK(cudaMemcpy(
			&result_count, pipeline[0]->d_result_count, sizeof(u32), cudaMemcpyDeviceToHost));

		if (result_count != 0) {
			u32 n_blocks_2 = (result_count + pipeline[1]->threads_per_block - 1) /
							 pipeline[1]->threads_per_block;
			kernel_2<<<n_blocks_2, pipeline[1]->threads_per_block, pipeline[1]->shared_mem_bytes>>>(
				pipeline[1]->d_result_array,
				pipeline[1]->d_result_count,
				pipeline[1]->d_shared_mem_contents,
				pipeline[0]->d_result_array,
				result_count);
		}
	}
} // namespace kernel0

int main() {
	constexpr uint32_t d = 0xdeadbeef;
	KernelPipeline pipeline;
	ConfiguredKernel ck0{"kernel_1",
		std::vector<u32>(),
		281474976710656ULL,
		4294967296ULL,
		256U,
		0U,
		0,
		65536,
		524288};
	pipeline.push_back(&ck0);
	{
		uint32_t shmem[] = {285483264,
			0,
			0,
			218370048,
			0,
			0,
			335810560,
			0,
			0,
			335810560,
			0,
			0,
			369369088,
			0,
			0,
			369369088,
			0,
			0,
			302256128,
			0,
			0,
			302256128,
			0,
			0,
			33820672,
			997,
			3735928559,
			33820672,
			997,
			3735928559,
			117706752,
			0,
			0,
			117706752,
			0,
			0,
			151261184,
			0,
			0,
			151261184,
			0,
			0,
			453267456,
			0,
			0,
			453267456,
			0,
			0,
			386142208,
			0,
			0,
			386142208,
			0,
			0,
			168038400,
			0,
			0,
			168038400,
			0,
			0,
			134483968,
			1133,
			3735928559,
			134483968,
			1133,
			3735928559,
			268701697,
			2,
			0,
			268701697,
			2,
			0,
			268701697,
			2,
			0,
			101752832,
			0,
			0,
			101752832,
			0,
			0,
			101752832,
			0,
			0,
			419704832,
			0,
			0,
			419704832,
			0,
			0,
			419704832,
			0,
			0,
			251936768,
			0,
			0,
			251936768,
			0,
			0,
			251936768,
			0,
			0,
			50655232,
			0,
			0,
			50655232,
			0,
			0,
			50655232,
			0,
			0,
			84160512,
			0,
			0,
			84160512,
			0,
			0,
			84160512,
			0,
			0,
			402976768,
			0,
			0,
			402976768,
			0,
			0,
			402976768,
			0,
			0,
			436473856,
			1269,
			3735928559,
			436473856,
			1269,
			3735928559,
			436473856,
			1269,
			3735928559,
			352595968,
			0,
			0,
			352595968,
			0,
			0,
			352595968,
			0,
			0,
			352595968,
			0,
			0,
			67383296,
			0,
			0,
			67383296,
			0,
			0,
			67383296,
			0,
			0,
			67383296,
			0,
			0,
			201601024,
			0,
			0,
			201601024,
			0,
			0,
			201601024,
			0,
			0,
			201601024,
			0,
			0,
			201601024,
			0,
			0,
			268702248,
			1048313726,
			159,
			268702248,
			1048313726,
			159,
			268702248,
			1048313726,
			159,
			268702248,
			1048313726,
			159,
			268702248,
			1048313726,
			159,
			269262848,
			0,
			0,
			269262848,
			0,
			0,
			269262848,
			0,
			0,
			269262848,
			0,
			0,
			269262848,
			0,
			0,
			323584,
			0,
			0,
			323584,
			0,
			0,
			323584,
			0,
			0,
			323584,
			0,
			0,
			323584,
			0,
			0,
			17100800,
			0,
			0,
			17100800,
			0,
			0,
			17100800,
			0,
			0,
			17100800,
			0,
			0,
			17100800,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			186183680,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			4278456320,
			0,
			0,
			235147264,
			0,
			0,
			235147264,
			0,
			0,
			235147264,
			0,
			0,
			235147264,
			0,
			0,
			319033344,
			0,
			0,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			9,
			20,
			39,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2};
		for (int k = 0; k < 689; k++)
			pipeline[0]->shared_memory.push_back(shmem[k]);
	}

	pipeline[0]->init_memory();
	ConfiguredKernel ck1{"kernel_2",
		std::vector<u32>(),
		281474976710656ULL,
		4294967296ULL,
		256U,
		0U,
		0,
		65536,
		262144};
	pipeline.push_back(&ck1);
	{
		uint32_t shmem[] = {0,
			1,
			2,
			2,
			3,
			3,
			4,
			4,
			5,
			5,
			6,
			6,
			7,
			7,
			8,
			8,
			9,
			9,
			10,
			10,
			11,
			11,
			12,
			12,
			12,
			13,
			13,
			13,
			14,
			14,
			14,
			15,
			15,
			15,
			16,
			16,
			16,
			17,
			17,
			17,
			18,
			18,
			18,
			19,
			19,
			19,
			20,
			20,
			20,
			20,
			21,
			21,
			21,
			21,
			22,
			22,
			22,
			22,
			22,
			23,
			23,
			23,
			23,
			23,
			24,
			24,
			24,
			24,
			24,
			25,
			25,
			25,
			25,
			25,
			26,
			26,
			26,
			26,
			26,
			27,
			27,
			27,
			27,
			27,
			27,
			27,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			1,
			1,
			2,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			3,
			9,
			20,
			39,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2,
			4,
			4,
			4,
			4,
			4,
			3,
			1,
			3,
			3,
			5,
			5,
			5,
			2,
			2,
			3,
			3,
			5,
			1,
			3,
			3,
			5,
			2,
			1,
			1,
			3,
			3,
			3,
			5,
			3,
			1,
			1,
			3,
			4,
			5,
			4,
			3,
			1,
			1,
			2,
			1};
		for (int k = 0; k < 398; k++)
			pipeline[1]->shared_memory.push_back(shmem[k]);
	}

	pipeline[1]->init_memory();

	launch_configured_kernel(kernel0::launch, pipeline, true);
	return 0;
}