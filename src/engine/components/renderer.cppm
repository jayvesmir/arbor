module;
#include <expected>
#include <memory>
#include <string>

export module engine.components.renderer;
import engine.window;
import engine.components;

namespace arbor {
    namespace engine {
        export class renderer : public engine::component {
            std::shared_ptr<engine::window> m_target;

          public:
            renderer(const std::shared_ptr<engine::window>& target);
            ~renderer() = default;

            void shutdown() override;
            std::expected<void, std::string> init() override;
        };

        renderer::renderer(const std::shared_ptr<engine::window>& target) : m_target(target) {
            m_identifier = "renderer";
            m_type       = etype::renderer;
        }

        void renderer::shutdown() {}

        std::expected<void, std::string> renderer::init() {
            initialize_component_logger();

            return std::unexpected("unimplemented");
        }
    } // namespace engine
} // namespace arbor