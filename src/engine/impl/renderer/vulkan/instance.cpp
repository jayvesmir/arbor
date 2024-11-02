#include "engine/components/renderer.hpp"

#include "SDL3/SDL_vulkan.h"
#include "fmt/ranges.h"

#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        VkBool32 VKAPI_PTR debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* data, void* user_data) {
            (void)(types);
            auto logger = reinterpret_cast<spdlog::logger*>(user_data);

            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                logger->trace("validation layer: {}", data->pMessage);
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                logger->info("validation layer: {}", data->pMessage);
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                logger->warn("validation layer: {}", data->pMessage);
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                logger->error("validation layer: {}", data->pMessage);

            return VK_TRUE;
        }

        std::expected<void, std::string> renderer::make_vk_instance() {
            VkInstanceCreateInfo create_info{};
            VkApplicationInfo app_info{};

            app_info.sType    = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            uint32_t n_ext    = 0;
            auto sdl_ext_data = SDL_Vulkan_GetInstanceExtensions(&n_ext);
            for (auto i = 0u; i < n_ext; i++)
                vk.instance_ext.push_back(sdl_ext_data[i]);

#ifndef NDEBUG
            vk.instance_ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            vk.instance_lay.push_back("VK_LAYER_KHRONOS_validation");
#endif

            m_logger->debug("creating vulkan instance with {} extensions: {}", vk.instance_ext.size(), fmt::join(vk.instance_ext, ", "));
            m_logger->debug("and {} layers: {}", vk.instance_lay.size(), fmt::join(vk.instance_lay, ", "));

            app_info.apiVersion         = VK_API_VERSION_1_3;
            app_info.pEngineName        = "arbor";
            app_info.pApplicationName   = "arbor";
            app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
            app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 0);

            create_info.pApplicationInfo        = &app_info;
            create_info.enabledLayerCount       = vk.instance_lay.size();
            create_info.enabledExtensionCount   = vk.instance_ext.size();
            create_info.ppEnabledLayerNames     = vk.instance_lay.data();
            create_info.ppEnabledExtensionNames = vk.instance_ext.data();

            if (auto res = vkCreateInstance(&create_info, nullptr, &vk.instance); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan instance: {}", string_VkResult(res)));

#ifndef NDEBUG
            m_logger->debug("creating a debug messeneger");

            VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};

            messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messenger_create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messenger_create_info.pfnUserCallback = debug_messenger_callback;
            messenger_create_info.pUserData       = m_logger.get();

            auto create_fn =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT"));
            if (auto res = create_fn(vk.instance, &messenger_create_info, nullptr, &vk.debug_messenger); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan debug messenged: {}", string_VkResult(res)));
#endif
            return {};
        } // namespace engine
    } // namespace engine
} // namespace arbor