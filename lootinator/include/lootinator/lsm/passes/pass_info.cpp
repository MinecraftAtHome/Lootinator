#ifndef LOOTINATOR_LSM_PASSES_PASS_INFO_H
#define LOOTINATOR_LSM_PASSES_PASS_INFO_H

#include "lootinator/lsm/passes/pass_info.hpp"

namespace lsm {
    lsm::LootFunctionData PassInfo::get_data_for_function(int pool_idx, int entry_idx, int func_idx) const
    {
        for (auto& fd : function_data) {
            auto& fr = fd.function_ref;
            if (fr.pool_id == pool_idx && fr.entry_id == entry_idx && fr.function_id == func_idx) {
                return fd;
            }
        }
    }
}


#endif