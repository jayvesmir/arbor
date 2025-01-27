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
                0, 1, 2, 2, 3, 0, // bottom
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
                0, 1, 2, 2, 3, 0, // bottom
                2, 3, 7, 2, 6, 7, // front
                1, 2, 6, 1, 5, 6, // left
                0, 1, 5, 0, 4, 5, // back
                0, 3, 7, 0, 4, 7, // right
                4, 5, 6, 6, 7, 4, // top
            };

            return out;
        }
    } // namespace engine
} // namespace arbor