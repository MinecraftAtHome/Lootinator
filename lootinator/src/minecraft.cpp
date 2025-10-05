#include "lootinator/minecraft.hpp"

namespace loot {
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