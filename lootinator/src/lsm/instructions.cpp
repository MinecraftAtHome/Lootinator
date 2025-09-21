#include "lootinator/lsm/instructions.hpp"

#include <iostream>
#include <vector>
#include <cassert>

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

        loot::lsm::FunctionInstruction::FunctionInstruction(FunctionType func_tp, std::vector<int> &args) {
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


#define TODO(str) std::cout << "TODO: " << str << "\n";
        void loot::lsm::Instruction::debug(int indent_level) {
            switch (this->tp) {
                case loot::lsm::INS_ROLL: {
                    loot::lsm::RollInstruction *roll_ins = dynamic_cast<loot::lsm::RollInstruction *>(this);
                    printf("%*s%s (%d)\n", indent_level, "", "ROLL START", roll_ins->roll_count);   
                    for (auto child : roll_ins->children) {
                        child->debug(indent_level + 3);
                    }
                    printf("%*s%s\n", indent_level, "", "ROLL END");          
                    break;
                }
                case INS_BLOCK: {
                    loot::lsm::BlockInstruction *block = dynamic_cast<loot::lsm::BlockInstruction *>(this);
                    printf("%*s%s\n", indent_level, "", "BLOCK START");   
                    for (auto child : block->children) {
                        child->debug(indent_level + 3);
                    }
                    printf("%*s%s\n", indent_level, "", "BLOCK END");        
                    break;
                } 
                case INS_VALUE: {
                    TODO("INS_VALUE");
                    break;
                }
                case INS_POOL: {
                    loot::lsm::PoolInstruction *pool_ins = dynamic_cast<loot::lsm::PoolInstruction *>(this);
                    printf("%*s%s\n", indent_level, "", "POOL START");   
                    for (auto child : pool_ins->children) {
                        child->debug(indent_level + 3);
                    }
                    printf("%*s%s\n", indent_level, "", "POOL END");                    
                    break;
                }
                case INS_CASE: {
                    TODO("INS_CASE");
                    break;
                }
                case INS_FUNC: {
                    // loot::lsm::FunctionInstruction *func_ins = dynamic_cast<loot::lsm::FunctionInstruction *>(this);
                    TODO("INS_CASE");
                    
                    break;
                }
                default: {
                    std::cout << this->tp << "\n";
                    assert(false && "instruction type is not accounted for");
                }
            }
        }   
    }
#undef TODO
}