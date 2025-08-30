#include <vector>
#include <fstream>

#include "lootinator/lootinator.h"
#include "lootinator/lsm/parser.hpp"
#include "lootinator/lsm/instructions.hpp"

int main() {
	loot::lsm::Parser *p = new loot::lsm::Parser();
	loot::lsm::BlockInstruction block = loot::lsm::BlockInstruction();
	
	std::ifstream file("test.lsm");
	if (!file) {

	}
	std::vector<loot::lsm::Instruction *> instructions = p->parse_from_file(file);
	// loot::hello();
	return 0;
}
