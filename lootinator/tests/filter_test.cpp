#include <iostream>
#include <algorithm>
#include "lootinator/assertions.h"
#include "lootinator/constraint/constraint.h"
#include "lootinator/constraint/filter.h"


int smoke_test() {
    try {
        loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_16_TO_1_20);
        util::debug(std::cerr, lt.item_names);
        loot::LootTableConstraintList ltcl(lt);
        std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../lootinator/tests/data/ruined_portal_constraints.json");
        ASSERT_EQ(ltcl.initialize_constraints(constr), true);
    } 
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 1;
	}
    return 0;
}

int correctness_test() {
    try {
        loot::LootTable lt("../../lootinator/tests/data/ruined_portal.json", mc::VersionRange::MC_1_16_TO_1_20);
        util::debug(std::cerr, lt.item_names);
        loot::LootTableConstraintList ltcl(lt);
        std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../lootinator/tests/data/ruined_portal_constraints.json");
        ASSERT_EQ(ltcl.initialize_constraints(constr), true);
        std::cerr << "\n\n";

        for (auto& filter : ltcl.available_filters) {
            util::DebugStruct(std::cerr, "PoolFilter")
                .add("attribute.type", filter.attribute.type)
                .add("attribute.min_level", filter.attribute.level_range.min)
                .add("attribute.max_level", filter.attribute.level_range.max)
                .add("reversal_type", filter.reversal_type)
                .add("pool_idx", filter.pool_idx)
                .add("entry_idx", filter.entry_idx)
                .add("entry_count", filter.entry_count)
                .add("reversal_type", filter.filter_score)
                .finish();
            std::cerr << "\n";
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 1;
	}
    return 0;
}

int LOOTINATOR_EXTERN tests_filter_test(int argc, char** const argv) {
    (void)argc; (void)argv;
    
    if (smoke_test()) return 1;
    if (correctness_test()) return 1;
    return 0;
}
