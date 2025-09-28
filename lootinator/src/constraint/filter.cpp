#include "lootinator/constraint/filter.h"
#include "lootinator/utility/mth.h"
#include <iostream>
#include "filter.h"


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
                if (entry_item_matches(constr, entry) && entry_attributes_match(constr, entry)) {
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

    PoolFilter::PoolFilter(ReversalType type, int pool_idx, int entry_idx, int entry_count, ItemAttribute attribute) 
        : attribute(attribute), reversal_type(type), pool_idx(pool_idx), entry_idx(entry_idx), entry_count(entry_count), filter_score(0.0f) {}

    void PoolFilter::compute_filter_score(const LootTable &loot_table)
    {
        // calculate item weight reduction
        nlohmann::json entry = loot_table.data["pools"][pool_idx]["entries"][entry_idx];
        int w = entry.contains("weight") ? entry["weight"] : 1;
        assert(w != 0);
        float weight_score = static_cast<float>(loot_table.total_weights[pool_idx]) / w;

        // calculate other stuff
        // FIXME for now entry weight is the only factor
        filter_score = weight_score;
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
        // TODO
        // compute an approximate performance boost the kernel would get due to 
        // multiple rolls of the same item being required.
        return 1.0f;
    }

    float PoolFilter::compute_backward_filter_penalty(const LootTable &loot_table) const {
        // TODO
        // compute an approximate performance loss the kernel suffers due to
        // checking multiple backward state advancement options
        return 1.0f;
    }

    bool LootTableConstraintList::entry_item_matches(const Constraint& constr, const nlohmann::json& entry) const {
        //std::cerr << entry["type"] << " " << entry["name"] << " ";
        if (entry["type"] != "minecraft:item") {
            return false;
        }
        int item_idx = loot_table.find_item_name(entry["name"]);
        //std::cerr << item_idx << '\n';
        return item_idx == constr.item;
    }

    bool LootTableConstraintList::entry_attributes_match(const Constraint& constr, const nlohmann::json& entry) const {
        // FIXME for now this is sufficient but if user wanted to filter e.g. bastion crossbows
        // they would be classified as multi-pool and thus couldn't be used as a pool constraint
        return true; 
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
            for (int j = 0; j < i; j++) {
                if (i == j) continue;
                if (available_filters.at(i) == available_filters.at(j)) {
                    available_filters.erase(available_filters.begin() + i);
                    i--;
                    j--;
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
        const RangeInclusive<uint32_t> pool_rolls = RangeInclusive<uint32_t>::from_json(pool["rolls"]);

        ReversalType rtypes[3] = {ReversalType::ITEM_ONLY, ReversalType::ITEM_AND_ATTRIBUTE, ReversalType::ITEM_AND_ATTRIBUTE_AND_LEVEL};
        for (auto reversal_type : rtypes) {
            if (!constraint_applicable(main_constraint, reversal_type))
                continue;

            loot::Constraint aggregated = aggregate_constraints(constraints, main_constraint, reversal_type);
            loot::ItemAttribute filtered_attribute = aggregated.attributes.empty() ? loot::ItemAttribute{0, {0, 0}} : aggregated.attributes.at(0);
            loot::PoolFilter base_filter(reversal_type, pool_idx, 0, 0, filtered_attribute);
            add_all_filter_variants(pool, pool_rolls, base_filter, aggregated);
        }
    }

    void LootTableConstraintList::add_all_filter_variants(const nlohmann::json& pool, const RangeInclusive<uint32_t>& pool_rolls, const loot::PoolFilter& base_filter, const loot::Constraint& aggregated_constraint)
    {
        // count how many rolls min/max
        RangeInclusive<uint32_t> items_per_roll(1, 1);
        int entry_idx = 0, e = 0;

        for (auto& entry: pool["entries"]) {
            // FIXME attributes should match too!!!
            if (entry["type"] == "minecraft:item" && aggregated_constraint.item == loot_table.find_item_name(entry["name"])) {
                entry_idx = e;
                for (auto& loot_fun : entry["functions"]) {
                    if (loot_fun["function"] == "minecraft:set_count") {
                        items_per_roll = RangeInclusive<uint32_t>::from_json(loot_fun["count"]);
                        break;
                    }
                }
            }
            e++;
        }

        // min items per roll -> max rolls needed
        RangeInclusive<uint32_t> roll_range(1, 1);
        roll_range.min = max(pool_rolls.min, ceil_div(aggregated_constraint.count_range.min, items_per_roll.max));
        roll_range.max = min(pool_rolls.max, aggregated_constraint.count_range.max / items_per_roll.min);

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
        if (roll_range.min > 1) {
            // X - X - X - ...
            if (roll_range.min > pool_rolls.max / 2) {
                pool_filter.entry_count = 2;
                available_filters.push_back(pool_filter); // advanced reversal on 2 items
            }
            // X X - X X - ...
            if (roll_range.min > pool_rolls.max*2 / 3) {
                pool_filter.entry_count = 3;
                available_filters.push_back(pool_filter); // advanced reversal on 3 items
            }
        }

        // TODO add epic's thing
    }

    loot::Constraint LootTableConstraintList::aggregate_constraints(const std::vector<loot::Constraint> &constraints, const loot::Constraint &main_constraint, loot::ReversalType reversal_type)
    {
        loot::Constraint aggregate{main_constraint.item, {0,0}, loot::SLOT_NONE, main_constraint.attributes};
        for (const auto& con : constraints) {
            if (constraints_match_for_reversal_type(main_constraint, con, reversal_type)) {
                aggregate.count_range = aggregate.count_range.merge(con.count_range);
            }
        }
        return aggregate;
    }

    bool LootTableConstraintList::constraint_applicable(const loot::Constraint &constr, ReversalType type) {
        return type == ReversalType::ITEM_ONLY || !constr.attributes.empty();
    }

    RangeInclusive<uint32_t> LootTableConstraintList::get_roll_range(const nlohmann::json &pool, const RangeInclusive<uint32_t>& pool_rolls, const loot::Constraint &aggregated_constraint) const {
        RangeInclusive<uint32_t> items_per_roll(1, 1);

        for (auto& entry: pool["entries"]) {
            // FIXME attributes should match too!!!
            if (entry["type"] != "minecraft:item") {
                continue;
            }
            if (aggregated_constraint.item == loot_table.find_item_name(entry["name"])) {
                for (auto& loot_fun : entry["functions"]) {
                    if (loot_fun["function"] == "minecraft:set_count") {
                        items_per_roll = RangeInclusive<uint32_t>::from_json(loot_fun["count"]);
                        break;
                    }
                }
            }
        }

        // min items per roll -> max rolls needed
        RangeInclusive<uint32_t> roll_range(1, 1);
        roll_range.min = max(pool_rolls.min, ceil_div(aggregated_constraint.count_range.min, items_per_roll.max));
        roll_range.max = min(pool_rolls.max, aggregated_constraint.count_range.max / items_per_roll.min);
        return roll_range;
    }

    // FIXME assumes the first constraint has only 1 relevant attribute and it's at index 0
    bool LootTableConstraintList::constraints_match_for_reversal_type(const loot::Constraint &main_constraint, const loot::Constraint &second, loot::ReversalType type) const
    {
        switch (type) {
            case ReversalType::ITEM_ONLY:
                return main_constraint.item == second.item;
            case ReversalType::ITEM_AND_ATTRIBUTE: {
                auto& main_attr = main_constraint.attributes.at(0);
                for (auto& attr : main_constraint.attributes) {
                    if (main_attr.type == attr.type)
                        return true;
                }
                return false;
            }
            case ReversalType::ITEM_AND_ATTRIBUTE_AND_LEVEL: {
                auto& main_attr = main_constraint.attributes.at(0);
                for (auto& attr : main_constraint.attributes) {
                    if (main_attr.type == attr.type && main_attr.level_range == attr.level_range)
                        return true;
                }
                return false;
            }
        }
    }
}