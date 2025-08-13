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
namespace kernel1 {
extern "C" {
    __global__ void naive_bruteforce(
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

        u64 state = tid;
        u64* rand = &state;

        int rolls = 4 + nextInt(rand, 5);
        int counter = 0;
        for (int r = 0; r < rolls; r++) {
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
} //namespace
namespace kernel2 {
extern "C" {
    __global__ void state_prediction_item(
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
        u64 state = tid * (28U << 17) + (25U << 17);
        u64* rand = &state;
        int counter = 1 + nextInt(rand, 2);

        for (int r = 0; r < 7; r++) {
            int item = data[nextInt(rand, 28)];
            if (item == 3)
                counter += nextInt(rand, 2) + 1;
            else if (item < 2) {
                state = (state * JRAND_MULTIPLIER + 11) & MASK_48;
            }
        }
        if (counter < 11)
            return;

        state = tid * (28U << 17) + (25U << 17);
        for (int back = 0; back < 10; back++) {
            state = (state * (-35320271006875LL) - 174426972345687LL) & MASK_48;
            u64 state2 = state;

            int rolls = nextInt(&state2, 5) + 4;
            int counter2 = 0;

            for (int r = 0; r < rolls; r++) {
                int item = data[nextInt(&state2, 28)];
                if (item == 3)
                    counter += nextInt(&state2, 2) + 1;
                else if (item < 2) {
                    state2 = (state2 * JRAND_MULTIPLIER + 11) & MASK_48;
                }
            }
            if (counter2 >= 11) {
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
namespace kernel1 {
void launch(const LaunchParameters& lp, const KernelMemory& mem, u32 num_blocks, u64 offset) 
{
    naive_bruteforce<<< num_blocks, lp.threads_per_block, mem.shared_mem_bytes >>> (
        mem.d_result_array, mem.d_result_count, mem.d_shared_mem_contents, mem.shared_mem_contents_length, offset
    );
}} //namespace
namespace kernel2 {
void launch(const LaunchParameters& lp, const KernelMemory& mem, u32 num_blocks, u64 offset) 
{
    state_prediction_item<<< num_blocks, lp.threads_per_block, mem.shared_mem_bytes >>> (
        mem.d_result_array, mem.d_result_count, mem.d_shared_mem_contents, mem.shared_mem_contents_length, offset
    );
}} //namespace

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

int main() {
    constexpr int num_kernels = 3;
    std::vector<launch_function> launchers;
    std::vector<LaunchParameters> configs;
    launchers.push_back(kernel0::launch);
    configs.push_back({"state_prediction_rolls", std::vector<u32>(), 56294995342132ULL, 4294967296ULL, 256U, 0U, 0, 13108});
    launchers.push_back(kernel1::launch);
    configs.push_back({"naive_bruteforce", std::vector<u32>(), 281474976710656ULL, 4294967296ULL, 256U, 0U, 0, 65536});
    launchers.push_back(kernel2::launch);
    configs.push_back({"state_prediction_item", std::vector<u32>(), 10052677739667ULL, 4294967296ULL, 256U, 0U, 0, 2341});
    {uint32_t shmem[] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,4,5}; for (int k = 0; k < 28; k++) configs[0].kernel_shared_memory.push_back(shmem[k]);}
    {uint32_t shmem[] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,4,5}; for (int k = 0; k < 28; k++) configs[1].kernel_shared_memory.push_back(shmem[k]);}
    {uint32_t shmem[] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,3,4,5}; for (int k = 0; k < 28; k++) configs[2].kernel_shared_memory.push_back(shmem[k]);}

    BenchmarkResults result_array[num_kernels];
    for (int k = 0; k < num_kernels; k++) {
        const LaunchParameters& config = configs[k];
        LaunchParameters work_config = config;
        KernelMemory kernel_memory(config);
        BenchmarkResults results{config.kernel_name, false, 0.0f, 0.0f};

        const i32 middle_batch = (config.start_batch + config.end_batch) / 2;
        work_config.threads_per_batch /= 4;

        // warmup (to get more accurate measurements)
        work_config.start_batch = middle_batch;
        work_config.end_batch = middle_batch + 1;

        CUDA_CHECK(cudaDeviceSynchronize());
        for (u32 i = 0; i < 3; i++) {
            launch_configured_kernel(launchers[k], work_config, kernel_memory, false);
        }

        // benchmarking with auto-tuning
        i32 batches = 1;
        float elapsed_ms = 0.0f;
        while (elapsed_ms < 100.0f) {
            work_config.end_batch = work_config.start_batch + batches;

            auto t0 = std::chrono::high_resolution_clock::now();
            launch_configured_kernel(launchers[k], work_config, kernel_memory, false);
            auto t1 = std::chrono::high_resolution_clock::now();
            elapsed_ms = (t1-t0).count() * 1e-6f;
            if (elapsed_ms < 100.0f)
                batches *= 2;
        }

        // results
        results.success = true;
        results.ms_per_batch = elapsed_ms * 4 / batches;
        results.ms_total_estimate = results.ms_per_batch * (config.end_batch - config.start_batch);
        result_array[k] = results;
    }
    
    int best_kernel = -1;
    double best_perf = result_array[0].ms_total_estimate;
    for (int k = 0; k < num_kernels; k++) {
        if (result_array[k].success && result_array[k].ms_total_estimate < best_perf) {
            best_perf = result_array[k].ms_total_estimate;
            best_kernel = k;
        }
    }
    if (best_kernel == -1) {
        std::cerr << "All kernels failed. Aborting...\n";
        return 1;
    }
    for (int k = 0; k < num_kernels; k++) {
        std::cerr << (k == best_kernel ? "BEST> " : "      ");
        if (result_array[k].success)
            std::cerr << configs[k].kernel_name << ", ETA = " << result_array[k].ms_total_estimate << " ms.\n";
        else
            std::cerr << configs[k].kernel_name << " (failed!)\n";
    }
    
    std::cerr << "Running kernel " << configs[best_kernel].kernel_name << "...\n";
    {
        const LaunchParameters& config = configs[best_kernel];
        KernelMemory kernel_memory(config);
        launch_configured_kernel(launchers[best_kernel], config, kernel_memory, true);
        std::cerr << "Finished.\n";
    }
    return 0;
}
