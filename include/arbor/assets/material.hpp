#pragma once

#include <unordered_map>

#include "arbor/assets/texture.hpp"

namespace arbor {
    namespace assets {
        class material {
            std::unordered_map<assets::texture::etype, assets::texture> m_textures;

          public:
            constexpr auto& textures() { return m_textures; }
            constexpr auto& textures() const { return m_textures; }

            static material make_default();
        };
    } // namespace assets
} // namespace arbor