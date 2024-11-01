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
import engine.logger_utils;
import engine.components.renderer;

export import engine.application;

namespace arbor {
    namespace engine {
        export class instance {
            friend class engine::component;

            engine::application_config m_config;
            std::shared_ptr<spdlog::logger> m_logger;

            std::shared_ptr<engine::window> m_window;
            std::vector<std::unique_ptr<engine::component>> m_components;

          public:
            instance();

            std::expected<void, std::string> start(const engine::application_config& app_config);

          private:
            auto create_renderer();
            auto initialize_components();
            std::expected<void, std::string> create_app();
            auto create_window(int32_t width, int32_t height, const std::string& title);
        };

        instance::instance() {
            m_logger = engine::make_logger("engine");
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

        auto instance::initialize_components() {
            for (auto& component : m_components) {
                m_logger->debug("initializing '{}'", component->identifier());

                if (auto res = component->init(); !res) {
                    m_logger->critical("failed to initialize the '{}' subsystem:\n\t{}", component->identifier(), res.error());
                    return res;
                }
            }

            return std::expected<void, std::string>();
        }

        std::expected<void, std::string> instance::create_app() {
            if (auto res = create_window(m_config.window.width, m_config.window.height, m_config.window.title); !res)
                return res;

            m_components.push_back(std::make_unique<engine::renderer>(m_window));

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
