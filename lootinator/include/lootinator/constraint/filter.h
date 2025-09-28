#ifndef LOOTINATOR_CONSTRAINT_FILTER_H
#define LOOTINATOR_CONSTRAINT_FILTER_H

#include "lootinator/constraint/constraint.h"

namespace loot {
    static const int NO_POOL_MATCH = -1;
    static const int MULTI_POOL_MATCH = -2;

    enum ReversalType {
        ITEM_ONLY,
        ITEM_AND_ATTRIBUTE,
        ITEM_AND_ATTRIBUTE_AND_LEVEL
    };

    /*
    Describes the actual data the kernel will use for the initial filter. Unlike constraints,
    pool filters never define ranges of possible values, which includes both item counts and attributes.
    The item's properties will all be directly mapped to code filtering them.
    */
    struct PoolFilter {
        ReversalType reversal_type; // scope of reversal
        int pool_idx; // for which pool the filter is defined
        int entry_idx; // which entry the filter targets
        int entry_count; // how many instances of the entry are targetted
        ItemAttribute attribute; // required attributes of the entry
        float filter_score; // heuristic performance estimate, bigger = faster

        PoolFilter(ReversalType type, int pool_idx, int entry_idx, int entry_count, ItemAttribute attribute);
        void compute_filter_score(const LootTable& loot_table);

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
        void add_all_filter_variants(const nlohmann::json &pool, const RangeInclusive<uint32_t> &pool_rolls, const loot::PoolFilter &base_filter, const loot::Constraint &aggregated_constraint);
        loot::Constraint aggregate_constraints(const std::vector<loot::Constraint> &constraints, const loot::Constraint &main_constraint, loot::ReversalType type);
        bool constraint_applicable(const loot::Constraint& constr, ReversalType type);
        bool constraints_match_for_reversal_type(const loot::Constraint& first, const loot::Constraint& second, loot::ReversalType type) const;
        RangeInclusive<uint32_t> get_roll_range(const nlohmann::json& pool, const RangeInclusive<uint32_t>& pool_rolls, const loot::Constraint& aggregated_constraint) const;
        int find_matching_loot_pool(const loot::Constraint& constr) const;
        bool entry_attributes_match(const loot::Constraint& constr, const nlohmann::json& entry) const;
        bool entry_item_matches(const loot::Constraint& constr, const nlohmann::json& entry) const;
    };
};

#endif