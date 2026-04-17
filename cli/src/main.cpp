#include <clipp.h>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"

int main(int argc, char** argv) {
	bool seedcracking = false;
	bool single_kernel = false;

	std::string loot_table = "";
	std::string constraint_file = "";
	std::string version_str = "";
	std::string output_file = "a.cu";

	auto cli = (clipp::required("--loot-table") & clipp::value("file_path.json", loot_table),
		clipp::required("--constraint-file") & clipp::value("file_path.json", constraint_file),
		clipp::required("-o", "--output") & clipp::value("filepath", output_file),
		clipp::option("-sc", "--seedcracking").set(seedcracking).doc("seedcracking mode"),
		clipp::option("-sk", "--single-kernel").set(single_kernel).doc("single kernel mode"),
		clipp::option("-v", "--version")
				.doc("the target minecraft version e.g. 1.16 or latest, currently latest is 26.1") &
			clipp::value("version", version_str));
	static_assert(mc::VersionRange::MC_LATEST == mc::VersionRange::MC_1_21_11_TO_26_1,
		"update the doc of version to reflect the latest version");

	if (!clipp::parse(argc, argv, cli)) {
		std::cout << clipp::make_man_page(cli, argv[0]);
		exit(1);
	}

	mc::VersionRange version = mc::parse_version(version_str);
	if (version == mc::MC_UNDEFINED) {
		fprintf(stderr, "Lootinator failed: version is undefined!\n");
		exit(1);
	}

	std::ofstream fout(output_file);

	std::string s;

	loot::LootinatorError err = loot::LootinatorError(loot::LootinatorErrorKind::SUCCESS);
	if (single_kernel) {
		err = loot::generate_best_pipeline_heur(
			loot_table, constraint_file, version, seedcracking, &s);
	} else {
		err =
			loot::generate_benchmark_source(loot_table, constraint_file, version, seedcracking, &s);
	}

	if (err.kind == loot::LootinatorErrorKind::SUCCESS) {
		fout << s;
		std::cout << "Cuda generation was successful.\n";
		std::cout << "==== Selected Options ====\n";
		std::cout << "  Version Range: " << mc::get_version_from_enum(version) << "\n";
		std::cout << "  Constraints:   " << constraint_file << "\n";
		std::cout << "  Seedcracking:  " << (seedcracking ? "true\n" : "false\n");
		std::cout << "  Single Kernel: " << (single_kernel ? "true\n" : "false\n");
		std::cout << "  Output File:   " << output_file << "\n";
		std::cout << "==========================\n";
	} else {
		std::cout << "Lootinator failed: " << err.message << '\n';
	}
}