#include <vector>
#include <fstream>

#include "lootinator/lootinator.h"
#include "lootinator/lsm/parser.hpp"
#include "lootinator/lsm/instructions.hpp"

/*
pool 0  {
	lcg-advance 1;
	roll 0  {
		case 0 {
			pool-assert 0 5 >= 2;
		}
	}
}
*/

int main() {	
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
