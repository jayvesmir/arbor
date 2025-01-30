#pragma once
#include <array>
#include <vector>

#include "arbor/types.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        struct vertex_3d {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texture_coord;

            static std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 3>> make_vk_binding();
        };

        struct model_3d {
            std::vector<vertex_3d> vertices;
            std::vector<uint32_t> indices;

            static model_3d cube(float scale = 1.0f);
            static model_3d cube_uv(float scale = 1.0f);

            static model_3d plane(float scale = 1.0f);
        };
    } // namespace engine
} // namespace arbor