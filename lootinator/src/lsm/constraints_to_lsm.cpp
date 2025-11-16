#include "lootinator/lsm/constraints_to_lsm.hpp"
#include <iostream>

namespace lsm {
    std::vector<lsm::BlockInstruction*> get_lsm_representations(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints)
    {
        std::vector<loot::Constraint> merged_constraints;
        loot::merge_contraints(constraints, merged_constraints);

        std::vector<lsm::BlockInstruction*> programs;
        for (auto& pool_filter : ltcl.available_filters) {
            lsm::BlockInstruction* main_block = new lsm::BlockInstruction();
            add_filter_on(ltcl, pool_filter, main_block);
            add_pool_forward_filters(ltcl, constraints, merged_constraints, main_block);
            programs.push_back(main_block);
        }

        return programs;
    }

    static void add_next_int(int bound, int min_incl, int max_incl, std::vector<int>& out)
    {
        out.push_back(bound);
        out.push_back(min_incl);
        out.push_back(max_incl);
    }

    void add_filter_on(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter, lsm::BlockInstruction *main_block)
    {
        // TODO

        //filter-on (bound1 min1 max1) (bound2 min2 max2) ...
        // - enchant_randomly enchant id -> enchant index for the loot function ()
        // - set_count item count -> nextInt output that generates that item count (~)

        // for enchant randomly:
        // - version range -> enchantment order mapping (V)
        // - check which enchantment can go on item (V)

        bool entry_set_count_advancement = false;
        auto& entry = ltcl.loot_table.data["pools"][pool_filter.pool_idx]["entries"][pool_filter.entry_idx];
        if (entry.contains("functions")) {
            for (auto& func : entry["functions"]) {
                if (func["function"] == "minecraft:set_count") {
                    util::RangeInclusive<uint32_t> count_range = util::RangeInclusive<uint32_t>::from_json(func["count"]);
                    entry_set_count_advancement = count_range.min != count_range.max;
                    break;
                }
            }
        }

        std::vector<int> next_int_vector;
        if (pool_filter.reversal_type == loot::ReversalType::ITEM_ONLY)
        {
            //std::cerr << "poolidx = " << pool_filter.pool_idx << " vecsize = " << ltcl.loot_table.total_weights.size() << "\n";
            int total_weight = ltcl.loot_table.total_weights[pool_filter.pool_idx];
            util::RangeInclusive<uint32_t> weight_range = get_weight_range_for_item(ltcl, pool_filter);
            for (int i = 0; i < pool_filter.entry_count; i++)
            {
                add_next_int(total_weight, weight_range.min, weight_range.max, next_int_vector);
                if (entry_set_count_advancement) {
                    add_next_int(0, 1, 1, next_int_vector); // skip 1 lcg state
                }
            }
        }

        main_block->add_instruction(new FunctionInstruction(FunctionType::FUNC_FILTER_ON, next_int_vector));
    }

    util::RangeInclusive<uint32_t> get_weight_range_for_item(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter)
    {
        int pool_idx = pool_filter.pool_idx;
        const auto& entries = ltcl.loot_table.data["pools"][pool_idx]["entries"];

        uint32_t current_weight = 0;
        int eid = 0;
        for (const auto& entry : entries) {
            int entry_weight = 1;
            if (entry.contains("weight")) {
                entry_weight = entry["weight"];
            }
            if (eid == pool_filter.entry_idx) {
                return util::RangeInclusive<uint32_t>(current_weight, current_weight+entry_weight-1);
            }
            current_weight += entry_weight;
            eid++;
        }

        std::cerr << "constraints_to_lsm.cpp: get_weight_range_for_item: failed to find entry" 
                     "matching the specified pool filter, pool_filter.entry_idx = " << pool_filter.entry_idx << '\n';
        return util::RangeInclusive<uint32_t>(0,0);
    }

    void add_pool_forward_filters(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints, const std::vector<loot::Constraint> &merged_constraints, lsm::BlockInstruction *main_block)
    {
        int pool_idx = 0;
        for (auto& pool : ltcl.loot_table.data["pools"])
        {
            PoolInstruction* pool_block = new PoolInstruction(pool_idx);
            util::RangeInclusive<std::uint32_t> roll_range = util::RangeInclusive<std::uint32_t>::from_json(pool["rolls"]);     
            FunctionInstruction* advance = new FunctionInstruction(FunctionType::FUNC_LCG_ADVANCE, {roll_range.min != roll_range.max ? 1 : 0}); 
            RollInstruction* roll_block = new RollInstruction(0/*TODO use actual data*/);
            pool_block->add_instruction(advance);
            
            int entry_idx = 0;
            for (auto& entry : pool["entries"])
            {
                // FIXME this could break if multiple entries have same item
                CaseInstruction* case_ins = new CaseInstruction(entry_idx);
                for (int func_idx = 0; func_idx < entry["functions"].size(); func_idx++)
                {
                    FunctionInstruction* func_ins = new FunctionInstruction(FunctionType::FUNC_FUNC, {pool_idx, entry_idx, func_idx});
                    case_ins->add_instruction(func_ins);
                }
                add_loot_assertions(ltcl, entry, merged_constraints, case_ins);

                roll_block->add_instruction(case_ins);
                entry_idx++;
            }

            pool_block->add_instruction(roll_block);
            main_block->add_instruction(pool_block);
            pool_idx++;
        }
    }

    static std::vector<int> vectorize_attributes(const loot::LootTableConstraintList &ltcl, const loot::Constraint& constraint)
    {
        (void)ltcl;
        (void)constraint;
        // TODO combine numeric values of all attributes that are represented
        // by a single point (ranges should get ignored)
        return std::vector<int>();
    }

    void add_loot_assertions(const loot::LootTableConstraintList &ltcl, const nlohmann::json& entry, const std::vector<loot::Constraint> &merged_constraints, lsm::CaseInstruction *case_ins)
    {
        for (auto& constraint : merged_constraints)
        {
            if (constraint.matches_entry(ltcl.loot_table, entry))
            {
                std::vector<int> attribute_vector = vectorize_attributes(ltcl, constraint);
                if (constraint.count_range.min == constraint.count_range.max)
                {
                    // '==' comparator, need a single assertion
                    PoolAssertFunctionInstruction* pool_assert_ins = new PoolAssertFunctionInstruction(
                        attribute_vector, lsm::Comparision::COMP_EQUAL, {static_cast<int>(constraint.count_range.min)}
                    );
                    case_ins->add_instruction(pool_assert_ins);
                    continue;
                }

                if (constraint.count_range.min != loot::COUNT_NONE)
                {
                    // min count is bounded, need a '>=' assertion
                    PoolAssertFunctionInstruction* pool_assert_ins = new PoolAssertFunctionInstruction(
                        attribute_vector, lsm::Comparision::COMP_GE, {static_cast<int>(constraint.count_range.min)}
                    );
                    case_ins->add_instruction(pool_assert_ins);
                }
                // if (constraint.count_range.max < loot::COUNT_INFINITE)
                // {
                //     // max count is bounded, need a '<=' assertion
                //     PoolAssertFunctionInstruction* pool_assert_ins = new PoolAssertFunctionInstruction(
                //         attribute_vector, lsm::Comparision::COMP_LE, {static_cast<int>(constraint.count_range.max)}
                //     );
                //     case_ins->add_instruction(pool_assert_ins);
                // }
            }
        }
    }
}
