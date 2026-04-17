#include <clipp.h>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"

<<<<<<< Updated upstream
=======
	/*
		Version scheme:
		major.minor
	*/

>>>>>>> Stashed changes
	int
	main(int argc, char** argv) {
	bool seedcracking = false;
	bool single_kernel = false;

	std::string loot_table = "";
	std::string constraint_file = "";
	std::string version_str = "";
	std::string output_file = "main.cu";

	auto cli = (clipp::required("--loot-table") & clipp::value("file_path.json", loot_table),
		clipp::required("--constraint-file") & clipp::value("file_path.json", constraint_file),
		clipp::required("-o", "--output") & clipp::value("filepath", output_file),
		clipp::option("-sc", "--seedcracking").set(seedcracking).doc("seedcracking mode"),
		clipp::option("-sk", "--single-kernel").set(single_kernel).doc("single kernel mode"),
		clipp::option("-v", "--version").doc("the target minecraft version") &
			clipp::value("version", version_str));

	if (!clipp::parse(argc, argv, cli)) {
		std::cout << clipp::make_man_page(cli, argv[0]);
		exit(1);
	}

	mc::VersionRange version = mc::parse_version(version_str);
	if (version == mc::MC_UNDEFINED) {
		fprintf(stderr, "version is undefined!\n");
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
<<<<<<< Updated upstream
=======
		std::cout << "==== Selected Options ====\n";
		std::cout << "	Version:       " << version << "\n";
		std::cout << "	Constraints:   " << constraint_file << "\n";
		std::cout << "	Seedcracking:  " << (seedcracking ? "true\n" : "false\n");
		std::cout << "	Single Kernel: " << (single_kernel ? "true\n" : "false\n");
		std::cout << "	Output File:   " << mc::get_version_from_enum(output_file) << "\n";
>>>>>>> Stashed changes
	} else {
		std::cout << "Lootinator failed: " << err.message << '\n';
	}
}