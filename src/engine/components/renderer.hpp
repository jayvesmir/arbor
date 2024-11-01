#pragma once
#include <expected>
#include <memory>
#include <string>

#include "arbor.hpp"
#include "engine/components/component.hpp"
#include "engine/engine.hpp"
#include "engine/window.hpp"

namespace arbor {
    namespace engine {
        class renderer : public engine::component {
            const engine::instance& m_parent;

          public:
            renderer(const engine::instance& parent);
            ~renderer() = default;

            void shutdown() override;
            std::expected<void, std::string> init() override;
        };
    } // namespace engine
} // namespace arbor