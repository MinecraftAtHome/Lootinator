#include "lootinator/probability/loot_prob.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// ---------------------------------------

namespace prob {
	struct PoolOutcome {
        int item_id;
        int item_count;
        double probability;
    };

    struct PoolItemStats {
        double q = 0.0;
        std::vector<int> counts;
        std::vector<double> conditional_pmf;
        int max_count = 0;
    };

    struct NormalizedLootPool {
        int pool_index;
        int rolls_min;
        int rolls_max;
        std::vector<PoolOutcome> outcomes;
    };

    struct PoolItemSubtotalDP {
        int rows = 0;
        int cols = 0;
        std::vector<double> subtotal_pmf;

        double at(int r, int c) const {
            return subtotal_pmf[static_cast<std::size_t>(r) * cols + c];
        }
    };

    struct PreparedLootModel {
        std::vector<NormalizedLootPool> pools;
        std::map<int, std::vector<int>> item_to_pools;
        std::map<int, std::vector<int>> item_to_allowed_counts;
        std::map<std::pair<int, int>, PoolItemStats> pool_item_stats;
        std::map<int, std::vector<double>> log_factorials_by_pool;
        std::map<int, std::pair<std::vector<double>, double>> scaled_roll_weights_by_pool;
        std::map<int, int> rolls_max_by_pool;
        std::map<int, int> max_allowed_count_by_item;
        std::map<int, int> max_reachable_count_by_item;
    };

    struct SparseFactor {
        std::vector<int> scope;
        std::vector<int> shape;
        std::vector<double> tensor;
        double log_scale = 0.0;

        bool is_zero() const {
            if (!std::isfinite(log_scale)) return true;
            return std::all_of(tensor.begin(), tensor.end(), [](double v) { return v == 0.0; });
        }

        bool is_scalar() const {
            return scope.empty();
        }

        std::size_t size() const {
            return tensor.size();
        }
    };

    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
    using PoolItemKey = std::pair<int, int>;

    struct IndexLayout {
        std::vector<int> shape;
        std::vector<int> strides;

        explicit IndexLayout(std::vector<int> s) : shape(std::move(s)), strides(shape.size(), 1) {
            for (int i = static_cast<int>(shape.size()) - 2; i >= 0; --i) {
                strides[i] = strides[i + 1] * shape[i + 1];
            }
        }

        std::size_t size() const {
            if (shape.empty()) return 1;
            return static_cast<std::size_t>(std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<int>()));
        }

        std::vector<int> unravel(std::size_t index) const {
            std::vector<int> coords(shape.size(), 0);
            for (std::size_t i = 0; i < shape.size(); ++i) {
                coords[i] = static_cast<int>(index / static_cast<std::size_t>(strides[i]));
                index %= static_cast<std::size_t>(strides[i]);
            }
            return coords;
        }

        std::size_t ravel(const std::vector<int>& coords) const {
            std::size_t index = 0;
            for (std::size_t i = 0; i < coords.size(); ++i) {
                index += static_cast<std::size_t>(coords[i] * strides[i]);
            }
            return index;
        }
    };

    int product(const std::vector<int>& shape) {
        return shape.empty() ? 1 : std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<int>());
    }

    double clamp_probability(double x) {
        return std::max(0.0, std::min(1.0, x));
    }

    std::vector<int> union_scope(const SparseFactor& a, const SparseFactor& b) {
        std::vector<int> scope;
        scope.reserve(a.scope.size() + b.scope.size());
        std::merge(a.scope.begin(), a.scope.end(), b.scope.begin(), b.scope.end(), std::back_inserter(scope));
        scope.erase(std::unique(scope.begin(), scope.end()), scope.end());
        return scope;
    }

    std::size_t estimate_union_size(const SparseFactor& a, const SparseFactor& b) {
        std::map<int, int> shape_by_pool;
        for (std::size_t i = 0; i < a.scope.size(); ++i) shape_by_pool[a.scope[i]] = a.shape[i];
        for (std::size_t i = 0; i < b.scope.size(); ++i) shape_by_pool.emplace(b.scope[i], b.shape[i]);

        std::size_t size = 1;
        // pair = [_, extent]
        for (const auto& pair : shape_by_pool) {
            size *= static_cast<std::size_t>(pair.second);
        }
        return size;
    }

    SparseFactor normalize_factor(const std::vector<int>& scope,
                                 const std::vector<int>& shape,
                                 const std::vector<double>& tensor,
                                 double log_scale = 0.0) {
        double max_value = 0.0;
        for (double v : tensor) max_value = std::max(max_value, std::abs(v));
        if (!std::isfinite(log_scale) || max_value == 0.0) {
            return SparseFactor{scope, shape, std::vector<double>(tensor.size(), 0.0), NEG_INF};
        }

        std::vector<double> scaled(tensor.size());
        for (std::size_t i = 0; i < tensor.size(); ++i) scaled[i] = tensor[i] / max_value;
        return SparseFactor{scope, shape, std::move(scaled), log_scale + std::log(max_value)};
    }

    SparseFactor identity_factor() {
        return SparseFactor{{}, {}, {1.0}, 0.0};
    }

    SparseFactor scalar_factor(double value) {
        return value > 0.0 ? normalize_factor({}, {}, {value}) : SparseFactor{{}, {}, {0.0}, NEG_INF};
    }

    double factor_value(const SparseFactor& factor) {
        if (factor.is_zero()) return 0.0;
        if (!factor.is_scalar()) throw std::runtime_error("Factor is not scalar.");
        if (factor.tensor[0] <= 0.0) return 0.0;
        return clamp_probability(std::exp(std::log(factor.tensor[0]) + factor.log_scale));
    }

    void validate_loot_table_light(const LootTable& loot_table) {
        if (loot_table.pools.empty()) {
            throw std::invalid_argument("LootTable must contain at least one pool.");
        }

        for (const auto& pool : loot_table.pools) {
            if (pool.rolls_min < 0 || pool.rolls_max < pool.rolls_min) {
                throw std::invalid_argument("Invalid roll range in loot pool.");
            }
            if (pool.entries.empty()) {
                throw std::invalid_argument("Each loot pool must contain at least one entry.");
            }

            long double weight_sum = 0.0L;
            for (const auto& entry : pool.entries) {
                if (entry.count_min < 0 || entry.count_max < entry.count_min || entry.weight < 0.0) {
                    throw std::invalid_argument("Invalid entry in loot pool.");
                }
                weight_sum += static_cast<long double>(entry.weight);
            }
            if (std::abs(weight_sum - 1.0L) > 1e-12L) {
                throw std::invalid_argument("Entry weights in each pool must sum to 1.");
            }
        }
    }

    std::vector<TargetItem> validate_target_items_light(const std::vector<TargetItem>& target_items,
                                                        const LootTable& loot_table) {
        std::set<int> available_items;
        for (const auto& pool : loot_table.pools) {
            for (const auto& entry : pool.entries) {
                available_items.insert(entry.item_id);
            }
        }

        std::set<int> seen;
        for (const auto& target : target_items) {
            if (target.item_count < 0) {
                throw std::invalid_argument("Target counts must be non-negative.");
            }
            if (!available_items.count(target.item_id)) {
                throw std::invalid_argument("Target item_id does not appear in the loot table.");
            }
            if (!seen.insert(target.item_id).second) {
                throw std::invalid_argument("Duplicate target item_id.");
            }
        }
        return target_items;
    }

    std::vector<NormalizedLootPool> expand_loot_table(const LootTable& loot_table) {
        std::vector<NormalizedLootPool> normalized;
        normalized.reserve(loot_table.pools.size());

        for (int pool_index = 0; pool_index < static_cast<int>(loot_table.pools.size()); ++pool_index) {
            const auto& pool = loot_table.pools[pool_index];
            std::vector<PoolOutcome> outcomes;
            for (const auto& entry : pool.entries) {
                if (entry.weight <= 0.0) continue;
                const int width = entry.count_max - entry.count_min + 1;
                const double p = entry.weight / static_cast<double>(width);
                for (int count = entry.count_min; count <= entry.count_max; ++count) {
                    outcomes.push_back(PoolOutcome{entry.item_id, count, p});
                }
            }
            normalized.push_back(NormalizedLootPool{pool_index, pool.rolls_min, pool.rolls_max, std::move(outcomes)});
        }
        return normalized;
    }

    std::map<int, std::vector<int>> build_item_to_pools(const std::vector<NormalizedLootPool>& pools,
                                                        const std::set<int>& all_item_ids) {
        std::map<int, std::set<int>> item_to_pool_set;
        for (int item_id : all_item_ids) item_to_pool_set[item_id] = {};

        for (const auto& pool : pools) {
            for (const auto& outcome : pool.outcomes) {
                item_to_pool_set[outcome.item_id].insert(pool.pool_index);
            }
        }

        std::map<int, std::vector<int>> result;
        // pair = [item_id, pool_set]
        for (const auto& pair : item_to_pool_set) {
            result[pair.first] = std::vector<int>(pair.second.begin(), pair.second.end());
        }
        return result;
    }

    std::map<PoolItemKey, PoolItemStats> aggregate_pool_item_stats(const std::vector<NormalizedLootPool>& pools) {
        std::map<PoolItemKey, PoolItemStats> stats;

        for (const auto& pool : pools) {
            std::map<int, long double> hit_probability_by_item;
            std::map<int, std::map<int, long double>> count_mass_by_item;

            for (const auto& outcome : pool.outcomes) {
                hit_probability_by_item[outcome.item_id] += static_cast<long double>(outcome.probability);
                count_mass_by_item[outcome.item_id][outcome.item_count] += static_cast<long double>(outcome.probability);
            }

            // pair = [item_id, q_ld]
            for (const auto& pair : hit_probability_by_item) {
                const double q = static_cast<double>(pair.second);
                const auto& mass = count_mass_by_item[pair.first];

                PoolItemStats item_stats;
                item_stats.q = q;
                item_stats.counts.reserve(mass.size());
                item_stats.conditional_pmf.reserve(mass.size());
                // pair2 = [count, probability]
                for (const auto& pair2 : mass) {
                    item_stats.counts.push_back(pair2.first);
                    item_stats.conditional_pmf.push_back(static_cast<double>(pair2.second / pair.second));
                }
                item_stats.max_count = item_stats.counts.empty() ? 0 : item_stats.counts.back();
                stats[{pool.pool_index, pair.first}] = std::move(item_stats);
            }
        }

        return stats;
    }

    std::map<int, std::vector<double>> precompute_log_factorials_by_pool(const std::vector<NormalizedLootPool>& pools) {
        std::map<int, std::vector<double>> out;
        for (const auto& pool : pools) {
            std::vector<double> log_factorials(pool.rolls_max + 1, 0.0);
            for (int i = 1; i <= pool.rolls_max; ++i) {
                log_factorials[i] = log_factorials[i - 1] + std::log(static_cast<double>(i));
            }
            out[pool.pool_index] = std::move(log_factorials);
        }
        return out;
    }

    std::map<int, std::pair<std::vector<double>, double>> precompute_scaled_roll_weights_by_pool(
        const std::vector<NormalizedLootPool>& pools,
        const std::map<int, std::vector<double>>& log_factorials_by_pool) {

        std::map<int, std::pair<std::vector<double>, double>> out;

        for (const auto& pool : pools) {
            std::vector<double> scaled(pool.rolls_max + 1, 0.0);
            const int width = pool.rolls_max - pool.rolls_min + 1;
            const auto& log_factorials = log_factorials_by_pool.at(pool.pool_index);
            const double log_scale = -std::log(static_cast<double>(width)) + log_factorials[pool.rolls_max];

            for (int r = pool.rolls_min; r <= pool.rolls_max; ++r) {
                scaled[r] = std::exp(log_factorials[r] - log_factorials[pool.rolls_max]);
            }
            out[pool.pool_index] = {std::move(scaled), log_scale};
        }

        return out;
    }

    std::map<int, int> compute_max_reachable_count_by_item(
        const std::map<int, std::vector<int>>& item_to_pools,
        const std::map<PoolItemKey, PoolItemStats>& pool_item_stats,
        const std::map<int, int>& rolls_max_by_pool) {

        std::map<int, int> out;
        // pair = [item_id, pool_scope]
        for (const auto& pair : item_to_pools) {
            int total = 0;
            for (int pool_index : pair.second) {
                total += rolls_max_by_pool.at(pool_index) * pool_item_stats.at({pool_index, pair.first}).max_count;
            }
            out[pair.first] = total;
        }
        return out;
    }

    std::map<int, std::vector<int>> convert_target_items_to_allowed_counts(
        const std::vector<TargetItem>& target_items,
        const std::map<int, int>& max_reachable_count_by_item) {

        std::map<int, std::vector<int>> out;
        for (const auto& target : target_items) {
            const int max_reachable = max_reachable_count_by_item.at(target.item_id);
            std::vector<int> allowed;
            if (target.need_exact_count) {
                if (target.item_count <= max_reachable) allowed.push_back(target.item_count);
            } else if (target.item_count <= 0) {
                allowed.resize(max_reachable + 1);
                std::iota(allowed.begin(), allowed.end(), 0);
            } else if (target.item_count <= max_reachable) {
                allowed.resize(max_reachable - target.item_count + 1);
                std::iota(allowed.begin(), allowed.end(), target.item_count);
            }
            out[target.item_id] = std::move(allowed);
        }
        return out;
    }

    std::map<int, int> compute_max_allowed_count_by_item(const std::map<int, std::vector<int>>& item_to_allowed_counts) {
        std::map<int, int> out;
        // pair = [item_id, allowed]
        for (const auto& pair : item_to_allowed_counts) {
            out[pair.first] = pair.second.empty() ? -1 : pair.second.back();
        }
        return out;
    }

    PreparedLootModel prepare_loot_model(const LootTable& loot_table, const std::vector<TargetItem>& target_items) {
        validate_loot_table_light(loot_table);
        const auto validated_targets = validate_target_items_light(target_items, loot_table);

        std::set<int> all_item_ids;
        for (const auto& pool : loot_table.pools) {
            for (const auto& entry : pool.entries) {
                all_item_ids.insert(entry.item_id);
            }
        }

        PreparedLootModel model;
        model.pools = expand_loot_table(loot_table);
        model.item_to_pools = build_item_to_pools(model.pools, all_item_ids);
        model.pool_item_stats = aggregate_pool_item_stats(model.pools);
        model.log_factorials_by_pool = precompute_log_factorials_by_pool(model.pools);
        model.scaled_roll_weights_by_pool = precompute_scaled_roll_weights_by_pool(model.pools, model.log_factorials_by_pool);
        for (const auto& pool : model.pools) model.rolls_max_by_pool[pool.pool_index] = pool.rolls_max;
        model.max_reachable_count_by_item = compute_max_reachable_count_by_item(
            model.item_to_pools, model.pool_item_stats, model.rolls_max_by_pool);
        model.item_to_allowed_counts = convert_target_items_to_allowed_counts(
            validated_targets, model.max_reachable_count_by_item);
        model.max_allowed_count_by_item = compute_max_allowed_count_by_item(model.item_to_allowed_counts);
        return model;
    }

    PoolItemSubtotalDP build_pool_item_subtotal_dp(const PoolItemStats& stats,
                                                   int max_rolls,
                                                   int max_total) {
        PoolItemSubtotalDP dp;
        dp.rows = max_rolls + 1;
        dp.cols = max_total + 1;
        dp.subtotal_pmf.assign(static_cast<std::size_t>(dp.rows) * dp.cols, 0.0);

        std::vector<double> subtotal(max_total + 1, 0.0);
        subtotal[0] = 1.0;
        std::copy(subtotal.begin(), subtotal.end(), dp.subtotal_pmf.begin());

        for (int hits = 1; hits <= max_rolls; ++hits) {
            std::vector<double> next(max_total + 1, 0.0);
            for (std::size_t i = 0; i < stats.counts.size(); ++i) {
                const int item_count = stats.counts[i];
                const double probability = stats.conditional_pmf[i];
                if (probability == 0.0 || item_count > max_total) continue;
                for (int total = 0; total + item_count <= max_total; ++total) {
                    next[total + item_count] += probability * subtotal[total];
                }
            }
            subtotal.swap(next);
            std::copy(subtotal.begin(), subtotal.end(), dp.subtotal_pmf.begin() + static_cast<std::size_t>(hits) * dp.cols);
        }

        return dp;
    }

    std::map<PoolItemKey, PoolItemSubtotalDP> precompute_subtotal_dp_cache(const PreparedLootModel& prepared) {
        std::map<PoolItemKey, PoolItemSubtotalDP> cache;
        // pair = [item_id, allowed_counts]
        for (const auto& pair : prepared.item_to_allowed_counts) {
            const auto& pool_scope = prepared.item_to_pools.at(pair.first);
            if (pair.second.empty() || pool_scope.empty()) continue;

            const int max_total = std::min(prepared.max_allowed_count_by_item.at(pair.first),
                                           prepared.max_reachable_count_by_item.at(pair.first));
            for (int pool_index : pool_scope) {
                cache[{pool_index, pair.first}] = build_pool_item_subtotal_dp(
                    prepared.pool_item_stats.at({pool_index, pair.first}),
                    prepared.rolls_max_by_pool.at(pool_index),
                    max_total);
            }
        }
        return cache;
    }

    SparseFactor build_hit_weight_factor(int pool_index, double q, const std::vector<double>& log_factorials) {
        const int max_rolls = static_cast<int>(log_factorials.size()) - 1;
        q = clamp_probability(q);

        std::vector<double> values(max_rolls + 1, 0.0);
        values[0] = 1.0;
        for (int m = 1; m <= max_rolls; ++m) {
            values[m] = values[m - 1] * (q / static_cast<double>(m));
        }
        return normalize_factor({pool_index}, {max_rolls + 1}, values);
    }

    std::pair<std::vector<int>, std::vector<double>> append_rows_truncated(const std::vector<int>& state_shape,
                                                                           const std::vector<double>& state,
                                                                           const PoolItemSubtotalDP& rows,
                                                                           int max_total) {
        const int subtotal_len = max_total + 1;
        if (state_shape.empty() || state_shape.back() != subtotal_len || rows.cols != subtotal_len) {
            throw std::runtime_error("Invalid shapes for subtotal append.");
        }

        const int prefix_cells = product(std::vector<int>(state_shape.begin(), state_shape.end() - 1));
        std::vector<int> out_shape = state_shape;
        out_shape.insert(out_shape.end() - 1, rows.rows);
        std::vector<double> out(static_cast<std::size_t>(product(out_shape)), 0.0);

        for (int prefix = 0; prefix < prefix_cells; ++prefix) {
            const std::size_t prefix_offset = static_cast<std::size_t>(prefix) * subtotal_len;
            for (int old_total = 0; old_total <= max_total; ++old_total) {
                const double base = state[prefix_offset + old_total];
                if (base == 0.0) continue;
                for (int r = 0; r < rows.rows; ++r) {
                    const std::size_t row_offset = (static_cast<std::size_t>(prefix) * rows.rows + r) * subtotal_len;
                    for (int add_total = 0; old_total + add_total <= max_total; ++add_total) {
                        const double p = rows.at(r, add_total);
                        if (p == 0.0) continue;
                        out[row_offset + old_total + add_total] += base * p;
                    }
                }
            }
        }

        return {out_shape, std::move(out)};
    }

    std::pair<std::vector<int>, std::vector<double>> build_constrained_item_probability_tensor(
        const PreparedLootModel& prepared,
        int item_id,
        const std::map<PoolItemKey, PoolItemSubtotalDP>& subtotal_dp_cache) {

        const auto& allowed_counts = prepared.item_to_allowed_counts.at(item_id);
        const auto& pool_scope = prepared.item_to_pools.at(item_id);
        std::vector<int> shape;
        shape.reserve(pool_scope.size());
        for (int pool_index : pool_scope) {
            shape.push_back(prepared.rolls_max_by_pool.at(pool_index) + 1);
        }

        if (allowed_counts.empty()) {
            return {shape, std::vector<double>(static_cast<std::size_t>(product(shape)), 0.0)};
        }

        const int reachable_max = prepared.max_reachable_count_by_item.at(item_id);
        std::vector<int> allowed_indices;
        allowed_indices.reserve(allowed_counts.size());
        for (int count : allowed_counts) {
            if (0 <= count && count <= reachable_max) allowed_indices.push_back(count);
        }
        if (allowed_indices.empty()) {
            return {shape, std::vector<double>(static_cast<std::size_t>(product(shape)), 0.0)};
        }

        const int max_total = allowed_indices.back();
        std::vector<int> state_shape{max_total + 1};
        std::vector<double> state(max_total + 1, 0.0);
        state[0] = 1.0;

        for (int pool_index : pool_scope) {
            const auto& rows = subtotal_dp_cache.at({pool_index, item_id});
            // pair = [next_shape, next_state]
            auto pair = append_rows_truncated(state_shape, state, rows, max_total);
            state_shape = std::move(pair.first);
            state = std::move(pair.second);
        }

        const int prefix_cells = product(std::vector<int>(state_shape.begin(), state_shape.end() - 1));
        std::vector<int> out_shape(state_shape.begin(), state_shape.end() - 1);
        std::vector<double> out(static_cast<std::size_t>(prefix_cells), 0.0);

        for (int prefix = 0; prefix < prefix_cells; ++prefix) {
            const std::size_t offset = static_cast<std::size_t>(prefix) * (max_total + 1);
            for (int count : allowed_indices) {
                out[prefix] += state[offset + count];
            }
        }

        return {out_shape, std::move(out)};
    }

    void multiply_tensor_by_axis_vector(std::vector<double>& tensor,
                                        const std::vector<int>& shape,
                                        int axis,
                                        const std::vector<double>& vec) {
        if (shape.empty()) {
            tensor[0] *= vec[0];
            return;
        }

        const IndexLayout layout(shape);
        for (std::size_t i = 0; i < tensor.size(); ++i) {
            tensor[i] *= vec[layout.unravel(i)[axis]];
        }
    }

    std::vector<SparseFactor> build_item_factors(
        const PreparedLootModel& prepared,
        const std::map<PoolItemKey, PoolItemSubtotalDP>& subtotal_dp_cache) {

        std::vector<SparseFactor> factors;

        // pair = [item_id, pool_scope]
        for (const auto& pair : prepared.item_to_pools) {
            const auto allowed_it = prepared.item_to_allowed_counts.find(pair.first);

            if (pair.second.empty()) {
                if (allowed_it != prepared.item_to_allowed_counts.end()) {
                    const bool ok = std::find(allowed_it->second.begin(), allowed_it->second.end(), 0) != allowed_it->second.end();
                    factors.push_back(scalar_factor(ok ? 1.0 : 0.0));
                }
                continue;
            }

            if (allowed_it == prepared.item_to_allowed_counts.end()) {
                for (int pool_index : pair.second) {
                    const auto& stats = prepared.pool_item_stats.at({pool_index, pair.first});
                    factors.push_back(build_hit_weight_factor(pool_index, stats.q, prepared.log_factorials_by_pool.at(pool_index)));
                }
                continue;
            }

            const auto& allowed_counts = allowed_it->second;
            const int reachable_max = prepared.max_reachable_count_by_item.at(pair.first);
            const bool any_reachable = std::any_of(allowed_counts.begin(), allowed_counts.end(), [reachable_max](int c) {
                return 0 <= c && c <= reachable_max;
            });
            if (!any_reachable) {
                factors.push_back(scalar_factor(0.0));
                continue;
            }

            // pair2 = [shape, coeff]
            auto pair2 = build_constrained_item_probability_tensor(prepared, pair.first, subtotal_dp_cache);
            double log_scale = 0.0;

            for (std::size_t axis = 0; axis < pair.second.size(); ++axis) {
                const int pool_index = pair.second[axis];
                const auto& stats = prepared.pool_item_stats.at({pool_index, pair.first});
                const auto unary = build_hit_weight_factor(pool_index, stats.q, prepared.log_factorials_by_pool.at(pool_index));
                multiply_tensor_by_axis_vector(pair2.second, pair2.first, static_cast<int>(axis), unary.tensor);
                log_scale += unary.log_scale;
            }

            factors.push_back(normalize_factor(pair.second, pair2.first, pair2.second, log_scale));
        }

        return factors;
    }

    SparseFactor multiply_two_factors(const SparseFactor& a,
                                      const SparseFactor& b,
                                      const PreparedLootModel& prepared) {
        if (a.is_zero() || b.is_zero()) return scalar_factor(0.0);

        const std::vector<int> scope = union_scope(a, b);
        std::vector<int> shape;
        shape.reserve(scope.size());
        for (int pool_index : scope) {
            shape.push_back(prepared.rolls_max_by_pool.at(pool_index) + 1);
        }

        const IndexLayout a_layout(a.shape);
        const IndexLayout b_layout(b.shape);
        const IndexLayout out_layout(shape);
        std::vector<double> out(out_layout.size(), 0.0);

        std::map<int, int> a_axis;
        std::map<int, int> b_axis;
        for (std::size_t i = 0; i < a.scope.size(); ++i) a_axis[a.scope[i]] = static_cast<int>(i);
        for (std::size_t i = 0; i < b.scope.size(); ++i) b_axis[b.scope[i]] = static_cast<int>(i);

        for (std::size_t ia = 0; ia < a.tensor.size(); ++ia) {
            const double va = a.tensor[ia];
            if (va == 0.0) continue;
            const auto a_coords = a_layout.unravel(ia);

            for (std::size_t ib = 0; ib < b.tensor.size(); ++ib) {
                const double vb = b.tensor[ib];
                if (vb == 0.0) continue;
                const auto b_coords = b_layout.unravel(ib);

                std::vector<int> coords(scope.size(), 0);
                bool valid = true;
                for (std::size_t axis = 0; axis < scope.size(); ++axis) {
                    const int pool_index = scope[axis];
                    const auto ita = a_axis.find(pool_index);
                    const auto itb = b_axis.find(pool_index);
                    const int av = ita == a_axis.end() ? 0 : a_coords[ita->second];
                    const int bv = itb == b_axis.end() ? 0 : b_coords[itb->second];
                    coords[axis] = av + bv;
                    if (coords[axis] >= shape[axis]) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    out[out_layout.ravel(coords)] += va * vb;
                }
            }
        }

        return normalize_factor(scope, shape, out, a.log_scale + b.log_scale);
    }

    SparseFactor multiply_many_factors(std::vector<SparseFactor> factors,
                                       const PreparedLootModel& prepared) {
        std::vector<SparseFactor> active;
        active.reserve(factors.size());
        for (const auto& factor : factors) {
            if (factor.is_scalar() && factor_value(factor) == 1.0) continue;
            active.push_back(factor);
        }
        if (active.empty()) return identity_factor();
        if (active.size() == 1) return active.front();

        while (active.size() > 1) {
            std::pair<int, int> best_pair{-1, -1};
            std::tuple<int, std::size_t, std::size_t> best_score{
                std::numeric_limits<int>::max(),
                std::numeric_limits<std::size_t>::max(),
                std::numeric_limits<std::size_t>::max()};

            for (int i = 0; i < static_cast<int>(active.size()); ++i) {
                for (int j = i + 1; j < static_cast<int>(active.size()); ++j) {
                    const auto scope = union_scope(active[i], active[j]);
                    const auto score = std::make_tuple(
                        static_cast<int>(scope.size()),
                        estimate_union_size(active[i], active[j]),
                        active[i].size() + active[j].size());
                    if (score < best_score) {
                        best_score = score;
                        best_pair = {i, j};
                    }
                }
            }

            SparseFactor merged = multiply_two_factors(active[best_pair.first], active[best_pair.second], prepared);
            std::vector<SparseFactor> next;
            next.reserve(active.size() - 1);
            for (int i = 0; i < static_cast<int>(active.size()); ++i) {
                if (i != best_pair.first && i != best_pair.second) next.push_back(std::move(active[i]));
            }
            next.push_back(std::move(merged));
            active = std::move(next);
            if (active.back().is_zero()) return active.back();
        }

        return active.front();
    }

    SparseFactor eliminate_pool_roll_count(const SparseFactor& factor,
                                           int pool_index,
                                           const PreparedLootModel& prepared) {
        if (factor.is_zero()) return factor;

        const auto it = std::find(factor.scope.begin(), factor.scope.end(), pool_index);
        if (it == factor.scope.end()) return factor;

        const int axis = static_cast<int>(std::distance(factor.scope.begin(), it));
        // pair = [weights, weights_log_scale]
        const auto& pair = prepared.scaled_roll_weights_by_pool.at(pool_index);
        if (factor.shape[axis] != static_cast<int>(pair.first.size())) {
            throw std::runtime_error("Factor shape does not match roll weight vector.");
        }

        std::vector<int> new_scope = factor.scope;
        std::vector<int> new_shape = factor.shape;
        new_scope.erase(new_scope.begin() + axis);
        new_shape.erase(new_shape.begin() + axis);

        const IndexLayout factor_layout(factor.shape);
        const IndexLayout reduced_layout(new_shape);
        std::vector<double> reduced(reduced_layout.size(), 0.0);

        for (std::size_t i = 0; i < factor.tensor.size(); ++i) {
            const double value = factor.tensor[i];
            if (value == 0.0) continue;
            auto coords = factor_layout.unravel(i);
            const double weight = pair.first[coords[axis]];
            if (weight == 0.0) continue;
            coords.erase(coords.begin() + axis);
            reduced[reduced_layout.ravel(coords)] += value * weight;
        }

        return normalize_factor(new_scope, new_shape, reduced, factor.log_scale + pair.second);
    }

    std::vector<int> choose_min_fill_elimination_order(const PreparedLootModel& prepared,
                                                       const std::vector<SparseFactor>& factors) {
        std::vector<int> pool_indices;
        // pair = [pool_index, _]
        for (const auto& pair : prepared.rolls_max_by_pool) pool_indices.push_back(pair.first);
        std::sort(pool_indices.begin(), pool_indices.end());

        std::map<int, std::set<int>> graph;
        for (int pool_index : pool_indices) graph[pool_index] = {};
        for (const auto& factor : factors) {
            for (std::size_t i = 0; i < factor.scope.size(); ++i) {
                for (std::size_t j = i + 1; j < factor.scope.size(); ++j) {
                    graph[factor.scope[i]].insert(factor.scope[j]);
                    graph[factor.scope[j]].insert(factor.scope[i]);
                }
            }
        }

        std::set<int> remaining(pool_indices.begin(), pool_indices.end());
        std::vector<int> order;
        order.reserve(pool_indices.size());

        while (!remaining.empty()) {
            int best_pool = -1;
            std::tuple<int, int, int> best_score{
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()};

            for (int pool_index : remaining) {
                std::vector<int> neighbors;
                for (int neighbor : graph[pool_index]) {
                    if (remaining.count(neighbor)) neighbors.push_back(neighbor);
                }

                int fill = 0;
                for (std::size_t i = 0; i < neighbors.size(); ++i) {
                    for (std::size_t j = i + 1; j < neighbors.size(); ++j) {
                        if (!graph[neighbors[i]].count(neighbors[j])) ++fill;
                    }
                }

                const auto score = std::make_tuple(
                    fill,
                    static_cast<int>(neighbors.size()),
                    prepared.rolls_max_by_pool.at(pool_index) + 1);
                if (score < best_score) {
                    best_score = score;
                    best_pool = pool_index;
                }
            }

            order.push_back(best_pool);
            std::vector<int> neighbors;
            for (int neighbor : graph[best_pool]) {
                if (remaining.count(neighbor)) neighbors.push_back(neighbor);
            }
            for (std::size_t i = 0; i < neighbors.size(); ++i) {
                for (std::size_t j = i + 1; j < neighbors.size(); ++j) {
                    graph[neighbors[i]].insert(neighbors[j]);
                    graph[neighbors[j]].insert(neighbors[i]);
                }
            }
            remaining.erase(best_pool);
        }

        return order;
    }

    double get_loot_probability(const LootTable& loot_table,
                                const std::vector<TargetItem>& target_items) {
        const PreparedLootModel prepared = prepare_loot_model(loot_table, target_items);

        // pair = [item_id, allowed_counts]
        for (const auto& pair : prepared.item_to_allowed_counts) {
            if (pair.second.empty()) return 0.0;
            if (prepared.max_reachable_count_by_item.at(pair.first) < pair.second.front()) return 0.0;
        }

        const auto subtotal_dp_cache = precompute_subtotal_dp_cache(prepared);
        auto active_factors = build_item_factors(prepared, subtotal_dp_cache);

        for (const auto& factor : active_factors) {
            if (factor.is_zero() && factor.is_scalar()) return 0.0;
        }

        for (int pool_index : choose_min_fill_elimination_order(prepared, active_factors)) {
            std::vector<SparseFactor> involved;
            std::vector<SparseFactor> untouched;
            for (const auto& factor : active_factors) {
                if (std::find(factor.scope.begin(), factor.scope.end(), pool_index) != factor.scope.end()) {
                    involved.push_back(factor);
                } else {
                    untouched.push_back(factor);
                }
            }

            if (!involved.empty()) {
                SparseFactor reduced = eliminate_pool_roll_count(multiply_many_factors(involved, prepared), pool_index, prepared);
                untouched.push_back(std::move(reduced));
                if (untouched.back().is_zero() && untouched.back().is_scalar()) return 0.0;
            }

            active_factors = std::move(untouched);
        }

        return factor_value(multiply_many_factors(active_factors, prepared));
    }

    //int main() {
    //    LootTable loot_table{
    //        {
    //            LootPool{
    //                1,
    //                2,
    //                {
    //                    LootEntry{1001, 0.50, 1, 1},
    //                    LootEntry{1002, 0.30, 1, 2},
    //                    LootEntry{1003, 0.20, 2, 3},
    //                }
    //            },
    //            LootPool{
    //                0,
    //                1,
    //                {
    //                    LootEntry{1002, 0.40, 1, 1},
    //                    LootEntry{1004, 0.60, 1, 3},
    //                }
    //            }
    //        }
    //    };
    //
    //    std::vector<TargetItem> target_items{
    //        TargetItem{1002, 2, false}, // at least 2 of item 1002
    //        TargetItem{1004, 0, true},  // exactly 0 of item 1004
    //    };
    //
    //    double probability = get_loot_probability(loot_table, target_items);
    //    std::cout << "Probability = " << probability << "\n";
    //    return 0;
    //}
}
