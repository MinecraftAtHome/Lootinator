Lootinator will be the most awesome sauceuom Loot Program ever for Minecraft - work in progress. 

- specifying seedfinding/cracking mode
- saying what we're primarily filtering on
- specifying loot pool

using loot_pool_1




filtering:
assuming picked an item to filter on:

filter-on item_type(minecraft:golden_chestplate) enchant_randomly_enchant_id(minecraft:thorns) enchant_randomly_enchant_level(3)
back N_MAX
forward-check 

--------------------------------------


using loot_pool_id
{
    filter-on item_type(minecraft:golden_apple) skip(1) item_type(minecraft:golden_apple)
    roll 6
    {
        count-items item_type(gold_nugget) item_type(iron_nugget)
        count-and-enchantment item_type(golden_sword)
    }
    assert item_type(golden_sword) enchant_randomly_enchantment_id(looting) >= enchant_randomly_enchantment_level(2) item_count(2)
}
using loot_pool_id_2
{
    roll natural
    {
        count-items item_type(lodestone)
    }
    assert item_type(lodestone) > item_count(2)
}

