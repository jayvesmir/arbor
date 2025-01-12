#pragma once

#include <unordered_map>

#include "arbor/model.hpp"
#include "arbor/scene/texture.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class asset_library {
          public:
            struct entry {
                engine::model_3d model;
                std::unordered_map<engine::texture::etype, engine::texture> textures;
            };

          private:
            std::unordered_map<uint64_t, asset_library::entry> m_entries;

          public:
            constexpr auto& entries() { return m_entries; }
            constexpr auto& entries() const { return m_entries; }

            constexpr auto& operator[](uint64_t id) { return m_entries[id]; }
            constexpr auto& at(uint64_t id) const { return m_entries.at(id); }
        };
    } // namespace engine
} // namespace arbor