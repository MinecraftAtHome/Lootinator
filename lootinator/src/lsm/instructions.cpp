#include <iostream>
#include <vector>
#include <cassert>
#include <numeric>
#include <string>
#include <map>

#include "lootinator/lsm/passes/loot_asserts.hpp"
#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "nlohmann/json.hpp"

template<typename T>
std::string join(const std::vector<T>& vec, const std::string& delimiter) {
    if (vec.empty()) {
        return "";
    }
    return std::accumulate(std::next(vec.begin()), vec.end(), std::to_string(vec[0]),
            [&](const std::string& a, const T& b) {
                return a + delimiter + std::to_string(b);
            });
}

namespace lsm {
    lsm::BlockInstruction::BlockInstruction() {
        this->tp = lsm::InstructionType::INS_BLOCK;
    }

    lsm::PoolInstruction::PoolInstruction(int id) {
        this->id = id;
        this->tp = lsm::InstructionType::INS_POOL;       
    }

    lsm::CaseInstruction::CaseInstruction(int item) {
        this->item = item;
        this->tp = lsm::InstructionType::INS_CASE;       
    }

    /**
     * @param min_roll_count The minimum number of generated loot rolls
     * @param extra_roll_bound The nextInt bound used for random roll count choice, or 0 if the roll is constant.
     */
    lsm::RollInstruction::RollInstruction(int min_roll_count, int extra_roll_bound) {
        this->min_roll_count = min_roll_count;
        this->extra_roll_bound = extra_roll_bound;
        this->tp = lsm::InstructionType::INS_ROLL;
    }

    lsm::FunctionInstruction::FunctionInstruction(FunctionType func_tp) {
        this->func_tp = func_tp;
        this->tp = lsm::InstructionType::INS_FUNC;
    } 

    lsm::PoolAssertFunctionInstruction::PoolAssertFunctionInstruction(std::vector<int> lvalues, Comparision comp, std::vector<int> rvalues) {
        this->lvalues = lvalues;
        this->comp = comp;
        this->rvalues = rvalues;
    }

    void lsm::PoolAssertFunctionInstruction::debug(int indent_level) {
        std::map<Comparision, const char *> lookup = {
            {COMP_EQUAL, "=="}, {COMP_GE, ">="}
        };
        printf("%*s%s %s %s %s\n", indent_level, "", "POOL ASSERT", join(this->lvalues, ",").c_str(), lookup[this->comp], join(this->rvalues, ",").c_str());
    }

    lsm::FunctionInstruction::FunctionInstruction(FunctionType func_tp, std::vector<int> args) {
        this->func_tp = func_tp;
        this->args = args;
        this->tp = lsm::InstructionType::INS_FUNC;
    }

            
    void lsm::BlockInstruction::add_instruction(Instruction *ins) {
        this->children.push_back(ins);
    }

    BlockInstruction::~BlockInstruction()
    {
        for (auto ins : children)
        {
            delete ins;
        }
    }

    lsm::Instruction *lsm::Instruction::as_ins()
    {
        return (Instruction *)this;
    }

    void lsm::CaseInstruction::debug(int indent_level) {
        printf("%*s%s %d\n", indent_level, "", "CASE", this->item);   
        for (auto child : this->children) {
            child->debug(indent_level + 3);
        }
        printf("%*s%s\n", indent_level, "", "CASE END");     
    }
        
    void lsm::BlockInstruction::debug(int indent_level) {
        printf("%*s%s\n", indent_level, "", "BLOCK START");   
        for (auto child : this->children) {
            child->debug(indent_level + 3);
        }
        printf("%*s%s\n", indent_level, "", "BLOCK END");        
    }

    void lsm::RollInstruction::debug(int indent_level) {
        printf("%*s%s (%d, %d)\n", indent_level, "", "ROLL START", this->min_roll_count, this->extra_roll_bound);   
        for (auto child : this->children) {
            child->debug(indent_level + 3);
        }
        printf("%*s%s\n", indent_level, "", "ROLL END");          
    }

    #define TODO(str) std::cout << "TODO: " << str << "\n";
    void lsm::FunctionInstruction::debug(int indent_level) {
        static const char* func_names[] = {
            "", "FAIL", "FUNC_FILTER_ON", "LCG_ADVANCE", "FUNC_FUNC"
        };

        printf("%*s%s ", indent_level, "", func_names[func_tp]);   
        for (auto &arg : this->args) {
            printf("%d ", arg);
        }
        puts("");
    }

    void lsm::Instruction::debug(int indent_level) {
        (void)indent_level;
        TODO("instruction not implemented yet!");
    }

    void lsm::Instruction::compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data) {
        
    }

    void lsm::Instruction::compile_pass2(void *data, int &num_assertions) {
        (void)data;
        (void)num_assertions;
    }

    void lsm::BlockInstruction::compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data) {
        for (auto &child : this->children) {
            child->compile_pass1(ltcl, function_data);
        }
    }   

    void lsm::BlockInstruction::compile_pass2(void *data, int &num_assertions) {
        for (auto &child : this->children) {
            child->compile_pass2(data, num_assertions); 
        }
    }

    void lsm::FunctionInstruction::compile_pass1(loot::LootTableConstraintList &ltcl, std::vector<lsm::LootFunctionData> &function_data) {
        if (this->func_tp != FUNC_FUNC) {
            return;
        }

        Function function_ref = {this->args[0], this->args[1], this->args[2]};

        lsm::LootFunctionData data = lsm::parse_loot_function_data(ltcl.loot_table, function_ref);           
        function_data.push_back(data);
    }   

    void lsm::PoolInstruction::compile_pass2(void *data, int &num_assertions) {
        std::vector<lsm::PoolAsserts> *pool_asserts = reinterpret_cast<std::vector<lsm::PoolAsserts> *>(data);
        num_assertions = 0;
        lsm::PoolAsserts pool_assert;
        for (auto &child : this->children) {
            child->compile_pass2(&pool_assert, num_assertions); 
        }
        pool_asserts->push_back(pool_assert);
    }

    static bool matches_filter_signature(std::vector<int> assertion_lvals, std::vector<int> filter_on_lvals) {
        size_t min_length = std::min(assertion_lvals.size(), filter_on_lvals.size());
        for (size_t i = 0; i < min_length; i++) {
            if (assertion_lvals[i] != filter_on_lvals[i]) {
                return false;
            }
        }
        return true;
    }

    void lsm::CaseInstruction::compile_pass2(void *data, int &num_assertions) {
        lsm::PoolAsserts *pool_assert = reinterpret_cast<lsm::PoolAsserts *>(data);
        lsm::AssertionGroup group;
        lsm::FunctionInstruction* filter_on_func = nullptr;

        for (auto &child : this->children) {
            lsm::FunctionInstruction* func_child = dynamic_cast<lsm::FunctionInstruction*>(child);
            if (func_child != nullptr && func_child->func_tp == FUNC_FILTER_ON) {
                filter_on_func = func_child;
            }
            child->compile_pass2(&group, num_assertions);
        }
        group.sort();
        pool_assert->groups.push_back(group);

        if (filter_on_func != nullptr) {
            int assertion_index = 0;
            for (auto &assertion : group.assertions) {
                if (matches_filter_signature(assertion.ref->lvalues, filter_on_func->args)) {
                    group.filter_on_affected_assertions.push_back(assertion_index);
                }
                assertion_index++;
            }
        }
    }

    void lsm::PoolAssertFunctionInstruction::compile_pass2(void *data, int &num_assertions) {
        lsm::AssertionGroup *group = reinterpret_cast<lsm::AssertionGroup *>(data);

        lsm::Assertion assertion = {num_assertions, this->comp, this->rvalues[0], this->lvalues.size(), this};
        num_assertions++;
        group->assertions.push_back(assertion);
    }
}
