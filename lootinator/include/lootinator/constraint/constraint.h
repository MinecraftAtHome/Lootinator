#ifndef LOOTINATOR_CONSTRAINT_CONSTRAINT_H
#define LOOTINATOR_CONSTRAINT_CONSTRAINT_H

#include "lootinator/utility/range.h"
#include "lootinator/utility/debug.h"
#include "lootinator/loot_table.h"

#include <cstdint>

namespace loot {
    constexpr int32_t SLOT_NONE = -1;

    // represents an additional attribute of an item, such as an enchantment
    // or type of music disc
    struct ItemAttribute {
        std::uint32_t type;
        RangeInclusive<std::uint32_t> level_range;

        static ItemAttribute from_json(nlohmann::json json) {
            uint32_t type = json["type"];
            loot::RangeInclusive<std::uint32_t> level_range = RangeInclusive<std::uint32_t>::from_json(json["level_range"]);
            return {type, level_range};
        }
        bool operator==(const ItemAttribute& other) const;
        bool operator!=(const ItemAttribute& other) const;
        friend std::ostream& operator<<(std::ostream& os, const ItemAttribute& attribute);
    };

    bool attributes_match(const std::vector<ItemAttribute>& first, const std::vector<ItemAttribute>& second);

    // stores loot constraints on individual slots of items
    struct Constraint {
        std::uint32_t item;
        RangeInclusive<std::uint32_t> count_range;
        std::int32_t slot_id; // contraints are shared by cracking and finding kernels, finding won't use this

        std::vector<ItemAttribute> attributes;

        bool item_equal(const Constraint& other) const;
        bool operator==(const Constraint& other) const;
        bool operator!=(const Constraint& other) const;
        friend std::ostream& operator<<(std::ostream& os, const Constraint& constraint);
    };

    void merge_contraints(const std::vector<loot::Constraint>& src, std::vector<loot::Constraint>& dest);
    std::vector<loot::Constraint> parse_constraints_from_json(const char *filepath);

    // ------------------------------------------------------------------------------------------------------
    // the following section is for handling constraints satisfiable only by specific pools of the loot table
    
    enum PoolMatchType {
        NOT_FOUND,
        SINGLE_POOL,
        MULTI_POOL
    };
    struct PoolConstraint : Constraint {
        int pool_idx = -1;
        float filter_score = 0.0f;

        PoolConstraint(const Constraint& constraint);
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
        std::vector<loot::PoolConstraint> per_pool_constraints;
        std::vector<loot::Constraint> global_constraints;

        LootTableConstraintList(LootTable& loot_table);
        bool initialize_constraints(const std::vector<loot::Constraint>& constraints);
    };
}

#endif
