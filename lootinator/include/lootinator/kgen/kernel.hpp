#ifndef LOOTINATOR_KERNGEN_KERNEL_H
#define LOOTINATOR_KERNGEN_KERNEL_H

#include "lootinator/data/loot_data.hpp"
#include <ostream>

namespace kgen {
    struct ConfiguredKernel {
        std::string kernel_name;
        std::string code;

        uint64_t total_threads;
        uint64_t threads_per_batch;
        uint32_t threads_per_block;

        std::vector<uint32_t> shared_mem;
    };

    class Kernel {
    public:
        std::vector<uint32_t> pool_memory_offsets;
        std::vector<uint32_t> function_memory_offsets;
        std::vector<uint32_t> combined_shared_memory; // final final

        static std::string generate_skip(std::string var, int amount);

        static int kernel_index = 0;
        std::string name;

        Kernel(data::LootTableRoot &root_node);

    protected:
        static void write_shared_definitions(std::ostream& out);
        void fill_function_shared_mem(data::LootTreeNode *current);
        virtual ConfiguredKernel generate() const = 0;
    };
}

#endif