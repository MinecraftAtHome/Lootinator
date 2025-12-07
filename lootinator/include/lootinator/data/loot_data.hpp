#ifndef LOOTINATOR_DATA_LOOT_DATA
#define LOOTINATOR_DATA_LOOT_DATA

#include "lootinator/utility/range.h"
#include "lootinator/constraint/constraint.h"
#include "lootinator/mc/minecraft.hpp"

#include "nlohmann/json.hpp"

namespace data {
    class LootTreeNode {
        public:
        LootTreeNode *parent;
        std::vector<LootTreeNode *> children;
        std::vector<loot::Constraint> constraints;

        mc::VersionRange get_version();
    };

    class LootEntry : public LootTreeNode {
        public:
        int item;
        mc::ItemType type;
        std::string name;
        int weight;

        LootEntry(LootTreeNode *parent, const nlohmann::json &json);
    };

    class LootPool : public LootTreeNode {
        public:
        util::RangeInclusive<std::uint32_t> rolls;
        std::vector<int> entry_lookup;
    
        LootPool(LootTreeNode *parent, const nlohmann::json &json);
    };

    class LootTableRoot : public LootTreeNode {
        public:
        std::string name;
        mc::VersionRange version;

	    LootTableRoot(const nlohmann::json &json, mc::VersionRange, std::vector<loot::Constraint> &constraints); 
        ~LootTableRoot(); 
    };

// LOOT FUNCTION DATA

    enum LootFunctionType {
        ENCHANT_RANDOMLY,
        ENCHANT_WITH_LEVELS,
        SET_COUNT,
        APPLY_DAMAGE,
        IGNORED
    };

    struct EnchantRandomlyData {
        std::vector<mc::Enchantment> enchantment_order;
    };

    struct EnchantWithLevelsData {
        util::RangeInclusive<std::uint32_t> level;
        util::RangeInclusive<std::uint32_t> effective_level; // TODO: revist
        // not storing enchantments, function output is skipped
    };

    class LootFunctionData : public LootTreeNode {
        public:
        std::vector<int> shared_mem;
        data::LootFunctionType type;

        // FIXME ideally use union
        EnchantRandomlyData enchant_randomly;
        EnchantWithLevelsData enchant_with_levels;
        util::RangeInclusive<std::uint32_t> set_count;

        LootFunctionData(LootTreeNode *parent, const nlohmann::json &json);
    };
}

#endif //LOOTINATOR_DATA_LOOT_DATA