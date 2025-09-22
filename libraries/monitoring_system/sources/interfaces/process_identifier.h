#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <cstdint>
#include <string>
#include <chrono>
#include <functional>

namespace monitoring_interface {

/**
 * @struct process_identifier
 * @brief 프로세스 식별자
 * 
 * 프로세스를 고유하게 식별하기 위한 구조체
 * PID, 프로세스명, 시작시간을 조합하여 고유성 보장
 */
struct process_identifier {
    std::uint32_t pid{0};                    // 프로세스 ID
    std::string process_name;                // 프로세스 이름
    std::chrono::steady_clock::time_point start_time; // 시작 시간
    
    bool operator==(const process_identifier& other) const {
        return pid == other.pid && process_name == other.process_name;
    }
    
    bool operator!=(const process_identifier& other) const {
        return !(*this == other);
    }
    
    bool operator<(const process_identifier& other) const {
        if (pid != other.pid) return pid < other.pid;
        return process_name < other.process_name;
    }
};

/**
 * @struct thread_pool_identifier
 * @brief Thread pool 식별자
 * 
 * Thread pool을 고유하게 식별하기 위한 구조체
 * 소속 프로세스 정보와 pool 고유 정보 포함
 */
struct thread_pool_identifier {
    process_identifier process_id;           // 소속 프로세스
    std::string pool_name;                   // Pool 이름
    std::uint32_t pool_instance_id{0};       // Pool 인스턴스 ID
    
    bool operator==(const thread_pool_identifier& other) const {
        return process_id == other.process_id && 
               pool_name == other.pool_name && 
               pool_instance_id == other.pool_instance_id;
    }
    
    bool operator!=(const thread_pool_identifier& other) const {
        return !(*this == other);
    }
    
    bool operator<(const thread_pool_identifier& other) const {
        if (process_id != other.process_id) return process_id < other.process_id;
        if (pool_name != other.pool_name) return pool_name < other.pool_name;
        return pool_instance_id < other.pool_instance_id;
    }
};

} // namespace monitoring_interface

// Hash 특수화
namespace std {
    template<>
    struct hash<monitoring_interface::process_identifier> {
        std::size_t operator()(const monitoring_interface::process_identifier& id) const {
            std::size_t h1 = std::hash<std::uint32_t>{}(id.pid);
            std::size_t h2 = std::hash<std::string>{}(id.process_name);
            return h1 ^ (h2 << 1);
        }
    };
    
    template<>
    struct hash<monitoring_interface::thread_pool_identifier> {
        std::size_t operator()(const monitoring_interface::thread_pool_identifier& id) const {
            std::size_t h1 = std::hash<monitoring_interface::process_identifier>{}(id.process_id);
            std::size_t h2 = std::hash<std::string>{}(id.pool_name);
            std::size_t h3 = std::hash<std::uint32_t>{}(id.pool_instance_id);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}