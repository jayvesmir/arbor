#include "engine/engine.hpp"
#include <chrono>
#include <thread>

#include "SDL3/SDL_events.h"
#include "engine/components/renderer.hpp"

namespace arbor {
    namespace engine {
        instance::instance() {
            m_logger = engine::make_logger("engine");
        }

        instance::~instance() {
            for (auto& component : m_components) {
                m_logger->info("destroying '{}'", component->identifier());
                component->shutdown();
            }
        }

        std::expected<void, std::string> instance::create_window(int32_t width, int32_t height, const std::string& title) {
            const auto res = m_window.create(width, height, title);
            if (res)
                m_logger->debug("created a window ('{}' {}x{})", m_window.title(), m_window.width(), m_window.height());
            else
                m_logger->critical("failed to create a window ('{}' {}x{}):\n\t{}", title, width, height, res.error());

            return res;
        }

        std::expected<void, std::string> instance::initialize_components() {
            for (auto& component : m_components) {
                m_logger->info("initializing '{}'", component->identifier());

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

            m_running = true;
            m_running.notify_all();

            while (m_running) {
                auto&& [has_event, window_event] = m_window.poll_event();
                if (has_event)
                    process_window_event(window_event);

                for (const auto& component : m_components) {
                    if (auto res = component->update(); !res) {
                        m_logger->critical("'{}' failed to update: {}", component->identifier(), res.error());
                        m_running = false;
                        return res;
                    }
                }
            }

            return {};
        }

        std::expected<void, std::string> instance::process_window_event(const SDL_Event& event) {
            if (event.type == SDL_EVENT_QUIT) {
                m_running = false;
                m_running.notify_all();
            }

            return {};
        }
    } // namespace engine
} // namespace arbor