#include "engine/components/renderer.hpp"

namespace arbor {
    namespace engine {
        renderer::renderer(const engine::instance& parent) : m_parent(parent) {
            m_identifier = "renderer";
            m_type       = etype::renderer;
        }

        void renderer::shutdown() {
            if (vk.surface)
                vkDestroySurfaceKHR(vk.instance, vk.surface, nullptr);

            if (vk.device)
                vkDestroyDevice(vk.device, nullptr);

            if (vk.debug_messenger) {
                auto destroy_fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
                destroy_fn(vk.instance, vk.debug_messenger, nullptr);
            }

            if (vk.instance)
                vkDestroyInstance(vk.instance, nullptr);
        }

        std::expected<void, std::string> renderer::init() {
            initialize_component_logger();
            m_logger->debug("parent: {}", fmt::ptr(&m_parent));
            m_logger->debug("parent window: {}", fmt::ptr(&m_parent.window()));

            if (auto res = make_vk_instance(); !res)
                return res;

            if (auto res = make_vk_surface(); !res)
                return res;

            if (auto res = make_vk_device(); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor