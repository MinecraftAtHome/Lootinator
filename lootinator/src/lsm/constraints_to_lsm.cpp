#include "lootinator/lsm/constraints_to_lsm.hpp"
#include <iostream>

namespace loot { namespace lsm {
    std::vector<loot::lsm::BlockInstruction> get_lsm_representations(const LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints)
    {
        std::vector<loot::Constraint> merged_constraints;
        loot::merge_contraints(constraints, merged_constraints);
        std::cerr << "4.1\n";

        std::vector<loot::lsm::BlockInstruction> programs;
        for (auto& pool_filter : ltcl.available_filters) {
            loot::lsm::BlockInstruction main_block;
            add_filter_on(ltcl, pool_filter, main_block);
            add_pool_forward_filters(ltcl, constraints, merged_constraints, main_block);
            programs.push_back(main_block);
        }
        std::cerr << "4.2\n";

        return programs;
    }

    void loot::lsm::add_filter_on(const loot::LootTableConstraintList &ltcl, const loot::PoolFilter &pool_filter, loot::lsm::BlockInstruction &main_block)
    {
        // (big) TODO
    }

    void loot::lsm::add_pool_forward_filters(const loot::LootTableConstraintList &ltcl, const std::vector<loot::Constraint> &constraints, const std::vector<loot::Constraint> &merged_constraints, loot::lsm::BlockInstruction &main_block)
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

                entry_idx++;
            }

            pool_block->add_instruction(roll_block);
            main_block.add_instruction(pool_block);
            pool_idx++;
        }
    }
}}


