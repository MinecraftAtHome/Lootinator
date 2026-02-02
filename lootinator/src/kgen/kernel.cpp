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

    void Kernel::write_shared_definitions(std::ostream& out) {
        return
R"(#ifndef SHARED_DEFINITIONS
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = ((1ULL << 48) - 1);

__device__ inline void setSeed(uint64_t* rand, uint64_t value){ *rand = (value ^ 0x5deece66d) & ((1ULL << 48) - 1); }
__device__ inline int next(uint64_t* rand, const int bits){ *rand = (*rand * 0x5deece66d + 0xb) & ((1ULL << 48) - 1); return (int)((int64_t)*rand >> (48 - bits)); }
__device__ inline int nextInt(uint64_t* rand, const int n){ if ((n-1 & n) == 0) {uint64_t x = n * (uint64_t)next(rand, 31); return (int)((int64_t)x >> 31);} else {return (int)(next(rand, 31) % n);} }
__device__ inline float nextFloat(uint64_t* rand){ return next(rand, 24) / (float)(1 << 24) }; 
__device__ inline int nextIntBounded(uint64_t* rand, const int min, const int max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline int nextIntNoAdvance(uint64_t *rand, const int n) {if ((n-1 & n) == 0) {uint64_t x = n * *rand; return (int)((int64_t)x >> 31);} else {return (int)(*rand % n);}} 
#endif
)";
    }
}