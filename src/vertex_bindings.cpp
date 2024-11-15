#include "arbor/model.hpp"

namespace arbor {
    namespace engine {
        std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 2>> vertex_2d::make_vk_binding() {
            VkVertexInputBindingDescription binding_description{};
            std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};

            binding_description.binding   = 0;
            binding_description.stride    = sizeof(engine::vertex_2d);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            attribute_descriptions[0].binding  = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[0].offset   = offsetof(engine::vertex_2d, position);

            attribute_descriptions[1].binding  = 0;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset   = offsetof(engine::vertex_2d, color);

            return {binding_description, attribute_descriptions};
        }
    } // namespace engine
} // namespace arbor