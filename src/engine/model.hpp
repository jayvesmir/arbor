#pragma once
#include <array>
#include <string>

#include "arbor.hpp"

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        struct vertex_2d {
            glm::vec2 position;
            glm::vec3 color;

            static std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 2>> make_vk_binding();
        };
    } // namespace engine
} // namespace arbor