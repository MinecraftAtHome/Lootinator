#include <iostream>
#include <sstream>
#include <fstream>

#include "launcher_data.h"

namespace launcher {
    int generate_benchmarker_source(std::vector<launcher::LaunchParameters> kernel_configs);
    int generate_runner_source(launcher::LaunchParameters& kernel_config);

    void print_preamble(std::ostream& out) {
        out << 
R"(#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cinttypes>
#include <iostream>

// start of shared definitions block
#define SHARED_DEFINITIONS
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = ((1ULL << 48) - 1);

__device__ inline void setSeed(uint64_t* rand, uint64_t value){ *rand = (value ^ 0x5deece66d) & ((1ULL << 48) - 1); }
__device__ inline int next(uint64_t* rand, const int bits){ *rand = (*rand * 0x5deece66d + 0xb) & ((1ULL << 48) - 1); return (int)((int64_t)*rand >> (48 - bits)); }
__device__ inline int nextInt(uint64_t* rand, const int n){ if ((n-1 & n) == 0) {uint64_t x = n * (uint64_t)next(rand, 31); return (int)((int64_t)x >> 31);} else {return (int)(next(rand, 31) % n);} }
__device__ inline float nextFloat(uint64_t* rand){ return next(rand, 24) / (float)(1 << 24) }; 
__device__ inline int nextIntBounded(uint64_t* rand, const int min, const int max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline int nextIntNoAdvance(uint64_t *rand, const int n) {if ((n-1 & n) == 0) {uint64_t x = n * *rand; return (int)((int64_t)x >> 31);} else {return (int)(*rand % n);}}
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
        cudaMalloc(&d_result_array, )" << launcher::RESULT_BUFFER_SIZE << R"( * sizeof(u64));
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
    u64 h_result_array[)" << launcher::RESULT_BUFFER_SIZE << R"(];
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
)" << "\n\n";
    }

    void print_shared_mem(std::ostream& out, const launcher::LaunchParameters& kernel_config) {
        out << "{";
        const std::vector<u32>& shmem = kernel_config.kernel_shared_memory;
        for (size_t i = 0; i < shmem.size(); i++) {
            out << kernel_config.kernel_shared_memory[i] << (i == shmem.size()-1 ? "};" : ",");
        }
    }

    void print_kernels(std::ostream& out, const std::vector<launcher::LaunchParameters>& kernel_configs) {
        for (int k = 0; k < kernel_configs.size(); k++) {
            // each kernel gets its own namespace to avoid device helper conflicts
            out << "namespace kernel" << k << " {\n";
            // shared header is stripped, for standard CUDA another header will be used
            // and we don't want to declare it for each kernel individually 
            size_t header_end = kernel_configs[k].kernel_code.find(HEADER_END_INDICATOR) + HEADER_END_INDICATOR_LENGTH;
            out << kernel_configs[k].kernel_code.substr(header_end + 1);
            out << "\n} //namespace\n";
        }
    }

    void print_kernel_launchers(std::ostream& out, const std::vector<launcher::LaunchParameters>& kernel_configs) {
        for (int k = 0; k < kernel_configs.size(); k++) {
            const auto& conf = kernel_configs.at(k);
            out << "namespace kernel" << k << " {";
            out << 
R"(
void launch(const LaunchParameters& lp, const KernelMemory& mem, u32 num_blocks, u64 offset) 
{
    )" << conf.kernel_name << R"(<<< num_blocks, lp.threads_per_block, mem.shared_mem_bytes >>> (
        mem.d_result_array, mem.d_result_count, mem.d_shared_mem_contents, mem.shared_mem_contents_length, offset
    );
}} //namespace
)";
        }
    }

    void print_benchmarker(std::ostream& out, const std::vector<launcher::LaunchParameters>& kernel_configs) {
        out << 
R"(
int main() {
    constexpr int num_kernels = )" << kernel_configs.size() << R"(;
    std::vector<launch_function> launchers;
    std::vector<LaunchParameters> configs;
)";

        for (int i = 0; i < kernel_configs.size(); i++) {
            const LaunchParameters& lp = kernel_configs[i];
            out << "    launchers.push_back(kernel" << i << "::launch);\n";
            out << "    configs.push_back({\"" << lp.kernel_name << "\", ";
            out << "std::vector<u32>(), " << lp.threads_total << "ULL, " << lp.threads_per_batch << "ULL, " << lp.threads_per_block << "U, ";
            out << lp.device_id << "U, " << lp.start_batch << ", " << lp.end_batch;
            out << "});\n"; 
        }
        for (int i = 0; i < kernel_configs.size(); i++) {
            out << "    {uint32_t shmem[] = ";
            print_shared_mem(out, kernel_configs.at(i));
            out << " for (int k = 0; k < " << kernel_configs.at(i).kernel_shared_memory.size() 
                << "; k++) configs[" << i << "].kernel_shared_memory.push_back(shmem[k]);}\n";
        }
        out << "\n";
        out << "    BenchmarkResults result_array[num_kernels];\n";
        out << "    for (int k = 0; k < num_kernels; k++) {\n";
        out <<
R"(        const LaunchParameters& config = configs[k];
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
)";
    }

    int generate_runner_source(launcher::LaunchParameters& lp) {
        std::vector<launcher::LaunchParameters> single_kernel;
        single_kernel.push_back(lp);

        std::ofstream fout(launcher::SOURCE_CODE_OUTPUT_FILE);
        print_preamble(fout);
        print_kernels(fout, single_kernel);
        print_kernel_launchers(fout, single_kernel);

        fout << 
R"(
int main() {
    LaunchParameters config = {")" << lp.kernel_name << "\", "
        << "std::vector<u32>(), " << lp.threads_total << "ULL, " 
        << lp.threads_per_batch << "ULL, " << lp.threads_per_block << "U, "
        << lp.device_id << "U, " << lp.start_batch << ", " << lp.end_batch
        << "};\n"; 

        fout << "    {uint32_t shmem[] = ";
        print_shared_mem(fout, lp);
        fout << " for (int k = 0; k < " << lp.kernel_shared_memory.size() << "; k++) config.kernel_shared_memory.push_back(shmem[k]);}\n";

        fout << 
R"(    const KernelMemory mem(config);
    launch_configured_kernel(kernel0::launch, config, mem, true);
    return 0;
}
)";
        return 0;
    }

    int generate_benchmarker_source(std::vector<launcher::LaunchParameters> kernel_configs) {
        std::ofstream fout(launcher::SOURCE_CODE_OUTPUT_FILE);
        print_preamble(fout);
        print_kernels(fout, kernel_configs);
        print_kernel_launchers(fout, kernel_configs);

        print_benchmarker(fout, kernel_configs);
        return 0;
    }
}