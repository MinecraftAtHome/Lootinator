#ifndef LOOTINATOR_CONSTRAINT_FILTER_H
#define LOOTINATOR_CONSTRAINT_FILTER_H

#include "lootinator/constraint/constraint.h"

namespace loot {
    enum PoolMatchType {
        NOT_FOUND,
        SINGLE_POOL,
        MULTI_POOL
    };
    enum PoolFilterType {
        NOT_FOUND,
        SINGLE_POOL,
        MULTI_POOL
    };

    struct PoolFilter {
        int pool_idx;
        int item_idx;
        int item_count;
        std::vector<ItemAttribute> attributes;
        float filter_score;

        PoolMatchType find_matching_loot_pool(const LootTable& loot_table);
        void compute_filter_score(const LootTable& loot_table);

    private:
        bool entry_attributes_match(const nlohmann::json& entry) const;
        bool entry_item_matches(const LootTable& loot_table, const nlohmann::json& entry) const;

        float compute_forward_filter_score(const LootTable& loot_table, const float item_rarity) const;
        float compute_backward_filter_penalty(const LootTable& loot_table) const;
    };

    // stores lists of loot constraints grouped into constraints on single pools and
    // global constraints (constraints depending on more than 1 loot pool).
    struct LootTableConstraintList {
        LootTable& loot_table;
        std::vector<loot::PoolFilter> per_pool_constraints;
        std::vector<loot::Constraint> global_constraints;

        LootTableConstraintList(LootTable& loot_table);
        bool initialize_constraints(const std::vector<loot::Constraint>& constraints);
    };
};

#endif