#include <iostream>
#include <algorithm>
#include "lootinator/assertions.h"
#include "lootinator/constraint/constraint.h"
#include "lootinator/constraint/filter.h"


int smoke_test() {
    try {
        loot::LootTable lt("../../lootinator/tests/ruined_portal.json");
        loot::debug(std::cerr, lt.item_names);
        loot::LootTableConstraintList ltcl(lt);
        std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../lootinator/tests/ruined_portal_constraints.json");
        ASSERT_EQ(ltcl.initialize_constraints(constr), true);
    } 
    catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 1;
	}
    return 0;
}

int LOOTINATOR_EXTERN tests_filter_test(int argc, char** const argv) {
    if (smoke_test()) return 1;
    return 0;
}
