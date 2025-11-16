#include <algorithm>
#include <iostream>

#include "lootinator/lsm/passes/loot_asserts.hpp"

namespace lsm {
    void AssertionGroup::sort() {
        std::sort(this->assertions.begin(), this->assertions.end(), [](const lsm::Assertion &a, const lsm::Assertion &b){
            return a.lvalue_length < b.lvalue_length;
        });
    }
    
    void Assertion::debug() {
        util::DebugStruct(std::cout, "Assertion")
            .add("index", this->index)
            .add("comp", this->comp)
            .add("rvalue", this->rvalue)
            .finish();
        std::cout << "\n";
    }

    void AssertionGroup::debug() {
        for (auto &assert : this->assertions) {
            assert.debug();
        }
    }

    void PoolAsserts::debug() {
        for (auto &group : this->groups) {
            group.debug();
        }
    }
}