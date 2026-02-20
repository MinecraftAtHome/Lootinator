#ifndef LOOTINATOR_DATA_LOOT_DATA
#define LOOTINATOR_DATA_LOOT_DATA

#include "lootinator/constraint/constraint.h"
#include "lootinator/mc/minecraft.hpp"
#include "lootinator/utility/range.h"

#include "nlohmann/json.hpp"

namespace data {
#define CAST_CHILD(name, type, child) type* name = dynamic_cast<type*>(child)

	class LootTreeNode {
	  public:
		static const int INDENT_SIZE = 4;

		LootTreeNode* parent;
		std::vector<LootTreeNode*> children;
		std::vector<loot::Constraint> constraints;

		mc::VersionRange get_version();
		virtual LootTreeNode* get_root_node();
		virtual int get_item_index(const std::string& item_name) const;
		virtual ~LootTreeNode();

		virtual uint32_t get_min_lcg_advancement() const;
		virtual uint32_t get_max_lcg_advancement() const;

		void indent(int indentation) const;
		virtual void print(int indentation) const;
		void print_constraints(int indentation) const;

		void sort_constraints();
	};

	class LootEntry : public LootTreeNode {
	  public:
		int item;
		mc::ItemType type;
		std::string name;
		int weight;
		// the range of nextInt values that produce this entry
		util::RangeInclusive<uint32_t> next_int_range;

		virtual uint32_t get_min_lcg_advancement() const;
		virtual uint32_t get_max_lcg_advancement() const;

		bool matches_constraint(const loot::Constraint& constraint) const;
		util::RangeInclusive<uint32_t> get_count_range() const;

		LootEntry(LootTreeNode* parent, const nlohmann::json& json,
			const util::RangeInclusive<uint32_t> next_int_range);
		virtual void print(int indentation) const override;
	};

	class LootPool : public LootTreeNode {
	  public:
		util::RangeInclusive<std::uint32_t> rolls;
		std::vector<int> entry_lookup; // goes straight into shared memory

		LootPool(LootTreeNode* parent, const nlohmann::json& json);
		uint32_t get_total_weight() const;

		virtual uint32_t get_min_lcg_advancement() const;
		virtual uint32_t get_max_lcg_advancement() const;

		bool matches_constraint(const loot::Constraint& constraint) const;

		virtual void print(int indentation) const override;
	};

	class LootTableRoot : public LootTreeNode {
	  public:
		int id_counter; // bleh
		mc::VersionRange version;
		std::unordered_map<std::string, int> item_map;

		LootTableRoot(const nlohmann::json& json, const std::string& item_map_filepath,
			mc::VersionRange version);
		virtual int get_item_index(const std::string& item_name) const override;
		virtual void print(int indentation) const override;

		void add_constraints(const std::vector<loot::Constraint>& constraints);
	};

	// LOOT FUNCTION DATA

	enum LootFunctionType {
		ENCHANT_RANDOMLY,
		ENCHANT_WITH_LEVELS,
		SET_COUNT,
		APPLY_DAMAGE,
		IGNORED
	};

	/**
	 * Enchant_randomly always selects one random enchantment from a predefined list. That list can
	 * either have preset values or be derived from the internal enchantment order for a specific
	 * version of MC. Either way, this structure stores that final order.
	 */
	struct EnchantRandomlyData {
		std::vector<mc::Enchantment> enchantment_order;
	};

	/**
	 * Stores data used for creating enchant_with_levels function skips. `level` is the base range
	 * of levels that can be selected by the particular function. `enchantability` is an
	 * item-dependent modifier value that can slightly offset the final (effective) level used to
	 * select enchantments.
	 */
	struct EnchantWithLevelsData {
		uint32_t enchantability;
		util::RangeInclusive<std::uint32_t> level = {0, 0};
		// not storing enchantments, function output is skipped
	};

	/**
	 * Stores the type, shared memory contents, and relevant processed data of a loot function.
	 */
	class LootFunctionData : public LootTreeNode {
	  public:
		uint32_t id;
		std::vector<int> shared_mem;
		data::LootFunctionType type;

		EnchantRandomlyData enchant_randomly;
		EnchantWithLevelsData enchant_with_levels;
		util::RangeInclusive<std::uint32_t> set_count = {0, 0};

		LootFunctionData(LootTreeNode* parent, const nlohmann::json& json);
		virtual void print(int indentation) const override;

		virtual uint32_t get_min_lcg_advancement() const;
		virtual uint32_t get_max_lcg_advancement() const;

	  private:
		uint32_t get_enchant_with_levels_max_advancement() const;
		uint32_t get_enchant_randomly_advancement(
			const uint32_t& (*compare_func)(const uint32_t&, const uint32_t&), bool is_min) const;

		void create_list_enchant_randomly(const nlohmann::json& list);
		void create_enchant_randomly(const nlohmann::json& function);
		void create_enchant_with_levels(const nlohmann::json& function);
	};
} // namespace data

#endif // LOOTINATOR_DATA_LOOT_DATA