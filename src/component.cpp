#include "engine/components/component.hpp"

namespace arbor {
    namespace engine {
        std::shared_ptr<spdlog::logger> component::initialize_component_logger() {
            return (m_logger = engine::make_logger(m_identifier));
        }
    } // namespace engine
} // namespace arbor