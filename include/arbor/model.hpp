#pragma once
#include <array>
#include <string>

#include "arbor/types.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        struct vertex_2d {
            glm::vec2 position;
            glm::vec3 color;

            static std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 2>> make_vk_binding();
        };

        struct mvp_ubo {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        };
    } // namespace engine
} // namespace arbor