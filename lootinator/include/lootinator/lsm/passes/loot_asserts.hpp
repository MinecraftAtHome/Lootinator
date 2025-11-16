#ifndef LOOTINATOR_LSM_LOOT_ASSERTS_H
#define LOOTINATOR_LSM_LOOT_ASSERTS_H

#include "lootinator/lsm/instructions.hpp"

namespace lsm {    
    struct Assertion {
        int index;
        loot::lsm::Comparision comp;
        int rvalue;
        size_t lvalue_length;
        loot::lsm::PoolAssertFunctionInstruction *ref;
        void debug();
    };

    struct AssertionGroup {
        std::vector<Assertion> assertions;
        void sort();
        void debug();
    };

    struct PoolAsserts {
        std::vector<AssertionGroup> groups;
        void debug();
    };
}

#endif
