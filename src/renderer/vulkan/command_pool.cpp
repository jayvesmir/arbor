#include "arbor/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"

#include <algorithm>
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_command_pool_and_buffers() {
            VkCommandPoolCreateInfo pool_create_info{};
            VkCommandBufferAllocateInfo buffer_alloc_info{};

            vk.command_buffers.resize(vk.sync.frames_in_flight);

            m_logger->trace("creating a vulkan command pool & allocating command buffers");

            pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_create_info.queueFamilyIndex = vk.physical_device.queue_family_indices.graphics_family;

            if (auto res = vkCreateCommandPool(vk.device, &pool_create_info, nullptr, &vk.command_pool); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan command pool: {}", string_VkResult(res)));

            buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            buffer_alloc_info.commandPool = vk.command_pool;
            buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            buffer_alloc_info.commandBufferCount = vk.command_buffers.size();

            if (auto res = vkAllocateCommandBuffers(vk.device, &buffer_alloc_info, vk.command_buffers.data()); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate a vulkan command buffer: {}", string_VkResult(res)));

            return {};
        }

        std::expected<VkCommandBuffer, std::string> renderer::begin_temporary_command_buffer() {
            VkCommandBuffer command_buffer;

            VkCommandBufferAllocateInfo buffer_alloc_info{};
            buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            buffer_alloc_info.commandPool = vk.command_pool;
            buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            buffer_alloc_info.commandBufferCount = 1;

            if (auto res = vkAllocateCommandBuffers(vk.device, &buffer_alloc_info, &command_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate a vulkan command buffer: {}", string_VkResult(res)));

            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(command_buffer, &begin_info);

            vk.temporary_command_buffers.push_back(command_buffer);

            return command_buffer;
        }

        std::expected<void, std::string> renderer::submit_command_buffer(VkCommandBuffer command_buffer, VkQueue queue) {
            VkSubmitInfo submit_info{};

            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            if (auto res = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to submit command buffer to queue: {}", string_VkResult(res)));

            vkQueueWaitIdle(queue);

            if (auto it = std::ranges::find(vk.temporary_command_buffers, command_buffer);
                it != vk.temporary_command_buffers.end()) {
                vkFreeCommandBuffers(vk.device, vk.command_pool, 1, &command_buffer);
                vk.temporary_command_buffers.erase(it);
            }

            return {};
        }
    } // namespace engine
} // namespace arbor