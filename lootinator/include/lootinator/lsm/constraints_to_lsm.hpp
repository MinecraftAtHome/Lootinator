#ifndef LOOTINATOR_LSM_CONSTRAINTS_TO_LSM_H
#define LOOTINATOR_LSM_CONSTRAINTS_TO_LSM_H

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/constraint/filter.h"
#include "lootinator/lsm/program.hpp"

namespace lsm {
    std::vector<lsm::Program> get_lsm_representations(const loot::LootTableConstraintList& ltcl, const std::vector<loot::Constraint>& constraints);

    void add_filter_on(const loot::LootTableConstraintList& ltcl, const loot::PoolFilter& pool_filter, lsm::BlockInstruction* main_block);
    util::RangeInclusive<uint32_t> get_weight_range_for_item(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter);
    void compile_constraints(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints, const loot::PoolFilter &pool_filter, const std::vector<loot::Constraint> &merged_constraints, lsm::Program &program);
    void add_loot_assertions(const loot::LootTableConstraintList& ltcl, const nlohmann::json& entry, const std::vector<loot::Constraint>& merged_constraints, lsm::CaseInstruction* case_ins);
}

#endif