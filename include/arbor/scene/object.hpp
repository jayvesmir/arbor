#pragma once
#include "arbor/configs.hpp"
#include "arbor/model.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class object {
            uint64_t m_id = -1;

            engine::object_callback_config m_callbacks;

            glm::mat4 m_transform = {1.0f};

          public:
            object(uint64_t id = -1) : m_id(id) {}

            constexpr auto id() const { return m_id; }

            constexpr auto& callbacks() { return m_callbacks; }
            constexpr auto& callbacks() const { return m_callbacks; }

            constexpr auto& transform() { return m_transform; }
            constexpr auto& transform() const { return m_transform; }
        };
    } // namespace engine
} // namespace arbor