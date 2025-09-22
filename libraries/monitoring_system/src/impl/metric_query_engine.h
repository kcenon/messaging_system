#pragma once

#include <algorithm>
#include <any>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <kcenon/monitoring/storage/metric_database.h>
#include <kcenon/monitoring/storage/timeseries_engine.h>
#include <kcenon/monitoring/utils/metric_types.h>

namespace monitoring_system {

/**
 * Query expression types
 */
enum class expression_type {
    literal,
    identifier,
    binary_op,
    unary_op,
    function_call,
    aggregation
};

/**
 * Binary operators
 */
enum class binary_operator {
    add,
    subtract,
    multiply,
    divide,
    modulo,
    power,
    equal,
    not_equal,
    less_than,
    less_equal,
    greater_than,
    greater_equal,
    logical_and,
    logical_or
};

/**
 * Unary operators
 */
enum class unary_operator {
    negate,
    logical_not,
    absolute
};

/**
 * Aggregation functions
 */
enum class aggregation_function {
    sum,
    avg,
    min,
    max,
    count,
    stddev,
    variance,
    percentile,
    rate,
    delta,
    derivative,
    integral
};

/**
 * Query expression AST node
 */
struct expression_node {
    expression_type type;
    std::variant<
        double,                                              // literal value
        std::string,                                         // identifier
        std::pair<binary_operator, std::vector<std::unique_ptr<expression_node>>>,  // binary op
        std::pair<unary_operator, std::unique_ptr<expression_node>>,                // unary op
        std::pair<std::string, std::vector<std::unique_ptr<expression_node>>>,      // function call
        std::pair<aggregation_function, std::unique_ptr<expression_node>>           // aggregation
    > value;

    // Helper constructors
    static std::unique_ptr<expression_node> make_literal(double val);
    static std::unique_ptr<expression_node> make_identifier(const std::string& name);
    static std::unique_ptr<expression_node> make_binary_op(binary_operator op,
                                                            std::unique_ptr<expression_node> left,
                                                            std::unique_ptr<expression_node> right);
    static std::unique_ptr<expression_node> make_unary_op(unary_operator op,
                                                           std::unique_ptr<expression_node> operand);
    static std::unique_ptr<expression_node> make_function(const std::string& name,
                                                           std::vector<std::unique_ptr<expression_node>> args);
    static std::unique_ptr<expression_node> make_aggregation(aggregation_function func,
                                                              std::unique_ptr<expression_node> expr);
};

/**
 * Query context for variable resolution
 */
class query_context {
  public:
    void set_variable(const std::string& name, double value);
    void set_time_series(const std::string& name, const time_series& ts);

    std::optional<double> get_variable(const std::string& name) const;
    std::optional<time_series> get_time_series(const std::string& name) const;

    void set_time_range(std::chrono::steady_clock::time_point start,
                         std::chrono::steady_clock::time_point end);

    std::chrono::steady_clock::time_point get_start_time() const { return start_time_; }
    std::chrono::steady_clock::time_point get_end_time() const { return end_time_; }

  private:
    std::unordered_map<std::string, double> variables_;
    std::unordered_map<std::string, time_series> time_series_data_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
};

/**
 * Query parser for SQL-like syntax
 */
class query_parser {
  public:
    struct parsed_query {
        std::vector<std::string> select_metrics;
        std::optional<std::unique_ptr<expression_node>> where_clause;
        std::optional<std::chrono::steady_clock::time_point> from_time;
        std::optional<std::chrono::steady_clock::time_point> to_time;
        std::vector<std::string> group_by_tags;
        std::optional<std::chrono::milliseconds> group_by_time;
        std::optional<aggregation_function> aggregation;
        std::optional<size_t> limit;
        std::vector<std::pair<std::string, bool>> order_by;  // column, ascending
    };

    /**
     * Parse a SQL-like query string
     * @param query Query string
     * @return Parsed query structure
     */
    parsed_query parse(const std::string& query);

    /**
     * Parse an expression string
     * @param expr Expression string
     * @return Expression AST
     */
    std::unique_ptr<expression_node> parse_expression(const std::string& expr);

  private:
    class tokenizer {
      public:
        enum class token_type {
            select, from, where, group, by, order, limit,
            and_op, or_op, not_op,
            eq, ne, lt, le, gt, ge,
            plus, minus, multiply, divide, modulo, power,
            lparen, rparen, comma, dot,
            identifier, number, string_literal,
            end_of_input
        };

        struct token {
            token_type type;
            std::string value;
            size_t position;
        };

        explicit tokenizer(const std::string& input);
        token next_token();
        token peek_token();
        void consume_token();

      private:
        std::string input_;
        size_t position_{0};
        std::optional<token> peeked_;

        void skip_whitespace();
        token read_identifier();
        token read_number();
        token read_string();
    };

    tokenizer tokenizer_;

    // Parsing methods
    parsed_query parse_select_statement();
    std::unique_ptr<expression_node> parse_where_clause();
    std::unique_ptr<expression_node> parse_logical_or();
    std::unique_ptr<expression_node> parse_logical_and();
    std::unique_ptr<expression_node> parse_comparison();
    std::unique_ptr<expression_node> parse_additive();
    std::unique_ptr<expression_node> parse_multiplicative();
    std::unique_ptr<expression_node> parse_unary();
    std::unique_ptr<expression_node> parse_primary();
    std::chrono::steady_clock::time_point parse_timestamp(const std::string& str);
};

/**
 * Query executor
 */
class query_executor {
  public:
    explicit query_executor(metric_database* db);

    /**
     * Execute a parsed query
     * @param query Parsed query
     * @return Query results
     */
    std::vector<time_series> execute(const query_parser::parsed_query& query);

    /**
     * Evaluate an expression in a context
     * @param expr Expression to evaluate
     * @param context Query context
     * @return Evaluation result
     */
    std::variant<double, time_series> evaluate(const expression_node& expr,
                                                 const query_context& context);

  private:
    metric_database* database_;

    // Execution methods
    std::vector<time_series> fetch_metrics(const std::vector<std::string>& metrics,
                                            std::chrono::steady_clock::time_point start,
                                            std::chrono::steady_clock::time_point end);
    std::vector<time_series> apply_where_clause(const std::vector<time_series>& data,
                                                 const expression_node& where_clause);
    std::vector<time_series> apply_grouping(const std::vector<time_series>& data,
                                             const std::vector<std::string>& group_by_tags,
                                             std::optional<std::chrono::milliseconds> group_by_time);
    std::vector<time_series> apply_aggregation(const std::vector<time_series>& data,
                                                aggregation_function func);
    std::vector<time_series> apply_ordering(const std::vector<time_series>& data,
                                             const std::vector<std::pair<std::string, bool>>& order_by);
    std::vector<time_series> apply_limit(const std::vector<time_series>& data, size_t limit);

    // Expression evaluation
    double evaluate_binary_op(binary_operator op, double left, double right);
    double evaluate_unary_op(unary_operator op, double operand);
    double evaluate_function(const std::string& name, const std::vector<double>& args);
    time_series evaluate_aggregation(aggregation_function func, const time_series& ts);
};

/**
 * High-level metric query engine
 */
class metric_query_engine {
  public:
    explicit metric_query_engine(metric_database* db);
    ~metric_query_engine();

    /**
     * Execute a SQL-like query string
     * @param query Query string
     * @return Query results
     */
    std::vector<time_series> query(const std::string& query_string);

    /**
     * Execute a query with parameter binding
     * @param query Query string with placeholders
     * @param params Parameters to bind
     * @return Query results
     */
    std::vector<time_series> query_with_params(
        const std::string& query_string,
        const std::unordered_map<std::string, std::any>& params);

    /**
     * Create a prepared statement
     * @param query Query string
     * @return Statement ID
     */
    size_t prepare(const std::string& query_string);

    /**
     * Execute a prepared statement
     * @param statement_id Statement ID
     * @param params Parameters to bind
     * @return Query results
     */
    std::vector<time_series> execute_prepared(
        size_t statement_id,
        const std::unordered_map<std::string, std::any>& params = {});

    /**
     * Register a custom function
     * @param name Function name
     * @param func Function implementation
     */
    void register_function(const std::string& name,
                           std::function<double(const std::vector<double>&)> func);

    /**
     * Register a custom aggregation
     * @param name Aggregation name
     * @param func Aggregation implementation
     */
    void register_aggregation(const std::string& name,
                              std::function<time_series(const time_series&)> func);

    /**
     * Get query execution plan
     * @param query Query string
     * @return Execution plan description
     */
    std::string explain(const std::string& query_string);

    /**
     * Get query statistics
     * @return Query statistics
     */
    struct query_stats {
        size_t total_queries{0};
        size_t cache_hits{0};
        size_t cache_misses{0};
        double average_execution_time_ms{0.0};
        double max_execution_time_ms{0.0};
        std::vector<std::pair<std::string, size_t>> top_queries;
    };
    query_stats get_stats() const;

    /**
     * Clear query cache
     */
    void clear_cache();

  private:
    metric_database* database_;
    std::unique_ptr<query_parser> parser_;
    std::unique_ptr<query_executor> executor_;

    // Prepared statements
    struct prepared_statement {
        std::string query_template;
        query_parser::parsed_query parsed;
        std::vector<std::string> parameters;
    };
    std::mutex prepared_mutex_;
    std::unordered_map<size_t, prepared_statement> prepared_statements_;
    size_t next_statement_id_{1};

    // Custom functions
    std::mutex functions_mutex_;
    std::unordered_map<std::string, std::function<double(const std::vector<double>&)>> custom_functions_;
    std::unordered_map<std::string, std::function<time_series(const time_series&)>> custom_aggregations_;

    // Query cache
    struct cache_entry {
        std::vector<time_series> results;
        std::chrono::steady_clock::time_point cached_at;
    };
    mutable std::mutex cache_mutex_;
    mutable std::unordered_map<std::string, cache_entry> query_cache_;
    const size_t max_cache_entries_{100};
    const std::chrono::seconds cache_ttl_{60};

    // Statistics
    mutable std::mutex stats_mutex_;
    mutable query_stats stats_;
    mutable std::unordered_map<std::string, size_t> query_counts_;

    // Internal methods
    std::string bind_parameters(const std::string& query_template,
                                 const std::unordered_map<std::string, std::any>& params);
    void update_stats(const std::string& query, std::chrono::milliseconds execution_time);
    void cleanup_cache();
};

/**
 * Query optimization hints
 */
struct query_hints {
    bool use_index{true};
    bool parallel_execution{false};
    size_t max_parallel_tasks{4};
    bool enable_cache{true};
    std::chrono::seconds cache_ttl{60};
    bool push_down_predicates{true};
    bool optimize_aggregations{true};
};

/**
 * Query plan optimizer
 */
class query_optimizer {
  public:
    struct optimization_result {
        query_parser::parsed_query optimized_query;
        std::vector<std::string> optimizations_applied;
        double estimated_cost;
    };

    /**
     * Optimize a parsed query
     * @param query Parsed query
     * @param hints Optimization hints
     * @return Optimization result
     */
    optimization_result optimize(const query_parser::parsed_query& query,
                                  const query_hints& hints = {});

  private:
    // Optimization techniques
    void push_down_predicates(query_parser::parsed_query& query);
    void merge_aggregations(query_parser::parsed_query& query);
    void optimize_time_ranges(query_parser::parsed_query& query);
    void reorder_joins(query_parser::parsed_query& query);
    double estimate_cost(const query_parser::parsed_query& query);
};

}  // namespace monitoring_system