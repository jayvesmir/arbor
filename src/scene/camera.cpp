#include "arbor/scene/camera.hpp"
#include "glm/geometric.hpp"

namespace arbor {
    namespace engine {
        glm::mat4 camera::view_matrix() const {
            auto position = glm::vec3(m_transform[3]);
            auto forward = -glm::normalize(glm::vec3(m_transform[2]));
            auto right = glm::normalize(glm::vec3(m_transform[0]));
            auto up = glm::normalize(glm::cross(right, forward));

            return glm::lookAt(position, position + forward, -up);
        }

        glm::vec3 camera::rotate(const glm::vec3& delta) {
            m_rotation += glm::radians(delta);
            m_rotation.y = glm::clamp(m_rotation.y, glm::radians(-179.999f), 0.0f);

            glm::mat4 rotation(1.0f);
            rotation = glm::rotate(rotation, m_rotation.x, glm::vec3(0.0f, 0.0f, 1.0f));
            rotation = glm::rotate(rotation, m_rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));

            auto pos = m_transform[3];
            m_transform = rotation;
            m_transform[3] = pos;

            return m_rotation;
        }

        glm::vec3 camera::translate(const glm::vec3& delta) {
            m_transform = glm::translate(m_transform, delta);
            return m_transform[3];
        }
    } // namespace engine
} // namespace arbor