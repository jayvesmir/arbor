#include "arbor/scene/camera.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace arbor {
    namespace engine {
        glm::mat4 camera::view_matrix() const {
            auto position = glm::vec3(m_transform[3]);
            auto forward = -glm::normalize(glm::vec3(m_transform[2]));
            auto right = glm::normalize(glm::vec3(m_transform[0]));
            auto up = glm::normalize(glm::cross(right, forward));

            return glm::lookAt(position, position + forward, -up);
        }
    } // namespace engine
} // namespace arbor