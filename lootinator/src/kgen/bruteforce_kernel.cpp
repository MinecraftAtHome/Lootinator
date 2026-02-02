#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"
#include "stdlib.h"


namespace kgen {
    void BruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out) {
        BruteforceKernel bk(root_node);
        out.push_back(bk.generate());
    }

    BruteforceKernel::BruteforceKernel(data::LootTableRoot &root_node) : Kernel(root_node) {}

    // -----------------------------------------------------

    ConfiguredKernel BruteforceKernel::generate() const
    {
        uint32_t idx = (rand() << 15) | rand();

        return ConfiguredKernel{
            "kernel_" + std::to_string(idx),
            to_string(),
            UINT64_C(1) << 48,
            UINT64_C(1) << 32,
            GlobalSettings::THREADS_PER_BLOCK,
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

    void BruteforceKernel::generate_helper_functions(std::ostream& out) const {
        // TODO Create loot function implementations based on shared memory.
        //      Each loot function should have a skip variant and a save variant
        //      to let the generator choose the intended effect of the function.
        // - skip signature: void skip_loot_function_name(u64*)
        // - save signature: SaveType save_loot_function_name(u64*)
    }

    void BruteforceKernel::generate_forward_filter(std::ostream& out) const {
        // TODO Create check_lootseed(u64) -> bool
        //      The function should use available loot pool information combined
        //      with existing implementations of loot functions and form a full,
        //      isolated boolean check of specified constraints
    }
}
