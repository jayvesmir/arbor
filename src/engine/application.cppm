module;
#include <cstdint>
#include <string>

export module engine.application;

namespace arbor {
    namespace engine {
        export struct application_config {
            struct window_config {
                std::string title;
                int32_t width, height;
            } window;
        };
    } // namespace engine
} // namespace arbor