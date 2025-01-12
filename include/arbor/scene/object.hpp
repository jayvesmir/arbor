#pragma once
#include "arbor/configs.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class object {
            uint64_t m_id = -1;

            callback_config m_callbacks;

          public:
            constexpr auto id() const { return m_id; }

            constexpr auto& callbacks() { return m_callbacks; }
            constexpr auto& callbacks() const { return m_callbacks; }
        };
    } // namespace engine
} // namespace arbor