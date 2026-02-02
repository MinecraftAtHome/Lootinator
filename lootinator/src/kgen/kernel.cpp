#include "lootinator/kgen/kernel.hpp"
#include "lootinator/global_settings.hpp"
#include <cinttypes>


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
        out << "#ifndef SHARED_DEFINITIONS\n"
            << "//@SharedDefinitionsStart\n"
            << "typedef " << GlobalSettings::UNSIGNED_32_TYPE << " u32\n"
            << "typedef " << GlobalSettings::SIGNED_32_TYPE << " i32\n"
            << "typedef " << GlobalSettings::UNSIGNED_64_TYPE << " u64\n"
            << "typedef " << GlobalSettings::SIGNED_64_TYPE << " i64\n\n";

        out <<
R"(constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = 0xffffffffffff;

__device__ inline void setSeed(u64* rand, u64 value){ *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline i32 next(u64* rand, const i32 bits){ *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48; return (i32)((i64)*rand >> (48 - bits)); }
__device__ inline i32 nextInt(u64* rand, const i32 n){ if ((n-1 & n) == 0) {u64 x = n * (u64)next(rand, 31); return (i32)((i64)x >> 31);} else {return (i32)(next(rand, 31) % n);} }
__device__ inline float nextFloat(u64* rand){ return next(rand, 24) / (float)(1 << 24) }; 
__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline i32 nextIntNoAdvance(u64 *rand, const i32 n) {if ((n-1 & n) == 0) {u64 x = n * *rand; return (i32)((i64)x >> 31);} else {return (i32)(*rand % n);}} 
//@SharedDefinitionsEnd
#endif
)";
    }
}