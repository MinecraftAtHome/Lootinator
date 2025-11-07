#include <iostream>
#include <algorithm>
#include "lootinator/assertions.h"
#include "lootinator/utility/debug.h"
#include "lootinator/mc/loot_functions.hpp"

static void smoke_test()
{
    loot::LootTable lt("../../lootinator/tests/ruined_portal.json", mc::VersionRange::MC_1_21_TO_1_21_9);
    auto& entry = lt.data["pools"][0]["entries"][7];
    mc::LootFunctionData data = mc::parse_loot_function_data(lt, entry, 0);
    ASSERT_NE(data.type, mc::LootFunctionType::IGNORED);
}

static void correctness_test_enchant_randomly_list()
{
    
}

static void correctness_test_enchant_randomly_natural()
{
    
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