#include <clipp.h>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"

int main(int argc, char** argv) {
	bool seedcracking = false;
	bool single_kernel = false;
	bool list_versions = false;

	std::string loot_table = "";
	std::string constraint_file = "";
	std::string version_str = "";
	std::string output_file = "a.cu";
	std::string seed_output_file = "loot_seeds.txt";

	auto cli =
		((clipp::required("--loot-table") &
				 clipp::value("file_path.json", loot_table).if_missing([] {
					 std::cout << "Failed: you need to specify a loot table\n";
				 }),
			 clipp::required("--constraints") &
				 clipp::value("file_path.json", constraint_file).if_missing([] {
					 std::cout << "Failed: you need to specify a constraint file\n";
				 }),
			 clipp::option("-o", "--output") &
				 clipp::value("filepath.cu", output_file)
					 .doc("specify the output file for cuda, default is " + output_file),
			 clipp::option("-sc", "--seedcracking").set(seedcracking).doc("seedcracking mode"),
			 clipp::option("-sk", "--single-kernel").set(single_kernel).doc("single kernel mode"),
			 clipp::option("-so", "--seed-output") &
				 clipp::value("file_path.txt", seed_output_file)
					 .doc("specify the output file for seeds, default is " + seed_output_file),
			 clipp::required("-v", "--version")
					 .doc("the target Minecraft version e.g. 1.16 or latest, currently latest is "
						  "26.2") &
				 clipp::value("version", version_str).if_missing([] {
					 std::cout << "Failed: you need to specify a minecraft version\n";
				 }))

			| clipp::required("--list-versions")
				  .set(list_versions)
				  .doc("list supported Minecraft versions"));
	static_assert(mc::VersionRange::MC_LATEST == mc::VersionRange::MC_1_21_11_TO_26_2,
		"update the doc of version to reflect the latest version");

	if (!clipp::parse(argc, argv, cli)) {
		std::cout << clipp::make_man_page(cli, argv[0]);
		exit(1);
	}

	if (list_versions) {
		std::cout << "Supported Minecraft Versions:\n";
		for (const auto& v : mc::get_supported_versions()) {
			std::cout << v << "\n";
		}
		exit(0);
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
			loot_table, constraint_file, version, seedcracking, &s, seed_output_file);
	} else {
		err = loot::generate_benchmark_source(
			loot_table, constraint_file, version, seedcracking, &s, seed_output_file);
	}

	if (err.kind == loot::LootinatorErrorKind::SUCCESS) {
		fout << s;
		std::cout << "========= Results =========\n";
		std::cout << "  Output File:   " << output_file << "\n";
		std::cout << "  Version Range: " << mc::get_version_from_enum(version) << "\n";
		std::cout << "  Constraints:   " << constraint_file << "\n";
		std::cout << "  Seedcracking:  " << (seedcracking ? "true\n" : "false\n");
		std::cout << "  Single Kernel: " << (single_kernel ? "true\n" : "false\n");
		std::cout << "===========================\n";
	} else {
		std::cout << "Lootinator failed: " << err.message << '\n';
	}
}