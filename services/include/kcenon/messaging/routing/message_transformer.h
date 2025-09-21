#pragma once

#include "../core/message_types.h"
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>

namespace kcenon::messaging::routing {

    // Transformation result
    enum class transform_result {
        success,
        skip,      // Skip this message
        error      // Error occurred
    };

    struct transformation_context {
        const core::message& original_message;
        std::unordered_map<std::string, std::string> metadata;
        size_t transformation_count = 0;
    };

    // Message transformer interface
    class message_transformer {
    public:
        virtual ~message_transformer() = default;
        virtual transform_result transform(const transformation_context& context,
                                         core::message& message) const = 0;
        virtual std::string get_name() const = 0;
        virtual std::string describe() const = 0;
    };

    // Topic rewriter transformer
    class topic_rewriter : public message_transformer {
    private:
        std::string pattern_;
        std::string replacement_;

    public:
        topic_rewriter(const std::string& pattern, const std::string& replacement)
            : pattern_(pattern), replacement_(replacement) {}

        transform_result transform(const transformation_context& context,
                                 core::message& message) const override {
            try {
                std::regex regex_pattern(pattern_);
                message.payload.topic = std::regex_replace(message.payload.topic,
                                                         regex_pattern, replacement_);
                return transform_result::success;
            } catch (const std::exception&) {
                return transform_result::error;
            }
        }

        std::string get_name() const override { return "TopicRewriter"; }
        std::string describe() const override {
            return "Rewrite topic: " + pattern_ + " -> " + replacement_;
        }
    };

    // Data enricher transformer
    class data_enricher : public message_transformer {
    private:
        std::unordered_map<std::string, core::message_value> enrichment_data_;

    public:
        void add_enrichment(const std::string& key, const core::message_value& value) {
            enrichment_data_[key] = value;
        }

        transform_result transform(const transformation_context& context,
                                 core::message& message) const override {
            for (const auto& [key, value] : enrichment_data_) {
                message.payload.data[key] = value;
            }
            return transform_result::success;
        }

        std::string get_name() const override { return "DataEnricher"; }
        std::string describe() const override {
            return "Enrich data with " + std::to_string(enrichment_data_.size()) + " fields";
        }
    };

    // Priority adjuster transformer
    class priority_adjuster : public message_transformer {
    private:
        core::message_priority new_priority_;

    public:
        explicit priority_adjuster(core::message_priority priority)
            : new_priority_(priority) {}

        transform_result transform(const transformation_context& context,
                                 core::message& message) const override {
            message.metadata.priority = new_priority_;
            return transform_result::success;
        }

        std::string get_name() const override { return "PriorityAdjuster"; }
        std::string describe() const override {
            return "Set priority to " + std::to_string(static_cast<int>(new_priority_));
        }
    };

    // Content validator transformer
    class content_validator : public message_transformer {
    private:
        std::vector<std::string> required_fields_;

    public:
        void add_required_field(const std::string& field) {
            required_fields_.push_back(field);
        }

        transform_result transform(const transformation_context& context,
                                 core::message& message) const override {
            for (const auto& field : required_fields_) {
                if (message.payload.data.find(field) == message.payload.data.end()) {
                    return transform_result::skip;  // Skip invalid messages
                }
            }
            return transform_result::success;
        }

        std::string get_name() const override { return "ContentValidator"; }
        std::string describe() const override {
            return "Validate " + std::to_string(required_fields_.size()) + " required fields";
        }
    };

    // Custom function transformer
    class function_transformer : public message_transformer {
    private:
        std::string name_;
        std::function<transform_result(const transformation_context&, core::message&)> transform_func_;

    public:
        function_transformer(const std::string& name,
                           std::function<transform_result(const transformation_context&, core::message&)> func)
            : name_(name), transform_func_(std::move(func)) {}

        transform_result transform(const transformation_context& context,
                                 core::message& message) const override {
            return transform_func_(context, message);
        }

        std::string get_name() const override { return name_; }
        std::string describe() const override { return "Custom: " + name_; }
    };

    // Transformation pipeline
    class transformation_pipeline {
    private:
        std::vector<std::unique_ptr<message_transformer>> transformers_;
        std::string name_;
        mutable std::atomic<uint64_t> messages_processed_{0};
        mutable std::atomic<uint64_t> messages_transformed_{0};
        mutable std::atomic<uint64_t> messages_skipped_{0};
        mutable std::atomic<uint64_t> messages_errored_{0};

    public:
        explicit transformation_pipeline(const std::string& name) : name_(name) {}

        void add_transformer(std::unique_ptr<message_transformer> transformer) {
            transformers_.push_back(std::move(transformer));
        }

        const std::string& get_name() const { return name_; }

        transform_result process_message(core::message& message) const {
            messages_processed_++;

            transformation_context context{message};
            bool any_transformation = false;

            for (const auto& transformer : transformers_) {
                context.transformation_count++;
                auto result = transformer->transform(context, message);

                switch (result) {
                    case transform_result::success:
                        any_transformation = true;
                        break;
                    case transform_result::skip:
                        messages_skipped_++;
                        return transform_result::skip;
                    case transform_result::error:
                        messages_errored_++;
                        return transform_result::error;
                }
            }

            if (any_transformation) {
                messages_transformed_++;
            }

            return transform_result::success;
        }

        struct pipeline_statistics {
            std::string name;
            uint64_t messages_processed;
            uint64_t messages_transformed;
            uint64_t messages_skipped;
            uint64_t messages_errored;
            size_t transformer_count;
            std::vector<std::string> transformer_descriptions;
        };

        pipeline_statistics get_statistics() const {
            pipeline_statistics stats;
            stats.name = name_;
            stats.messages_processed = messages_processed_;
            stats.messages_transformed = messages_transformed_;
            stats.messages_skipped = messages_skipped_;
            stats.messages_errored = messages_errored_;
            stats.transformer_count = transformers_.size();

            for (const auto& transformer : transformers_) {
                stats.transformer_descriptions.push_back(transformer->describe());
            }

            return stats;
        }

        void reset_statistics() {
            messages_processed_ = 0;
            messages_transformed_ = 0;
            messages_skipped_ = 0;
            messages_errored_ = 0;
        }
    };

    // Message relay system
    class message_relay {
    private:
        std::unordered_map<std::string, std::unique_ptr<transformation_pipeline>> pipelines_;
        std::function<void(const core::message&)> output_handler_;
        mutable std::mutex relay_mutex_;

    public:
        void register_pipeline(const std::string& name,
                             std::unique_ptr<transformation_pipeline> pipeline) {
            std::lock_guard<std::mutex> lock(relay_mutex_);
            pipelines_[name] = std::move(pipeline);
        }

        transformation_pipeline* get_pipeline(const std::string& name) const {
            std::lock_guard<std::mutex> lock(relay_mutex_);
            auto it = pipelines_.find(name);
            return it != pipelines_.end() ? it->second.get() : nullptr;
        }

        void set_output_handler(std::function<void(const core::message&)> handler) {
            output_handler_ = std::move(handler);
        }

        bool relay_message(const std::string& pipeline_name, core::message message) const {
            auto pipeline = get_pipeline(pipeline_name);
            if (!pipeline) return false;

            auto result = pipeline->process_message(message);

            if (result == transform_result::success && output_handler_) {
                output_handler_(message);
                return true;
            }

            return result != transform_result::error;
        }

        std::vector<std::string> get_pipeline_names() const {
            std::lock_guard<std::mutex> lock(relay_mutex_);
            std::vector<std::string> names;
            for (const auto& [name, pipeline] : pipelines_) {
                names.push_back(name);
            }
            return names;
        }

        std::vector<transformation_pipeline::pipeline_statistics> get_all_statistics() const {
            std::lock_guard<std::mutex> lock(relay_mutex_);
            std::vector<transformation_pipeline::pipeline_statistics> all_stats;

            for (const auto& [name, pipeline] : pipelines_) {
                all_stats.push_back(pipeline->get_statistics());
            }

            return all_stats;
        }
    };

    // Pipeline builder for easy configuration
    class pipeline_builder {
    private:
        std::unique_ptr<transformation_pipeline> pipeline_;

    public:
        explicit pipeline_builder(const std::string& name)
            : pipeline_(std::make_unique<transformation_pipeline>(name)) {}

        pipeline_builder& rewrite_topic(const std::string& pattern, const std::string& replacement) {
            pipeline_->add_transformer(std::make_unique<topic_rewriter>(pattern, replacement));
            return *this;
        }

        pipeline_builder& enrich_data(const std::unordered_map<std::string, core::message_value>& data) {
            auto enricher = std::make_unique<data_enricher>();
            for (const auto& [key, value] : data) {
                enricher->add_enrichment(key, value);
            }
            pipeline_->add_transformer(std::move(enricher));
            return *this;
        }

        pipeline_builder& adjust_priority(core::message_priority priority) {
            pipeline_->add_transformer(std::make_unique<priority_adjuster>(priority));
            return *this;
        }

        pipeline_builder& validate_content(const std::vector<std::string>& required_fields) {
            auto validator = std::make_unique<content_validator>();
            for (const auto& field : required_fields) {
                validator->add_required_field(field);
            }
            pipeline_->add_transformer(std::move(validator));
            return *this;
        }

        pipeline_builder& add_custom(const std::string& name,
                                    std::function<transform_result(const transformation_context&, core::message&)> func) {
            pipeline_->add_transformer(std::make_unique<function_transformer>(name, std::move(func)));
            return *this;
        }

        std::unique_ptr<transformation_pipeline> build() {
            return std::move(pipeline_);
        }
    };

} // namespace kcenon::messaging::routing