#include <clipp.h>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"

int main(int argc, char** argv) {
	std::ofstream fout("aaaaa.cu");
	std::string s;

	loot::LootinatorError err = loot::generate_best_pipeline_heur(
		"C:\\Users\\kludw\\Source\\Repos\\Lootinator\\examples\\bastion-treasure\\loot_table_26_1.json", //loot table
		"C:\\Users\\kludw\\Source\\Repos\\Lootinator\\examples\\bastion-treasure\\constraints\\three_swords.json", //constraint_file, 
		mc::MC_LATEST, //version, 
		false, //seedcracking, 
		&s,
		"loot_seeds.txt"
	);

	fout << s;
}