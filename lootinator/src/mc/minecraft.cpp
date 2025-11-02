#include "lootinator/mc/minecraft.hpp"
#include "lootinator/utility/enum_bimap.hpp"

#include <iostream>
#include <vector>

namespace mc {
    std::unordered_map<mc::ItemType, std::vector<Enchantment>> ITEM_ENCHANTMENTS({
            {CHESTPLATE, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,THORNS,CURSE_OF_BINDING}},
            {HELMET, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,RESPIRATION,AQUA_AFFINITY,CURSE_OF_BINDING}},
            {LEGGINGS, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,CURSE_OF_BINDING,SWIFT_SNEAK}},
            {BOOTS, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,FEATHER_FALLING,DEPTH_STRIDER,FROST_WALKER,SOUL_SPEED,CURSE_OF_BINDING}},
            
            {SWORD, {SHARPNESS,SMITE,BANE_OF_ARTHROPODS,LOOTING,KNOCKBACK,FIRE_ASPECT,SWEEPING_EDGE}},
            {MACE, {DENSITY,BREACH,WIND_BURST,SMITE,BANE_OF_ARTHROPODS,FIRE_ASPECT}},
            
            {BOW, {POWER,PUNCH,FLAME,INFINITY_ENCHANTMENT}},
            {CROSSBOW, {PIERCING,MULTISHOT,QUICK_CHARGE}},
            {TRIDENT, {IMPALING,RIPTIDE,LOYALTY,CHANNELING}},
            
            {PICKAXE, {EFFICIENCY,SILK_TOUCH,FORTUNE}},
            {AXE, {EFFICIENCY,SILK_TOUCH,FORTUNE}},
            {SHOVEL, {EFFICIENCY,SILK_TOUCH,FORTUNE}},
            {HOE, {EFFICIENCY,SILK_TOUCH,FORTUNE}},
            {FISHING_ROD, {LUCK_OF_THE_SEA,LURE}}
    });

    std::unordered_map<Enchantment, uint32_t> ENCHANTMENT_MAX_LEVEL({
            {EFFICIENCY, 5}, {SHARPNESS, 5}, {SMITE, 5}, {BANE_OF_ARTHROPODS, 5}, {POWER, 5}, {IMPALING, 5}, {DENSITY, 5},
            {PROTECTION, 4}, {FIRE_PROTECTION, 4}, {PROJECTILE_PROTECTION, 4}, {BLAST_PROTECTION, 4}, 
            {FEATHER_FALLING, 4}, {BREACH, 4}, {PIERCING, 4},
            {FORTUNE, 3}, {LUCK_OF_THE_SEA, 3}, {LURE, 3}, {UNBREAKING, 3}, {LOYALTY, 3}, {THORNS, 3}, {WIND_BURST, 3}, {LOOTING, 3},
            {RESPIRATION, 3}, {QUICK_CHARGE, 3}, {RIPTIDE, 3}, {SWIFT_SNEAK, 3}, {DEPTH_STRIDER, 3}, {SOUL_SPEED, 3}, {SWEEPING_EDGE, 3},
            {FROST_WALKER, 2}, {PUNCH, 2}, {KNOCKBACK, 2}, {FIRE_ASPECT, 2},
            {MENDING, 1}, {CURSE_OF_BINDING, 1}, {CURSE_OF_VANISHING, 1}, {MULTISHOT, 1}, {SILK_TOUCH, 1}, 
            {INFINITY_ENCHANTMENT, 1}, {FLAME, 1}, {CHANNELING, 1}, {AQUA_AFFINITY, 1},
    });

    // enchantability data

    constexpr uint32_t BASE_ENCHANTABILITY = 1;
    constexpr uint32_t STONE_ENCHANTABILITY = 5;
    constexpr uint32_t TURTLE_ENCHANTABILITY = 9;
    constexpr uint32_t DIAMOND_ENCHANTABILITY = 10;
    constexpr uint32_t CHAINMAIL_ENCHANTABILITY = 12;
    constexpr uint32_t WOOD_ENCHANTABILITY = 15;
    constexpr uint32_t LEATHER_ENCHANTABILITY = 15;
    constexpr uint32_t NETHERITE_ENCHANTABILITY = 15;
    constexpr uint32_t MACE_ENCHANTABILITY = 15;

    constexpr uint32_t COPPER_ENCHANTABILITY_TOOLS = 13;
    constexpr uint32_t COPPER_ENCHANTABILITY_ARMOR = 8;
    constexpr uint32_t IRON_ENCHANTABILITY_TOOLS = 14;
    constexpr uint32_t IRON_ENCHANTABILITY_ARMOR = 9;
    constexpr uint32_t GOLD_ENCHANTABILITY_TOOLS = 22;
    constexpr uint32_t GOLD_ENCHANTABILITY_ARMOR = 25;
    
    std::unordered_map<std::string, uint32_t> ITEM_NAME_TO_ENCHANTABILITY({
        {"leather_chestplate", LEATHER_ENCHANTABILITY}, {"chainmail_chestplate", CHAINMAIL_ENCHANTABILITY},
        {"copper_chestplate", COPPER_ENCHANTABILITY_ARMOR}, {"iron_chestplate", IRON_ENCHANTABILITY_ARMOR}, {"golden_chestplate", GOLD_ENCHANTABILITY_ARMOR},
        {"diamond_chestplate", DIAMOND_ENCHANTABILITY}, {"netherite_chestplate", NETHERITE_ENCHANTABILITY},

        {"leather_helmet", LEATHER_ENCHANTABILITY}, {"chainmail_helmet", CHAINMAIL_ENCHANTABILITY},
        {"copper_helmet", COPPER_ENCHANTABILITY_ARMOR}, {"iron_helmet", IRON_ENCHANTABILITY_ARMOR}, {"golden_helmet", GOLD_ENCHANTABILITY_ARMOR},
        {"diamond_helmet", DIAMOND_ENCHANTABILITY}, {"netherite_helmet", NETHERITE_ENCHANTABILITY}, {"turtle_helmet", TURTLE_ENCHANTABILITY},

        {"leather_leggings", LEATHER_ENCHANTABILITY}, {"chainmail_leggings", CHAINMAIL_ENCHANTABILITY},
        {"copper_leggings", COPPER_ENCHANTABILITY_ARMOR}, {"iron_leggings", IRON_ENCHANTABILITY_ARMOR}, {"golden_leggings", GOLD_ENCHANTABILITY_ARMOR},
        {"diamond_leggings", DIAMOND_ENCHANTABILITY}, {"netherite_leggings", NETHERITE_ENCHANTABILITY},

        {"leather_boots", LEATHER_ENCHANTABILITY}, {"chainmail_boots", CHAINMAIL_ENCHANTABILITY},
        {"copper_boots", COPPER_ENCHANTABILITY_ARMOR}, {"iron_boots", IRON_ENCHANTABILITY_ARMOR}, {"golden_boots", GOLD_ENCHANTABILITY_ARMOR},
        {"diamond_boots", DIAMOND_ENCHANTABILITY}, {"netherite_boots", NETHERITE_ENCHANTABILITY},

        {"wooden_sword", WOOD_ENCHANTABILITY}, {"stone_sword", STONE_ENCHANTABILITY},
        {"copper_sword", COPPER_ENCHANTABILITY_TOOLS}, {"iron_sword", IRON_ENCHANTABILITY_TOOLS}, {"golden_sword", GOLD_ENCHANTABILITY_TOOLS},
        {"diamond_sword", DIAMOND_ENCHANTABILITY}, {"netherite_sword", NETHERITE_ENCHANTABILITY},

        {"mace", MACE_ENCHANTABILITY}, {"bow", BASE_ENCHANTABILITY}, {"crossbow", BASE_ENCHANTABILITY}, {"trident", BASE_ENCHANTABILITY},

        {"wooden_pickaxe", WOOD_ENCHANTABILITY}, {"stone_pickaxe", STONE_ENCHANTABILITY},
        {"copper_pickaxe", COPPER_ENCHANTABILITY_TOOLS}, {"iron_pickaxe", IRON_ENCHANTABILITY_TOOLS}, {"golden_pickaxe", GOLD_ENCHANTABILITY_TOOLS},
        {"diamond_pickaxe", DIAMOND_ENCHANTABILITY}, {"netherite_pickaxe", NETHERITE_ENCHANTABILITY},

        {"wooden_axe", WOOD_ENCHANTABILITY}, {"stone_axe", STONE_ENCHANTABILITY},
        {"copper_axe", COPPER_ENCHANTABILITY_TOOLS}, {"iron_axe", IRON_ENCHANTABILITY_TOOLS}, {"golden_axe", GOLD_ENCHANTABILITY_TOOLS},
        {"diamond_axe", DIAMOND_ENCHANTABILITY}, {"netherite_axe", NETHERITE_ENCHANTABILITY},

        {"wooden_shovel", WOOD_ENCHANTABILITY}, {"stone_shovel", STONE_ENCHANTABILITY},
        {"copper_shovel", COPPER_ENCHANTABILITY_TOOLS}, {"iron_shovel", IRON_ENCHANTABILITY_TOOLS}, {"golden_shovel", GOLD_ENCHANTABILITY_TOOLS},
        {"diamond_shovel", DIAMOND_ENCHANTABILITY}, {"netherite_shovel", NETHERITE_ENCHANTABILITY},

        {"wooden_hoe", WOOD_ENCHANTABILITY}, {"stone_hoe", STONE_ENCHANTABILITY},
        {"copper_hoe", COPPER_ENCHANTABILITY_TOOLS}, {"iron_hoe", IRON_ENCHANTABILITY_TOOLS}, {"golden_hoe", GOLD_ENCHANTABILITY_TOOLS},
        {"diamond_hoe", DIAMOND_ENCHANTABILITY}, {"netherite_hoe", NETHERITE_ENCHANTABILITY},

        {"fishing_rod", BASE_ENCHANTABILITY}, {"book", BASE_ENCHANTABILITY}, {"enchanted_book", BASE_ENCHANTABILITY}
    });

    // item name to enum translation (and reverse for debugging purposes)

    std::unordered_map<mc::ItemType, std::string> ITEM_TYPE_TO_NAME({
            {CHESTPLATE, "chestplate"}, {HELMET, "helmet"}, {LEGGINGS, "leggings"}, {BOOTS, "boots"},
            {SWORD, "sword"}, {MACE, "mace"}, {BOW, "bow"}, {CROSSBOW, "crossbow"}, {TRIDENT, "trident"},
            {PICKAXE, "pickaxe"}, {AXE, "axe"}, {SHOVEL, "shovel"}, {HOE, "hoe"},
            {FISHING_ROD, "fishing_rod"}, {BOOK, "book"}
    });
    std::unordered_map<std::string, mc::ItemType> ITEM_NAME_TO_ITEM_TYPE({
        {"chestplate", CHESTPLATE}, {"leather_chestplate", CHESTPLATE}, {"chainmail_chestplate", CHESTPLATE},
        {"copper_chestplate", CHESTPLATE}, {"iron_chestplate", CHESTPLATE}, {"golden_chestplate", CHESTPLATE},
        {"diamond_chestplate", CHESTPLATE}, {"netherite_chestplate", CHESTPLATE},

        {"helmet", HELMET}, {"leather_helmet", HELMET}, {"chainmail_helmet", HELMET},
        {"copper_helmet", HELMET}, {"iron_helmet", HELMET}, {"golden_helmet", HELMET},
        {"diamond_helmet", HELMET}, {"netherite_helmet", HELMET}, {"turtle_helmet", HELMET},

        {"leggings", LEGGINGS}, {"leather_leggings", LEGGINGS}, {"chainmail_leggings", LEGGINGS},
        {"copper_leggings", LEGGINGS}, {"iron_leggings", LEGGINGS}, {"golden_leggings", LEGGINGS},
        {"diamond_leggings", LEGGINGS}, {"netherite_leggings", LEGGINGS},

        {"boots", BOOTS}, {"leather_boots", BOOTS}, {"chainmail_boots", BOOTS},
        {"copper_boots", BOOTS}, {"iron_boots", BOOTS}, {"golden_boots", BOOTS},
        {"diamond_boots", BOOTS}, {"netherite_boots", BOOTS},

        {"sword", SWORD}, {"wooden_sword", SWORD}, {"stone_sword", SWORD},
        {"copper_sword", SWORD}, {"iron_sword", SWORD}, {"golden_sword", SWORD},
        {"diamond_sword", SWORD}, {"netherite_sword", SWORD},

        {"mace", MACE}, {"bow", BOW}, {"crossbow", CROSSBOW}, {"trident", TRIDENT},

        {"pickaxe", PICKAXE}, {"wooden_pickaxe", PICKAXE}, {"stone_pickaxe", PICKAXE},
        {"copper_pickaxe", PICKAXE}, {"iron_pickaxe", PICKAXE}, {"golden_pickaxe", PICKAXE},
        {"diamond_pickaxe", PICKAXE}, {"netherite_pickaxe", PICKAXE},

        {"axe", AXE}, {"wooden_axe", AXE}, {"stone_axe", AXE},
        {"copper_axe", AXE}, {"iron_axe", AXE}, {"golden_axe", AXE},
        {"diamond_axe", AXE}, {"netherite_axe", AXE},

        {"shovel", SHOVEL}, {"wooden_shovel", SHOVEL}, {"stone_shovel", SHOVEL},
        {"copper_shovel", SHOVEL}, {"iron_shovel", SHOVEL}, {"golden_shovel", SHOVEL},
        {"diamond_shovel", SHOVEL}, {"netherite_shovel", SHOVEL},

        {"hoe", HOE}, {"wooden_hoe", HOE}, {"stone_hoe", HOE},
        {"copper_hoe", HOE}, {"iron_hoe", HOE}, {"golden_hoe", HOE},
        {"diamond_hoe", HOE}, {"netherite_hoe", HOE},

        {"fishing_rod", FISHING_ROD}, {"book", BOOK}, {"enchanted_book", BOOK}
    });

    // bidirectional map for enchantment name <-> enum translation

    util::EnumToStringBimap<mc::Enchantment> ENCHANTMENT_TO_NAME({
        {EFFICIENCY, "efficiency"},
        {SHARPNESS, "sharpness"},
        {SMITE, "smite"},
        {BANE_OF_ARTHROPODS, "bane_of_arthropods"},
        {POWER, "power"},
        {IMPALING, "impaling"},
        {DENSITY, "density"},
        {PROTECTION, "protection"},
        {FIRE_PROTECTION, "fire_protection"},
        {PROJECTILE_PROTECTION, "projectile_protection"},
        {BLAST_PROTECTION, "blast_protection"},
        {FEATHER_FALLING, "feather_falling"},
        {BREACH, "breach"},
        {PIERCING, "piercing"},
        {FORTUNE, "fortune"},
        {LUCK_OF_THE_SEA, "luck_of_the_sea"},
        {LURE, "lure"},
        {UNBREAKING, "unbreaking"},
        {LOYALTY, "loyalty"},
        {THORNS, "thorns"},
        {WIND_BURST, "wind_burst"},
        {LOOTING, "looting"},
        {RESPIRATION, "respiration"},
        {QUICK_CHARGE, "quick_charge"},
        {RIPTIDE, "riptide"},
        {SWIFT_SNEAK, "swift_sneak"},
        {DEPTH_STRIDER, "depth_strider"},
        {SOUL_SPEED, "soul_speed"},
        {SWEEPING_EDGE, "sweeping_edge"},
        {FROST_WALKER, "frost_walker"},
        {PUNCH, "punch"},
        {KNOCKBACK, "knockback"},
        {FIRE_ASPECT, "fire_aspect"},
        {MENDING, "mending"},
        {CURSE_OF_BINDING, "curse_of_binding"},
        {CURSE_OF_VANISHING, "curse_of_vanishing"},
        {MULTISHOT, "multishot"},
        {SILK_TOUCH, "silk_touch"},
        {INFINITY_ENCHANTMENT, "infinity"},
        {FLAME, "flame"},
        {CHANNELING, "channeling"},
        {AQUA_AFFINITY, "aqua_affinity"},
        {NO_ENCHANTMENT, "no_enchantment"}
    });

    // public api

    /**
     * Returns whether the provided enchantment can be obtained for the given item type
     * in an enchanting table. When extra_enchants=true, the applicability is extended to
     * all anvil-placeable enchantments for the given item type (thorns for all armor 
     * pieces, sharpness & smite & bane of arth. for axes)
     */
    bool is_enchantment_applicable(mc::Enchantment enchantment, mc::ItemType item_type, bool extra_enchants) 
    {
        if (item_type == ItemType::NO_ITEM || enchantment == NO_ENCHANTMENT) {
            return false; // safeguard
        }
        if (item_type == ItemType::BOOK || enchantment == MENDING || enchantment == UNBREAKING || enchantment == CURSE_OF_VANISHING) {
            return true;
        }

        const auto& enchant_vec = ITEM_ENCHANTMENTS.at(item_type);
        for (const auto& ench : enchant_vec) {
            if (ench == enchantment) {
                return true;
            }
        }

        if (extra_enchants) {
            if (enchantment == THORNS && (item_type == HELMET || item_type == LEGGINGS || item_type == BOOTS)) {
                return true;
            }
            if (item_type == AXE && (enchantment == SHARPNESS || enchantment == SMITE || enchantment == BANE_OF_ARTHROPODS)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns the maximum level of the provided enchantment or 0 if the enchantment is invalid
     */
    uint32_t get_max_level(mc::Enchantment enchantment)
    {
        if (ENCHANTMENT_MAX_LEVEL.find(enchantment) != ENCHANTMENT_MAX_LEVEL.end())
        {
            return ENCHANTMENT_MAX_LEVEL.at(enchantment);
        }
        else
        {
            return 0;
        }
    }

    /**
     * Returns the enchantability for the provided item name or 0 if the item is invalid
     */
    uint32_t get_enchantability(const std::string& item_name)
    {
        if (ITEM_NAME_TO_ENCHANTABILITY.find(item_name) != ITEM_NAME_TO_ENCHANTABILITY.end())
        {
            return ITEM_NAME_TO_ENCHANTABILITY.at(item_name);
        }
        else
        {
            return 0;
        }
    }

    /**
     * Returns the order of enchantments for a provided version range (as a std::vector)
     */
    std::vector<int> get_enchantments_for_version(const mc::VersionRange version_range)
    {
        if (version_range == mc::VersionRange::MC_1_13)
        {
            return std::vector<int>({{ 
                PROTECTION, FIRE_PROTECTION, FEATHER_FALLING, BLAST_PROTECTION, PROJECTILE_PROTECTION,
                RESPIRATION, AQUA_AFFINITY, THORNS, DEPTH_STRIDER, FROST_WALKER, CURSE_OF_BINDING,
                SHARPNESS, SMITE, BANE_OF_ARTHROPODS, KNOCKBACK, FIRE_ASPECT, LOOTING, SWEEPING_EDGE,
                EFFICIENCY, SILK_TOUCH, UNBREAKING, FORTUNE, 
                POWER, PUNCH, FLAME, INFINITY_ENCHANTMENT,
                LUCK_OF_THE_SEA, LURE, LOYALTY, IMPALING, RIPTIDE, CHANNELING, 
                MENDING, CURSE_OF_VANISHING
            }});
        }
        else if (version_range == mc::VersionRange::MC_1_14_TO_1_15 || version_range == mc::VersionRange::MC_1_16_TO_1_20)
        {
            return std::vector<int>({{
                PROTECTION, FIRE_PROTECTION, FEATHER_FALLING, BLAST_PROTECTION, PROJECTILE_PROTECTION,
                RESPIRATION, AQUA_AFFINITY, THORNS, DEPTH_STRIDER, FROST_WALKER, CURSE_OF_BINDING,
                SHARPNESS, SMITE, BANE_OF_ARTHROPODS, KNOCKBACK, FIRE_ASPECT, LOOTING, SWEEPING_EDGE,
                EFFICIENCY, SILK_TOUCH, UNBREAKING, FORTUNE, 
                POWER, PUNCH, FLAME, INFINITY_ENCHANTMENT,
                LUCK_OF_THE_SEA, LURE, LOYALTY, IMPALING, RIPTIDE, CHANNELING, 
                MULTISHOT, QUICK_CHARGE, PIERCING, MENDING, CURSE_OF_VANISHING
            }});
        }
        else if (version_range == mc::VersionRange::MC_1_21_TO_1_21_9)
        {
            return std::vector<int>({{
                PROTECTION, FIRE_PROTECTION, FEATHER_FALLING, BLAST_PROTECTION, PROJECTILE_PROTECTION,
                RESPIRATION, AQUA_AFFINITY, THORNS, DEPTH_STRIDER, 
                SHARPNESS, SMITE, BANE_OF_ARTHROPODS, KNOCKBACK, FIRE_ASPECT, LOOTING, SWEEPING_EDGE,
                EFFICIENCY, SILK_TOUCH, UNBREAKING, FORTUNE, 
                POWER, PUNCH, FLAME, INFINITY_ENCHANTMENT,
                LUCK_OF_THE_SEA, LURE, LOYALTY, IMPALING, RIPTIDE, CHANNELING,
                MULTISHOT, QUICK_CHARGE, PIERCING, DENSITY, BREACH,
                CURSE_OF_BINDING, CURSE_OF_VANISHING, FROST_WALKER, MENDING
            }});
        }
        else
        {
            std::cerr << "minecraft.cpp: get_enchantments_for_version(): Bad version range: " << version_range << '\n';
            return std::vector<int>();
        }
    }

    // -----------------------------------------------------------------------------------

    std::string item_type_to_string(const mc::ItemType type)
    {
        if (ITEM_TYPE_TO_NAME.find(type) != ITEM_TYPE_TO_NAME.end())
        {
            return ITEM_TYPE_TO_NAME.at(type); // TODO
        }
        else 
        {
            return "lootinator::null";
        }
    }

    mc::ItemType string_to_item_type(const std::string &item_type_string)
    {
        if (ITEM_NAME_TO_ITEM_TYPE.find(item_type_string) != ITEM_NAME_TO_ITEM_TYPE.end()) 
        {
            return ITEM_NAME_TO_ITEM_TYPE.at(item_type_string); // TODO
        }
        else 
        {
            return mc::ItemType::NO_ITEM;
        }
    }

    std::string enchantment_to_string(const mc::Enchantment type)
    {
        if (ENCHANTMENT_TO_NAME.contains_enum(type))
        {
            return ENCHANTMENT_TO_NAME.lookup_enum(type); // TODO
        }
        else 
        {
            return "lootinator::null";
        }
    }

    mc::Enchantment string_to_enchantment(const std::string &enchantment_string)
    {
        if (ENCHANTMENT_TO_NAME.contains_string(enchantment_string)) 
        {
            return ENCHANTMENT_TO_NAME.lookup_string(enchantment_string); // TODO
        }
        else 
        {
            return mc::Enchantment::NO_ENCHANTMENT;
        }
    }
}