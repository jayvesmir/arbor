#include "arbor/model.hpp"

namespace arbor {
    namespace engine {
        model_3d model_3d::cube(float scale) {
            model_3d out;

            out.vertices = {
                {{-scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{scale, scale, -scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{-scale, scale, -scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

                {{-scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{scale, scale, scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{-scale, scale, scale}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            };

            out.indices = {
                0, 1, 2, 0, 2, 3, // bottom
                2, 3, 7, 2, 6, 7, // front
                1, 2, 6, 1, 5, 6, // left
                0, 1, 5, 0, 4, 5, // back
                0, 3, 7, 0, 4, 7, // right
                4, 5, 6, 6, 7, 4, // top
            };

            return out;
        }

        // TODO
        model_3d model_3d::cube_uv(float scale) {
            model_3d out;

            out.vertices = {
                {{scale, scale, scale}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 1.0f / 3.0f}}, // A
                {{scale, scale, -scale}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 2.0f / 3.0f}}, // B
                {{-scale, scale, -scale}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 2.0f / 3.0f}}, // C
                {{-scale, scale, scale}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 1.0f / 3.0f}}, // D

                {{scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 3.0f / 3.0f}}, // R
                {{-scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 3.0f / 3.0f}}, // S

                {{scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {0.0f / 4.0f, 1.0f / 3.0f}}, // E
                {{scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {0.0f / 4.0f, 2.0f / 3.0f}}, // F

                {{-scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {3.0f / 4.0f, 2.0f / 3.0f}}, // K
                {{-scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {3.0f / 4.0f, 1.0f / 3.0f}}, // L

                {{scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {1.0f / 4.0f, 0.0f / 3.0f}}, // M
                {{-scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {2.0f / 4.0f, 0.0f / 3.0f}}, // P

                {{scale, -scale, scale}, {1.0f, 1.0f, 1.0f}, {4.0f / 4.0f, 1.0f / 3.0f}}, // X
                {{scale, -scale, -scale}, {1.0f, 1.0f, 1.0f}, {4.0f / 4.0f, 2.0f / 3.0f}}, // W
            };

            out.indices = {
                0,  1,  2, 0,  2,  3, // front | ABC, ACD
                1,  4,  5, 1,  5,  2, // bottom | BRS, BSC
                6,  7,  1, 6,  0,  1, // left | EFB, EAB
                3,  8,  2, 3,  9,  8, // right | DKC, DLK
                10, 3,  0, 10, 11, 3, // top | MDA, MPD
                9,  13, 8, 9,  12, 13, // back | LWK, LXW
            };

            return out;
        }
    } // namespace engine
} // namespace arbor