__device__ void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
	const u32 enchantability = array_pointer[0];
	const u32 minLevel = array_pointer[1];
	const u32 maxLevel = array_pointer[2];

	// calculate effective level
	i32 level = minLevel;
	if (minLevel != maxLevel) {
		level += nextInt(rand, maxLevel - minLevel + 1);
    }
	const i32 delta = enchantability / 4 + 1;
	level += 1 + nextInt(rand, delta) + nextInt(rand, delta);
	const float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0F) * 0.15F;
	level = floor( ((float)level + (float)level * amplifier) + 0.5F );

	i32 nGroups = array_pointer[3 + level];
	if (nGroups == 0) {
        return;
    }
	advance(rand); // select first ench

	while (nextInt(rand, 50) <= level) {
        nGroups--;
		if (nGroups == 0) {
            break;
        }
        advance(rand);
		level /= 2;
	}
}