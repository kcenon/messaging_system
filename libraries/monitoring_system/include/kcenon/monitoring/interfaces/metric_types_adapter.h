#pragma once

/**
 * @file metric_types_adapter.h
 * @brief Adapter for metric types to support interface definitions
 *
 * This file provides type definitions to bridge the gap between
 * the interface definitions and the actual metric implementation.
 */

#include "../utils/metric_types.h"
#include <string>
#include <unordered_map>
#include <variant>

namespace monitoring_system {

/**
 * @brief Type alias for metric value
 */
using metric_value = std::variant<double, int64_t, std::string>;

/**
 * @struct metric
 * @brief Basic metric structure for interface compatibility
 */
struct metric {
    std::string name;
    metric_value value;
    std::unordered_map<std::string, std::string> tags;
    metric_type type{metric_type::gauge};
    std::chrono::system_clock::time_point timestamp;

    metric() : timestamp(std::chrono::system_clock::now()) {}

    metric(const std::string& n, const metric_value& v,
           const std::unordered_map<std::string, std::string>& t,
           metric_type mt = metric_type::gauge)
        : name(n), value(v), tags(t), type(mt),
          timestamp(std::chrono::system_clock::now()) {}

    // Convert to compact representation
    compact_metric_value to_compact() const {
        // Simple hash function for name
        uint32_t hash = 0;
        for (char c : name) {
            hash = hash * 31 + static_cast<uint32_t>(c);
        }

        metric_metadata meta(hash, type, static_cast<uint8_t>(tags.size()));

        if (std::holds_alternative<double>(value)) {
            return compact_metric_value(meta, std::get<double>(value));
        } else if (std::holds_alternative<int64_t>(value)) {
            return compact_metric_value(meta, std::get<int64_t>(value));
        } else {
            return compact_metric_value(meta, std::get<std::string>(value));
        }
    }
};

/**
 * @struct metric_stats
 * @brief Statistics about metric collection
 */
struct metric_stats {
    uint64_t total_collected{0};
    uint64_t total_errors{0};
    uint64_t total_dropped{0};
    std::chrono::milliseconds avg_collection_time{0};
    std::chrono::system_clock::time_point last_collection;

    double success_rate() const {
        if (total_collected == 0) return 0.0;
        return 1.0 - (static_cast<double>(total_errors) / total_collected);
    }

    void reset() {
        total_collected = 0;
        total_errors = 0;
        total_dropped = 0;
        avg_collection_time = std::chrono::milliseconds{0};
    }
};

} // namespace monitoring_system