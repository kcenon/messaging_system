#include <chrono>
#include <iostream>
#include <thread>

// Note: These storage headers do not exist in include directory
// #include <kcenon/monitoring/query/metric_query_engine.h>
// #include <kcenon/monitoring/storage/metric_database.h>
// #include <kcenon/monitoring/storage/timeseries_engine.h>

using namespace monitoring_system;

int main() {
    std::cout << "=== Time Series Storage Example ===" << std::endl;

    // Configure storage
    storage_config config;
    config.data_directory = "./tsdb_test_data";
    config.compression = compression_algorithm::lz4;
    config.memtable_size_mb = 16;

    // Create time series engine
    auto engine = std::make_unique<timeseries_engine>(config);

    // Write some test metrics
    std::cout << "\nWriting test metrics..." << std::endl;
    auto now = std::chrono::steady_clock::now();

    for (int i = 0; i < 10; ++i) {
        double value = 100.0 + i * 10;
        auto timestamp = now - std::chrono::seconds(60 * (10 - i));

        engine->write("cpu.usage", value, timestamp, {{"host", "server1"}});
        engine->write("memory.usage", value * 0.8, timestamp, {{"host", "server1"}});

        std::cout << "  Written metric at t-" << (10 - i) << " minutes: " << value << std::endl;
    }

    // Query metrics
    std::cout << "\nQuerying CPU usage for last hour..." << std::endl;
    auto results = engine->query("cpu.usage",
                                  now - std::chrono::hours(1),
                                  now,
                                  {{"host", "server1"}});

    for (const auto& series : results) {
        std::cout << "Series: " << series.metric_name << std::endl;
        std::cout << "  Points: " << series.points.size() << std::endl;
        std::cout << "  Min: " << series.min_value << std::endl;
        std::cout << "  Max: " << series.max_value << std::endl;
        std::cout << "  Avg: " << series.average() << std::endl;
    }

    // Test database with partitioning
    std::cout << "\n=== Metric Database Example ===" << std::endl;

    database_config db_config;
    db_config.data_directory = "./metrics_test_db";
    db_config.partition_strategy = partition_strategy::by_metric_name;

    auto database = std::make_unique<metric_database>(db_config);

    // Write batch of metrics
    std::vector<metric> batch;
    for (int i = 0; i < 5; ++i) {
        metric m;
        m.name = "test.metric";
        m.value = metric_value{50.0 + i};
        m.timestamp = std::chrono::system_clock::now();
        m.tags = {{"env", "test"}, {"instance", std::to_string(i)}};
        batch.push_back(m);
    }

    size_t written = database->write_batch(batch);
    std::cout << "Written " << written << " metrics to database" << std::endl;

    // Query with aggregation
    std::cout << "\n=== Query Engine Example ===" << std::endl;

    auto query_engine = std::make_unique<metric_query_engine>(database.get());

    // Simple query example (would need parser implementation for full functionality)
    std::string query_str = "SELECT cpu.usage WHERE host='server1' FROM -1h";
    std::cout << "Query: " << query_str << std::endl;

    // Get database stats
    auto stats = database->get_stats();
    std::cout << "\nDatabase Statistics:" << std::endl;
    std::cout << "  Total metrics: " << stats.total_metrics << std::endl;
    std::cout << "  Total points: " << stats.total_points << std::endl;
    std::cout << "  Total partitions: " << stats.total_partitions << std::endl;

    std::cout << "\nStorage example completed successfully!" << std::endl;
    return 0;
}