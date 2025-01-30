#include "arbor/components/component.hpp"

namespace arbor {
    namespace engine {
        std::shared_ptr<spdlog::logger> component::initialize_component_logger() {
            return (m_logger = arbor::make_logger(m_identifier));
        }
    } // namespace engine
} // namespace arbor