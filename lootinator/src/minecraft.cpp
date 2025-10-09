#include "lootinator/minecraft.hpp"

namespace loot {
    std::unordered_map<ItemType, std::vector<Enchantment>> ITEM_ENCHANTMENTS({
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

    std::unordered_map<ItemType, std::string> ITEM_TYPE_TO_NAME({
            {CHESTPLATE, "chestplate"}, {HELMET, "helmet"}, {LEGGINGS, "leggings"}, {BOOTS, "boots"},
            {SWORD, "sword"}, {MACE, "mace"}, {BOW, "bow"}, {CROSSBOW, "crossbow"}, {TRIDENT, "trident"},
            {PICKAXE, "pickaxe"}, {AXE, "axe"}, {SHOVEL, "shovel"}, {HOE, "hoe"},
            {FISHING_ROD, "fishing_rod"}, {BOOK, "book"}
    });
    std::unordered_map<std::string, ItemType> ITEM_NAME_TO_ITEM_TYPE({
        {"chestplate", CHESTPLATE}, {"leather_chestplate", CHESTPLATE}, {"chainmail_chestplate", CHESTPLATE},
        {"copper_chestplate", CHESTPLATE}, {"iron_chestplate", CHESTPLATE}, {"golden_chestplate", CHESTPLATE},
        {"diamond_chestplate", CHESTPLATE}, {"netherite_chestplate", CHESTPLATE},

        {"helmet", HELMET}, {"leather_helmet", HELMET}, {"chainmail_helmet", HELMET},
        {"copper_helmet", HELMET}, {"iron_helmet", HELMET}, {"golden_helmet", HELMET},
        {"diamond_helmet", HELMET}, {"netherite_helmet", HELMET},

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
    std::unordered_map<std::string, uint32_t> ITEM_NAME_TO_ENCHANTABILITY({
        // TODO add enchantability for all the items
        {"leather_chestplate", CHESTPLATE}, {"chainmail_chestplate", CHESTPLATE},
        {"copper_chestplate", CHESTPLATE}, {"iron_chestplate", CHESTPLATE}, {"golden_chestplate", CHESTPLATE},
        {"diamond_chestplate", CHESTPLATE}, {"netherite_chestplate", CHESTPLATE},

        {"leather_helmet", HELMET}, {"chainmail_helmet", HELMET},
        {"copper_helmet", HELMET}, {"iron_helmet", HELMET}, {"golden_helmet", HELMET},
        {"diamond_helmet", HELMET}, {"netherite_helmet", HELMET},

        {"leather_leggings", LEGGINGS}, {"chainmail_leggings", LEGGINGS},
        {"copper_leggings", LEGGINGS}, {"iron_leggings", LEGGINGS}, {"golden_leggings", LEGGINGS},
        {"diamond_leggings", LEGGINGS}, {"netherite_leggings", LEGGINGS},

        {"leather_boots", BOOTS}, {"chainmail_boots", BOOTS},
        {"copper_boots", BOOTS}, {"iron_boots", BOOTS}, {"golden_boots", BOOTS},
        {"diamond_boots", BOOTS}, {"netherite_boots", BOOTS},

        {"wooden_sword", SWORD}, {"stone_sword", SWORD},
        {"copper_sword", SWORD}, {"iron_sword", SWORD}, {"golden_sword", SWORD},
        {"diamond_sword", SWORD}, {"netherite_sword", SWORD},

        {"mace", MACE}, {"bow", BOW}, {"crossbow", CROSSBOW}, {"trident", TRIDENT},

        {"wooden_pickaxe", PICKAXE}, {"stone_pickaxe", PICKAXE},
        {"copper_pickaxe", PICKAXE}, {"iron_pickaxe", PICKAXE}, {"golden_pickaxe", PICKAXE},
        {"diamond_pickaxe", PICKAXE}, {"netherite_pickaxe", PICKAXE},

        {"wooden_axe", AXE}, {"stone_axe", AXE},
        {"copper_axe", AXE}, {"iron_axe", AXE}, {"golden_axe", AXE},
        {"diamond_axe", AXE}, {"netherite_axe", AXE},

        {"wooden_shovel", SHOVEL}, {"stone_shovel", SHOVEL},
        {"copper_shovel", SHOVEL}, {"iron_shovel", SHOVEL}, {"golden_shovel", SHOVEL},
        {"diamond_shovel", SHOVEL}, {"netherite_shovel", SHOVEL},

        {"wooden_hoe", HOE}, {"stone_hoe", HOE},
        {"copper_hoe", HOE}, {"iron_hoe", HOE}, {"golden_hoe", HOE},
        {"diamond_hoe", HOE}, {"netherite_hoe", HOE},

        {"fishing_rod", FISHING_ROD}, {"book", 1}, {"enchanted_book", 1}
    });

    std::unordered_map<Enchantment, std::string> ENCHANTMENT_TO_NAME({
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

    std::unordered_map<std::string, Enchantment> NAME_TO_ENCHANTMENT({
        {"efficiency", EFFICIENCY},
        {"sharpness", SHARPNESS},
        {"smite", SMITE},
        {"bane_of_arthropods", BANE_OF_ARTHROPODS},
        {"power", POWER},
        {"impaling", IMPALING},
        {"density", DENSITY},
        {"protection", PROTECTION},
        {"fire_protection", FIRE_PROTECTION},
        {"projectile_protection", PROJECTILE_PROTECTION},
        {"blast_protection", BLAST_PROTECTION},
        {"feather_falling", FEATHER_FALLING},
        {"breach", BREACH},
        {"piercing", PIERCING},
        {"fortune", FORTUNE},
        {"luck_of_the_sea", LUCK_OF_THE_SEA},
        {"lure", LURE},
        {"unbreaking", UNBREAKING},
        {"loyalty", LOYALTY},
        {"thorns", THORNS},
        {"wind_burst", WIND_BURST},
        {"looting", LOOTING},
        {"respiration", RESPIRATION},
        {"quick_charge", QUICK_CHARGE},
        {"riptide", RIPTIDE},
        {"swift_sneak", SWIFT_SNEAK},
        {"depth_strider", DEPTH_STRIDER},
        {"soul_speed", SOUL_SPEED},
        {"sweeping_edge", SWEEPING_EDGE},
        {"frost_walker", FROST_WALKER},
        {"punch", PUNCH},
        {"knockback", KNOCKBACK},
        {"fire_aspect", FIRE_ASPECT},
        {"mending", MENDING},
        {"curse_of_binding", CURSE_OF_BINDING},
        {"curse_of_vanishing", CURSE_OF_VANISHING},
        {"multishot", MULTISHOT},
        {"silk_touch", SILK_TOUCH},
        {"infinity", INFINITY_ENCHANTMENT},
        {"flame", FLAME},
        {"channeling", CHANNELING},
        {"aqua_affinity", AQUA_AFFINITY}
    });

    // string -> enum & enum -> string lookups

    std::string loot::item_type_to_string(const ItemType type)
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

    ItemType loot::string_to_item_type(const std::string &item_type_string)
    {
        if (ITEM_NAME_TO_ITEM_TYPE.find(item_type_string) != ITEM_NAME_TO_ITEM_TYPE.end()) 
        {
            return ITEM_NAME_TO_ITEM_TYPE.at(item_type_string); // TODO
        }
        else 
        {
            return ItemType::NO_ITEM;
        }
    }

    std::string loot::enchantment_to_string(const Enchantment type)
    {
        if (ENCHANTMENT_TO_NAME.find(type) != ENCHANTMENT_TO_NAME.end())
        {
            return ENCHANTMENT_TO_NAME.at(type); // TODO
        }
        else 
        {
            return "lootinator::null";
        }
    }

    Enchantment loot::string_to_enchantment(const std::string &enchantment_string)
    {
        if (NAME_TO_ENCHANTMENT.find(enchantment_string) != NAME_TO_ENCHANTMENT.end()) 
        {
            return NAME_TO_ENCHANTMENT.at(enchantment_string); // TODO
        }
        else 
        {
            return Enchantment::NO_ENCHANTMENT;
        }
    }
}