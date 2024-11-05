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
        namespace vk {
            struct device_queue_family_indices {
                uint32_t graphics_family;
                uint32_t present_family;
            };
        } // namespace vk

        class renderer : public engine::component {
            const engine::instance& m_parent;
            struct {
                VkInstance instance = VK_NULL_HANDLE;
                std::vector<const char*> instance_ext;
                std::vector<const char*> instance_lay;

                VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

                struct {
                    VkPhysicalDevice handle = VK_NULL_HANDLE;
                    VkPhysicalDeviceFeatures features;
                    VkPhysicalDeviceProperties properties;

                    vk::device_queue_family_indices queue_family_indices;
                } physical_device;

                VkSurfaceKHR surface = VK_NULL_HANDLE;

                VkDevice device        = VK_NULL_HANDLE;
                VkQueue graphics_queue = VK_NULL_HANDLE;
                VkQueue present_queue  = VK_NULL_HANDLE;

                std::vector<const char*> device_ext;
                std::vector<const char*> device_lay;
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
            std::expected<void, std::string> make_vk_device();
            std::expected<void, std::string> make_vk_surface();
        };
    } // namespace engine
} // namespace arbor