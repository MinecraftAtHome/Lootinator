#ifndef LOOTINATOR_KERNGEN_KERNEL_H
#define LOOTINATOR_KERNGEN_KERNEL_H

#include "lootinator/data/loot_data.hpp"
#include "lootinator/mc/minecraft.hpp"
#include <ostream>

namespace kgen {
	struct KernelGenConfig {
		bool seedcracking;
		// std::string constraint_path;
		// std::string item_map_path;
		// std::string loot_table_path;
		mc::VersionRange version;

		nlohmann::json loot_table_json;
		std::unordered_map<std::string, int> item_map;
		std::vector<loot::Constraint> constraints;

		// derived tree config
		int bytes_per_entry;
		std::vector<data::LootFunctionType> function_order;
		bool no_fast_filter;

		KernelGenConfig(mc::VersionRange version, std::string loot_table_path,
			std::string constraint_path, std::string item_map_path, bool seedcracking);

		void traverse_and_derive(data::LootTreeNode* root, bool** edges);
		void derive_mode_from_tree();
	};

	struct ConfiguredKernel {
		std::string kernel_name;
		std::string code;

		uint64_t total_threads;
		uint64_t threads_per_batch;
		uint32_t threads_per_block;

		std::vector<uint32_t> shared_mem;

		uint32_t device_id;
		uint32_t start_batch;
		uint32_t end_batch;

		uint32_t max_results;
	};

	class Kernel {
	  public:
		data::LootTableRoot& root_node;
		kgen::KernelGenConfig kgen_config;

		std::vector<uint32_t> pool_memory_offsets;
		std::vector<uint32_t> function_memory_offsets;
		std::vector<uint32_t> combined_shared_memory; // final final

		static std::string generate_skip(std::string var, int amount);

		std::string name;

		Kernel(data::LootTableRoot& root_node, kgen::KernelGenConfig kgen_config);

	  protected:
		static void write_shared_definitions(std::ostream& out);
		virtual void setup_shared_memory();
		virtual void fill_function_shared_mem(data::LootTreeNode* current);
		virtual ConfiguredKernel generate() = 0;
	};
} // namespace kgen

#endif