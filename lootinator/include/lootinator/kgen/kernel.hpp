#ifndef LOOTINATOR_KERNGEN_KERNEL_H
#define LOOTINATOR_KERNGEN_KERNEL_H

#include "lootinator/data/loot_data.hpp"

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
    protected:
        virtual ConfiguredKernel generate() const = 0;
    };
}

#endif