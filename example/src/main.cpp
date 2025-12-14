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

#include "lootinator/data/loot_data.hpp"

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
	std::cout << "here0\n";
	std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/example_constraints.json");

    std::ifstream f("../../example/src/ruined_portal.json");
	std::cout << "here1\n";
	nlohmann::json loot_table_json = nlohmann::json::parse(f);
	std::cout << "here2\n";
	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../example/src/item_map.txt", mc::VersionRange::MC_1_16_TO_1_20); 
	std::cout << "here3\n";
}

int main_lsmtest() {	
	lsm::BlockInstruction program = lsm::BlockInstruction();
	lsm::PoolInstruction pool0 = lsm::PoolInstruction(0);
	lsm::RollInstruction roll_ins = lsm::RollInstruction(0, 0);	
	lsm::FunctionInstruction f = lsm::FunctionInstruction(lsm::FunctionType::FUNC_FAIL);
	lsm::FunctionInstruction advance = lsm::FunctionInstruction(lsm::FunctionType::FUNC_LCG_ADVANCE, {1});
	lsm::PoolAssertFunctionInstruction pool_assert0 = lsm::PoolAssertFunctionInstruction({0, 5}, lsm::Comparision::COMP_GE, {2});
	lsm::PoolAssertFunctionInstruction pool_assert1 = lsm::PoolAssertFunctionInstruction({1, 1}, lsm::Comparision::COMP_EQUAL, {3});

	lsm::CaseInstruction case0 = lsm::CaseInstruction(0);
	lsm::CaseInstruction case1 = lsm::CaseInstruction(1);

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
