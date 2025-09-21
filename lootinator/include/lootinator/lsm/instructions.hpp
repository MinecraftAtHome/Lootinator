#pragma once

#include <vector>

namespace loot {
    namespace lsm {
        enum InstructionType {
            INS_REGULAR, INS_BLOCK, INS_VALUE, INS_POOL, INS_CASE, INS_ROLL, INS_FUNC
        };

        enum FunctionType {
            FUNC_ASSERT, FUNC_FAIL,
        };

        class Instruction {
            public:
                InstructionType tp;
                Instruction *as_ins();
                void debug(int indent_level);
                virtual ~Instruction() {};
        };

        class FunctionInstruction : public Instruction {
            public: 
                FunctionType func_tp;
                std::vector<int> args;
                FunctionInstruction(FunctionType func_tp);
                FunctionInstruction(FunctionType func_tp, std::vector<int> &args);
        };

        class BlockInstruction : public Instruction {
            public:
                std::vector<Instruction *> children;
                BlockInstruction();
                void add_instruction(Instruction *ins);
                virtual ~BlockInstruction() {};
        };

        class PoolInstruction : public BlockInstruction {
            public:
                int id;
                PoolInstruction(int id);       
                virtual ~PoolInstruction() {};
        };

        class CaseInstruction : public BlockInstruction {
            public:
                int item;
                CaseInstruction(int item);
        };

        class RollInstruction : public BlockInstruction {
            public:
                int roll_count;
                RollInstruction(int roll_count);
        };
    }
}