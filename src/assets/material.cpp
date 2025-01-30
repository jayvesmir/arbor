#include "arbor/assets/material.hpp"

namespace arbor {
    namespace assets {
        material material::make_default() {
            material out;

            out.m_textures[texture::albedo].load(32, 32, {255, 0, 255, 1});

            return out;
        }
    } // namespace assets
} // namespace arbor