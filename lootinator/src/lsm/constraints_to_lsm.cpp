#include "lootinator/lsm/constraints_to_lsm.hpp"
#include <iostream>

namespace loot { namespace lsm {
    std::vector<loot::lsm::BlockInstruction*> get_lsm_representations(const LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints)
    {
        std::vector<loot::Constraint> merged_constraints;
        loot::merge_contraints(constraints, merged_constraints);

        std::vector<loot::lsm::BlockInstruction*> programs;
        for (auto& pool_filter : ltcl.available_filters) {
            loot::lsm::BlockInstruction* main_block = new loot::lsm::BlockInstruction();
            add_filter_on(ltcl, pool_filter, main_block);
            add_pool_forward_filters(ltcl, constraints, merged_constraints, main_block);
            programs.push_back(main_block);
        }

        return programs;
    }

    void add_filter_on(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter, loot::lsm::BlockInstruction *main_block)
    {
        (void)ltcl;
        (void)pool_filter;
        (void)main_block;
        // (big) TODO
    }

    void add_pool_forward_filters(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints, const std::vector<loot::Constraint> &merged_constraints, loot::lsm::BlockInstruction *main_block)
    {
        int pool_idx = 0;
        for (auto& pool : ltcl.loot_table.data["pools"])
        {
            PoolInstruction* pool_block = new PoolInstruction(pool_idx);
            FunctionInstruction* advance = new FunctionInstruction(FunctionType::FUNC_LCG_ADVANCE, {1});
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

    void add_loot_assertions(const loot::LootTableConstraintList &ltcl, const nlohmann::json& entry, const std::vector<loot::Constraint> &merged_constraints, loot::lsm::CaseInstruction *case_ins)
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
                        attribute_vector, Comparision::COMP_EQUAL, {static_cast<int>(constraint.count_range.min)}
                    );
                    case_ins->add_instruction(pool_assert_ins);
                    continue;
                }

                if (constraint.count_range.min != COUNT_NONE)
                {
                    // min count is bounded, need a '>=' assertion
                    PoolAssertFunctionInstruction* pool_assert_ins = new PoolAssertFunctionInstruction(
                        attribute_vector, Comparision::COMP_GE, {static_cast<int>(constraint.count_range.min)}
                    );
                    case_ins->add_instruction(pool_assert_ins);
                }
                if (constraint.count_range.max < COUNT_INFINITE)
                {
                    // max count is bounded, need a '<=' assertion
                    PoolAssertFunctionInstruction* pool_assert_ins = new PoolAssertFunctionInstruction(
                        attribute_vector, Comparision::COMP_LE, {static_cast<int>(constraint.count_range.max)}
                    );
                    case_ins->add_instruction(pool_assert_ins);
                }
            }
        }
    }
}}


