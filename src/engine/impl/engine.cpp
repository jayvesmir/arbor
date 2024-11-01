#include "engine/engine.hpp"

#include "engine/components/renderer.hpp"

namespace arbor {
    namespace engine {
        instance::instance() {
            m_logger = engine::make_logger("engine");
        }

        std::expected<void, std::string> instance::create_window(int32_t width, int32_t height, const std::string& title) {
            m_window = {};

            const auto res = m_window.create(width, height, title);
            if (res)
                m_logger->debug("created a window ('{}' {}x{})", m_window.title(), m_window.width(), m_window.height());
            else
                m_logger->critical("failed to create a window ('{}' {}x{}):\n\t{}", title, width, height, res.error());

            return res;
        }

        std::expected<void, std::string> instance::initialize_components() {
            for (auto& component : m_components) {
                m_logger->debug("initializing '{}'", component->identifier());

                if (auto res = component->init(); !res) {
                    m_logger->critical("failed to initialize the '{}' subsystem:\n\t{}", component->identifier(), res.error());
                    return res;
                }
            }

            return {};
        }

        std::expected<void, std::string> instance::create_app() {
            if (auto res = create_window(m_config.window.width, m_config.window.height, m_config.window.title); !res)
                return res;

            m_components.push_back(std::make_unique<engine::renderer>(*this));

            return {};
        }

        std::expected<void, std::string> instance::start(const engine::application_config& app_config) {
            m_config = app_config;
            if (auto res = create_app(); !res)
                return res;

            if (auto res = initialize_components(); !res)
                return res;

            return std::unexpected("unimplemented");
        }
    } // namespace engine
} // namespace arbor