module;
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

export module engine;
import arbor.types;
import engine.window;
import engine.components.renderer;

export import engine.application;

namespace arbor {
    namespace engine {
        export class instance {
            engine::application_config m_config;
            std::shared_ptr<spdlog::logger> m_logger;

            std::vector<std::unique_ptr<engine::component>> m_components;
            std::shared_ptr<engine::window> m_window;

          public:
            instance();

            std::expected<void, std::string> create_app(const engine::application_config& app_config);

          protected:
            auto create_window(int32_t width, int32_t height, const std::string& title);
        };

        instance::instance() {
            m_logger = spdlog::stdout_color_mt("engine");

            m_logger->disable_backtrace();
            m_logger->set_pattern("(%X.%e) [%^%l%$] %v");
#if NDEBUG
            m_logger->set_level(spdlog::level::info);
#else
            m_logger->set_level(spdlog::level::trace);
#endif
        }

        auto instance::create_window(int32_t width, int32_t height, const std::string& title) {
            if (!m_window)
                m_window = std::make_shared<arbor::engine::window>();

            const auto res = m_window->create(width, height, title);
            if (res)
                m_logger->debug("created a window ('{}' {}x{})", m_window->title(), m_window->width(), m_window->height());
            else
                m_logger->critical("failed to create a window ('{}' {}x{}):\n\t{}", title, width, height, res.error());

            return res;
        }

        std::expected<void, std::string> instance::create_app(const engine::application_config& app_config) {
            m_config = app_config;

            if (auto res = create_window(m_config.window.width, m_config.window.height, m_config.window.title); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor
