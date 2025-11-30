#ifndef LOOTINATOR_LSM_INSTRUCTIONS_H
#define LOOTINATOR_LSM_INSTRUCTIONS_H

#include <vector>
#include "lootinator/constraint/filter.h"
#include "lootinator/lsm/passes/loot_functions.hpp"

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
            virtual void compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data);
            virtual void compile_pass2(void *data, int &num_assertions);
            // virtual void compile_pass3();
    };

    class FunctionInstruction : public Instruction {
        public: 
            FunctionType func_tp;
            std::vector<int> args;
            FunctionInstruction(FunctionType func_tp);
            FunctionInstruction(FunctionType func_tp, std::vector<int> args);
            void debug(int indent_level) override;
            void compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data) override;
    };

    class PoolAssertFunctionInstruction : public Instruction {
        public:
            std::vector<int> lvalues;// enchant id, enchant level?
            Comparision comp;
            std::vector<int> rvalues; // FIXME: not sure why this is a vector... it should just be one value
            PoolAssertFunctionInstruction(std::vector<int> lvalues, Comparision comp, std::vector<int> rvalues);
            void debug(int indent_level) override;
            void compile_pass2(void *data, int &num_assertions) override;
    };

    class BlockInstruction : public Instruction {
        public:
            std::vector<Instruction *> children;
            BlockInstruction();
            void add_instruction(Instruction *ins);
            virtual ~BlockInstruction() override;
            void debug(int indent_level) override;
            void compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data) override;
            void compile_pass2(void *data, int &num_assertions) override;
    };

    class PoolInstruction : public BlockInstruction {
        public:
            int id;
            PoolInstruction(int id);
            void compile_pass2(void *data, int &num_assertions) override;
    };

    class CaseInstruction : public BlockInstruction {
        public:
            int item;
            CaseInstruction(int item);
            void debug(int indent_level) override;
            void compile_pass2(void *data, int &num_assertions) override;
    };

    class RollInstruction : public BlockInstruction {
        public:
            int min_roll_count;
            int extra_roll_bound;
            RollInstruction(int min_roll_count, int extra_roll_bound);
            void debug(int indent_level) override;
    };
}

#endif