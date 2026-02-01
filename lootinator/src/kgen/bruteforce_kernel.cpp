#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "stdlib.h"


namespace kgen {
    void BruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out) {
        BruteforceKernel bk(root_node);
        out.push_back(bk.generate());
    }

    BruteforceKernel::BruteforceKernel(data::LootTableRoot &root_node) : Kernel(root_node) {}

    ConfiguredKernel BruteforceKernel::generate() const
    {
        uint32_t idx = (rand() << 15) | rand();

        return ConfiguredKernel{
            "kernel_" + std::to_string(idx),
            to_string(),
            UINT64_C(1) << 48,
            UINT64_C(1) << 32,
            256,
            combined_shared_memory
        };
    }

    std::string BruteforceKernel::to_string() const {
        return 
R"(
This is where the next session should start!
Put the source code for a bruteforce kernel here, make use of generate_forward_filter()
)";
    }

    std::string BruteforceKernel::generate_forward_filter() const {
        return std::string(); // TODO
    }
}
