#include "lootinator/kgen/bruteforce_kernel.hpp"

namespace kgen {
    void kgen::BruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out)
    {
        BruteforceKernel bk;
        out.push_back(bk.generate());
    }

    ConfiguredKernel kgen::BruteforceKernel::generate() const
    {
        return ConfiguredKernel(
            //name
            //code

            //threadstotal
            //
        );
    }

    std::string kgen::BruteforceKernel::generate_forward_filter() const
    {
        return std::string();
    }
}
