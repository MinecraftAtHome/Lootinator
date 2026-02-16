#ifndef LOOTINATOR_KERNGEN_KERNEL_H
#define LOOTINATOR_KERNGEN_KERNEL_H

#include "lootinator/data/loot_data.hpp"
#include <ostream>

namespace kgen {
    struct KernelGenConfig {
        bool seedcracking;
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
        int flags;

        Kernel(data::LootTableRoot &root_node, kgen::KernelGenConfig kgen_config);

    protected:
        static void write_shared_definitions(std::ostream& out);
        virtual void setup_shared_memory();
        virtual void fill_function_shared_mem(data::LootTreeNode *current);
        virtual ConfiguredKernel generate() = 0;
    };
}

#endif