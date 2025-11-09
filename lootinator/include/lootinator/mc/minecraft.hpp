#ifndef LOOTINATOR_MINECRAFT_H
#define LOOTINATOR_MINECRAFT_H

#include <unordered_map>
#include <string>
#include <vector>

namespace mc {
    enum VersionRange {
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

    enum AttributeCategory {
        NO_ATTRIBUTE,
        ENCHANTMENT_ATTRIBUTE,
        DURABILITY_ATTRIBUTE,
        POTION_EFFECT_ATTRIBUTE,
    };
    
    // ---------------------------------------------------------------------------------------

    std::string strip_prefix(const std::string& str);

    bool is_treasure_enchantment(mc::Enchantment enchantment);
    int count_unique_groups(const std::vector<mc::Enchantment> &enchantments);
    bool is_enchantment_applicable(mc::Enchantment enchantment, mc::ItemType item_type, bool extra_enchants);
    bool is_enchantment_available_at_level(mc::Enchantment enchantment, int enchantment_level, int level);
    uint32_t get_max_level(mc::Enchantment enchantment);
    uint32_t get_enchantability(const std::string &item_name);

    std::vector<mc::Enchantment> get_enchantments_for_version(const mc::VersionRange version_range);

    std::string item_type_to_string(mc::ItemType type);
    mc::ItemType string_to_item_type(const std::string& item_type_string);
    std::string enchantment_to_string(mc::Enchantment type);
    mc::Enchantment string_to_enchantment(const std::string& enchantment_string);
}


#endif