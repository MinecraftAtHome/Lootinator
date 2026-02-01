#include "lootinator/kgen/bruteforce_kernel.hpp"


namespace kgen {
    void Kernel::fill_function_shared_mem(data::LootTreeNode* current) {
        data::LootFunctionData* func = dynamic_cast<data::LootFunctionData*>(current);
        if (func == nullptr) {
            for (auto child : current->children) {
                fill_function_shared_mem(child);
            }
        }
        else {
            uint32_t size = func->shared_mem.size();
            uint32_t last = function_memory_offsets.back();
            function_memory_offsets.push_back(last + size);

            for (auto i : func->shared_mem) {
                combined_shared_memory.push_back(i);
            }
        }
    }

    Kernel::Kernel(data::LootTableRoot &root_node)
    {
        function_memory_offsets.push_back(0);
        pool_memory_offsets.push_back(0);

        // parse shared mem for pools
        for (auto pool : root_node.children) {
            data::LootPool* pool = dynamic_cast<data::LootPool*>(pool);
            uint32_t size = pool->entry_lookup.size();
            uint32_t last = pool_memory_offsets.back();
            pool_memory_offsets.push_back(last + size);
            
            for (auto i : pool->entry_lookup) {
                combined_shared_memory.push_back(i);
            }
        }

        fill_function_shared_mem(&root_node);
        uint32_t last_pool_offset = pool_memory_offsets.back();
        for (auto& func_off : function_memory_offsets) {
            func_off += last_pool_offset;
        }
    }
}