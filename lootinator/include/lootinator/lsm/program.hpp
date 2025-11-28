#ifndef LOOTINATOR_LSM_PROGRAM
#define LOOTINATOR_LSM_PROGRAM

#include "lootinator/lsm/passes/pass_info.hpp"
#include "lootinator/lsm/instructions.hpp"

namespace lsm {
    struct Program {
        lsm::BlockInstruction* main_block;
        lsm::PassInfo pass_info;

        Program(lsm::BlockInstruction *main_block, lsm::PassInfo pass_info) : pass_info(pass_info) {
            this->main_block = main_block;
        }
    };
}
#endif
