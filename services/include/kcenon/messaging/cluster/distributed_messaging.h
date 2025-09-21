#pragma once

#include "../core/message_types.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace kcenon::messaging::cluster {

    // Node status
    enum class node_status {
        unknown,
        joining,
        active,
        leaving,
        failed,
        maintenance
    };

    // Node information
    struct cluster_node {
        std::string node_id;
        std::string address;
        uint16_t port;
        node_status status = node_status::unknown;
        std::chrono::steady_clock::time_point last_heartbeat;
        std::chrono::steady_clock::time_point joined_time;
        std::atomic<uint64_t> message_count{0};
        std::atomic<uint64_t> load_factor{0};  // 0-100 percentage
        std::unordered_map<std::string, std::string> metadata;

        cluster_node(const std::string& id, const std::string& addr, uint16_t p)
            : node_id(id), address(addr), port(p),
              last_heartbeat(std::chrono::steady_clock::now()),
              joined_time(std::chrono::steady_clock::now()) {}

        bool is_alive() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat);
            return elapsed.count() < 30;  // 30 second timeout
        }

        double uptime_hours() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - joined_time);
            return elapsed.count();
        }
    };

    // Cluster membership management
    class cluster_membership {
    private:
        mutable std::shared_mutex nodes_mutex_;
        std::unordered_map<std::string, std::unique_ptr<cluster_node>> nodes_;
        std::string local_node_id_;
        std::atomic<size_t> cluster_size_{0};

        // Heartbeat management
        std::atomic<bool> heartbeat_running_{false};
        std::thread heartbeat_thread_;
        std::condition_variable heartbeat_cv_;
        std::mutex heartbeat_mutex_;

        std::function<void(const cluster_node&)> node_joined_callback_;
        std::function<void(const cluster_node&)> node_left_callback_;
        std::function<void(const cluster_node&)> node_failed_callback_;

    public:
        explicit cluster_membership(const std::string& local_id) : local_node_id_(local_id) {}

        ~cluster_membership() {
            stop_heartbeat();
        }

        // Node management
        void add_node(std::unique_ptr<cluster_node> node) {
            std::unique_lock<std::shared_mutex> lock(nodes_mutex_);
            std::string node_id = node->node_id;
            node->status = node_status::active;
            nodes_[node_id] = std::move(node);
            cluster_size_ = nodes_.size();

            if (node_joined_callback_) {
                node_joined_callback_(*nodes_[node_id]);
            }
        }

        bool remove_node(const std::string& node_id) {
            std::unique_lock<std::shared_mutex> lock(nodes_mutex_);
            auto it = nodes_.find(node_id);
            if (it != nodes_.end()) {
                if (node_left_callback_) {
                    node_left_callback_(*it->second);
                }
                nodes_.erase(it);
                cluster_size_ = nodes_.size();
                return true;
            }
            return false;
        }

        cluster_node* get_node(const std::string& node_id) const {
            std::shared_lock<std::shared_mutex> lock(nodes_mutex_);
            auto it = nodes_.find(node_id);
            return it != nodes_.end() ? it->second.get() : nullptr;
        }

        std::vector<cluster_node*> get_all_nodes() const {
            std::shared_lock<std::shared_mutex> lock(nodes_mutex_);
            std::vector<cluster_node*> result;
            for (const auto& [id, node] : nodes_) {
                result.push_back(node.get());
            }
            return result;
        }

        std::vector<cluster_node*> get_active_nodes() const {
            std::shared_lock<std::shared_mutex> lock(nodes_mutex_);
            std::vector<cluster_node*> result;
            for (const auto& [id, node] : nodes_) {
                if (node->status == node_status::active && node->is_alive()) {
                    result.push_back(node.get());
                }
            }
            return result;
        }

        size_t get_cluster_size() const { return cluster_size_; }
        const std::string& get_local_node_id() const { return local_node_id_; }

        // Heartbeat management
        void start_heartbeat() {
            if (heartbeat_running_) return;

            heartbeat_running_ = true;
            heartbeat_thread_ = std::thread([this]() { heartbeat_loop(); });
        }

        void stop_heartbeat() {
            if (!heartbeat_running_) return;

            heartbeat_running_ = false;
            heartbeat_cv_.notify_all();
            if (heartbeat_thread_.joinable()) {
                heartbeat_thread_.join();
            }
        }

        void update_heartbeat(const std::string& node_id) {
            auto node = get_node(node_id);
            if (node) {
                node->last_heartbeat = std::chrono::steady_clock::now();
            }
        }

        // Callbacks for cluster events
        void set_node_joined_callback(std::function<void(const cluster_node&)> callback) {
            node_joined_callback_ = std::move(callback);
        }

        void set_node_left_callback(std::function<void(const cluster_node&)> callback) {
            node_left_callback_ = std::move(callback);
        }

        void set_node_failed_callback(std::function<void(const cluster_node&)> callback) {
            node_failed_callback_ = std::move(callback);
        }

    private:
        void heartbeat_loop() {
            while (heartbeat_running_) {
                check_node_health();

                std::unique_lock<std::mutex> lock(heartbeat_mutex_);
                heartbeat_cv_.wait_for(lock, std::chrono::seconds(10));
            }
        }

        void check_node_health() {
            std::shared_lock<std::shared_mutex> lock(nodes_mutex_);
            std::vector<std::string> failed_nodes;

            for (const auto& [id, node] : nodes_) {
                if (!node->is_alive() && node->status == node_status::active) {
                    node->status = node_status::failed;
                    failed_nodes.push_back(id);
                }
            }

            lock.unlock();

            // Notify about failed nodes
            for (const auto& node_id : failed_nodes) {
                auto node = get_node(node_id);
                if (node && node_failed_callback_) {
                    node_failed_callback_(*node);
                }
            }
        }
    };

    // Load balancing strategies
    class load_balancer {
    public:
        virtual ~load_balancer() = default;
        virtual cluster_node* select_node(const std::vector<cluster_node*>& available_nodes,
                                         const core::message& message) = 0;
        virtual std::string get_strategy_name() const = 0;
    };

    // Round-robin load balancer
    class round_robin_balancer : public load_balancer {
    private:
        mutable std::atomic<size_t> current_index_{0};

    public:
        cluster_node* select_node(const std::vector<cluster_node*>& available_nodes,
                                const core::message& message) override {
            if (available_nodes.empty()) return nullptr;

            size_t index = current_index_++ % available_nodes.size();
            return available_nodes[index];
        }

        std::string get_strategy_name() const override { return "RoundRobin"; }
    };

    // Least loaded balancer
    class least_loaded_balancer : public load_balancer {
    public:
        cluster_node* select_node(const std::vector<cluster_node*>& available_nodes,
                                const core::message& message) override {
            if (available_nodes.empty()) return nullptr;

            cluster_node* best_node = available_nodes[0];
            uint64_t min_load = best_node->load_factor;

            for (auto* node : available_nodes) {
                if (node->load_factor < min_load) {
                    min_load = node->load_factor;
                    best_node = node;
                }
            }

            return best_node;
        }

        std::string get_strategy_name() const override { return "LeastLoaded"; }
    };

    // Consistent hash balancer
    class consistent_hash_balancer : public load_balancer {
    private:
        std::hash<std::string> hasher_;

    public:
        cluster_node* select_node(const std::vector<cluster_node*>& available_nodes,
                                const core::message& message) override {
            if (available_nodes.empty()) return nullptr;

            // Use topic or sender for consistent hashing
            std::string hash_key = message.payload.topic + message.metadata.sender;
            size_t hash_value = hasher_(hash_key);
            size_t index = hash_value % available_nodes.size();

            return available_nodes[index];
        }

        std::string get_strategy_name() const override { return "ConsistentHash"; }
    };

    // Distributed message broker
    class distributed_broker {
    private:
        std::unique_ptr<cluster_membership> membership_;
        std::unique_ptr<load_balancer> load_balancer_;
        std::function<void(const core::message&, const std::string&)> message_forwarder_;

        // Message replication
        size_t replication_factor_ = 1;
        bool enable_replication_ = false;

        // Statistics
        mutable std::atomic<uint64_t> messages_distributed_{0};
        mutable std::atomic<uint64_t> messages_replicated_{0};
        mutable std::atomic<uint64_t> node_failures_{0};

    public:
        explicit distributed_broker(const std::string& local_node_id)
            : membership_(std::make_unique<cluster_membership>(local_node_id)),
              load_balancer_(std::make_unique<round_robin_balancer>()) {

            // Set up cluster event handlers
            membership_->set_node_failed_callback([this](const cluster_node& node) {
                handle_node_failure(node);
            });
        }

        // Cluster management
        void join_cluster() {
            membership_->start_heartbeat();
        }

        void leave_cluster() {
            membership_->stop_heartbeat();
        }

        void add_node(const std::string& node_id, const std::string& address, uint16_t port) {
            auto node = std::make_unique<cluster_node>(node_id, address, port);
            membership_->add_node(std::move(node));
        }

        void remove_node(const std::string& node_id) {
            membership_->remove_node(node_id);
        }

        // Load balancing configuration
        void set_load_balancer(std::unique_ptr<load_balancer> balancer) {
            load_balancer_ = std::move(balancer);
        }

        void set_message_forwarder(std::function<void(const core::message&, const std::string&)> forwarder) {
            message_forwarder_ = std::move(forwarder);
        }

        // Replication configuration
        void enable_replication(size_t factor) {
            enable_replication_ = true;
            replication_factor_ = factor;
        }

        void disable_replication() {
            enable_replication_ = false;
        }

        // Message distribution
        bool distribute_message(const core::message& message) {
            auto active_nodes = membership_->get_active_nodes();
            if (active_nodes.empty()) {
                return false;  // No available nodes
            }

            // Select primary node for the message
            auto* primary_node = load_balancer_->select_node(active_nodes, message);
            if (!primary_node) {
                return false;
            }

            // Forward to primary node
            if (message_forwarder_) {
                message_forwarder_(message, primary_node->node_id);
                messages_distributed_++;
                primary_node->message_count++;
            }

            // Handle replication
            if (enable_replication_ && replication_factor_ > 1) {
                replicate_message(message, primary_node, active_nodes);
            }

            return true;
        }

        // Cluster information
        cluster_membership* get_membership() const { return membership_.get(); }

        struct cluster_statistics {
            size_t total_nodes;
            size_t active_nodes;
            size_t failed_nodes;
            uint64_t messages_distributed;
            uint64_t messages_replicated;
            uint64_t node_failures;
            std::string load_balancer_strategy;
            std::vector<std::pair<std::string, uint64_t>> node_message_counts;
        };

        cluster_statistics get_statistics() const {
            cluster_statistics stats;
            stats.total_nodes = membership_->get_cluster_size();
            stats.messages_distributed = messages_distributed_;
            stats.messages_replicated = messages_replicated_;
            stats.node_failures = node_failures_;
            stats.load_balancer_strategy = load_balancer_->get_strategy_name();

            auto all_nodes = membership_->get_all_nodes();
            stats.active_nodes = 0;
            stats.failed_nodes = 0;

            for (const auto* node : all_nodes) {
                if (node->status == node_status::active && node->is_alive()) {
                    stats.active_nodes++;
                } else if (node->status == node_status::failed) {
                    stats.failed_nodes++;
                }

                stats.node_message_counts.emplace_back(node->node_id, node->message_count.load());
            }

            return stats;
        }

    private:
        void replicate_message(const core::message& message,
                             const cluster_node* primary_node,
                             const std::vector<cluster_node*>& available_nodes) {

            size_t replicas_sent = 0;
            size_t target_replicas = std::min(replication_factor_ - 1, available_nodes.size() - 1);

            for (auto* node : available_nodes) {
                if (node != primary_node && replicas_sent < target_replicas) {
                    if (message_forwarder_) {
                        message_forwarder_(message, node->node_id);
                        messages_replicated_++;
                        replicas_sent++;
                    }
                }
            }
        }

        void handle_node_failure(const cluster_node& failed_node) {
            node_failures_++;
            // Here you could implement failure recovery logic
            // such as redistributing messages from the failed node
        }
    };

    // Distributed broker builder
    class distributed_broker_builder {
    private:
        std::unique_ptr<distributed_broker> broker_;

    public:
        explicit distributed_broker_builder(const std::string& local_node_id)
            : broker_(std::make_unique<distributed_broker>(local_node_id)) {}

        distributed_broker_builder& with_round_robin_balancing() {
            broker_->set_load_balancer(std::make_unique<round_robin_balancer>());
            return *this;
        }

        distributed_broker_builder& with_least_loaded_balancing() {
            broker_->set_load_balancer(std::make_unique<least_loaded_balancer>());
            return *this;
        }

        distributed_broker_builder& with_consistent_hash_balancing() {
            broker_->set_load_balancer(std::make_unique<consistent_hash_balancer>());
            return *this;
        }

        distributed_broker_builder& with_replication(size_t factor) {
            broker_->enable_replication(factor);
            return *this;
        }

        distributed_broker_builder& with_message_forwarder(
            std::function<void(const core::message&, const std::string&)> forwarder) {
            broker_->set_message_forwarder(std::move(forwarder));
            return *this;
        }

        std::unique_ptr<distributed_broker> build() {
            return std::move(broker_);
        }
    };

} // namespace kcenon::messaging::cluster