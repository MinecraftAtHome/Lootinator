#include "lootinator/assertions.h"
#include "lootinator/constraint/constraint.h"
#include "lootinator/utility/debug.h"
#include <algorithm>
#include <iostream>

static int test_json_parse() {
	try {
		std::vector<loot::Constraint> cons =
			loot::parse_constraints_from_json("../../lootinator/tests/data/constraints.json");

		std::vector<mc::ItemAttribute> attributes;
		attributes.push_back({1, 17});
		attributes.push_back({2, 21});

		loot::Constraint test_constraint = {10, {0, 15}, 1, attributes};
		util::debug(std::cerr, attributes);

		ASSERT_EQ(cons[0], test_constraint);
	} catch (const std::exception& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int LOOTINATOR_EXTERN tests_constraint_json_test(int argc, char** const argv) {
	(void)argc;
	(void)argv;

	if (test_json_parse())
		return 1;
	return 0;
}