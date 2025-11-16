#include <iostream>
#include <algorithm>
#include "lootinator/assertions.h"
#include "lootinator/utility/debug.h"
#include "lootinator/lsm/passes/loot_functions.hpp"
#include "lootinator/lsm/function_ref.hpp"

static void smoke_test()
{
    {
        loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_21_TO_1_21_9);
        loot::lsm::Function function_ref = {0, 7, 0};
        mc::LootFunctionData data = mc::parse_loot_function_data(lt, function_ref);
        ASSERT_NE(data.type, mc::LootFunctionType::IGNORED);
    }
    {
        loot::LootTable lt("../../lootinator/tests/data/end_city_1_21_8.json", mc::VersionRange::MC_1_21_TO_1_21_9);
        loot::lsm::Function function_ref = {0, 10, 0};
        mc::LootFunctionData data = mc::parse_loot_function_data(lt, function_ref);
        ASSERT_NE(data.type, mc::LootFunctionType::IGNORED);
    }
}

static void correctness_test_enchant_randomly_list()
{
    loot::LootTable lt("../../lootinator/tests/data/bastion_1_17_1.json", mc::VersionRange::MC_1_16_TO_1_20);
    loot::lsm::Function function_ref = {0, 10, 0};
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, function_ref);
    ASSERT_EQ(data.type, mc::LootFunctionType::ENCHANT_RANDOMLY);

    std::vector<mc::Enchantment> target_order({mc::SOUL_SPEED});
    std::vector<int> target_shmem({3});

    ASSERT_EQ(std::equal(target_order.begin(), target_order.end(), data.enchant_randomly.enchantment_order.begin()), true);
    ASSERT_EQ(std::equal(target_shmem.begin(), target_shmem.end(), data.shared_mem.begin()), true);
}

static void correctness_test_enchant_randomly_natural()
{
    loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_21_TO_1_21_9);
    loot::lsm::Function function_ref = {0, 14, 0};    
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, function_ref);
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
    loot::LootTable lt("../../lootinator/tests/data/end_city_1_21_8.json", mc::VersionRange::MC_1_21_TO_1_21_9);
    loot::lsm::Function function_ref = {0, 10, 0};        
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, function_ref);
    ASSERT_EQ(data.type, mc::LootFunctionType::ENCHANT_WITH_LEVELS);

    const int target_minlevel = 20, target_maxlevel = 39;
    ASSERT_EQ(target_minlevel, data.enchant_with_levels.min_level);
    ASSERT_EQ(target_maxlevel, data.enchant_with_levels.max_level);

    const int target_emin = static_cast<int>(std::floor((target_minlevel + 1) * 0.85f));
    const int target_emax = static_cast<int>(std::ceil((target_maxlevel + 1 + 2*(10/4)) * 1.15f));
    ASSERT_EQ(target_emin, data.enchant_with_levels.min_effective_level);
    ASSERT_EQ(target_emax, data.enchant_with_levels.max_effective_level);

    std::cerr << "levels: (min, max, emin, emax): (" << target_minlevel << ", " << target_maxlevel << ", "
              << target_emin << ", " << target_emax << ")\n";

    std::vector<int> target_shmem({ 
        0, 1, 1, 1, 1, 3, 3, 3, 3, 3, 
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
        6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 2 
    });

    std::cerr << "Got:\n";
    util::debug(std::cerr, data.shared_mem) << std::endl;
    std::cerr << "Expected:\n";
    util::debug(std::cerr, target_shmem) << std::endl;

    ASSERT_EQ(std::equal(target_shmem.begin(), target_shmem.end(), data.shared_mem.begin()), true);
}

int LOOTINATOR_EXTERN tests_loot_functions_test(int argc, char** const argv) {
    (void)argc; (void)argv;
    
    ASSERT_NOEXCEPT(smoke_test());
    ASSERT_NOEXCEPT(correctness_test_enchant_randomly_list());
    ASSERT_NOEXCEPT(correctness_test_enchant_randomly_natural());
    ASSERT_NOEXCEPT(correctness_test_enchant_with_levels());

    return 0;
}