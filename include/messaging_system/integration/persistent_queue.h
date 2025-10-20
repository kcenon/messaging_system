#pragma once

#include <kcenon/database/database_manager.h>
#include <kcenon/database/orm/entity_manager.h>
#include <kcenon/common/patterns/result.h>
#include "../core/messaging_container.h"
#include <chrono>
#include <string>
#include <vector>

namespace messaging {

struct MessageEntity {
    int64_t id;
    std::string topic;
    std::string payload;
    std::string status;  // PENDING, PROCESSING, COMPLETED, FAILED
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    int retry_count;

    static database::entity_metadata metadata();
};

struct ConsumerOffset {
    std::string consumer_group;
    std::string topic;
    int64_t offset;
    std::chrono::system_clock::time_point updated_at;

    static database::entity_metadata metadata();
};

class PersistentMessageQueue {
    std::shared_ptr<database::connection_pool> pool_;
    std::shared_ptr<database::entity_manager> entity_manager_;

public:
    PersistentMessageQueue(
        std::shared_ptr<database::connection_pool> pool
    );

    // Queue operations
    common::Result<void> enqueue(const MessagingContainer& msg);
    common::Result<std::vector<MessageEntity>> dequeue_batch(int limit = 100);

    // Status updates
    common::Result<void> mark_completed(int64_t message_id);
    common::Result<void> mark_failed(int64_t message_id);

    // Reprocessing
    common::Result<void> reprocess_failed(int max_retries = 3);
    common::Result<size_t> count_pending() const;

    // Consumer offset management
    common::Result<void> save_offset(const std::string& group, const std::string& topic, int64_t offset);
    common::Result<int64_t> load_offset(const std::string& group, const std::string& topic);
};

} // namespace messaging
