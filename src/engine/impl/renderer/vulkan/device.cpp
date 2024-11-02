#include "engine/components/renderer.hpp"
#include <algorithm>
#include <ranges>
#include <tuple>

#include "fmt/ranges.h"

#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan_core.h"

namespace arbor {
    namespace engine {
        std::expected<std::tuple<VkPhysicalDevice, VkPhysicalDeviceFeatures, VkPhysicalDeviceProperties, vk::device_queue_family_indices>,
                      std::string>
        find_physical_device(const std::vector<VkPhysicalDevice>& devices) {
            for (const auto& device : devices) {
                VkPhysicalDeviceFeatures features;
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(device, &properties);
                vkGetPhysicalDeviceFeatures(device, &features);

                uint32_t n_queue_families = 0;
                std::vector<VkQueueFamilyProperties> queue_families;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &n_queue_families, nullptr);
                queue_families.resize(n_queue_families);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &n_queue_families, queue_families.data());

                if (queue_families.empty())
                    continue;

                auto graphics_qf_it = std::ranges::find_if(
                    queue_families, [](const auto& qf) { return static_cast<bool>(qf.queueFlags & VK_QUEUE_GRAPHICS_BIT); });

                if (graphics_qf_it == queue_families.end())
                    continue;

                vk::device_queue_family_indices qf_indices = {
                    .graphics_family = static_cast<uint32_t>(std::distance(graphics_qf_it, queue_families.begin())),
                };

                return {{device, features, properties, qf_indices}};
            }

            return std::unexpected("failed to find a vulkan device");
        }

        std::expected<void, std::string> renderer::make_vk_device() {
            uint32_t n_physical_devices = 0;
            std::vector<VkPhysicalDevice> physical_devices;
            vkEnumeratePhysicalDevices(vk.instance, &n_physical_devices, nullptr);
            physical_devices.resize(n_physical_devices);
            vkEnumeratePhysicalDevices(vk.instance, &n_physical_devices, physical_devices.data());

            if (physical_devices.empty())
                return std::unexpected("failed to find a vulkan device");

            if (auto physical_device = find_physical_device(physical_devices); physical_device) {
                vk.physical_device.handle               = std::get<VkPhysicalDevice>(*physical_device);
                vk.physical_device.features             = std::get<VkPhysicalDeviceFeatures>(*physical_device);
                vk.physical_device.properties           = std::get<VkPhysicalDeviceProperties>(*physical_device);
                vk.physical_device.queue_family_indices = std::get<vk::device_queue_family_indices>(*physical_device);
            } else
                return std::unexpected(physical_device.error());

            m_logger->info("using '{}' as vulkan device", vk.physical_device.properties.deviceName);

            VkDeviceCreateInfo create_info{};
            VkDeviceQueueCreateInfo queue_create_info{};

            vk.device_lay.resize(vk.instance_lay.size());
            std::ranges::copy(vk.instance_lay, vk.device_lay.begin());

            m_logger->debug("creating a vulkan device with {} extensions: {}", vk.device_ext.size(), fmt::join(vk.device_ext, ", "));
            m_logger->debug("and {} layers: {}", vk.device_lay.size(), fmt::join(vk.device_lay, ", "));

            float queue_priority               = 1.0f;
            queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueCount       = 1;
            queue_create_info.queueFamilyIndex = vk.physical_device.queue_family_indices.graphics_family;
            queue_create_info.pQueuePriorities = &queue_priority;

            create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount    = 1;
            create_info.pQueueCreateInfos       = &queue_create_info;
            create_info.pEnabledFeatures        = &vk.physical_device.features;
            create_info.enabledLayerCount       = vk.device_lay.size();
            create_info.enabledExtensionCount   = vk.device_ext.size();
            create_info.ppEnabledLayerNames     = vk.device_lay.data();
            create_info.ppEnabledExtensionNames = vk.device_ext.data();

            if (auto res = vkCreateDevice(vk.physical_device.handle, &create_info, nullptr, &vk.device); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan device: {}", string_VkResult(res)));

            vkGetDeviceQueue(vk.device, vk.physical_device.queue_family_indices.graphics_family, 0, &vk.graphics_queue);

            return {};
        }
    } // namespace engine
} // namespace arbor