module;
#include <expected>
#include <string>
#include <utility>

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "fmt/format.h"

export module engine.window;
import arbor.types;

namespace arbor {
    namespace engine {
        class instance;

        export class window {
            friend class instance;

            bool m_initialized = false;

            std::string m_title;
            int32_t m_width = 0, m_height = 0;

            SDL_Event m_current_event;
            SDL_Window* m_sdl_handle = nullptr;

            constexpr static SDL_InitFlags m_init_flags     = SDL_INIT_EVENTS | SDL_INIT_VIDEO;
            constexpr static SDL_WindowFlags m_window_flags = SDL_WINDOW_VULKAN;

          protected:
            constexpr auto sdl_handle() const { return m_sdl_handle; }
            constexpr auto current_event() const { return std::ref(m_current_event); }

          public:
            ~window();

            auto poll_event() { return std::pair(SDL_PollEvent(&m_current_event), std::ref(m_current_event)); }
            std::expected<bool, std::string> create(int32_t width, int32_t height, const std::string& title);

            constexpr auto title() const { return m_title; }
            constexpr auto width() const { return m_width; }
            constexpr auto height() const { return m_height; }
        };

        window::~window() {
            if (m_sdl_handle)
                SDL_DestroyWindow(m_sdl_handle);

            SDL_QuitSubSystem(m_init_flags);
        }

        std::expected<bool, std::string> window::create(int32_t width, int32_t height, const std::string& title) {
#define sdl_error(fmt_str, ...)                                  \
    {                                                            \
        const auto __fmt = fmt::format(fmt_str, SDL_GetError()); \
        SDL_ClearError();                                        \
        return std::unexpected(__fmt);                           \
    }
            if (m_initialized)
                return std::unexpected(fmt::format("this window has already been instantiated ({})", fmt::ptr(this)));

            m_width  = width;
            m_height = height;
            m_title  = title;

            if (!SDL_InitSubSystem(m_init_flags))
                sdl_error("failed to initialize SDL: {}");

            m_sdl_handle = SDL_CreateWindow(m_title.c_str(), m_width, m_height, m_window_flags);
            if (!m_sdl_handle)
                sdl_error("failed to create an SDL window: {}");

            m_initialized = true;
            return {};
        } // namespace engine
    } // namespace engine
} // namespace arbor