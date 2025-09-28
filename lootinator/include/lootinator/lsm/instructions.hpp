#pragma once

#include <vector>

namespace loot {
    namespace lsm {
        enum InstructionType {
            INS_REGULAR, INS_BLOCK, INS_VALUE, INS_POOL, INS_CASE, INS_ROLL, INS_FUNC
        };

        enum FunctionType {
            FUNC_ASSERT, FUNC_FAIL, FUNC_LCG_ADVANCE,
        };

        enum Comparision {
            COMP_EQUAL, COMP_LE, COMP_GE, COMP_G, COMP_L
        };

        class Instruction {
            public:
                InstructionType tp;
                Instruction *as_ins();
                virtual void debug(int indent_level);
                virtual ~Instruction() {};
        };

        class FunctionInstruction : public Instruction {
            public: 
                FunctionType func_tp;
                std::vector<int> args;
                FunctionInstruction(FunctionType func_tp);
                FunctionInstruction(FunctionType func_tp, std::vector<int> args);
                void debug(int indent_level) override;
        };

        class PoolAssertFunctionInstruction : public Instruction {
            public:
                std::vector<int> lvalues;
                Comparision comp;
                std::vector<int> rvalues;
                PoolAssertFunctionInstruction(std::vector<int> lvalues, Comparision comp, std::vector<int> rvalues);
                void debug(int indent_level) override;
        };

        class BlockInstruction : public Instruction {
            public:
                std::vector<Instruction *> children;
                BlockInstruction();
                void add_instruction(Instruction *ins);
                virtual ~BlockInstruction() {};
                void debug(int indent_level) override;
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
                void debug(int indent_level) override;
        };

        class RollInstruction : public BlockInstruction {
            public:
                int roll_count;
                RollInstruction(int roll_count);
                void debug(int indent_level) override;
            };
    }
}