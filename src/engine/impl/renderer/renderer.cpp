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
            m_logger->debug("parent: {}", fmt::ptr(&m_parent));
            m_logger->debug("parent window: {}", fmt::ptr(&m_parent.window()));

            if (auto res = make_vk_instance(); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor