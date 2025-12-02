#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/lsm/program.hpp"
#include <iostream>

namespace lsm {
    std::vector<lsm::Program> get_lsm_representations(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints)
    {
        std::vector<loot::Constraint> merged_constraints;
        loot::merge_contraints(constraints, merged_constraints);

        std::vector<lsm::Program> programs;
        for (auto& pool_filter : ltcl.available_filters) {
            lsm::BlockInstruction* main_block = new lsm::BlockInstruction();
            lsm::PassInfo pass_info = lsm::PassInfo(pool_filter);
            lsm::Program program = lsm::Program(main_block, pass_info);
            compile_constraints(ltcl, constraints, pool_filter, merged_constraints, program);
            programs.push_back(program);
        }

        return programs;
    }

    util::RangeInclusive<uint32_t> get_weight_range_for_item(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter) {
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

    /**
     * Adds the rng-level data necessary to execute the enchant_randomly function:
     * - the total number of enchantments the target is being chosen from (maps to `nextInt` bound)
     * - the index of the target enchantment in that list (maps to expected `nextInt` value)
     * - the maximum level of the target enchantment (maps to `nextInt` bound of the level call)
     * - the target level of the target enchantment, or `0` if the value is ignored (maps to expected `nextInt` value of the level call)
     */
    static void add_enchant_randomly_filter_data(const loot::LootTable &loot_table, const loot::PoolFilter &pool_filter, lsm::Program& program)
    {
        mc::Enchantment filtered_ench = mc::get_enchantment_from_attribute(pool_filter.attribute);

        // find the enchant randomly function and extract its data
        const auto& entry = loot_table.data["pools"][pool_filter.pool_idx]["entries"][pool_filter.entry_idx];
        int func_idx = 0;
        for (auto& func : entry["functions"]) {
            if (func["function"] == "minecraft:enchant_randomly") {
                break;
            }
            func_idx++;
        }
        lsm::LootFunctionData func_data = program.pass_info.get_data_for_function(pool_filter.pool_idx, pool_filter.entry_idx, func_idx);
        
        int target_idx = 0;
        for (auto& ench : func_data.enchant_randomly.enchantment_order) {
            if (filtered_ench == ench) {
                break;
            }
            target_idx++;
        }

        std::vector<int>& code_data = program.pass_info.filter_on_code_data;
        code_data.push_back(func_data.enchant_randomly.enchantment_order.size()); // total enchantments (#1)
        code_data.push_back(target_idx); // target index (#2)
        code_data.push_back(mc::get_max_level(filtered_ench)); // max level (#3)
        code_data.push_back(pool_filter.attribute.level); // target level (#4)
    }

    std::vector<int> get_next_int_vector(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter, lsm::Program& program) {
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

        std::vector<int> signature_vec;

        // TODO when new types of kernel structures are added, they need to be handled here
        if (pool_filter.reversal_type == lsm::KernelStructureType::STATE_PREDICTION_WEIGHT || pool_filter.reversal_type == lsm::KernelStructureType::ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT)
        {
            //std::cerr << "poolidx = " << pool_filter.pool_idx << " vecsize = " << ltcl.loot_table.total_weights.size() << "\n";
            int total_weight = ltcl.loot_table.total_weights[pool_filter.pool_idx];
            util::RangeInclusive<uint32_t> weight_range = get_weight_range_for_item(ltcl, pool_filter);
            program.pass_info.filter_on_code_data.push_back(total_weight);
            program.pass_info.filter_on_code_data.push_back(weight_range.min);
            program.pass_info.filter_on_code_data.push_back(weight_range.max);
        }
        if (pool_filter.attribute.get_category() == mc::AttributeCategory::ENCHANTMENT_ATTRIBUTE)
        {
            if (pool_filter.reversal_type == lsm::KernelStructureType::ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL || pool_filter.reversal_type == lsm::KernelStructureType::ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT)
            {
                mc::Enchantment ench = mc::get_enchantment_from_attribute(pool_filter.attribute);
                signature_vec.push_back(static_cast<int>(ench));
                add_enchant_randomly_filter_data(ltcl.loot_table, pool_filter, program);
            }
            if (pool_filter.reversal_type == lsm::KernelStructureType::ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL)
            {
                signature_vec.push_back(pool_filter.attribute.level);
            }
        }
        
        return signature_vec;
    }

    void compile_constraints(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints, const loot::PoolFilter &pool_filter, const std::vector<loot::Constraint> &merged_constraints, lsm::Program &program) {
        int pool_idx = 0;
        for (auto& pool : ltcl.loot_table.data["pools"])
        {
            PoolInstruction* pool_block = new PoolInstruction(pool_idx);
            util::RangeInclusive<std::uint32_t> roll_range = util::RangeInclusive<std::uint32_t>::from_json(pool["rolls"]);     
            FunctionInstruction* advance = new FunctionInstruction(FunctionType::FUNC_LCG_ADVANCE, {roll_range.min != roll_range.max ? 1 : 0}); 
            RollInstruction* roll_block = new RollInstruction(roll_range.min, roll_range.max - roll_range.min + 1);
            pool_block->add_instruction(advance);
            
            int entry_idx = 0;
            for (auto& entry : pool["entries"])
            {
                // FIXME this could break if multiple entries have same item
                CaseInstruction* case_ins = new CaseInstruction(entry_idx);
                for (int func_idx = 0; func_idx < entry["functions"].size(); func_idx++)
                {
                    if (entry_idx == pool_filter.entry_idx) {
                        std::vector<int> next_int_vector = get_next_int_vector(ltcl, pool_filter, program);
                        case_ins->add_instruction(new FunctionInstruction(FunctionType::FUNC_FILTER_ON, next_int_vector));
                    }
                    FunctionInstruction* func_ins = new FunctionInstruction(FunctionType::FUNC_FUNC, {pool_idx, entry_idx, func_idx});
                    case_ins->add_instruction(func_ins);
                }
                add_loot_assertions(ltcl, entry, merged_constraints, case_ins);

                roll_block->add_instruction(case_ins);
                entry_idx++;
            }

            pool_block->add_instruction(roll_block);
            program.main_block->add_instruction(pool_block);
            pool_idx++;
        }
    }

    static std::vector<int> vectorize_attributes(const loot::LootTableConstraintList &ltcl, const loot::Constraint& constraint)
    {
        std::vector<int> attributes;
        for (auto const &attr : constraint.attributes) {
            attributes.push_back(attr.type);
            attributes.push_back(attr.level);   
        }
        return attributes;
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
