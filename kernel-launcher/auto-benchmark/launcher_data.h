#ifndef LAUNCHER_DATA
#define LAUNCHER_DATA

#include <cstdint>
#include <string>
#include <vector>

namespace launcher {
	// The actual kernels will use different definitions provided by the
	// Settings class. Our assumption here is that the Settings
	// class will be configured correctly for the target environment.
	typedef uint64_t u64;
	typedef uint32_t u32;
	typedef int32_t i32;

	// these can be command line args if necessary
	constexpr const char* BENCHMARK_RESULTS_FILE = "benchmark_results.txt";
	constexpr const char* SOURCE_CODE_OUTPUT_FILE = "source.cu";

	constexpr i32 UNSPECIFIED = -1;
	constexpr u32 RESULT_BUFFER_SIZE = 16u * 1024u; // max results per kernel launch

	enum AppMode { NONE, BENCHMARK, RUN_SINGLE };

	struct AppParameters {
		AppMode mode;
		bool debug_info;
	};

	struct LaunchParameters {
		// internal
		std::string kernel_name;
		std::string kernel_source_file;
		std::string
			kernel_code; // can be either PTX for NVRTC-based runs or CUDA for source code gen mode
		std::vector<u32> kernel_shared_memory;
		u64 threads_total;
		u64 threads_per_batch;

		// user-provided
		u32 threads_per_block;
		u32 device_id;

		// non-kernel
		i32 start_batch;
		i32 end_batch;
	};
} // namespace launcher

#endif