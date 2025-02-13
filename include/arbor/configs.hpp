#pragma once
#include <expected>
#include <functional>
#include <string>

#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class instance;

        struct callback_config {
            std::optional<std::function<void(engine::instance&)>> on_init;
            std::optional<std::function<void(engine::instance&)>> on_update;
        };

        struct object_callback_config {
            std::optional<std::function<void(engine::instance&, uint64_t id)>> on_init;
            std::optional<std::function<void(engine::instance&, uint64_t id)>> on_update;
        };

        struct internal_callback_config {
            std::optional<std::function<std::expected<void, std::string>()>> on_scene_change;
        };

        struct application_config {
            struct {
                std::string title;
                int32_t width, height;
            } window;

            callback_config callbacks;
        };

    } // namespace engine
} // namespace arbor