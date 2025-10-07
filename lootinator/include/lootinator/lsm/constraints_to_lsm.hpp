#ifndef LOOTINATOR_LSM_CONSTRAINTS_TO_LSM_H
#define LOOTINATOR_LSM_CONSTRAINTS_TO_LSM_H

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/constraint/filter.h"

namespace loot { namespace lsm {
    std::vector<loot::lsm::BlockInstruction*> get_lsm_representations(const LootTableConstraintList& ltcl, const std::vector<loot::Constraint>& constraints);

    void add_filter_on(const loot::LootTableConstraintList& ltcl, const loot::PoolFilter& pool_filter, loot::lsm::BlockInstruction* main_block);
    void add_pool_forward_filters(const loot::LootTableConstraintList& ltcl, const std::vector<loot::Constraint>& constraints, const std::vector<loot::Constraint>& merged_constraints, loot::lsm::BlockInstruction* main_block);
    void add_loot_assertions(const loot::LootTableConstraintList& ltcl, const nlohmann::json& entry, const std::vector<loot::Constraint>& merged_constraints, loot::lsm::CaseInstruction* case_ins);
}}

#endif