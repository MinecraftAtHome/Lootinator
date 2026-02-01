#ifndef LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H
#define LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
    class BruteforceKernel : public Kernel {
    public:
        static std::vector<ConfiguredKernel> gen_kernels(data::LootTableRoot* root_node);

    protected:
        virtual ConfiguredKernel generate() const override;
        std::string generate_forward_filter() const;
    };
}

#endif