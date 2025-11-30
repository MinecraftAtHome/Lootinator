#ifndef LOOTINATOR_CONSTRAINT_FILTER_H
#define LOOTINATOR_CONSTRAINT_FILTER_H

#include "lootinator/constraint/constraint.h"
#include "lootinator/lsm/lsm.hpp"
#include "lootinator/mc/minecraft.hpp"

namespace loot {
    static const int NO_POOL_MATCH = -1;
    static const int MULTI_POOL_MATCH = -2;

    /*
    Describes the actual data the kernel will use for the initial filter. Unlike constraints,
    pool filters never define ranges of possible values, which includes both item counts and attributes.
    The item's properties will all be directly mapped to code filtering them.
    */
    struct PoolFilter {
        lsm::KernelStructureType reversal_type; // scope of reversal
        int pool_idx; // for which pool the filter is defined
        int entry_idx; // which entry the filter targets
        int entry_count; // how many instances of the entry are targetted
        mc::ItemAttribute attribute; // required attributes of the entry
        float filter_score; // heuristic performance estimate, bigger = faster

        PoolFilter(lsm::KernelStructureType type, int pool_idx, int entry_idx, int entry_count, mc::ItemAttribute attribute);
        void compute_filter_score(const LootTable& loot_table);
        bool operator==(const PoolFilter& other) const;
        bool operator!=(const PoolFilter& other) const;

    private:
        float compute_forward_filter_score(const LootTable& loot_table, const float item_rarity) const;
        float compute_backward_filter_penalty(const LootTable& loot_table) const;
    };

    // stores lists of loot constraints grouped into constraints on single pools and
    // global constraints (constraints depending on more than 1 loot pool).
    struct LootTableConstraintList {
        loot::LootTable& loot_table;
        std::vector<loot::PoolFilter> available_filters;
        std::vector<loot::Constraint> global_constraints;

        LootTableConstraintList(LootTable& loot_table);
        bool initialize_constraints(const std::vector<loot::Constraint>& constraints);

    private:
        void add_possible_filters(const std::vector<loot::Constraint>& constraints, const loot::Constraint& main_constraint, int pool_idx);
        void add_all_filter_variants(const nlohmann::json &pool, const util::RangeInclusive<uint32_t> &pool_rolls, const loot::PoolFilter &base_filter, const loot::Constraint &aggregated_constraint);
        loot::Constraint aggregate_constraints(const std::vector<loot::Constraint> &constraints, const loot::Constraint &main_constraint, lsm::KernelStructureType type);
        bool constraint_applicable(const loot::Constraint& constr, lsm::KernelStructureType type);
        bool constraints_match_for_reversal_type(const loot::Constraint& first, const loot::Constraint& second, lsm::KernelStructureType type) const;
        util::RangeInclusive<uint32_t> get_roll_range(const nlohmann::json& pool, const util::RangeInclusive<uint32_t>& pool_rolls, const loot::Constraint& aggregated_constraint) const;
        int find_matching_loot_pool(const loot::Constraint& constr) const;
        //bool entry_attributes_match(const loot::Constraint& constr, const nlohmann::json& entry) const;
        //bool entry_item_matches(const loot::Constraint& constr, const nlohmann::json& entry) const;
    };
};

#endif