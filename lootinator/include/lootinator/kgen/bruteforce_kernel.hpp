#ifndef LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H
#define LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
    class BruteforceKernel : public Kernel {
    public:
        static void gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out);
        BruteforceKernel(data::LootTableRoot &root_node);

    protected:
        virtual ConfiguredKernel generate() override;
     
        std::unordered_map<std::string, std::pair<std::string, int>> var_name_map;

        std::string to_string();
        void generate_helper_functions(std::ostream& out) const;
        void emit_cuda_for_pool(std::ostream &out, data::LootPool *pool, int pool_idx);
        void generate_forward_filter(std::ostream &out);

        void materialize_level(data::LootTreeNode *node);
    };
}

#endif