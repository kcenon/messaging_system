#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <queue>
#include <optional>
#include <variant>
#include <regex>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <future>
#include <array>
#include <cstring>
#include <ctime>

namespace kcenon::monitoring::web {

// HTTP method types
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

// HTTP status codes
enum class HttpStatus {
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    CONFLICT = 409,
    TOO_MANY_REQUESTS = 429,
    INTERNAL_SERVER_ERROR = 500,
    SERVICE_UNAVAILABLE = 503
};

// WebSocket opcodes
enum class WebSocketOpcode {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
};

// WebSocket close codes
enum class WebSocketCloseCode {
    NORMAL = 1000,
    GOING_AWAY = 1001,
    PROTOCOL_ERROR = 1002,
    UNSUPPORTED_DATA = 1003,
    INVALID_PAYLOAD = 1007,
    POLICY_VIOLATION = 1008,
    MESSAGE_TOO_BIG = 1009,
    INTERNAL_ERROR = 1011
};

// HTTP request
struct HttpRequest {
    HttpMethod method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::string body;
    std::string client_ip;
    uint16_t client_port;
    std::chrono::system_clock::time_point received_at;
};

// HTTP response
struct HttpResponse {
    HttpStatus status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool keep_alive = true;
};

// WebSocket frame
struct WebSocketFrame {
    bool fin;
    WebSocketOpcode opcode;
    bool masked;
    std::vector<uint8_t> payload;
    std::array<uint8_t, 4> masking_key;
};

// WebSocket message
struct WebSocketMessage {
    WebSocketOpcode type;
    std::string data;
    std::chrono::system_clock::time_point timestamp;
};

// Session information
struct SessionInfo {
    std::string session_id;
    std::string user_id;
    std::string ip_address;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_activity;
    std::unordered_map<std::string, std::string> attributes;
    bool is_authenticated = false;
};

// Rate limit configuration
struct RateLimitConfig {
    size_t requests_per_minute = 60;
    size_t burst_size = 100;
    std::chrono::seconds window_size = std::chrono::seconds(60);
    bool enabled = true;
};

// CORS configuration
struct CorsConfig {
    std::vector<std::string> allowed_origins;
    std::vector<std::string> allowed_methods;
    std::vector<std::string> allowed_headers;
    std::vector<std::string> exposed_headers;
    std::chrono::seconds max_age = std::chrono::seconds(3600);
    bool allow_credentials = false;
    bool enabled = true;
};

// Authentication configuration
struct AuthConfig {
    enum AuthType { NONE, BASIC, BEARER, API_KEY, SESSION };
    AuthType type = NONE;
    std::string realm = "Monitoring Dashboard";
    std::function<bool(const std::string&, const std::string&)> validate_credentials;
    std::function<bool(const std::string&)> validate_token;
    std::function<bool(const std::string&)> validate_api_key;
    std::chrono::seconds session_timeout = std::chrono::seconds(3600);
    bool enabled = false;
};

// Route handler function types
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;
using WebSocketHandler = std::function<void(const std::string& client_id, const WebSocketMessage&)>;
using MiddlewareHandler = std::function<bool(HttpRequest&, HttpResponse&)>;

// Route definition
struct Route {
    std::string path_pattern;
    HttpMethod method;
    HttpHandler handler;
    std::vector<MiddlewareHandler> middlewares;
    bool requires_auth = false;
    std::regex path_regex;
};

// WebSocket client connection
class WebSocketConnection {
public:
    WebSocketConnection(const std::string& id, int socket_fd);
    ~WebSocketConnection();

    // Send message to client
    bool send_text(const std::string& message);
    bool send_binary(const std::vector<uint8_t>& data);
    bool send_ping(const std::vector<uint8_t>& data = {});
    bool send_pong(const std::vector<uint8_t>& data = {});
    bool send_close(WebSocketCloseCode code, const std::string& reason = "");

    // Connection management
    void close();
    bool is_connected() const { return connected_.load(); }
    const std::string& get_id() const { return id_; }

    // Message handling
    void set_message_handler(WebSocketHandler handler);
    void process_incoming_data(const std::vector<uint8_t>& data);

private:
    std::string id_;
    int socket_fd_;
    std::atomic<bool> connected_{true};
    WebSocketHandler message_handler_;
    std::mutex send_mutex_;
    std::vector<uint8_t> receive_buffer_;

    bool send_frame(const WebSocketFrame& frame);
    std::optional<WebSocketFrame> parse_frame(const std::vector<uint8_t>& data);
    void handle_control_frame(const WebSocketFrame& frame);
};

// HTTP/WebSocket server
class DashboardServer {
public:
    DashboardServer(uint16_t port = 8080);
    ~DashboardServer();

    // Server lifecycle
    bool start();
    void stop();
    bool is_running() const { return running_.load(); }

    // Route registration
    void add_route(const std::string& path, HttpMethod method, HttpHandler handler);
    void add_route(const Route& route);
    void add_static_route(const std::string& url_prefix, const std::string& directory);

    // WebSocket endpoints
    void add_websocket_endpoint(const std::string& path, WebSocketHandler handler);
    void broadcast_to_websockets(const std::string& path, const std::string& message);
    void send_to_websocket(const std::string& client_id, const std::string& message);

    // Middleware
    void add_global_middleware(MiddlewareHandler middleware);
    void add_route_middleware(const std::string& path, MiddlewareHandler middleware);

    // Authentication & Authorization
    void set_auth_config(const AuthConfig& config);
    bool authenticate_request(const HttpRequest& request) const;
    std::string create_session(const std::string& user_id);
    bool validate_session(const std::string& session_id) const;
    void invalidate_session(const std::string& session_id);

    // Rate limiting
    void set_rate_limit_config(const RateLimitConfig& config);
    bool check_rate_limit(const std::string& client_ip);

    // CORS configuration
    void set_cors_config(const CorsConfig& config);
    void apply_cors_headers(HttpResponse& response, const HttpRequest& request) const;

    // Server configuration
    void set_max_connections(size_t max_connections);
    void set_request_timeout(std::chrono::seconds timeout);
    void set_keep_alive_timeout(std::chrono::seconds timeout);
    void set_max_request_size(size_t max_size);

    // Statistics
    struct ServerStats {
        size_t total_requests = 0;
        size_t active_connections = 0;
        size_t websocket_connections = 0;
        std::unordered_map<int, size_t> status_counts;
        std::chrono::system_clock::time_point start_time;
        double average_response_time_ms = 0.0;
    };

    ServerStats get_stats() const;

private:
    uint16_t port_;
    int server_socket_;
    std::atomic<bool> running_{false};
    std::thread accept_thread_;
    std::vector<std::thread> worker_threads_;

    // Connection management
    std::mutex connections_mutex_;
    std::unordered_map<int, std::shared_ptr<SessionInfo>> connections_;
    std::unordered_map<std::string, std::shared_ptr<WebSocketConnection>> websocket_connections_;

    // Routing
    std::mutex routes_mutex_;
    std::vector<Route> routes_;
    std::unordered_map<std::string, WebSocketHandler> websocket_endpoints_;
    std::unordered_map<std::string, std::string> static_routes_;
    std::vector<MiddlewareHandler> global_middlewares_;

    // Session management
    mutable std::mutex sessions_mutex_;
    std::unordered_map<std::string, SessionInfo> sessions_;

    // Rate limiting
    mutable std::mutex rate_limit_mutex_;
    std::unordered_map<std::string, std::deque<std::chrono::system_clock::time_point>> rate_limit_buckets_;

    // Configuration
    AuthConfig auth_config_;
    RateLimitConfig rate_limit_config_;
    CorsConfig cors_config_;
    size_t max_connections_ = 1000;
    std::chrono::seconds request_timeout_ = std::chrono::seconds(30);
    std::chrono::seconds keep_alive_timeout_ = std::chrono::seconds(60);
    size_t max_request_size_ = 10 * 1024 * 1024; // 10MB

    // Statistics
    mutable std::mutex stats_mutex_;
    ServerStats stats_;

    // Worker pool
    std::queue<int> connection_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // Server operations
    void accept_connections();
    void worker_loop();
    void handle_connection(int client_socket);
    HttpResponse handle_request(const HttpRequest& request);
    bool handle_websocket_upgrade(int client_socket, const HttpRequest& request);

    // Request parsing
    std::optional<HttpRequest> parse_request(const std::string& raw_request);
    std::string build_response(const HttpResponse& response) const;

    // Route matching
    std::optional<Route> find_route(const std::string& path, HttpMethod method) const;
    bool match_path_pattern(const std::string& pattern, const std::string& path) const;

    // Static file serving
    HttpResponse serve_static_file(const std::string& filepath) const;
    std::string get_content_type(const std::string& extension) const;

    // Utility methods
    std::string generate_session_id() const;
    std::string get_http_date() const;
    void cleanup_expired_sessions();
};

// HTTP response builder for fluent API
class ResponseBuilder {
public:
    ResponseBuilder(HttpStatus status = HttpStatus::OK) {
        response_.status = status;
    }

    ResponseBuilder& status(HttpStatus s) {
        response_.status = s;
        return *this;
    }

    ResponseBuilder& header(const std::string& key, const std::string& value) {
        response_.headers[key] = value;
        return *this;
    }

    ResponseBuilder& content_type(const std::string& type) {
        response_.headers["Content-Type"] = type;
        return *this;
    }

    ResponseBuilder& json(const std::string& json_body) {
        response_.headers["Content-Type"] = "application/json";
        response_.body = json_body;
        return *this;
    }

    ResponseBuilder& html(const std::string& html_body) {
        response_.headers["Content-Type"] = "text/html; charset=utf-8";
        response_.body = html_body;
        return *this;
    }

    ResponseBuilder& text(const std::string& text_body) {
        response_.headers["Content-Type"] = "text/plain; charset=utf-8";
        response_.body = text_body;
        return *this;
    }

    ResponseBuilder& body(const std::string& b) {
        response_.body = b;
        return *this;
    }

    ResponseBuilder& keep_alive(bool ka) {
        response_.keep_alive = ka;
        return *this;
    }

    HttpResponse build() const {
        return response_;
    }

private:
    HttpResponse response_;
};

// URL router for organizing routes
class Router {
public:
    Router(const std::string& prefix = "") : prefix_(prefix) {}

    void get(const std::string& path, HttpHandler handler) {
        add_route(path, HttpMethod::GET, handler);
    }

    void post(const std::string& path, HttpHandler handler) {
        add_route(path, HttpMethod::POST, handler);
    }

    void put(const std::string& path, HttpHandler handler) {
        add_route(path, HttpMethod::PUT, handler);
    }

    void del(const std::string& path, HttpHandler handler) {
        add_route(path, HttpMethod::DELETE, handler);
    }

    void use(MiddlewareHandler middleware) {
        middlewares_.push_back(middleware);
    }

    std::vector<Route> get_routes() const {
        return routes_;
    }

private:
    std::string prefix_;
    std::vector<Route> routes_;
    std::vector<MiddlewareHandler> middlewares_;

    void add_route(const std::string& path, HttpMethod method, HttpHandler handler) {
        Route route;
        route.path_pattern = prefix_ + path;
        route.method = method;
        route.handler = handler;
        route.middlewares = middlewares_;
        routes_.push_back(route);
    }
};

} // namespace monitoring_system::web