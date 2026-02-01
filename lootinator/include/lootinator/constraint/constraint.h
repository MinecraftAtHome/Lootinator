#ifndef LOOTINATOR_CONSTRAINT_CONSTRAINT_H
#define LOOTINATOR_CONSTRAINT_CONSTRAINT_H

#include "lootinator/utility/range.h"
#include "lootinator/utility/debug.h"
#include "lootinator/mc/minecraft.hpp"

#include <cstdint>

namespace loot {
    constexpr int32_t SLOT_NONE = -1;
    constexpr uint32_t COUNT_NONE = 0;
    constexpr uint32_t COUNT_INFINITE = 10000;

    bool attributes_match(const std::vector<mc::ItemAttribute>& first, const std::vector<mc::ItemAttribute>& second);

    // stores loot constraints on individual slots of items
    struct Constraint {
        std::uint32_t item;
        util::RangeInclusive<std::uint32_t> count_range;
        std::int32_t slot_id; // contraints are shared by cracking and finding kernels, finding won't use this
        std::vector<mc::ItemAttribute> attributes; // TODO: make this not a vector...

        bool item_equal(const Constraint& other) const;

        bool operator==(const Constraint& other) const;
        bool operator!=(const Constraint& other) const;
        
        friend std::ostream& operator<<(std::ostream& os, const Constraint& constraint);
    };

    void merge_contraints(const std::vector<loot::Constraint>& src, std::vector<loot::Constraint>& dest);
    std::vector<loot::Constraint> parse_constraints_from_json(const char *filepath);
}

#endif
