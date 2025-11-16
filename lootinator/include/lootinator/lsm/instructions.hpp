#ifndef LOOTINATOR_LSM_INSTRUCTIONS_H
#define LOOTINATOR_LSM_INSTRUCTIONS_H

#include <vector>
#include "lootinator/constraint/filter.h"
#include "lootinator/lsm/passes/loot_functions.hpp"

namespace loot {
    namespace lsm {
        enum InstructionType {
            INS_REGULAR, INS_BLOCK, INS_VALUE, INS_POOL, INS_CASE, INS_ROLL, INS_FUNC
        };

        enum FunctionType {
            FUNC_ASSERT, FUNC_FAIL, FUNC_FILTER_ON, FUNC_LCG_ADVANCE, FUNC_FUNC
        };

        enum Comparision {
            COMP_EQUAL, COMP_GE
        };

        class Instruction {
            public:
                InstructionType tp;
                Instruction *as_ins();
                virtual void debug(int indent_level);
                virtual ~Instruction() {};
                // note: these need be implemented! right now uncommenting this will result in an error!
                virtual void compile_pass1(LootTableConstraintList &ltcl, std::vector<mc::LootFunctionData> &function_data);
                // virtual void compile_pass2();
                // virtual void compile_pass3();
        };

        class FunctionInstruction : public Instruction {
            public: 
                FunctionType func_tp;
                std::vector<int> args;
                FunctionInstruction(FunctionType func_tp);
                FunctionInstruction(FunctionType func_tp, std::vector<int> args);
                void debug(int indent_level) override;
                void compile_pass1(LootTableConstraintList &ltcl, std::vector<mc::LootFunctionData> &function_data) override;
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
                virtual ~BlockInstruction() override;
                void debug(int indent_level) override;
                void compile_pass1(LootTableConstraintList &ltcl, std::vector<mc::LootFunctionData> &function_data) override;
        };

        class PoolInstruction : public BlockInstruction {
            public:
                int id;
                PoolInstruction(int id);
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

#endif