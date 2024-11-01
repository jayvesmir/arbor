#include "engine/components/renderer.hpp"

namespace arbor {
    namespace engine {
        renderer::renderer(const engine::instance& parent) : m_parent(parent) {
            m_identifier = "renderer";
            m_type       = etype::renderer;
        }

        void renderer::shutdown() {}

        std::expected<void, std::string> renderer::init() {
            initialize_component_logger();

            m_logger->info("parent: {}", fmt::ptr(&m_parent));
            m_logger->info("parent window: {}", fmt::ptr(&m_parent.window()));

            return std::unexpected("unimplemented");
        }
    } // namespace engine
} // namespace arbor