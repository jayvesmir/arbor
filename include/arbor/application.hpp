#pragma once
#include <string>

#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        struct application_config {
            struct window_config {
                std::string title;
                int32_t width, height;
            } window;
        };
    } // namespace engine
} // namespace arbor