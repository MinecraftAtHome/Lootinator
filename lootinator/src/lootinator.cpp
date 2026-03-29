#include "lootinator/lootinator.h"
#include "lootinator/kgen/pipeline_generator.hpp"

#include <iostream>
#include <sstream>

namespace loot {
	LootinatorError generate_best_pipeline_heur(
		const std::string loot_table_filepath,
		const std::string constraint_filepath, 
		const mc::VersionRange version_range,
		const bool use_seedcracking_mode,
		std::string* result
	) {
		try {
			kgen::KernelGenConfig kgen_config = kgen::KernelGenConfig(
				version_range,
				loot_table_filepath,
				constraint_filepath,
				use_seedcracking_mode
			);

			kgen::PipelineGenerator pipeline_gen(kgen_config);
			auto pipelines = pipeline_gen.add_state_prediction().add_bruteforce().build();

			std::stringstream ss;
			kgen::generate_runner_source(pipelines[0], ss);
			*result = ss.str();
		}
		catch(loot::LootinatorError err) {
			return err;
		}
		catch(std::runtime_error& err) {
			return loot::LootinatorError::INTERNAL_ERROR;
		}
		return loot::LootinatorError::SUCCESS;
	}

	LootinatorError generate_benchmark_source(
		const std::string loot_table_filepath,
		const std::string constraint_filepath, 
		const mc::VersionRange version_range,
		const bool use_seedcracking_mode,
		std::string* result
	) {
		try {
			kgen::KernelGenConfig kgen_config = kgen::KernelGenConfig(
				version_range,
				loot_table_filepath,
				constraint_filepath,
				use_seedcracking_mode
			);

			kgen::PipelineGenerator pipeline_gen(kgen_config);
			auto pipelines = pipeline_gen.add_state_prediction().add_bruteforce().build();

			std::stringstream ss;
			kgen::generate_benchmarker_source(pipelines, ss);
			*result = ss.str();
		}
		catch(loot::LootinatorError err) {
			return err;
		}
		catch(std::runtime_error& err) {
			return loot::LootinatorError::INTERNAL_ERROR;
		}
		return loot::LootinatorError::SUCCESS;
	}

	std::string parse_errno(LootinatorError error) {
		switch(error) {
		case SUCCESS:
			return "operation successful";
		case USER_CONSTRAINT_TOO_WEAK:
			return "the provided constraints are not rare enough";
		case USER_CONSTRAINT_NOT_POSSIBLE:
			return "the provided constraints are impossible to satisfy";
		case BAD_CONSTRAINT_FILE:
			return "invalid constraint file contents";
		case BAD_LOOT_TABLE:
			return "invalid loot table file contents";
		case INTERNAL_ERROR:
			return "an internal error occurred";
		default:
			return "an unknown error occurred";
		}
	}
}
