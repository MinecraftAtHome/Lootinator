#include "lootinator/constraint/filter.h"

namespace loot {
    // iterates over the entries for each loot pool and find one that matches
    // the stored item index and the stored attributes
    PoolMatchType PoolFilter::find_matching_loot_pool(const LootTable &loot_table) {
        PoolMatchType pool_match = PoolMatchType::NOT_FOUND;
        int idx = 0;

        const auto& pools = loot_table.data["pools"];
        for (const auto& loot_pool : pools) {
            const auto& entries = loot_table.data["entries"];
            for (const auto& entry : entries) {
                if (entry_item_matches(loot_table, entry) && entry_attributes_match(entry)) {
                    if (pool_match != PoolMatchType::NOT_FOUND) {
                        pool_match = PoolMatchType::MULTI_POOL;
                    }
                    else {
                        pool_match = PoolMatchType::SINGLE_POOL;
                        pool_idx = idx;
                    }
                }
            }
            idx++;
        }

        return pool_match;
    }

    void PoolFilter::compute_filter_score(const LootTable &loot_table) {
        // calculate item weight reduction
        int w = 0;
        for (int i : loot_table.precomputed_loot[pool_idx]) {
            w += (item == i ? 1 : 0);
        }
        assert(w != 0);

        float weight_score = static_cast<float>(loot_table.total_weights[pool_idx]) / w;
        float attribute_bonus = attributes.empty() ? 1.0f : 10.0f; // FIXME magic number, also probably requires tuning
        float item_rarity = weight_score * attribute_bonus;

        float forward_score = compute_forward_filter_score(loot_table, item_rarity);
        float backward_penalty = compute_backward_filter_penalty(loot_table);

        filter_score = item_rarity * forward_score * backward_penalty;
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

    bool PoolFilter::entry_item_matches(const LootTable &loot_table, const nlohmann::json& entry) const {
        if (entry["type"] != "minecraft:item") {
            return false;
        }
        int item_idx = loot_table.find_item_name(entry["name"]);
        return item_idx == item;
    }

    bool PoolFilter::entry_attributes_match(const nlohmann::json& entry) const {
        // FIXME for now this is sufficient but if user wanted to filter e.g. bastion crossbows
        // they would be classified as multi-pool and thus couldn't be used as a pool constraint
        return true; 
    }

    // -------------------------------------------------------------------------------------------------------

    LootTableConstraintList::LootTableConstraintList(LootTable &loot_table) : loot_table(loot_table) {}
    
    bool LootTableConstraintList::initialize_constraints(const std::vector<loot::Constraint> &constraints) {
        for (const auto& con : constraints) {
            loot::PoolFilter pool_con(con);
            PoolMatchType match_result = pool_con.find_matching_loot_pool(loot_table);
            if (match_result == PoolMatchType::NOT_FOUND) {
                return false; // illegal constraint specified
            }
            else if (match_result == PoolMatchType::SINGLE_POOL) {
                pool_con.compute_filter_score(loot_table);
                per_pool_constraints.push_back(pool_con);
            }
            else {
                global_constraints.push_back(con);
            }
        }

        // sort the per-pool constraint vector in descending order using the computed heuristic score
        std::sort(per_pool_constraints.begin(), per_pool_constraints.end(), 
            [](const PoolFilter& a, const PoolFilter& b) {
                return a.filter_score > b.filter_score;
            }
        );
        
        return true;
    }
}