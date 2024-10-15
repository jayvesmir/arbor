module;
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

export module engine;
import arbor.types;
import engine.window;
import engine.components;

namespace arbor {
    namespace engine {
        export class instance {
            std::shared_ptr<spdlog::logger> m_logger;
            std::vector<std::unique_ptr<engine::component>> m_components;

            std::shared_ptr<engine::window> m_window;

          public:
            instance();

            auto create_window(uint32_t width, uint32_t height, const std::string& title);
        };

        instance::instance() {
            m_logger = spdlog::get("engine");
        }

        auto instance::create_window(uint32_t width, uint32_t height, const std::string& title) {
            m_window = std::make_shared<arbor::engine::window>();
            return m_window->create(width, height, title);
        }
    } // namespace engine
} // namespace arbor
