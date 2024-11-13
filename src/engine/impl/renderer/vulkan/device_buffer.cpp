#include "engine/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::device_buffer::make(uint64_t size, VkBufferUsageFlags usage,
                                                                       VkMemoryPropertyFlags properties, VkDevice device,
                                                                       VkPhysicalDevice physical_device) {
            VkBufferCreateInfo create_info{};
            VkMemoryAllocateInfo allocation_info{};

            m_device          = device;
            m_physical_device = physical_device;

            create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            create_info.size        = size;
            create_info.usage       = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (auto res = vkCreateBuffer(m_device, &create_info, nullptr, &m_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create buffer: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(m_device, m_buffer, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

            uint32_t memory_type = memory_requirements.memoryTypeBits;

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_type & (1 << memory_type_idx)) && (memory_properties.memoryTypes[memory_type_idx].propertyFlags & properties))
                    break;
            }

            allocation_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize  = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(m_device, &allocation_info, nullptr, &m_memory); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate buffer memory: {}", string_VkResult(res)));

            if (auto res = vkBindBufferMemory(m_device, m_buffer, m_memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind buffer memory: {}", string_VkResult(res)));

            return {};
        }

        std::expected<void, std::string> renderer::device_buffer::write_data(const void* bytes, uint64_t size, VkQueue transfer_queue,
                                                                             VkCommandPool command_pool) {
            if (!transfer_queue || !command_pool) {
                void* mapped;
                if (auto res = vkMapMemory(m_device, m_memory, 0, size, 0, &mapped); res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to map device buffer memory: {}", string_VkResult(res)));

                std::memcpy(mapped, bytes, size);

                vkUnmapMemory(m_device, m_memory);

                return {};
            }

            renderer::device_buffer staging_buffer;
            staging_buffer.make(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device, m_physical_device);

            staging_buffer.write_data(bytes, size);

            free();
            make(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_device,
                 m_physical_device);

            VkSubmitInfo submit_info{};
            VkCommandBufferBeginInfo begin_info{};
            VkCommandBufferAllocateInfo command_buffer_allocate_info{};

            command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.commandBufferCount = 1;
            command_buffer_allocate_info.commandPool        = command_pool;
            command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VkCommandBuffer command_buffer;
            if (auto res = vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &command_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate a staging command buffer", string_VkResult(res)));

            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (auto res = vkBeginCommandBuffer(command_buffer, &begin_info); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to begin a staging command buffer", string_VkResult(res)));

            VkBufferCopy copy{};
            copy.size = size;

            vkCmdCopyBuffer(command_buffer, staging_buffer.m_buffer, m_buffer, 1, &copy);

            if (auto res = vkEndCommandBuffer(command_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to end staging command buffer: {}", string_VkResult(res)));

            submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers    = &command_buffer;

            if (auto res = vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to stage buffer to GPU: {}", string_VkResult(res)));

            vkQueueWaitIdle(transfer_queue);

            vkFreeCommandBuffers(m_device, command_pool, 1, &command_buffer);

            return {};
        }

        void renderer::device_buffer::free() {
            if (m_buffer && m_device) {
                vkDestroyBuffer(m_device, m_buffer, nullptr);
                m_buffer = VK_NULL_HANDLE;
            }

            if (m_memory && m_device) {
                vkFreeMemory(m_device, m_memory, nullptr);
                m_memory = VK_NULL_HANDLE;
            }
        }

        renderer::device_buffer::~device_buffer() {
            free();
        }

        std::expected<void, std::string> renderer::make_vertex_buffer() {
            auto size = m_test_vertices.size() * sizeof(m_test_vertices[0]);

            m_logger->trace("allocating vertex buffer of {} vertices ({} bytes)", m_test_vertices.size(), size);

            if (auto res = vk.vertex_buffer.make(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk.device,
                                                 vk.physical_device.handle);
                !res)
                return res;

            if (auto res = vk.vertex_buffer.write_data(m_test_vertices.data(), size, vk.graphics_queue, vk.command_pool); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor