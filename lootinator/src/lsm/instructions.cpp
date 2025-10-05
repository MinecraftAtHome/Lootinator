#include "lootinator/lsm/instructions.hpp"

#include <iostream>
#include <vector>
#include <cassert>
#include <numeric>
#include <string>
#include <map>

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

namespace loot {
    namespace lsm {
        loot::lsm::BlockInstruction::BlockInstruction() {
            this->tp = loot::lsm::InstructionType::INS_BLOCK;
        }

        loot::lsm::PoolInstruction::PoolInstruction(int id) {
            this->id = id;
            this->tp = loot::lsm::InstructionType::INS_POOL;       
        }

        loot::lsm::CaseInstruction::CaseInstruction(int item) {
            this->item = item;
            this->tp = loot::lsm::InstructionType::INS_CASE;       
        }

        loot::lsm::RollInstruction::RollInstruction(int roll_count) {
            this->roll_count = roll_count;
            this->tp = loot::lsm::InstructionType::INS_ROLL;
        }

        loot::lsm::FunctionInstruction::FunctionInstruction(FunctionType func_tp) {
            this->func_tp = func_tp;
            this->tp = loot::lsm::InstructionType::INS_FUNC;
        } 

        loot::lsm::PoolAssertFunctionInstruction::PoolAssertFunctionInstruction(std::vector<int> lvalues, Comparision comp, std::vector<int> rvalues) {
            this->lvalues = lvalues;
            this->comp = comp;
            this->rvalues = rvalues;
        }

        void loot::lsm::PoolAssertFunctionInstruction::debug(int indent_level) {
            std::map<Comparision, const char *> lookup = {
                {COMP_EQUAL, "=="}, {COMP_LE, "<="}, {COMP_GE, ">="}, {COMP_G, ">"}, {COMP_L, "<"}
            };
            printf("%*s%s %s %s %s\n", indent_level, "", "POOL ASSERT", join(this->lvalues, ",").c_str(), lookup[this->comp], join(this->rvalues, ",").c_str());
        }

        loot::lsm::FunctionInstruction::FunctionInstruction(FunctionType func_tp, std::vector<int> args) {
            this->func_tp = func_tp;
            this->args = args;
            this->tp = loot::lsm::InstructionType::INS_FUNC;
        }
                
        void loot::lsm::BlockInstruction::add_instruction(Instruction *ins) {
            this->children.push_back(ins);
        }

        loot::lsm::Instruction *loot::lsm::Instruction::as_ins() {
            return (Instruction *)this;
        }

        void loot::lsm::CaseInstruction::debug(int indent_level) {
            printf("%*s%s %d\n", indent_level, "", "CASE", this->item);   
            for (auto child : this->children) {
                child->debug(indent_level + 3);
            }
            printf("%*s%s\n", indent_level, "", "CASE END");     
        }
            
        void loot::lsm::BlockInstruction::debug(int indent_level) {
            printf("%*s%s\n", indent_level, "", "BLOCK START");   
            for (auto child : this->children) {
                child->debug(indent_level + 3);
            }
            printf("%*s%s\n", indent_level, "", "BLOCK END");        
        }

        void loot::lsm::RollInstruction::debug(int indent_level) {
            printf("%*s%s (%d)\n", indent_level, "", "ROLL START", this->roll_count);   
            for (auto child : this->children) {
                child->debug(indent_level + 3);
            }
            printf("%*s%s\n", indent_level, "", "ROLL END");          
        }

        #define TODO(str) std::cout << "TODO: " << str << "\n";
        void loot::lsm::FunctionInstruction::debug(int indent_level) {
            switch (this->func_tp) {
                case FUNC_FAIL: {
                    printf("%*s%s\n", indent_level, "", "FAIL");   
                    break;
                }
                case FUNC_LCG_ADVANCE: {
                    printf("%*s%s %d\n", indent_level, "", "LCG_ADVANCE", this->args[0]);   
                    break;
                }
                default: {
                    TODO("Function instruction not implemented yet!");
                    break;
                }
            }
        }

        void loot::lsm::Instruction::debug(int indent_level) {
            TODO("instruction not implemented yet!");
        }
    }
}
