/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "messaging_integration.h"
#include <sstream>
#include <iomanip>

namespace container_module::integration {

#ifdef HAS_PERFORMANCE_METRICS
messaging_integration::metrics messaging_integration::metrics_;
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
std::vector<messaging_integration::container_callback_t> messaging_integration::creation_callbacks_;
std::vector<messaging_integration::container_callback_t> messaging_integration::serialization_callbacks_;
std::mutex messaging_integration::callback_mutex_;
#endif

std::shared_ptr<value_container> messaging_integration::create_optimized_container(
    const std::string& message_type) {

    auto container = std::make_shared<value_container>();
    container->set_message_type(message_type);

#ifdef HAS_PERFORMANCE_METRICS
    metrics_.containers_created.fetch_add(1, std::memory_order_relaxed);
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        for (const auto& callback : creation_callbacks_) {
            if (callback) {
                callback(container);
            }
        }
    }
#endif

    return container;
}

std::string messaging_integration::serialize_for_messaging(
    const std::shared_ptr<value_container>& container,
    bool compress) {

    if (!container) {
        return {};
    }

#ifdef HAS_PERFORMANCE_METRICS
    auto start = std::chrono::high_resolution_clock::now();
#endif

    std::string result = container->serialize();

    // TODO: Add compression support when ENABLE_COMPRESSION is available
    if (compress) {
        // Placeholder for compression implementation
    }

#ifdef HAS_PERFORMANCE_METRICS
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    metrics_.serializations_performed.fetch_add(1, std::memory_order_relaxed);
    metrics_.total_serialize_time_us.fetch_add(duration.count(), std::memory_order_relaxed);
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        for (const auto& callback : serialization_callbacks_) {
            if (callback) {
                callback(container);
            }
        }
    }
#endif

    return result;
}

std::shared_ptr<value_container> messaging_integration::deserialize_from_messaging(
    const std::string& data,
    bool decompress) {

    if (data.empty()) {
        return nullptr;
    }

#ifdef HAS_PERFORMANCE_METRICS
    auto start = std::chrono::high_resolution_clock::now();
#endif

    std::string processed_data = data;

    // TODO: Add decompression support when ENABLE_COMPRESSION is available
    if (decompress) {
        // Placeholder for decompression implementation
    }

    auto container = std::make_shared<value_container>(processed_data);

#ifdef HAS_PERFORMANCE_METRICS
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    metrics_.deserializations_performed.fetch_add(1, std::memory_order_relaxed);
    metrics_.total_deserialize_time_us.fetch_add(duration.count(), std::memory_order_relaxed);
#endif

    return container;
}

#ifdef HAS_PERFORMANCE_METRICS
messaging_integration::metrics& messaging_integration::get_metrics() {
    return metrics_;
}

void messaging_integration::reset_metrics() {
    metrics_.containers_created.store(0, std::memory_order_relaxed);
    metrics_.serializations_performed.store(0, std::memory_order_relaxed);
    metrics_.deserializations_performed.store(0, std::memory_order_relaxed);
    metrics_.total_serialize_time_us.store(0, std::memory_order_relaxed);
    metrics_.total_deserialize_time_us.store(0, std::memory_order_relaxed);
}

std::string messaging_integration::get_metrics_summary() {
    std::ostringstream oss;

    auto containers = metrics_.containers_created.load(std::memory_order_relaxed);
    auto serializations = metrics_.serializations_performed.load(std::memory_order_relaxed);
    auto deserializations = metrics_.deserializations_performed.load(std::memory_order_relaxed);
    auto serialize_time = metrics_.total_serialize_time_us.load(std::memory_order_relaxed);
    auto deserialize_time = metrics_.total_deserialize_time_us.load(std::memory_order_relaxed);

    oss << "Container System Metrics:\n";
    oss << "  Containers created: " << containers << "\n";
    oss << "  Serializations: " << serializations << "\n";
    oss << "  Deserializations: " << deserializations << "\n";

    if (serializations > 0) {
        oss << "  Avg serialize time: " << std::fixed << std::setprecision(2)
            << (static_cast<double>(serialize_time) / serializations) << " μs\n";
    }

    if (deserializations > 0) {
        oss << "  Avg deserialize time: " << std::fixed << std::setprecision(2)
            << (static_cast<double>(deserialize_time) / deserializations) << " μs\n";
    }

    return oss.str();
}
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
void messaging_integration::register_creation_callback(container_callback_t callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    creation_callbacks_.push_back(std::move(callback));
}

void messaging_integration::register_serialization_callback(container_callback_t callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    serialization_callbacks_.push_back(std::move(callback));
}

void messaging_integration::unregister_callbacks() {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    creation_callbacks_.clear();
    serialization_callbacks_.clear();
}
#endif

// messaging_container_builder implementation

messaging_container_builder::messaging_container_builder()
    : container_(std::make_shared<value_container>()) {
}

messaging_container_builder& messaging_container_builder::source(
    const std::string& id, const std::string& sub_id) {
    container_->set_source(id, sub_id);
    return *this;
}

messaging_container_builder& messaging_container_builder::target(
    const std::string& id, const std::string& sub_id) {
    container_->set_target(id, sub_id);
    return *this;
}

messaging_container_builder& messaging_container_builder::message_type(const std::string& type) {
    container_->set_message_type(type);
    return *this;
}

messaging_container_builder& messaging_container_builder::optimize_for_size() {
    size_optimized_ = true;
    speed_optimized_ = false;
    return *this;
}

messaging_container_builder& messaging_container_builder::optimize_for_speed() {
    speed_optimized_ = true;
    size_optimized_ = false;
    return *this;
}

std::shared_ptr<value_container> messaging_container_builder::build() {
    // Apply optimizations based on settings
    if (size_optimized_) {
        // TODO: Apply size optimizations
    } else if (speed_optimized_) {
        // TODO: Apply speed optimizations
    }

    auto result = container_;
    container_ = std::make_shared<value_container>();  // Reset for potential reuse
    return result;
}

// container_performance_monitor implementation

container_performance_monitor::container_performance_monitor(const std::string& operation_name)
    : operation_name_(operation_name)
    , start_time_(std::chrono::high_resolution_clock::now()) {
}

container_performance_monitor::~container_performance_monitor() {
#ifdef HAS_PERFORMANCE_METRICS
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);

    // This could be extended to log to a performance monitoring system
    // For now, it's just a placeholder for future monitoring integration
#endif
}

void container_performance_monitor::set_container_size(size_t size) {
    container_size_ = size;
}

void container_performance_monitor::set_result_size(size_t size) {
    result_size_ = size;
}

} // namespace container_module::integration