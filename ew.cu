#include <stdint.h>
#include <stdio.h>

//@SharedDefinitionsStart
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

inline void setSeed(u64* rand, u64 value) {
	*rand = (value ^ JRAND_MULTIPLIER) & MASK_48;
}
inline i32 next(u64* rand, const i32 bits) {
	*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
	return (i32)((i64)*rand >> (48 - bits));
}
inline i32 nextInt(u64* rand, const i32 n) {
	if ((n - 1 & n) == 0) {
		u64 x = n * (u64)next(rand, 31);
		return (i32)((i64)x >> 31);
	} else {
		return (i32)(next(rand, 31) % n);
	}
}
inline float nextFloat(u64* rand) {
	return next(rand, 24) / (float)(1 << 24);
};
inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {
	if (min >= max) {
		return min;
	}
	return nextInt(rand, max - min + 1) + min;
}
inline i32 nextIntNoAdvance(u64* rand, const i32 n) {
	if ((n - 1 & n) == 0) {
		u64 x = n * *rand;
		return (i32)((i64)x >> 31);
	} else {
		return (i32)(*rand % n);
	}
}

void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
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
	level = floor(((float)level + (float)level * amplifier) + 0.5F);

	u32 nGroups = array_pointer[3 + level];
	if (nGroups == 0) {
		return;
	}
	*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;

	while (nextInt(rand, 50) <= level) {
		nGroups--;
		if (nGroups == 0) {
			break;
		}
		*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
		level /= 2;
	}
}

bool forward_filter_full(u64 loot_seed, u32 data[]) {
{
    i32 local_constraints[2] = {0};
    i32 rolls = nextIntBounded(&loot_seed, 5, 10);
    for (i32 roll = 0; roll < rolls; roll++) {
        int item = data[0 + nextInt(&loot_seed, 86)];
        switch (item) {
            case 0: { // minecraft:enchanted_golden_apple
                i32 item_count = 1;
                item_count = 1 + nextInt(&loot_seed, 2);
                local_constraints[0] += item_count;
                break;
            }
            case 1: { // minecraft:music_disc_otherside
                break;
            }
            case 2: { // minecraft:compass
                break;
            }
            case 3: { // minecraft:sculk_catalyst
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 4: { // minecraft:name_tag
                break;
            }
            case 5: { // minecraft:diamond_hoe
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                // ENCHANT WITH LEVELS >:)
                enchant_with_levels_function(&loot_seed, &(data[166]));
                break;
            }
            case 6: { // minecraft:lead
                break;
            }
            case 7: { // minecraft:diamond_horse_armor
                break;
            }
            case 8: { // minecraft:leather
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 9: { // minecraft:music_disc_13
                break;
            }
            case 10: { // minecraft:music_disc_cat
                break;
            }
            case 11: { // minecraft:diamond_leggings
                // ENCHANT WITH LEVELS >:)
                enchant_with_levels_function(&loot_seed, &(data[234]));
                break;
            }
            case 12: { // minecraft:book
                u32 eid = nextInt(&loot_seed, 1);
                bool r = (0b1 & (1 << eid));
                uint64_t m = !r - 1;
                loot_seed = (loot_seed * (1 | (25214903917 & m)) + (11 & m)) & MASK_48;
                break;
            }
            case 13: { // minecraft:sculk
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 14: { // minecraft:sculk_sensor
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 15: { // minecraft:candle
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 16: { // minecraft:amethyst_shard
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 17: { // minecraft:experience_bottle
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 18: { // minecraft:glow_berries
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 19: { // minecraft:iron_leggings
                // ENCHANT WITH LEVELS >:)
                enchant_with_levels_function(&loot_seed, &(data[303]));
                break;
            }
            case 20: { // minecraft:echo_shard
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 21: { // minecraft:disc_fragment_5
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 22: { // minecraft:potion
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 23: { // minecraft:book
                i32 item_count = 1;
                i32 enchantment = nextInt(&loot_seed, 39);
                u32 max_level = data[358 + enchantment];
                i32 level = nextIntBounded(&loot_seed, 1, max_level);
                if (enchantment == 14 && level == 3) {
                    local_constraints[1] += item_count;
                }
                break;
            }
            case 24: { // minecraft:book
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 25: { // minecraft:bone
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 26: { // minecraft:soul_torch
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            case 27: { // minecraft:coal
                loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
                break;
            }
            default: {
                return false;
            }
        }
    }
    if (!(local_constraints[0] >= 7))
        return false;
    if (!(local_constraints[1] == 1))
        return false;
}
return true;
}

int main() {    
uint32_t shmem[] = {0,
			1,
			2,
			2,
			3,
			3,
			4,
			4,
			5,
			5,
			6,
			6,
			7,
			7,
			8,
			8,
			9,
			9,
			10,
			10,
			11,
			11,
			12,
			12,
			12,
			13,
			13,
			13,
			14,
			14,
			14,
			15,
			15,
			15,
			16,
			16,
			16,
			17,
			17,
			17,
			18,
			18,
			18,
			19,
			19,
			19,
			20,
			20,
			20,
			20,
			21,
			21,
			21,
			21,
			22,
			22,
			22,
			22,
			22,
			23,
			23,
			23,
			23,
			23,
			24,
			24,
			24,
			24,
			24,
			25,
			25,
			25,
			25,
			25,
			26,
			26,
			26,
			26,
			26,
			27,
			27,
			27,
			27,
			27,
			27,
			27,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			1,
			1,
			2,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			3,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			4,
			10,
			30,
			50,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			3,
			9,
			20,
			39,
			0,
			1,
			1,
			1,
			1,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			5,
			4,
			4,
			4,
			4,
			4,
			2,
			4,
			4,
			4,
			4,
			4,
			3,
			1,
			3,
			3,
			5,
			5,
			5,
			2,
			2,
			3,
			3,
			5,
			1,
			3,
			3,
			5,
			2,
			1,
			1,
			3,
			3,
			3,
			5,
			3,
			1,
			1,
			3,
			4,
			5,
			4,
			3,
			1,
			1,
			2,
			1};
            
    printf("%d\n", forward_filter_full(21608326261ull ^ JRAND_MULTIPLIER, shmem));
}