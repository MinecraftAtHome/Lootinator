#ifndef LOOTINATOR_LOOTINATOR_H
#define LOOTINATOR_LOOTINATOR_H

#include "lootinator/mc/minecraft.hpp"

#include <string>
#include <vector>

namespace loot {
	/*
		error codes:
			0: success
			1: user constraints too weak
			2: user constraints not possible
			3: bad constraint file
			4: bad loot table
	*/

	enum LootinatorError {
		SUCCESS,
		USER_CONSTRAINT_TOO_WEAK,
		USER_CONSTRAINT_NOT_POSSIBLE,
		BAD_CONSTRAINT_FILE,
		BAD_LOOT_TABLE,
		INTERNAL_ERROR
	};

	LootinatorError generate_best_pipeline_heur(
		const std::string loot_table_filepath,
		const std::string constraint_filepath, 
		const mc::VersionRange version_range,
		const bool use_seedcracking_mode,
		std::string* result
	);
	
	LootinatorError generate_benchmark_source(
		const std::string loot_table_filepath,
		const std::string constraint_filepath, 
		const mc::VersionRange version_range,
		const bool use_seedcracking_mode,
		std::string* result
	);
	
	std::string parse_errno(LootinatorError error);
} // namespace loot

#endif
