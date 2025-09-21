#include <vector>
#include <fstream>

#include "lootinator/lootinator.h"
#include "lootinator/lsm/parser.hpp"
#include "lootinator/lsm/instructions.hpp"

/*
pool 0 
{
	roll 0 
	{
		fail;
	}
}
*/

int main() {
	// loot::lsm::Parser *p = new loot::lsm::Parser();
	// loot::lsm::BlockInstruction block = loot::lsm::BlockInstruction();
	
	// std::ifstream file("test.lsm");
	// if (!file) {

	// }
	// std::vector<loot::lsm::Instruction *> instructions = p->parse_from_file(file);
	// loot::hello();
	// std::vector<loot::lsm::Instruction *> instructions;
	// instructions.push_back();

	
	
	// printf("pool0 pointer: %p\n", i);
	
	loot::lsm::BlockInstruction program = loot::lsm::BlockInstruction();
	loot::lsm::PoolInstruction pool0 = loot::lsm::PoolInstruction(0);
	loot::lsm::RollInstruction roll_ins = loot::lsm::RollInstruction(0);	
	loot::lsm::FunctionInstruction f = loot::lsm::FunctionInstruction(loot::lsm::FunctionType::FUNC_FAIL);
	
	roll_ins.add_instruction(f.as_ins());
	pool0.add_instruction(roll_ins.as_ins());
	program.add_instruction(pool0.as_ins());

	program.debug(0); 

	return 0;
}
