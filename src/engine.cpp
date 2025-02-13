#include "arbor/engine.hpp"
#include <chrono>

#include "arbor/components/renderer.hpp"
#include "arbor/scene/camera.hpp"

#include "glm/trigonometric.hpp"
#include "imgui_impl_sdl3.h"

namespace arbor {
    namespace engine {
        instance::instance() {
            m_logger = arbor::make_logger("engine");
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

        std::expected<scene::instance*, std::string> instance::push_scene_and_set_current(const scene::instance& scene) {
            static engine::internal_callback_config default_scene_callbacks = {
                .on_scene_change = std::bind(&instance::on_scene_change, this),
            };

            m_scenes.emplace(scene.name(), scene);
            m_scenes.at(scene.name()).internal_callbacks(default_scene_callbacks);
            m_current_scene = m_scenes.find(scene.name());
            return &m_scenes.at(scene.name());
        }

        std::expected<void, std::string> instance::run(const engine::application_config& app_config) {
            if (m_running)
                return std::unexpected("already running");

            m_config = app_config;
            if (auto res = create_app(); !res)
                return res;

            if (m_config.callbacks.on_init)
                std::invoke(*m_config.callbacks.on_init, *this);

            if (!m_current_scene || !m_scenes.contains(current_scene().name()))
                return std::unexpected("failed to run arbor: no scene loaded");

            if (auto res = initialize_components(); !res)
                return res;

            m_running = true;
            m_running.notify_all();

            while (m_running) {
                const auto frame_start = std::chrono::high_resolution_clock::now();

                while (m_window.poll_event().first)
                    process_window_event(m_window.current_event());

                if (auto res = invoke_callbacks(); !res)
                    m_logger->critical("failed to invoke callbacks: {}", res.error());

                if (m_current_scene) {
                    for (const auto& [type, component] : m_components) {
                        if (auto res = component->update(); !res) {
                            m_logger->critical("'{}' failed to update: {}", component->identifier(), res.error());
                            m_running = false;
                            return res;
                        }
                    }

                    if (m_input_manager.key_down(SDL_SCANCODE_SPACE)) {
                        m_camera_ownership = true;
                    } else {
                        m_camera_ownership = false;
                    }

                    if (m_camera_ownership) {
                        auto camera_speed = 0.01f;
                        auto camera_sensitivity = 0.025f;

                        glm::vec3 translation = {0.0f, 0.0f, 0.0f};

                        if (m_input_manager.key_down(SDL_SCANCODE_LSHIFT))
                            camera_speed *= 4;

                        if (m_input_manager.key_down(SDL_SCANCODE_W))
                            translation += glm::vec3(0.0f, 0.0f, -camera_speed);

                        if (m_input_manager.key_down(SDL_SCANCODE_A))
                            translation += glm::vec3(camera_speed, 0.0f, 0.0f);

                        if (m_input_manager.key_down(SDL_SCANCODE_S))
                            translation += glm::vec3(0.0f, 0.0f, camera_speed);

                        if (m_input_manager.key_down(SDL_SCANCODE_D))
                            translation += glm::vec3(-camera_speed, 0.0f, 0.0f);

                        if (m_input_manager.key_down(SDL_SCANCODE_E))
                            translation += glm::vec3(0.0f, -camera_speed, 0.0f);

                        if (m_input_manager.key_down(SDL_SCANCODE_Q))
                            translation += glm::vec3(0.0f, camera_speed, 0.0f);

                        current_scene().camera().translate(translation);
                        current_scene().camera().rotate(glm::vec3(-camera_sensitivity * m_input_manager.mouse_delta().x,
                                                                  camera_sensitivity * m_input_manager.mouse_delta().y, 0.0f) *
                                                        static_cast<float32_t>(frame_time_ms()));
                    }
                }

                m_frame_count++;
                m_frame_time_ns = (std::chrono::high_resolution_clock::now() - frame_start).count();
            }

            return {};
        }

        std::expected<void, std::string> instance::invoke_callbacks() {
            if (m_config.callbacks.on_update) {
                std::invoke(*m_config.callbacks.on_update, *this);
            }

            if (m_current_scene) {
                for (auto& [id, object] : m_current_scene.value()->second.objects()) {
                    if (object.callbacks().on_update.has_value()) {
                        std::invoke(*object.callbacks().on_update, *this, id);
                    }
                }
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

            m_input_manager.update_from_event(event);

            return {};
        }

        std::expected<void, std::string> instance::on_scene_change() {
            auto renderer = dynamic_cast<engine::renderer*>(m_components.at(component::etype::renderer).get());
            return renderer->scene_reload_deferred();
        }
    } // namespace engine
} // namespace arbor