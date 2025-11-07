#include <iostream>
#include <algorithm>
#include "lootinator/assertions.h"
#include "lootinator/utility/debug.h"
#include "lootinator/mc/loot_functions.hpp"

static void smoke_test()
{
    loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_21_TO_1_21_9);
    auto& entry = lt.data["pools"][0]["entries"][7];
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, entry, 0);
    ASSERT_NE(data.type, mc::LootFunctionType::IGNORED);
}

static void correctness_test_enchant_randomly_list()
{
    
}

static void correctness_test_enchant_randomly_natural()
{
    loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_21_TO_1_21_9);
    auto& entry = lt.data["pools"][0]["entries"][14];
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, entry, 0);
    ASSERT_EQ(data.type, mc::LootFunctionType::ENCHANT_RANDOMLY);

    std::vector<mc::Enchantment> target_order({
        mc::PROTECTION, mc::FIRE_PROTECTION, mc::BLAST_PROTECTION, mc::PROJECTILE_PROTECTION, 
        mc::RESPIRATION, mc::AQUA_AFFINITY, mc::THORNS, mc::UNBREAKING, mc::CURSE_OF_BINDING,
        mc::CURSE_OF_VANISHING, mc::MENDING
    });
    std::vector<int> target_shmem({4, 4, 4, 4, 3, 1, 3, 3, 1, 1, 1});

    ASSERT_EQ(std::equal(target_order.begin(), target_order.end(), data.enchant_randomly.enchantment_order.begin()), true);
    ASSERT_EQ(std::equal(target_shmem.begin(), target_shmem.end(), data.shared_mem.begin()), true);
}

static void correctness_test_enchant_with_levels()
{
    
}

int LOOTINATOR_EXTERN tests_loot_functions_test(int argc, char** const argv) {
    (void)argc; (void)argv;
    
    ASSERT_NOEXCEPT(smoke_test());
    ASSERT_NOEXCEPT(correctness_test_enchant_randomly_list());
    ASSERT_NOEXCEPT(correctness_test_enchant_randomly_natural());
    ASSERT_NOEXCEPT(correctness_test_enchant_with_levels());

    return 0;
}