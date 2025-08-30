#pragma once

#include <vector>

namespace loot {
    namespace lsm {
        enum InstructionType {
            INS_REGULAR, INS_BLOCK, INS_VALUE, INS_POOL
        };

        class Instruction {
            public:
                InstructionType tp;
        };

        class BlockInstruction : public Instruction {
            public:
                std::vector<Instruction> children;
        };

        class PoolInstruction : public BlockInstruction {
            public:
                int id;
                PoolInstruction(int id);       
        };
    }
}