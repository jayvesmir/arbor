#pragma once
#include <expected>
#include <string>
#include <utility>

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "fmt/format.h"

#include "engine/components/components.hpp"

namespace arbor {
    namespace engine {
        class instance;

        class window {
            friend class engine::instance;
            friend class engine::component;
            friend class engine::renderer;

            bool m_initialized = false;

            std::string m_title;
            int32_t m_width = 0, m_height = 0;

            SDL_Event m_current_event;
            SDL_Window* m_sdl_handle = nullptr;

            constexpr static SDL_InitFlags m_init_flags     = SDL_INIT_EVENTS | SDL_INIT_VIDEO;
            constexpr static SDL_WindowFlags m_window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

          protected:
            constexpr auto sdl_handle() const { return m_sdl_handle; }
            constexpr auto current_event() const { return std::ref(m_current_event); }

          public:
            window() = default;
            ~window();

            window(window&&)      = delete;
            window(const window&) = delete;

            auto poll_event() { return std::pair(SDL_PollEvent(&m_current_event), std::ref(m_current_event)); }
            std::expected<void, std::string> create(int32_t width, int32_t height, const std::string& title);
            std::expected<void, std::string> update_dimensions();

            constexpr auto title() const { return m_title; }
            constexpr auto width() const { return m_width; }
            constexpr auto height() const { return m_height; }
            std::expected<void, std::string> title(const std::string& new_title);
        };
    } // namespace engine
} // namespace arbor