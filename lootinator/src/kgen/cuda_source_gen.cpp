#include <fstream>
#include <iostream>
#include <sstream>

#include "lootinator/kgen/cuda_source_gen.hpp"

namespace kgen {
	typedef uint32_t u32;

	constexpr u32 RESULT_BUFFER_SIZE = 16u * 1024u; // max results per kernel launch

	void print_preamble(std::ostream& out, const kgen::ConfiguredKernel& reference_kernel) {
		std::stringstream sout;
		sout <<
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
//@InjectSharedDefinitions
// end of shared definitions block

#define CUDA_CHECK(ans) do { gpuAssert((ans), __FILE__, __LINE__); } while(false)
void gpuAssert(cudaError_t code, const char *file, int line) {
    if (code != cudaSuccess) {
        std::cerr << "CUDA error: " << cudaGetErrorString(code) << " at " << file << ":" << line << std::endl;
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
        cudaMemcpy(d_shared_mem_contents, shared_memory.data(), shared_mem_bytes, cudaMemcpyHostToDevice);
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

typedef std::vector<ConfiguredKernel> KernelPipeline;
typedef void (*launch_function)(const KernelPipeline&, u32, u64);

void launch_configured_kernel(launch_function lf, const KernelPipeline& pipeline, bool print_results) {
    u64* h_result_array = (u64*)malloc(pipeline.back().max_results * sizeof(u64));
    const u32 num_blocks = pipeline[0].threads_per_batch / pipeline[0].threads_per_block;
    
    for (u32 b = pipeline[0].start_batch; b < pipeline[0].end_batch; b++) {
        u32 h_result_count = 0;
        for (const auto& ck : pipeline) {
            CUDA_CHECK(cudaMemcpy(ck.d_result_count, &h_result_count, sizeof(u32), cudaMemcpyHostToDevice));
        }
        lf(pipeline, num_blocks, b*pipeline[0].threads_per_batch);
        CUDA_CHECK(cudaDeviceSynchronize());

        CUDA_CHECK(cudaMemcpy(&h_result_count, pipeline.back().d_result_count, sizeof(u32), cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h_result_array, pipeline.back().d_result_array, h_result_count * sizeof(u64), cudaMemcpyDeviceToHost));
        
        if (!print_results) continue;
        for (u32 i = 0; i < h_result_count; i++) {
            std::cout << h_result_array[i] << '\n';
        }
        std::cout << std::flush;
    }

    delete[] h_result_array;
}
)"
			 << "\n\n";

		std::string preamble = sout.str();
		size_t inject_start = preamble.find("//@InjectSharedDefinitions");
		size_t inject_size = std::string("//@InjectSharedDefinitions").length();
		size_t definitions_start = reference_kernel.code.find("//@SharedDefinitionsStart");
		size_t definitions_end = reference_kernel.code.find("//@SharedDefinitionsEnd");
		size_t definitions_size = definitions_end - definitions_start;

		if (inject_start == std::string::npos || definitions_start == std::string::npos ||
			definitions_end == std::string::npos) {
			std::cerr << "WARN: Missing injection annotations! Inject step skipped.\n";
			out << preamble;
			return;
		}

		std::string definitions = reference_kernel.code.substr(definitions_start, definitions_size);
		preamble.replace(inject_start, inject_size, definitions);
		out << preamble;
	}

	void print_shared_mem(std::ostream& out, const kgen::ConfiguredKernel& kernel_config) {
		out << "{";
		const std::vector<u32>& shmem = kernel_config.shared_mem;
		for (size_t i = 0; i < shmem.size(); i++) {
			out << kernel_config.shared_mem[i] << (i == shmem.size() - 1 ? "};" : ",");
		}
	}

	void print_kernels(
        std::ostream& out, const std::vector<KernelPipeline>& kernel_pipelines) {
		for (int k = 0; k < (int)kernel_pipelines.size(); k++) {
			// each kernel gets its own namespace to avoid device helper conflicts
			out << "namespace kernel" << k << " {\n";
            for (const auto& configured_kernel : kernel_pipelines[k]) {
                out << configured_kernel.code;
            }
			out << "\n} //namespace\n";
		}
	}

	void print_kernel_launchers(
		std::ostream& out, const std::vector<KernelPipeline>& kernel_pipelines) {
		for (int k = 0; k < (int)kernel_pipelines.size(); k++) {
			const auto& pipeline = kernel_pipelines[k];

			out << "namespace kernel" << k << " {";
			out << R"(
void launch(const KernelPipeline& pipeline, u32 num_blocks, u64 offset) 
{
    )"; 
            out << pipeline[0].kernel_name << R"(<<< num_blocks, pipeline[0].threads_per_block, pipeline[0].shared_mem_bytes >>> (
        pipeline[0].d_result_array, pipeline[0].d_result_count, pipeline[0].d_shared_mem_contents, offset
    );
    )";
            if (pipeline.size() == 2) {
                out << R"(
    CUDA_CHECK(cudaDeviceSynchronize());

    u32 result_count;
    CUDA_CHECK(cudaMemcpy(&result_count, mem.d_result_count, sizeof(u32), cudaMemcpyDeviceToHost));

    if (result_count != 0) {
        u32 n_blocks_2 = (result_count + pipeline[1].threads_per_block - 1) / pipeline[1].threads_per_block;
        secondary<<< n_blocks_2, pipeline[1].threads_per_block, pipeline[1].shared_mem_bytes >>> ();
    })";
            }

		out << R"(	
}} //namespace
)";
		}
	}

    // TODO revive
    /*
	void print_benchmarker(
		std::ostream& out, const std::vector<KernelPipeline>& kernel_configs) {
		out <<
			R"(
int main() {
    constexpr int num_kernels = )"
			<< kernel_configs.size() << R"(;
    std::vector<launch_function> launchers;
    std::vector<ConfiguredKernel> configs;
)";

		for (int i = 0; i < (int)kernel_configs.size(); i++) {
			const ConfiguredKernel& lp = kernel_configs[i];
			out << "    launchers.push_back(kernel" << i << "::launch);\n";
			out << "    configs.push_back({\"" << lp.kernel_name << "\", ";
			out << "std::vector<u32>(), " << lp.total_threads << "ULL, " << lp.threads_per_batch
				<< "ULL, " << lp.threads_per_block << "U, ";
			out << lp.device_id << "U, " << lp.start_batch << ", " << lp.end_batch;
			out << "});\n";
		}
		for (int i = 0; i < (int)kernel_configs.size(); i++) {
			out << "    {uint32_t shmem[] = ";
			print_shared_mem(out, kernel_configs.at(i));
			out << " for (int k = 0; k < " << kernel_configs.at(i).shared_mem.size()
				<< "; k++) configs[" << i << "].shared_memory.push_back(shmem[k]);}\n";
		}
		out << "\n";
		out << "    BenchmarkResults result_array[num_kernels];\n";
		out << "    for (int k = 0; k < num_kernels; k++) {\n";
		out <<
			R"(        const ConfiguredKernel& config = configs[k];
        ConfiguredKernel work_config = config;
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
        const ConfiguredKernel& config = configs[best_kernel];
        KernelMemory kernel_memory(config);
        launch_configured_kernel(launchers[best_kernel], config, kernel_memory, true);
        std::cerr << "Finished.\n";
    }
    return 0;
}
)";
	}*/

	void generate_runner_source(KernelPipeline& kp, std::ostream& fout) {
		std::vector<KernelPipeline> single_kernel;
		single_kernel.push_back(kp);

		print_preamble(fout, kp[0]);
		print_kernels(fout, single_kernel);
		print_kernel_launchers(fout, single_kernel);

		fout <<
			R"(
int main() {
    KernelPipeline pipeline;
    )";
        for (int i = 0; i < 2; i++) {
            fout << R"(pipeline.push_back(ConfiguredKernel{")"
                << kp[i].kernel_name << "\", "
                << "std::vector<u32>(), " << kp[i].total_threads << "ULL, " << kp[i].threads_per_batch
                << "ULL, " << kp[i].threads_per_block << "U, " << kp[i].device_id << "U, " << kp[i].start_batch
                << ", " << kp[i].end_batch << "});\n";

            fout << "    {uint32_t shmem[] = ";
            print_shared_mem(fout, kp[i]);
            fout << " for (int k = 0; k < " << kp[i].shared_mem.size()
                << "; k++) pipeline[" << i << "].shared_memory.push_back(shmem[k]);}\n}"
                << "\npipeline[" << i << "].init_memory();\n";
        }

		fout << R"(
    launch_configured_kernel(kernel0::launch, pipeline, true);
    return 0;
}
)";
	}

    // TODO revive
    /*
	int generate_benchmarker_source(
		std::vector<KernelPipeline> kernel_configs, std::ostream& fout) {
		if (kernel_configs.size() == 0) {
			std::cerr << "ERROR: No kernels provided.\n";
			return 1;
		}

		print_preamble(fout, kernel_configs[0]);
		print_kernels(fout, kernel_configs);
		print_kernel_launchers(fout, kernel_configs);

		print_benchmarker(fout, kernel_configs);
		return 0;
	}
    */
} // namespace kgen