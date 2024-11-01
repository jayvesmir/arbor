module;
#include <expected>
#include <string>

export module engine.components.renderer;
export import engine.components;

namespace arbor {
    namespace engine {
        export class renderer : public engine::component {
          public:
            renderer() { m_type = etype::renderer; }

            ~renderer() = default;

            void shutdown() override;
            std::expected<void, std::string> init() override;
        };

        void renderer::shutdown() {}
        std::expected<void, std::string> renderer::init() {}
    } // namespace engine
} // namespace arbor