#pragma once

#include <unordered_map>

#include "arbor/model.hpp"
#include "arbor/scene/texture.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class asset_manager {
            std::unordered_map<uint64_t, engine::model_3d> m_models;
            std::unordered_map<uint64_t, engine::texture> m_textures;
        };
    } // namespace engine
} // namespace arbor