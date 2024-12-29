#include "arbor/engine.hpp"
#include <algorithm>
#include <chrono>

#include "SDL3/SDL_events.h"
#include "arbor/components/renderer.hpp"
#include "imgui_impl_sdl3.h"

namespace arbor {
    namespace engine {
        instance::instance() {
            m_logger = engine::make_logger("engine");
        }

        instance::~instance() {
            for (auto& [type, component] : m_components) {
                m_logger->debug("destroying '{}'", component->identifier());
                component->shutdown();
            }
        }

        std::expected<void, std::string> instance::create_window(int32_t width, int32_t height, const std::string& title) {
            const auto res = m_window.create(width, height, title);
            if (res)
                m_logger->trace("created a window ('{}' {}x{})", m_window.title(), m_window.width(), m_window.height());
            else
                m_logger->critical("failed to create a window ('{}' {}x{}):\n\t{}", title, width, height, res.error());

            return res;
        }

        std::expected<void, std::string> instance::initialize_components() {
            for (auto& [type, component] : m_components) {
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

            m_components[component::renderer] = std::make_unique<engine::renderer>(*this);

            return {};
        }

        std::expected<void, std::string> instance::push_scene_and_set_current(const engine::scene& scene) {
            m_scenes.emplace(scene.name(), scene);
            m_current_scene = m_scenes.find(scene.name());
            return {};
        }

        std::expected<void, std::string> instance::run(const engine::application_config& app_config) {
            m_config = app_config;
            if (auto res = create_app(); !res)
                return res;

            if (!m_current_scene || !m_scenes.contains(current_scene().name()))
                return std::unexpected("failed to run arbor: no scene loaded");

            if (auto res = initialize_components(); !res)
                return res;

            m_running = true;
            m_running.notify_all();

            auto frame_start = std::chrono::high_resolution_clock::now();

            while (m_running) {
                frame_start = std::chrono::high_resolution_clock::now();

                while (m_window.poll_event().first)
                    process_window_event(m_window.current_event());

                for (const auto& [type, component] : m_components) {
                    if (auto res = component->update(); !res) {
                        m_logger->critical("'{}' failed to update: {}", component->identifier(), res.error());
                        m_running = false;
                        return res;
                    }
                }

                m_frame_count++;
                m_frame_time_ms = (std::chrono::high_resolution_clock::now() - frame_start).count() * 1e-6;
            }

            return {};
        }

        std::expected<void, std::string> instance::process_window_event(const SDL_Event& event) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                m_running = false;
                m_running.notify_all();
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                auto renderer = dynamic_cast<engine::renderer*>(m_components.at(component::etype::renderer).get());
                renderer->resize_viewport();
            }

            return {};
        }
    } // namespace engine
} // namespace arbor