#include "arbor/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_command_pool_and_buffers() {
            VkCommandPoolCreateInfo pool_create_info{};
            VkCommandBufferAllocateInfo buffer_alloc_info{};

            vk.command_buffers.resize(vk.sync.frames_in_flight);

            m_logger->trace("creating a vulkan command pool & allocating command buffers");

            pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_create_info.queueFamilyIndex = vk.physical_device.queue_family_indices.graphics_family;

            if (auto res = vkCreateCommandPool(vk.device, &pool_create_info, nullptr, &vk.command_pool); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan command pool: {}", string_VkResult(res)));

            buffer_alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            buffer_alloc_info.commandPool        = vk.command_pool;
            buffer_alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            buffer_alloc_info.commandBufferCount = vk.command_buffers.size();

            if (auto res = vkAllocateCommandBuffers(vk.device, &buffer_alloc_info, vk.command_buffers.data()); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate a vulkan command buffer: {}", string_VkResult(res)));

            return {};
        }
    } // namespace engine
} // namespace arbor