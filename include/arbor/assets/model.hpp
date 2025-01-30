#pragma once
#include <array>
#include <vector>

#include "arbor/types.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vulkan/vulkan.h"

namespace arbor {
    namespace assets {
        struct vertex_3d {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texture_coord;

            static std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 3>> make_vk_binding();
        };

        struct model_3d {
            std::vector<vertex_3d> vertices;
            std::vector<uint32_t> indices;

            static model_3d cube(float32_t scale_x = 1.0f, float32_t scale_y = 1.0f, float32_t scale_z = 1.0f);
            static model_3d cube_uv(float32_t scale_x = 1.0f, float32_t scale_y = 1.0f, float32_t scale_z = 1.0f);

            static model_3d plane(float32_t scale_x = 1.0f, float32_t scale_y = 1.0f);
        };
    } // namespace assets
} // namespace arbor