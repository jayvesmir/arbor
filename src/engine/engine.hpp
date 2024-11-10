#pragma once
#include <atomic>
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "SDL3/SDL_events.h"
#include "spdlog/spdlog.h"

#include "arbor.hpp"
#include "engine/application.hpp"
#include "engine/components/components.hpp"
#include "engine/logger_utils.hpp"
#include "engine/window.hpp"

namespace arbor {
    namespace engine {
        class instance {
            friend class engine::component;
            friend class engine::renderer;

            engine::application_config m_config;
            std::shared_ptr<spdlog::logger> m_logger;

            engine::window m_window;
            std::vector<std::unique_ptr<engine::component>> m_components;

            std::atomic<bool> m_running;

          public:
            instance();
            ~instance();

            instance(instance&&)      = delete;
            instance(const instance&) = delete;

            std::expected<void, std::string> start(const engine::application_config& app_config);

          protected:
            constexpr auto& window() { return m_window; }
            constexpr auto& window() const { return m_window; }

          private:
            std::expected<void, std::string> create_renderer();
            std::expected<void, std::string> initialize_components();
            std::expected<void, std::string> create_app();
            std::expected<void, std::string> create_window(int32_t width, int32_t height, const std::string& title);

            std::expected<void, std::string> process_window_event(const SDL_Event& event);
        };
    } // namespace engine
} // namespace arbor
