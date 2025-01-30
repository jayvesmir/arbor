#include "arbor/assets/model.hpp"

namespace arbor {
    namespace assets {
        std::pair<VkVertexInputBindingDescription, std::array<VkVertexInputAttributeDescription, 3>>
        vertex_3d::make_vk_binding() {
            VkVertexInputBindingDescription binding_description{};
            std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

            binding_description.binding = 0;
            binding_description.stride = sizeof(assets::vertex_3d);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(assets::vertex_3d, position);

            attribute_descriptions[1].binding = 0;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(assets::vertex_3d, color);

            attribute_descriptions[2].binding = 0;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[2].offset = offsetof(assets::vertex_3d, texture_coord);

            return {binding_description, attribute_descriptions};
        }

        model_3d model_3d::cube(float32_t scale_x, float32_t scale_y, float32_t scale_z) {
            model_3d out;

            out.vertices = {
                {{scale_x, scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // A
                {{scale_x, scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // B
                {{-scale_x, scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // C
                {{-scale_x, scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // D

                {{scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // E
                {{scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // F
                {{-scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // G
                {{-scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // H
            };

            out.indices = {
                0, 1, 2, 0, 2, 3, // front | ABC, ACD
                1, 5, 6, 1, 6, 2, // bottom | BFG, BGC
                4, 0, 3, 4, 3, 7, // top | EAD, EDH
                4, 5, 1, 4, 1, 0, // left | EFB, EBA
                3, 2, 6, 3, 6, 7, // right | DCG, DGH
                7, 6, 5, 7, 5, 4, // back | HGF, HFE
            };

            return out;
        }

        model_3d model_3d::cube_uv(float32_t scale_x, float32_t scale_y, float32_t scale_z) {
            model_3d out;

            out.vertices = {
                {{scale_x, scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 1.0f / 3.0f}}, // A
                {{scale_x, scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 2.0f / 3.0f}}, // B
                {{-scale_x, scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 2.0f / 3.0f}}, // C
                {{-scale_x, scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 1.0f / 3.0f}}, // D

                {{scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 3.0f / 3.0f}}, // R
                {{-scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 3.0f / 3.0f}}, // S

                {{scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f / 4.0f, 1.0f / 3.0f}}, // E
                {{scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {0.0f / 4.0f, 2.0f / 3.0f}}, // F

                {{-scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {3.0f / 4.0f, 2.0f / 3.0f}}, // K
                {{-scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {3.0f / 4.0f, 1.0f / 3.0f}}, // L

                {{scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 0.0f / 3.0f}}, // M
                {{-scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 0.0f / 3.0f}}, // P

                {{scale_x, -scale_y, scale_z}, {1.0f, 1.0f, 1.0f}, {4.0f / 4.0f, 1.0f / 3.0f}}, // X
                {{scale_x, -scale_y, -scale_z}, {1.0f, 1.0f, 1.0f}, {4.0f / 4.0f, 2.0f / 3.0f}}, // W
            };

            out.indices = {
                0,  1, 2,  0,  2,  3, // front | ABC, ACD
                1,  4, 5,  1,  5,  2, // bottom | BRS, BSC
                10, 0, 3,  10, 3,  11, // top | MAD, MDP
                6,  7, 1,  6,  1,  0, // left | EFB, EBA
                3,  2, 8,  3,  8,  9, // right | DCK, DKL
                9,  8, 13, 9,  13, 12, // back | LKW, LWX
            };

            return out;
        }

        model_3d model_3d::plane(float32_t scale_x, float32_t scale_y) {
            model_3d out;

            out.vertices = {
                {{scale_x, -scale_y, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // A
                {{scale_x, scale_y, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // B
                {{-scale_x, scale_y, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // C
                {{-scale_x, -scale_y, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // D
            };

            out.indices = {
                0, 1, 2, 0, 2, 3, // front | ABC, ACD
                3, 2, 0, 2, 1, 0, // back | DCA, CBA
            };

            return out;
        }
    } // namespace assets
} // namespace arbor