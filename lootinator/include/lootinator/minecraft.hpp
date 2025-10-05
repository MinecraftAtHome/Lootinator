#ifndef LOOTINATOR_MINECRAFT_H
#define LOOTINATOR_MINECRAFT_H

#include <unordered_map>
#include <string>
#include "lootinator/utility/range.h"

namespace loot {
    enum MCVersionRange {
        MC_1_13,
        MC_1_14_TO_1_15,
        MC_1_16_TO_1_20,
        MC_1_21_TO_1_21_9
    };

    enum Enchantment {
        NO_ENCHANTMENT,

        // armor
        PROTECTION,
        FIRE_PROTECTION,
        BLAST_PROTECTION,
        PROJECTILE_PROTECTION,
        RESPIRATION,
        AQUA_AFFINITY,
        THORNS,
        SWIFT_SNEAK,
        FEATHER_FALLING,
        DEPTH_STRIDER,
        FROST_WALKER,
        SOUL_SPEED,

        // swords
        SHARPNESS,
        SMITE,
        BANE_OF_ARTHROPODS,
        KNOCKBACK,
        FIRE_ASPECT,
        LOOTING,
        SWEEPING_EDGE,

        // tools
        EFFICIENCY,
        SILK_TOUCH,
        FORTUNE,

        // fishing rods
        LUCK_OF_THE_SEA,
        LURE,

        // bows
        POWER,
        PUNCH,
        FLAME,
        INFINITY_ENCHANTMENT,

        // crossbows
        QUICK_CHARGE,
        MULTISHOT,
        PIERCING,

        // tridents
        IMPALING,
        RIPTIDE,
        LOYALTY,
        CHANNELING,

        // maces
        DENSITY,
        BREACH,
        WIND_BURST,

        // general
        MENDING,
        UNBREAKING,
        CURSE_OF_VANISHING,
        CURSE_OF_BINDING
    };

    enum ItemType {
        NO_ITEM,
        HELMET,
        CHESTPLATE,
        LEGGINGS,
        BOOTS,
        SWORD,
        PICKAXE,
        SHOVEL,
        AXE,
        HOE,
        FISHING_ROD,
        BOW,
        CROSSBOW,
        TRIDENT,
        MACE,
        BOOK
    };

    std::unordered_map<ItemType, std::vector<Enchantment>> ITEM_ENCHANTMENTS({
            {CHESTPLATE, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,THORNS,CURSE_OF_BINDING}},
            {HELMET, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,RESPIRATION,AQUA_AFFINITY,CURSE_OF_BINDING}},
            {LEGGINGS, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,CURSE_OF_BINDING,SWIFT_SNEAK}},
            {BOOTS, {PROTECTION,FIRE_PROTECTION,BLAST_PROTECTION,PROJECTILE_PROTECTION,FEATHER_FALLING,DEPTH_STRIDER,FROST_WALKER,SOUL_SPEED,CURSE_OF_BINDING}},
            //TODO
    });

    std::unordered_map<Enchantment, RangeInclusive<uint32_t>> ENCHANTMENT_LEVEL_RANGES({
            //TODO
    });

    // ---------------------------------------------------------------------------------------
    // string -> enum & enum -> string lookups

    std::unordered_map<ItemType, std::string> ITEM_TYPE_TO_NAME({
            //TODO
    });
    std::unordered_map<std::string, ItemType> NAME_TO_ITEM_TYPE({
            //TODO
    });
    std::string item_type_to_string(const ItemType type);
    ItemType string_to_item_type(const std::string& item_type_string);

    std::unordered_map<Enchantment, std::string> ENCHANTMENT_TO_NAME({
            //TODO
    });
    std::unordered_map<std::string, Enchantment> NAME_TO_ENCHANTMENT({
            //TODO
    });
    std::string enchantment_to_string(const Enchantment type);
    Enchantment string_to_enchantment(const std::string& enchantment_string);
}


#endif