#include "lootinator/lsm/instructions.hpp"

#include <iostream>
#include <vector>

namespace loot {
    namespace lsm {
        loot::lsm::PoolInstruction::PoolInstruction(int id) {
            this->id = id;
            this->tp = loot::lsm::InstructionType::INS_POOL;       
        }
    }
}