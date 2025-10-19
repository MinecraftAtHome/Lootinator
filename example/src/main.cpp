#include <vector>
#include <fstream>

#include "lootinator/lootinator.h"
#include "lootinator/loot_table.h"
#include "lootinator/utility/debug.h"
#include "lootinator/constraint/filter.h"
#include "lootinator/lsm/parser.hpp"
#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/lsm/cuda/lsm_to_cuda.hpp"

/*
loot table: ruined portal
constraints:
[
	{
		"item": 4,
		"range": {
            "min": 20,
            "max": 100000
        }
	}
]

pool 0  {
	lcg-advance 1;
	roll 0 {
		//...
		case 4 {
			pool-assert >= 20;
		}
		//...
	}
}
*/

int main() {
	try {
        loot::LootTable lt("../../example/src/ruined_portal.json", loot::MC_1_16_TO_1_20);

        loot::LootTableConstraintList ltcl(lt);
        std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/example_constraints.json");
		ltcl.initialize_constraints(constr);

		std::vector<loot::lsm::BlockInstruction*> programs = loot::lsm::get_lsm_representations(ltcl, constr);
		
		int prog_idx = 0;
		for (auto& prog : programs)
		{	
			lsm_to_cuda(ltcl, prog, "output_" + std::to_string(prog_idx));
			delete prog;
			prog_idx++;
		}
    } 
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 1;
	}
}

int main_lsmtest() {	
	loot::lsm::BlockInstruction program = loot::lsm::BlockInstruction();
	loot::lsm::PoolInstruction pool0 = loot::lsm::PoolInstruction(0);
	loot::lsm::RollInstruction roll_ins = loot::lsm::RollInstruction(0);	
	loot::lsm::FunctionInstruction f = loot::lsm::FunctionInstruction(loot::lsm::FunctionType::FUNC_FAIL);
	loot::lsm::FunctionInstruction advance = loot::lsm::FunctionInstruction(loot::lsm::FunctionType::FUNC_LCG_ADVANCE, {1});
	loot::lsm::PoolAssertFunctionInstruction pool_assert0 = loot::lsm::PoolAssertFunctionInstruction({0, 5}, loot::lsm::Comparision::COMP_GE, {2});
	loot::lsm::PoolAssertFunctionInstruction pool_assert1 = loot::lsm::PoolAssertFunctionInstruction({1, 1}, loot::lsm::Comparision::COMP_EQUAL, {3});

	loot::lsm::CaseInstruction case0 = loot::lsm::CaseInstruction(0);
	loot::lsm::CaseInstruction case1 = loot::lsm::CaseInstruction(1);

	case0.add_instruction(pool_assert0.as_ins());
	case1.add_instruction(pool_assert1.as_ins());

	roll_ins.add_instruction(case0.as_ins());
	roll_ins.add_instruction(case1.as_ins());

	pool0.add_instruction(advance.as_ins());
	pool0.add_instruction(roll_ins.as_ins());
	program.add_instruction(pool0.as_ins());

	program.debug(0); 
	return 0;
}
