__device__ __host__ bool range_exists(int a, int b, int N) {

}

__device__ __host__ bool check_seed_full(uint64_t loot_seed) {
    uint64_t rng = loot_seed;
    set_seed(&rng, rng);
    int rolls = next_int_bounded(&rng, 4, 8);

    int assertion_count[3] = {0};

    for (int r = 0; r < rolls; r++) {
        int index = next_int(&rng, 398);
        ItemType item = (ItemType)rp_table[index];
        switch (item) {
            case OBSIDIAN:
            case FLINT:
            case IRON_NUGGET: 
            case FLINT_AND_STEEL:
            case FIRE_CHARGE:
            case GOLDEN_APPLE: 
            case GOLD_NUGGET: 
            case GOLDEN_SWORD: 
            case GOLDEN_AXE: 
            case GOLDEN_HOE:
            case GOLDEN_SHOVEL:
            case GOLDEN_PICKAXE: {
                int enchant = next_int(&rng, 6);
                int level = shared_mem[OFFSET + enchant];
                if (level != 1) {
                    level = next_int(&rng, level);
                }
                assertion_count[0] += 1;
                if (enchant == 4) {
                    assertion_count[1] += 1;
                }
                if (enchant == 4 && level == 5) {
                    assertion_count[2] += 1;
                } 
                break;
            }
            case GOLDEN_BOOTS:
            case GOLDEN_HELMET:
            case GOLDEN_LEGGINGS:
            case GOLDEN_CHESTPLATE: 
            case GLISTERING_MELON_SLICE:
            case ENCHANTED_GOLDEN_APPLE:
            case GOLDEN_HORSE_ARMOR: 
            case CLOCK: 
            case BELL: 
            case LIGHT_WEIGHTED_PRESSURE_PLATE:
            case GOLD_INGOT:
            case GOLD_BLOCK:
            case GOLDEN_CARROT:
        }
        
        int max_consumed;
        if (!(assertion_count[2] >= 3)) {
            return;
        }
        max_consumed = assertion_count[2];
        if (!(assertion_count[1] - 3 >= 2)) {
            return;
        }
        max_consumed += assertion_count[1]; 
        if (!(assertion_count[0] - 5 >= 1 && 1 >= assertion_count[0] - max_consumed)) {
            return;
        }
    }
    // return (gapples >= 4 && chestplate); 
}