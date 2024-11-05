#include "engine/components/renderer.hpp"

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_vulkan.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_surface() {
            m_logger->debug("creating a window surface");
            if (!SDL_Vulkan_CreateSurface(m_parent.window().sdl_handle(), vk.instance, nullptr, &vk.surface))
                return std::unexpected(fmt::format("failed to create window surface: {}", SDL_GetError()));

            return {};
        }
    } // namespace engine
} // namespace arbor