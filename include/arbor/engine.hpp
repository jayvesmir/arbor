#pragma once
#include <atomic>
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "SDL3/SDL_events.h"
#include "spdlog/spdlog.h"

#include "arbor/components/components.hpp"
#include "arbor/configs.hpp"
#include "arbor/input_manager.hpp"
#include "arbor/logger_utils.hpp"
#include "arbor/scene/scene.hpp"
#include "arbor/types.hpp"
#include "arbor/window.hpp"

namespace arbor {
    namespace engine {
        class instance {
            friend class engine::component;
            friend class engine::renderer;

            engine::application_config m_config;
            std::shared_ptr<spdlog::logger> m_logger;

            engine::window m_window;
            engine::input_manager m_input_manager;
            std::unordered_map<engine::component::etype, std::unique_ptr<engine::component>> m_components;

            std::atomic<bool> m_running;
            std::unordered_map<std::string, scene::instance> m_scenes;
            std::optional<std::unordered_map<std::string, scene::instance>::iterator> m_current_scene;

            uint64_t m_frame_count = 0;
            float64_t m_frame_time_ns = 0;
            bool m_camera_ownership = false;

          public:
            instance();
            ~instance();

            instance(instance&&) = delete;
            instance(const instance&) = delete;

            std::expected<void, std::string> run(const engine::application_config& app_config);
            std::expected<scene::instance*, std::string> push_scene_and_set_current(const scene::instance& scene);

            constexpr auto frame_count() const { return m_frame_count; }
            constexpr auto frame_time_ns() const { return m_frame_time_ns; };
            constexpr auto frame_time_ms() const { return m_frame_time_ns * 1e-6; };

            constexpr auto& scenes() { return m_scenes; }
            constexpr auto& scenes() const { return m_scenes; }
            constexpr auto& input_manager() const { return m_input_manager; }
            constexpr auto& current_scene() { return m_current_scene.value()->second; }
            constexpr auto& current_scene() const { return m_current_scene.value()->second; }

            constexpr auto owns_camera() const { return m_camera_ownership; }

          protected:
            constexpr auto& window() { return m_window; }
            constexpr auto& window() const { return m_window; }

          private:
            std::expected<void, std::string> on_scene_change();

            std::expected<void, std::string> create_renderer();
            std::expected<void, std::string> initialize_components();
            std::expected<void, std::string> create_app();
            std::expected<void, std::string> create_window(int32_t width, int32_t height, const std::string& title);

            std::expected<void, std::string> invoke_callbacks();
            std::expected<void, std::string> process_window_event(const SDL_Event& event);
        };
    } // namespace engine
} // namespace arbor
