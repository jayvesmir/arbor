#include "arbor/model.hpp"

namespace arbor {
    namespace engine {
        model_3d model_3d::cube(float scale_x, float scale_y, float scale_z) {
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

        // TODO
        model_3d model_3d::cube_uv(float scale_x, float scale_y, float scale_z) {
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

        model_3d model_3d::plane(float scale_x, float scale_y) {
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
    } // namespace engine
} // namespace arbor