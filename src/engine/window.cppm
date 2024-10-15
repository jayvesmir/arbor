module;
#include <expected>
#include <string>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"

export module engine.window;
import arbor.types;

namespace arbor {
    namespace engine {
        export class window {
            SDL_Window* m_sdl_handle = nullptr;
            SDL_Event m_current_event;

          public:
            std::expected<bool, std::string> create(uint32_t width, uint32_t height, const std::string& title);
        };
    } // namespace engine
} // namespace arbor

std::expected<bool, std::string> arbor::engine::window::create(uint32_t width, uint32_t height, const std::string& title) {
    return std::unexpected("unimplemented");
}