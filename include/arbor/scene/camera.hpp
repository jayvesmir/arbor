#pragma once

#include "arbor/model.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class camera {
            uint64_t m_id = -1;

            glm::mat4 m_transform = {1.0f};

          public:
            camera(uint64_t id = -1) : m_id(id) {}

            constexpr auto id() const { return m_id; }

            constexpr auto& transform() { return m_transform; }
            constexpr auto& transform() const { return m_transform; }

            glm::mat4 view_matrix() const;
        };
    } // namespace engine
} // namespace arbor