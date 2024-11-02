#pragma once
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "arbor.hpp"
#include "engine/components/component.hpp"
#include "engine/engine.hpp"
#include "engine/window.hpp"

#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        class renderer : public engine::component {
            const engine::instance& m_parent;

            struct {
                VkInstance instance;
                VkDebugUtilsMessengerEXT debug_messenger;
                std::vector<const char*> instance_ext;
                std::vector<const char*> instance_lay;
            } vk;

          public:
            renderer(const engine::instance& parent);
            ~renderer() = default;

            renderer(renderer&&)      = delete;
            renderer(const renderer&) = delete;

            void shutdown() override;
            std::expected<void, std::string> init() override;

          private:
            std::expected<void, std::string> make_vk_instance();
        };
    } // namespace engine
} // namespace arbor