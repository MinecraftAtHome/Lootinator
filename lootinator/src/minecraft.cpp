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
            
            {PICKAXE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
            {AXE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
            {SHOVEL, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
            {HOE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
            {FISHING_ROD, {LUCK_OF_THE_SEA, LURE}}
    });

    std::unordered_map<Enchantment, uint32_t> ENCHANTMENT_MAX_LEVEL({
            {EFFICIENCY, {5}}, {SHARPNESS, {5}}, {SMITE, {5}}, {BANE_OF_ARTHROPODS, {5}}, {POWER, {5}}, {IMPALING, {5}}, {DENSITY, {5}},
            {PROTECTION, {4}}, {FIRE_PROTECTION, {4}}, {PROJECTILE_PROTECTION, {4}}, {BLAST_PROTECTION, {4}}, 
            {FEATHER_FALLING, {4}}, {BREACH, {4}}, {PIERCING, {4}},
            {FORTUNE, {3}}, {LUCK_OF_THE_SEA, {3}}, {LURE, {3}}, {UNBREAKING, {3}}, {LOYALTY, {3}}, {THORNS, {3}}, {WIND_BURST, {3}}, {LOOTING, {3}},
            {RESPIRATION, {3}}, {QUICK_CHARGE, {3}}, {RIPTIDE, {3}}, {SWIFT_SNEAK, {3}}, {DEPTH_STRIDER, {3}}, {SOUL_SPEED, {3}}, {SWEEPING_EDGE, {3}},
            {FROST_WALKER, {2}}, {PUNCH, {2}}, {KNOCKBACK, {2}}, {FIRE_ASPECT, {2}},
            {MENDING, {1}}, {CURSE_OF_BINDING, {1}}, {CURSE_OF_VANISHING, {1}}, {MULTISHOT, {1}}, {SILK_TOUCH, {1}}, 
            {INFINITY_ENCHANTMENT, {1}}, {FLAME, {1}}, {CHANNELING, {1}}, {AQUA_AFFINITY, {1}},
    });

    std::unordered_map<ItemType, std::string> ITEM_TYPE_TO_NAME({
            //TODO
    });
    std::unordered_map<std::string, ItemType> NAME_TO_ITEM_TYPE({
            //TODO
    });

    std::unordered_map<Enchantment, std::string> ENCHANTMENT_TO_NAME({
            //TODO
    });
    std::unordered_map<std::string, Enchantment> NAME_TO_ENCHANTMENT({
            //TODO
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
        if (NAME_TO_ITEM_TYPE.find(item_type_string) != NAME_TO_ITEM_TYPE.end()) 
        {
            return NAME_TO_ITEM_TYPE.at(item_type_string); // TODO
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