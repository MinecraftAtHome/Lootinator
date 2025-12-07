#ifndef LOOTINATOR_LSM_LOOT_ASSERTS_H
#define LOOTINATOR_LSM_LOOT_ASSERTS_H

#include "lootinator/lsm/instructions.hpp"

namespace lsm {   
    /**
     * Imagine a piece of beautiful documentation here... (TODO) nope!
     */ 
    struct Assertion {
        int index;
        lsm::Comparision comp;
        int rvalue;
        size_t lvalue_length;
        lsm::PoolAssertFunctionInstruction *ref;
        void debug();
    };

    /**
     * Imagine a piece of beautiful documentation here... (TODO)
     */
    struct AssertionGroup {
        // stores indices of all pool asserts which can be influenced by the results of the filter-on call
        // since each assertion corresponds to 1 entry in the accumulator arrays, this will let the filter-on
        // code generator know which array entries it can write to.
        std::vector<int> filter_on_affected_assertions;

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
