import json
import sys


def read_loot_table(filepath: str) -> json:
    with open(filepath, 'r') as f:
        return json.load(f)
    

def get_unique_items(loot_table: json) -> set[str]:
    item_names = set[str]()
    for pool in loot_table["pools"]:
        for entry in pool["entries"]:
            if entry["type"] == "minecraft:item":
                item_names.add(entry["name"])
    return item_names


loot_table_filepath = sys.argv[1]
output_filepath = sys.argv[2]
with open(output_filepath, 'w') as fout:
    item_set = get_unique_items(read_loot_table(loot_table_filepath))
    sz = len(item_set)
    for ix,item in enumerate(item_set):
        fout.write(item)
        if ix != sz - 1:
            fout.write('\n')
