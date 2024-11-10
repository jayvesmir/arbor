#include "engine/window.hpp"

#include "SDL3/SDL_video.h"

namespace arbor {
    namespace engine {
        window::~window() {
            if (m_sdl_handle)
                SDL_DestroyWindow(m_sdl_handle);

            SDL_QuitSubSystem(m_init_flags);
        }

        std::expected<void, std::string> window::create(int32_t width, int32_t height, const std::string& title) {
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
        }

        std::expected<void, std::string> window::title(const std::string& new_title) {
            m_title = new_title;
            if (!SDL_SetWindowTitle(m_sdl_handle, m_title.c_str()))
                return std::unexpected(fmt::format("failed to change the title of the window: {}", SDL_GetError()));

            return {};
        }
    } // namespace engine
} // namespace arbor