#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cinttypes>
#include <iostream>

typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = ((1ULL << 48) - 1);

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

__device__ inline void setSeed(u64* rand, u64 value){ *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline int next(u64* rand, const int bits){ *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48; return (int)((i64)*rand >> (48 - bits)); }
__device__ inline int nextInt(u64* rand, const int n){ if ((n-1 & n) == 0) {u64 x = n * (u64)next(rand, 31); return (int)((i64)x >> 31);} else {return (int)(next(rand, 31) % n);} }
__device__ inline float nextFloat(u64* rand){ return next(rand, 24) / (float)(1 << 24); }


namespace kernel0 {
extern "C" {
    __global__ void state_prediction_rolls(
        u64* result_array, u32* result_count, 
        u32* shared_mem_contents, u32 shared_mem_contents_length, 
        u64 offset)
    {
        extern __shared__ u32 data[];
        if (threadIdx.x < shared_mem_contents_length) {
            for (int i = threadIdx.x; i < shared_mem_contents_length; i += blockDim.x) {
                data[i] = shared_mem_contents[i];
            }
        }
        __syncthreads();
        
        const u64 tid = blockIdx.x * blockDim.x + threadIdx.x + offset;
        u64 base_state = tid * (5U << 17);

        for (u32 rem = 2U; rem < 5U; rem++) {
            u64 state = base_state + rem<<17;
            u64* rand = &state;

            int counter = 0;
            for (u32 r = 0; r < rem+4; r++) {
                int item = data[nextInt(rand, 28)];
                if (item == 3)
                    counter += nextInt(rand, 2) + 1;
                else if (item < 2) {
                    state = (state * JRAND_MULTIPLIER + 11) & MASK_48;
                }
            }

            if (counter >= 11) {
                u32 ix = atomicAdd(result_count, 1);
                result_array[ix] = tid ^ JRAND_MULTIPLIER;
            }
        }
    }
}
} //namespace
namespace kernel0 {
void launch(const LaunchParameters& lp, const KernelMemory& mem, u32 num_blocks, u64 offset) 
{
    state_prediction_rolls<<< num_blocks, lp.threads_per_block, mem.shared_mem_bytes >>> (
        mem.d_result_array, mem.d_result_count, mem.d_shared_mem_contents, mem.shared_mem_contents_length, offset
    );
}} //namespace

int main() {
    LaunchParameters config = {"state_prediction_rolls", std::vector<u32>(), 56294995342132ULL, 4294967296ULL, 256U, 0U, 0, 13108};
    {uint32_t shmem[] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,4,5}; for (int k = 0; k < 28; k++) config.kernel_shared_memory.push_back(shmem[k]);}
    const KernelMemory mem(config);
    launch_configured_kernel(kernel0::launch, config, mem, true);
    return 0;
}
