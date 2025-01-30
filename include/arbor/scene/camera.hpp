#pragma once

#include "arbor/assets/model.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class camera {
            uint64_t m_id = -1;

            glm::mat4 m_transform = {1.0f};
            glm::vec3 m_rotation = {0.0f, 0.0f, 0.0f};

          public:
            camera(uint64_t id = -1) : m_id(id) {}

            constexpr auto id() const { return m_id; }
            constexpr auto& transform() const { return m_transform; }
            glm::mat4 view_matrix() const;

            constexpr auto rotation() const { return m_rotation; }
            constexpr auto position() const { return glm::vec3(m_transform[3]); }

            glm::vec3 rotate(const glm::vec3& delta);
            glm::vec3 translate(const glm::vec3& delta);
        };
    } // namespace engine
} // namespace arbor