#include "lootinator/constraint/filter.h"
#include "lootinator/utility/mth.h"
#include "lootinator/mc/minecraft.hpp"
#include "lootinator/lsm/lsm.hpp"

#include <iostream>

namespace loot {
    // iterates over the entries for each loot pool and find one that matches
    // the stored item index and the stored attributes
    int LootTableConstraintList::find_matching_loot_pool(const Constraint& constr) const {
        int pool_match = loot::NO_POOL_MATCH;
        int pool_idx = 0;

        const auto& pools = loot_table.data["pools"];
        for (const auto& loot_pool : pools) {
            const auto& entries = loot_pool["entries"];
            for (const auto& entry : entries) {
                if (constr.matches_entry(loot_table, entry)) {
                    if (pool_match != loot::NO_POOL_MATCH) {
                        pool_match = loot::MULTI_POOL_MATCH;
                    }
                    else {
                        pool_match = pool_idx;
                    }
                }
            }
            pool_idx++;
        }

        return pool_match;
    }

    PoolFilter::PoolFilter(lsm::KernelStructureType type, int pool_idx, int entry_idx, int entry_count, mc::ItemAttribute attribute) 
        : attribute(attribute), reversal_type(type), pool_idx(pool_idx), entry_idx(entry_idx), entry_count(entry_count), filter_score(0.0f) {}

    void PoolFilter::compute_filter_score(const LootTable &loot_table)
    {
        // calculate item weight reduction
        nlohmann::json entry = loot_table.data["pools"][pool_idx]["entries"][entry_idx];
        int w = entry.contains("weight") ? (int)entry["weight"] : 1;
        assert(w != 0);
        float weight_score = static_cast<float>(loot_table.total_weights[pool_idx]) / w;

        // calculate other stuff
        // FIXME for now entry weight is the only factor
        filter_score = weight_score;
    }

    bool PoolFilter::is_equivalent_to(const PoolFilter& other) const {
        if (reversal_type != other.reversal_type)
            return false;
            
        switch (reversal_type)
        {
        case lsm::KernelStructureType::STATE_PREDICTION_WEIGHT:
            return pool_idx == other.pool_idx && entry_idx == other.entry_idx;
        case lsm::KernelStructureType::ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL:
            return (*this) == other;
        case lsm::KernelStructureType::ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT:
            return pool_idx == other.pool_idx && entry_idx == other.entry_idx && attribute.type == other.attribute.type;
        }
        return false;
    }

    bool PoolFilter::operator==(const PoolFilter &other) const {
        return reversal_type == other.reversal_type
            && pool_idx == other.pool_idx
            && entry_idx == other.entry_idx
            && entry_count == other.entry_count
            && attribute == other.attribute;
    }

    bool PoolFilter::operator!=(const PoolFilter &other) const {
        return !(*this == other);
    }

    float PoolFilter::compute_forward_filter_score(const LootTable &loot_table, const float item_rarity) const {
        (void)loot_table;
        (void)item_rarity;
        // TODO
        // compute an approximate performance boost the kernel would get due to 
        // multiple rolls of the same item being required.
        return 1.0f;
    }

    float PoolFilter::compute_backward_filter_penalty(const LootTable &loot_table) const {
        (void)loot_table;
        // TODO
        // compute an approximate performance loss the kernel suffers due to
        // checking multiple backward state advancement options
        return 1.0f;
    }

    // -------------------------------------------------------------------------------------------------------

    LootTableConstraintList::LootTableConstraintList(LootTable &loot_table) : loot_table(loot_table) {}
    
    bool LootTableConstraintList::initialize_constraints(const std::vector<loot::Constraint> &constraints) {
        for (const auto& constr : constraints) {
            int pool_idx = find_matching_loot_pool(constr);
            if (pool_idx == loot::NO_POOL_MATCH) {
                return false; // illegal constraint specified
            }
            else if (pool_idx == loot::MULTI_POOL_MATCH) {
                global_constraints.push_back(constr);
            }
            else {
                add_possible_filters(constraints, constr, pool_idx);
            }
        }
        if (available_filters.empty()) {
            return false;
        }

        // sort the per-pool constraint vector in descending order using the computed heuristic score
        std::sort(available_filters.begin(), available_filters.end(), 
            [](const PoolFilter& a, const PoolFilter& b) {
                return a.filter_score > b.filter_score;
            }
        );

        // deduplicate in O(n^3) cause why not
        for (int i = 0; i < available_filters.size(); i++) {
            for (int j = 0; j < available_filters.size(); j++) {
                if (i == j) continue;
                if (available_filters.at(i).is_equivalent_to(available_filters.at(j))) {
                    printf("%d %d %d\n", available_filters[i].pool_idx, available_filters[i].entry_idx, available_filters[i].reversal_type);
                    available_filters.erase(available_filters.begin() + i);
                    if (i < j) {
                        j--;
                    }
                    i--;
                }
            }
        }
        
        return true;
    }

    // {
    //      >= 3 obsidian -> 2 obby every time = 2 rolls, 1 obby every time = 3 rolls
    //      >= 2 fire_charge
    //      == 1 golden_boots, protection >= 2
    // }

    void LootTableConstraintList::add_possible_filters(const std::vector<loot::Constraint>& constraints, const Constraint& main_constraint, int pool_idx) {
        const nlohmann::json& pool = loot_table.data["pools"][pool_idx];
        const util::RangeInclusive<uint32_t> pool_rolls = util::RangeInclusive<uint32_t>::from_json(pool["rolls"]);

        lsm::KernelStructureType rtypes[] = {
            lsm::KernelStructureType::STATE_PREDICTION_WEIGHT, 
            lsm::KernelStructureType::ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL, 
            lsm::KernelStructureType::ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT
        };

        for (auto reversal_type : rtypes) {
            if (!constraint_applicable(main_constraint, reversal_type))
                continue;

            loot::Constraint aggregated = aggregate_constraints(constraints, main_constraint, reversal_type);
            mc::ItemAttribute filtered_attribute = aggregated.attributes.empty() ? mc::ItemAttribute{0, 0} : aggregated.attributes.at(0);
            loot::PoolFilter base_filter(reversal_type, pool_idx, 0, 0, filtered_attribute);
            
            add_all_filter_variants(pool, pool_rolls, base_filter, aggregated);
        }
    }

    void LootTableConstraintList::add_all_filter_variants(const nlohmann::json& pool, const util::RangeInclusive<uint32_t>& pool_rolls, const loot::PoolFilter& base_filter, const loot::Constraint& aggregated_constraint)
    {
        // count how many rolls min/max
        util::RangeInclusive<uint32_t> items_per_roll(1, 1);
        int entry_idx = 0, e = 0;

        for (auto& entry: pool["entries"]) {
            if (aggregated_constraint.matches_entry(loot_table, entry)) {
                entry_idx = e;
                //std::cerr << "entry = " << entry_idx << '\n';
                if (!entry.contains("functions"))
                    continue;
                for (auto& loot_fun : entry["functions"]) {
                    if (loot_fun["function"] == "minecraft:set_count") {
                        items_per_roll = util::RangeInclusive<uint32_t>::from_json(loot_fun["count"]);
                        //std::cerr << "setcount: min = " << items_per_roll.min << " max = " << items_per_roll.max << '\n';
                        break;
                    }
                }
            }
            e++;
        }

        // min items per roll -> max rolls needed
        util::RangeInclusive<uint32_t> roll_range(1, 1);
        roll_range.min = max(pool_rolls.min, ceil_div(aggregated_constraint.count_range.min, items_per_roll.max));
        roll_range.max = min(pool_rolls.max, aggregated_constraint.count_range.max / items_per_roll.min);
        //std::cerr << "rollmin = " << roll_range.min << " rollmax = " << roll_range.max << '\n';

        loot::PoolFilter pool_filter(
            base_filter.reversal_type,
            base_filter.pool_idx,
            entry_idx,
            1,
            base_filter.attribute
        );
        // always add simple, single-item filter
        available_filters.push_back(pool_filter); // state prediction

        // try adding a consecutive multi-item filter (for 2 or 3 rolls)
        // if (roll_range.min > 1) {
        //     // X - X - X - ...
        //     if (roll_range.min > pool_rolls.max / 2) {
        //         pool_filter.entry_count = 2;
        //         available_filters.push_back(pool_filter); // advanced reversal on 2 items
        //     }
        //     // X X - X X - ...
        //     if (roll_range.min > pool_rolls.max*2 / 3) {
        //         pool_filter.entry_count = 3;
        //         available_filters.push_back(pool_filter); // advanced reversal on 3 items
        //     }
        // }

        // TODO add epic's thing
    }

    loot::Constraint LootTableConstraintList::aggregate_constraints(const std::vector<loot::Constraint> &constraints, const loot::Constraint &main_constraint, lsm::KernelStructureType reversal_type)
    {
        loot::Constraint aggregate{main_constraint.item, {0,0}, loot::SLOT_NONE, main_constraint.attributes};
        for (const auto& con : constraints) {
            if (constraints_match_for_reversal_type(main_constraint, con, reversal_type)) {
                aggregate.count_range = aggregate.count_range.merge(con.count_range);
            }
        }
        return aggregate;
    }

    bool LootTableConstraintList::constraint_applicable(const loot::Constraint &constr, lsm::KernelStructureType type) {
        return type == lsm::KernelStructureType::STATE_PREDICTION_WEIGHT || !constr.attributes.empty();
    }

    util::RangeInclusive<uint32_t> LootTableConstraintList::get_roll_range(const nlohmann::json &pool, const util::RangeInclusive<uint32_t>& pool_rolls, const loot::Constraint &aggregated_constraint) const {
        util::RangeInclusive<uint32_t> items_per_roll(1, 1);

        for (auto& entry: pool["entries"]) {
            if (aggregated_constraint.matches_entry(loot_table, entry)) {
                for (auto& loot_fun : entry["functions"]) {
                    if (loot_fun["function"] == "minecraft:set_count") {
                        items_per_roll = util::RangeInclusive<uint32_t>::from_json(loot_fun["count"]);
                        break;
                    }
                }
            }
        }

        // min items per roll -> max rolls needed
        util::RangeInclusive<uint32_t> roll_range(1, 1);
        roll_range.min = max(pool_rolls.min, ceil_div(aggregated_constraint.count_range.min, items_per_roll.max));
        roll_range.max = min(pool_rolls.max, aggregated_constraint.count_range.max / items_per_roll.min);
        return roll_range;
    }

    // FIXME assumes the first constraint has only 1 relevant attribute and it's at index 0
    bool LootTableConstraintList::constraints_match_for_reversal_type(const loot::Constraint &main_constraint, const loot::Constraint &second, lsm::KernelStructureType type) const
    {
        switch (type) {
            case lsm::KernelStructureType::BRUTEFORCE:
                return false; // this should never happen, bruteforce is eliminated as an option before we enter this func

            case lsm::KernelStructureType::STATE_PREDICTION_WEIGHT:
                return main_constraint.item == second.item;

            case lsm::KernelStructureType::ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT: {
                auto& main_attr = main_constraint.attributes.at(0);
                for (auto& attr : main_constraint.attributes) {
                    if (main_attr.type == attr.type)
                        return true;
                }
                return false;
            }

            case lsm::KernelStructureType::ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL: {
                auto& main_attr = main_constraint.attributes.at(0);
                for (auto& attr : main_constraint.attributes) {
                    if (main_attr.type == attr.type && main_attr.level == attr.level)
                        return true;
                }
                return false;
            }
        }
        std::cerr << "LootTableConstraintList::constraints_match_for_reversal_type: Unknown reversal type: " << type << '\n';
        return false;
    }
}