#ifndef LOOTINATOR_LSM_CONSTRAINTS_TO_LSM
#define LOOTINATOR_LSM_CONSTRAINTS_TO_LSM

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/constraint/filter.h"

namespace loot::lsm {
    BlockInstruction* get_lsm_representation(const LootTableConstraintList& ltcl, const std::vector<loot::Constraint>& constraints);
}


#endif